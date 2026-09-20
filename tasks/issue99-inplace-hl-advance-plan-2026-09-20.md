# Issue 99 / 249: In-place HL advance and 16-bit constant store lowering

**Date:** 2026-09-20  
**Context:** Triage of remaining XFAIL tests tracked by #338 (Item 1: `issue-97a-bc-pingpong-i16-counter.ll`, tracked by #99).

---

## 1. Problem Statement

In single-basic-block loops with an i16 pointer walk `*p++` and an i16 counter (such as `countdown_i16_counter` in `issue-97a` and `fill_seq` in `word_fill` / #99), LLVM produces heavily degraded code with redundant register shuttles (`ld c,l; ld b,h` / `ld l,c; ld h,b` or `push iy; pop hl`) and BSS static-frame spills (`ld (frame), de; ld de, (frame)`).

### Observed vs Ideal Codegen for `countdown_i16_counter`:

**Observed (unmodified HEAD):**
```z80
_countdown_i16_counter:
; %bb.0:
    ld      c,l
    ld      b,h
    ld      de,#256
    ld      (L_countdown_i16_counter.frame),de ; BSS Spill
.LBB0_1:
    ld      de,#0                   ; Clobbers DE!
    ld      l,c
    ld      h,b                     ; Shuttle BC -> HL
    ld      (hl),e
    inc     hl
    ld      (hl),d
    inc     bc
    inc     bc                      ; Pointer advance in BC
    ld      de,(L_countdown_i16_counter.frame) ; BSS Reload
    dec     de
    ld      a,e
    ld      (L_countdown_i16_counter.frame),de ; BSS Spill
    or      d
    jr      nz,.LBB0_1
    ret
```

**Ideal (zsdcc parity / optimal Z80):**
```z80
_countdown_i16_counter:
; %bb.0:
    ld      bc,#256
.LBB0_1:
    ld      (hl),#0
    inc     hl
    ld      (hl),#0
    inc     hl                      ; In-place advance to p+2!
    dec     bc                      ; Counter in BC!
    ld      a,b
    or      c
    jr      nz,.LBB0_1
    ret
```

---

## 2. Root Cause Analysis

Two interacting causes create this pathology:

### Cause 1: 16-bit store lowering in `Z80InstructionSelector.cpp`
`Z80InstructionSelector.cpp:2741` hardcodes a physical copy of the store source to `DE`:
```cpp
BuildMI(MBB, MI, DL, TII.get(TargetOpcode::COPY), Z80::DE).addReg(SrcReg);
BuildMI(MBB, MI, DL, TII.get(TargetOpcode::COPY), Z80::HL).addReg(AddrReg);
auto &StoreLo = *Z80::buildStoreHL(MBB, MI, DL, TII, Z80::E);
Z80::buildIncDec16(MBB, MI, DL, TII, Z80::INC_rr, Z80::HL);
auto &StoreHi = *Z80::buildStoreHL(MBB, MI, DL, TII, Z80::D);
```
Even when `SrcReg` is a compile-time constant (like `store volatile i16 0`), it materializes the constant into `$de = LD_rr_nn 0` inside the loop. Because it targets a physical register (`$de`), MachineLICM cannot hoist it. The clobber of `DE` on every iteration prevents register allocation from using `DE` for the counter or the pointer, forcing spills to BSS.

Furthermore, Z80 has `LD (HL), n` (`LD_HLind_n`, 2 B, 10 T). Emitting two `LD_HLind_n` separated by `INC_rr $hl` takes 5 B and 26 T, completely avoiding physical register `DE`.

### Cause 2: Decoupled pointer advance (`*p` and `p += 2`)
During ISel:
1. `store i16 ..., ptr %p` copies `%p` to `$hl`, writes byte 0, increments `$hl` once (leaving `$hl = %p + 1`), and writes byte 1.
2. `%p.next = getelementptr %p, 2` is lowered independently as `%p.next = INC_rr (INC_rr %p)`.

Because `$hl` is left at `%p + 1`, greedy register allocator cannot allocate `%p` to `HL`. Greedy therefore allocates `%p` to `BC` (or `IY`), creating the `ld c,l; ld b,h` shuttle and competing with the loop counter for `BC`.

If the 2-byte store through `HL` is immediately followed by one more `inc hl`, `$hl` reaches `%p + 2` in place. `%p` can remain pinned to `HL` throughout the entire loop, eliminating the advance instructions `%p = INC_rr (INC_rr %p)` and leaving `BC` free for the counter.

---

## 3. Implementation Plan

### Step 1: Constant 16-bit Store Lowering
In `Z80InstructionSelector.cpp`:
Check if `SrcReg` is defined by `G_CONSTANT`:
```cpp
MachineInstr *SrcDef = MRI.getVRegDef(SrcReg);
if (SrcDef && SrcDef->getOpcode() == TargetOpcode::G_CONSTANT) {
  int64_t Val = SrcDef->getOperand(1).getCImm()->getZExtValue();
  if (!RBI.constrainGenericRegister(AddrReg, Z80::GR16RegClass, MRI))
    return false;
  BuildMI(MBB, MI, DL, TII.get(TargetOpcode::COPY), Z80::HL).addReg(AddrReg);
  BuildMI(MBB, MI, DL, TII.get(Z80::LD_HLind_n)).addImm(Val & 0xFF);
  Z80::buildIncDec16(MBB, MI, DL, TII, Z80::INC_rr, Z80::HL);
  BuildMI(MBB, MI, DL, TII.get(Z80::LD_HLind_n)).addImm((Val >> 8) & 0xFF);
  MI.eraseFromParent();
  return true;
}
```
*Effect:* Immediately eliminates BSS spill in `countdown_i16_counter` and reduces code size.

### Step 2: In-place HL Advance in `Z80KeepLoopPointerInPair` (or Pre-RA Pinning Pass)
In `Z80KeepLoopPointerInPair.cpp` / new transform:
Detect self-loop where:
1. `%ptr` is copied to `$hl` at the start of a 2-byte store through `(hl)`.
2. The store leaves `$hl` at `%ptr + 1`.
3. `%ptr` is advanced by +2 (`INC_rr` twice) for the back-edge, with no other uses in the block.
4. Replace the two `INC_rr %ptr` with a second `INC_rr $hl`, and pin `%ptr` to `HLReg`.

### Step 3: Verification
1. Run lit test `issue-97a-bc-pingpong-i16-counter.ll` (verify PASS).
2. Run full lit suite `ninja check-llvm-codegen-z80`.
3. Run runtime test runner `cargo run --manifest-path z80-utils/test-runner/Cargo.toml -- clang`.
4. Measure benchmark impact on `word_fill`.
