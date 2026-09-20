# Issue #216 Deep Dive: `cp-sbc-and` Idiom (`(x < imm) ? value : 0`)

**Date**: 2026-09-20  
**Issue**: [ravn/llvm-z80#216](https://github.com/ravn/llvm-z80/issues/216)  
**Test**: `llvm/test/CodeGen/Z80/issue-216-cp-sbc-and.ll` (`XFAIL: *`)  
**Status**: Backlog / Architecture Documented (Canonical XFAIL test maintained)

---

## 1. Problem Definition & Core Idiom

In C code targeting small embedded systems or 8-bit CPUs, conditional masking against zero is a common idiom:
```c
uint8_t branch_bot_or_zero(uint8_t line, uint8_t bot) {
    return (line < 11) ? bot : 0;
}
```
Or symmetrically:
```c
(x >= imm) ? 0 : value
```

### The Z80 `sbc a, a` Bit-Broadcast Trick

On the Z80 CPU, subtracting register `A` from itself with borrow (`sbc a, a`) computes:
$$A \leftarrow A - A - \text{CF} = -\text{CF}$$

- If **$\text{CF} = 1$**: $0 - 1 = 255 \ (\texttt{0xFF})$. All bits are set to 1.
- If **$\text{CF} = 0$**: $0 - 0 = 0 \ (\texttt{0x00})$. All bits are set to 0.

Because `cp imm` directly compares `A` with an unsigned immediate `imm` via subtraction, it sets $\text{CF} = 1$ if and only if $A < \text{imm}$ (unsigned less-than).

Combining `cp`, `sbc a, a`, and `and`:
```z80
    cp   11         ; 2 Bytes, 7 T-states   (CF = 1 if line < 11, CF = 0 if line >= 11)
    sbc  a, a       ; 1 Byte,  4 T-states   (A = 0xFF if line < 11, A = 0x00 if line >= 11)
    and  l          ; 1 Byte,  4 T-states   (A = bot & 0xFF == bot, or bot & 0x00 == 0)
    ret             ; 1 Byte, 10 T-states
```
**Total footprint**: **5 bytes**, **25 T-states** (including `ret`), **zero branch instructions**, **zero branch misprediction / condition evaluation branches**, **zero pipeline stalls**, **no scratch registers consumed**.

---

## 2. Current Suboptimal Codegen

Currently, Clang and LLVM Z80 emit a full control-flow triangle for this expression:

```z80
_branch_bot_or_zero:
    ld   a, e               ; 1 B, 4 ts
    cp   11                 ; 2 B, 7 ts
    jr   c, .LBB0_2         ; 2 B, 7/12 ts (jump if line < 11)
    ld   l, 0               ; 2 B, 7 ts    (zero out result)
.LBB0_2:
    ld   a, l               ; 1 B, 4 ts
    ret                     ; 1 B, 10 ts
```
**Footprint**: **8–9 bytes**, **32–37 T-states**, takes 2 basic blocks and a conditional jump.

---

## 3. Why Prior Implementation Attempts Failed

On 2026-06-21, an attempt was made to capture this pattern in `Z80LowerSelect.cpp` (see Git history on branch `peephole-cp-sbc-and-216` and commit `916373562fd9`).

### The Root Cause of Failure: `IRTranslator` CFG Triangle Expansion

In LLVM GlobalISel:
1. `IRTranslator` translates LLVM IR to Machine IR (MIR).
2. For scalar `select i1 %cond, i8 %val, i8 0`:
   `IRTranslator::translateSelect` checks if the target supports `G_SELECT` or prefers control-flow expansion.
   By default for targets without full vector/predicated hardware selects, `IRTranslator` expands `select` into a control-flow triangle before legalizer or instruction selection ever run:
   ```mir
   bb.1:
     %0:gr8 = COPY $a
     %1:gr8 = COPY $l
     %2:gr8 = G_CONSTANT i8 11
     %3:s1 = G_ICMP ult, %0, %2
     G_BRCOND %3, %bb.3
     G_BR %bb.2

   bb.2.select.false:
     ; empty fallthrough to bb.3

   bb.3.select.end:
     %4:gr8 = G_PHI %1, %bb.1, %5(G_CONSTANT 0), %bb.2
   ```
3. Because the IR was already lowered to a control-flow triangle (`G_BRCOND` + `G_PHI`), **no `G_SELECT` instruction existed in the MIR**.
4. When `Z80LowerSelect` ran, it searched for `G_SELECT` opcodes, but found none. Instrumentation with `llvm::errs()` confirmed that `Z80LowerSelect` never encountered the instruction.

---

## 4. Viable Architecture & Fix Options

To implement `#216`, two primary architectural paths exist:

### Path A: Pre-Legalize Combiner (GISel Layer)

- **Location**: `llvm/lib/Target/Z80/GISel/Z80PreLegalizerCombiner.cpp`
- **Mechanism**:
  Configure `IRTranslator` or catch pre-expansion `select` operations before they are lowered to CFG triangles.
  Alternatively, teach the Combiner to recognize the pattern:
  `G_AND %val, (G_SEXT (G_ICMP ult, %x, %imm))`
  When `G_ICMP ult` is used with a constant operand and extended via sign-extension (`s1 -> s8`), lower `G_SEXT` directly into `sbc a, a`.
- **Pros**:
  - Operates on generic Machine IR with SSA semantics intact.
  - Generates optimal code early, preventing register allocation bloat.
- **Cons**:
  - Requires tuning GlobalISel combiner rules.
  - Risk of phase-ordering issues if `IRTranslator` still insists on splitting basic blocks.

### Path B: Post-RA Control Flow Triangle Collapse (`Z80LateOptimization`)

- **Location**: `llvm/lib/Target/Z80/Z80LateOptimization.cpp`
- **Mechanism**:
  Pattern-match the 3-block triangle after register allocation and instruction selection:
  1. `MBB1`: ends with `CP_a_n imm; JR_c MBB3` (or `JR_nc`).
  2. `MBB2`: contains only a store/copy of `0` into the destination register and falls through to `MBB3`.
  3. `MBB3`: consumes the value.
  When detected:
  - Collapse `MBB1` and `MBB2` into `CP imm; SBC_A_A; AND_A_r val`.
  - Remove the branch and eliminate the dead basic block `MBB2`.
- **Pros**:
  - Immune to how frontend, middle-end, or `IRTranslator` chose to lower the `select`.
  - Simple local pattern-match on machine instructions.
- **Cons**:
  - Modifying CFG post-RA requires updating successors/predecessors and liveness.
  - Must ensure flags (carry, zero) are dead after the sequence or match consumer expectations.

---

## 5. Production Audit & Cost/Benefit Analysis

In June 2026, a sweep across all production targets was conducted:
- `autoload` (boot loader): **1 candidate call site** (in `sextants` tail calculation: `ld b, 0; cp $b; ...`). Potential win: **2–3 bytes**.
- `cpnos PROM1`: **0 sites**.
- `rcbios BIOS`: **0 sites**.
- `AES-256 corpus`: **0 sites**.
- `compiler-comparison-corpus`: **0 sites**.

**Trade-off**:
- Implementation effort: ~2–3 hours for Path A, ~1–2 hours for Path B.
- Production payout: ~2–3 bytes across the entire operating system codebase.
- Decision: Per the project rule (*focus on high-leverage production bottlenecks first*), the issue remains parked with the canonical XFAIL lit test (`issue-216-cp-sbc-and.ll`) safeguarding the design.

---

## 6. Checklist for Future Implementation

When picking up `#216`:
1. [ ] Choose Path B (Post-RA triangle collapse in `Z80LateOptimization.cpp`) as the least intrusive approach.
2. [ ] Identify `CP_a_n` followed by conditional jump to skip a 1-instruction block loading `0`.
3. [ ] Verify destination register is `A` or can be coalesced with `A`.
4. [ ] Replace CFG triangle with `SBC_A_A` + `AND_A_r`.
5. [ ] Update CFG edges (`removeSuccessor`).
6. [ ] Un-XFAIL `llvm/test/CodeGen/Z80/issue-216-cp-sbc-and.ll`.
7. [ ] Run lit suite and `z80-test-runner clang` to verify zero regressions.
