# Cluster 4 fix sketch — known-value tracking

Unblocks issues **#60, #18, #79, #83**, plus closes the ground for
follow-up cleanups (e.g. the `ld a,$0` that's currently caught only
by my pattern scan).

## Where it lives

New code inside `Z80LateOptimization.cpp` (the 3920-line late-pass
already running at the right pipeline point: post-RA, post-RA scavenging,
pre-Z80PostRACompareMerge).  Add as a self-contained sub-pass at the
top of `runOnMachineFunction`, before the existing IX-propagation.

The pass already iterates per-MBB; we extend that iteration with a
forward dataflow over per-physical-register known constants.

## Data structure

```cpp
struct KnownReg {
  enum { Unknown, KnownConst, KnownCopyOf } K = Unknown;
  int64_t Imm = 0;       // valid when K == KnownConst (i8, i16, or i32 for IX/IY)
  Register CopySrc;      // valid when K == KnownCopyOf
  unsigned Bits = 8;     // i8 vs i16 (DE/BC/HL/IX/IY)
};

using KnownState = DenseMap<Register, KnownReg>;
```

State is computed per-MBB.  At MBB entry, intersect predecessor
out-states (a value is known iff every predecessor agrees).  At MBB
exit, the state is recorded for successor processing.  Iteration to
fixpoint over the CFG (linear in #MBBs * #regs).

## Update rules per instruction

For each `MachineInstr` walked forward:

| Instruction | State update |
|---|---|
| `LD r, imm`           | `r.K = KnownConst; r.Imm = imm` |
| `LD r, r2`            | If `r2.K == KnownConst`, propagate; else `KnownCopyOf(r2)` |
| `XOR A`               | `A.K = KnownConst; A.Imm = 0` |
| `LD A, (nn)`          | `A.K = Unknown` |
| `LD r, (nn)`          | likewise |
| `IN A, (n)` / `IN A, (C)` | `A.K = Unknown` |
| `OUT (n), A` / `OUT (C), r` | no register state change |
| `INC HL` / `DEC HL`   | `HL.K = Unknown` (refining is possible if known) |
| `INC (HL)` / `DEC (HL)` | A unchanged |
| arithmetic: `ADD`, `SUB`, `AND`, `OR`, `XOR (with A)` | invalidate A unless both operands are known |
| `EX DE, HL`           | swap state |
| `PUSH r` / `POP r`    | depends — `POP` invalidates dest |
| `LD (nn), r`          | no register state change |
| `CALL`                | conservatively invalidate all caller-saved regs (= almost everything on Z80) |
| `RET`                 | end of state |

## Rewrite rules

For each `MachineInstr`:

### #60 (LD r, imm where r already holds imm)

```
LD r, imm    when r.K == KnownConst && r.Imm == imm   →   delete
LD r, r2     when r.K == KnownConst && r2.K == KnownConst && r.Imm == r2.Imm   →   delete
```

For the immediate case, also catch `LD A, 0` when `A.K == KnownConst &&
A.Imm == 0` (which `XOR A` established).

### #18 (LD r, r2 where r already equals r2)

Subsumed by the above.  `KnownCopyOf` chains let you see through
`LD c, e; LD e, c` cycles and other shuffles.

### #83 (dead AND/OR after constant load)

```
AND imm      when A.K == KnownConst && (A.Imm & imm) == A.Imm   →   delete
OR  imm      when A.K == KnownConst && (A.Imm | imm) == A.Imm   →   delete
XOR imm      when imm == 0                                       →   delete
```

The flag side-effects of these are usually consumed by a later JR/JP;
when followed only by a dead-flag-consumer (or when the deleted
instruction already had matching flags from the source), the rewrite
is sound.  Conservative version: only delete when no flag use within
the next N instructions.

### #79 (`(x != y) ? 0xFF : 0` mask chain)

This isn't a known-value rewrite per se -- it's a peephole on the
8-instruction chain.  Detect:

```
SUB a, r       (or CP r)
ADD a, $FF
SBC a, a
AND  a, $1
RRCA
AND  a, $80
ADD  a, a
SBC  a, a
```

and replace with:

```
SUB a, r
ADD a, $FF
SBC a, a
```

Since the input is already in canonical mask-from-flag form by
instruction 3, the trailing 5 instructions are dead.  The fix is to
*not generate them* at GISel time: legalize `sext (icmp ne)` to that
3-instruction sequence directly via a tablegen pattern, instead of
going through the generic `i1 → i8` widening that produces the chain.

Look for `select_cc` / `sext_inreg` of an `icmp` result in
`Z80LegalizerInfo.cpp` -- that's the place.

## Per-issue test mapping

| Issue | Test file (added in this commit) | Status |
|---|---|---|
| #60 | `llvm/test/CodeGen/Z80/known-zero-a.ll` | XFAIL initially |
| #79 | `llvm/test/CodeGen/Z80/mask-from-flag.ll` | XFAIL initially |
| #83 | `llvm/test/CodeGen/Z80/bool-store-no-mask.ll` | XFAIL initially |
| #18 | covered by #60 cases | (no separate test) |

When the fix lands, each test's `; XFAIL: *` line is removed and the
test starts passing.  Anyone who *re-introduces* the pessimization
later breaks CI on a named failure.

## Reproducer outputs (current emission, for the PR body)

### #79 — observed 12 B, expected 4 B

```c
uint8_t mask_neq(uint8_t x, uint8_t y) { return (x != y) ? 0xFF : 0; }
```

Current (`clang --target=z80 -Oz`):
```
0: 95           sub  a, l        ; A = x - y
1: c6 ff        add  a, $ff
3: 9f           sbc  a, a        ; A is already 0xFF or 0
4: e6 01        and  a, $1       ; <-- 0xFF & 1 = 1, OR 0 & 1 = 0
6: 0f           rrca             ; <-- so 0x01 → 0x80, 0x00 → 0x00
7: e6 80        and  a, $80      ; <-- 0x80 & 0x80 = 0x80
9: 87           add  a, a        ; <-- 0x80 + 0x80 = 0x100, carry=1
a: 9f           sbc  a, a        ; <-- A = -carry = 0xFF or 0
b: c9           ret              ; total: 12 B
```

Expected:
```
0: 95           sub  a, l        ; A = x - y
1: c6 ff        add  a, $ff      ; carry := (x != y)
3: 9f           sbc  a, a        ; A = -carry
4: c9           ret              ; total: 4 B  (-8 B per call site)
```

The current sequence is the optimizer effectively *reproducing* the
mask via the mask-from-flag idiom -- twice.  The first 3 instructions
already produce the correct mask; the last 5 are a no-op that happens
to round-trip through `(A & 1) → (A & 0x80) → (carry := A.bit7) → -carry`.

### #83 — observed 5 B, expected 3 B

```c
static volatile uint8_t pio_b_dir;
void set_dir_output(void) {
    if (pio_b_dir == 1) return;
    /* ... */
    pio_b_dir = 1;
}
```

Current (relevant bytes):
```
ld   a, $1
and  a, $1     ; <-- DEAD: A is provably 1 because we just loaded 1
ld   ($f618), a
```

Expected:
```
ld   a, $1
ld   ($f618), a
```

clang's `_Bool`-narrowing emits the AND to enforce the i1 invariant,
not knowing that the immediate already satisfies it.

### #60 — caught only in hand-asm currently; #60 covers compiler-output too

The pattern scan on the cpnos-rom build found ONE compiler-emitted
known-zero candidate (in `_isr_crt`) and that was hand-written asm.
But `rcbios` builds and other Z80 codebases would benefit; the test
locks the invariant for future regressions.

## Implementation effort

- Data structures + per-instruction state-update table: ~150 lines
- Forward dataflow over CFG: ~80 lines (reuses LLVM's worklist
  patterns from existing passes)
- Rewrite rules: ~80 lines
- Lit tests already drafted (3 files in this directory, will be
  copied to `llvm/test/CodeGen/Z80/` when fix lands)

Estimate: ~310 lines added to `Z80LateOptimization.cpp`, ~3 lit
tests, no headers touched.

## Risks / known unknowns

1. **Flag side-effects.**  `AND imm` and `OR imm` set N=0/H=1/C=0/Z/S
   flags.  Deleting the instruction loses those flags.  If a downstream
   `JR Z` / `JR NZ` consumes them without a re-establishment, the
   rewrite is unsafe.  Conservative version: only delete when the
   source `LD A, n` had the same flag-establishing semantics (it
   doesn't -- LD doesn't touch flags).  Need an explicit check.

   Mitigation: scan the next N=4 instructions; if any consumes Z/N/H
   flags before another flag-establishing instr, abort.

2. **CALL invalidation may be too coarse.**  Z80 has no formal callee-
   saved register set in the standard ABI; our `sdcccall(0/1)` defines
   one.  The pass should consult the calling convention rather than
   assuming all-volatile.

3. **Cross-MBB tracking accuracy.**  Joining states at merges via
   intersection means losing precision after every branch.  The fix
   helps mostly within a single straight-line region.  This is ok;
   the alternative (partial dataflow with phi-like join nodes) is
   significantly more complex and the marginal payoff (extra 1-2 B
   per function) doesn't justify it for a -Oz pass.

## Upstream packaging

PR title: `[Z80] Track known register values across late peephole`

PR body:
- Reproducers (the three `.c` files in this directory).
- Current vs expected disassembly per case.
- Lit tests added (the three `.ll` files in `llvm/test/CodeGen/Z80/`).
- Implementation diff in `Z80LateOptimization.cpp` (~310 lines).
- Measured size delta on cpnos-rom + rcbios (run baseline + fix
  builds; record in PR description).

## Existing infrastructure to extend

`Z80LateOptimization.cpp:1647` already has a partial #60 peephole:

```cpp
// --- Peephole: redundant LD A,reg removal (issue #60) ---
// When LD reg,A is followed by LD A,reg with no A-modifying or
// reg-modifying instructions between, the second LD is redundant.
```

This catches one specific pattern: `LD r,A; ...; LD A,r` round-trip.
It does NOT track:
- `LD A, imm` after another `LD A, imm` with same value (incl `XOR A`)
- `AND imm` after `LD A, imm` where mask is dead
- Multi-MBB known-value carry

**Recommendation: extend in place rather than write a new pass.**
The existing peephole already establishes the right scanning idiom
(forward, up to 8 instrs, bail on CALL/clobber/regmask).  Extending
its state-tracking from "single-register-value-from-A-copy" to
"any-register-known-constant" is a 50-100 line diff localised in
the same file.

Smaller PR, lower review burden, easier merge.  And it keeps the
known-value logic next to the `#60` comment marker so a reviewer
doesn't have to context-switch between files.
