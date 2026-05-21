# Implementation plan: #174 gf_log/gf_alog inner loop peepholes

Date: 2026-05-21 (session 73p continued).  Plans the work to close
~70 % (~2.0 M ts) of the clang-vs-SDCC AES speed gap.

## What the analysis found

Reading post-Z80LateOpt MIR of `gf_alog` (representative; `gf_log`
identical), the inner loop body emits two redundant-reload
patterns that account for most of the per-iter cost gap vs SDCC.

### Pattern P1 — counter test through reload

Current (final asm):
```asm
LD_A_C        ; 4 ts -- load counter from C
DEC_A         ; 4 ts -- A := counter - 1; flags from result
LD_E_A        ; 4 ts -- save dec'd value to E
LD_A_C        ; 4 ts -- RELOAD counter from C  ← redundant
OR_A          ; 4 ts -- test if pre-dec counter was 0
JR_Z exit     ; 7/12 ts
```
**Total: 6 instructions, ~27 ts (fall-through path).**

The reload is needed because DEC_A modified A but the test wants
the *original* counter.  C is preserved across DEC_A so the reload
is correct, just expensive.

### Pattern P2 — bit-7 test through reload after shift

Current (final asm):
```asm
LD_A_D        ; 4 ts -- load accumulator from D
ADD_A_A       ; 4 ts -- A := A << 1; CARRY := bit 7 of D  ← carry available here!
LD_H_A        ; 4 ts -- save shifted value to H
LD_A_D        ; 4 ts -- RELOAD accumulator from D  ← redundant
RLCA          ; 4 ts -- A := rotated; CARRY := bit 7 of D (same as ADD_A_A's carry)
JR_C body     ; 7/12 ts
```
**Total: 6 instructions, ~27 ts (fall-through path).**

The carry from ADD_A_A already IS bit 7 of D.  Nothing between
ADD_A_A and the JR modified flags (LD H, A is a register move).
The `LD A, D; RLCA` re-derives the same carry.

## Proposed peepholes

Both fit the existing `Z80LateOptimization.cpp` peephole model
(MBB-local pattern match + rewrite).  Neither requires regalloc
changes, MachineLoopInfo, or LiveIntervals.

### Peephole #174-A: SUB-1-and-test idiom

Replace P1 with:
```asm
LD_A_C        ; 4 ts
SUB_n 1       ; 7 ts -- A := A - 1; CARRY := (A pre was 0)
LD_E_A        ; 4 ts
JR_C exit     ; 7/12 ts
```
**Total: 4 instructions, ~22 ts.  Saving: -2 inst / -5 ts per iter.**

Size cost: SUB n is 2 B vs DEC A 1 B, but we delete 2 instructions
(LD_A_C 1 B + OR_A 1 B), so net **−1 B per match**.  Both size and
speed move correctly.

Pattern match:
```mir
LD_A_<R>      ; LD A from physreg R (R in {B,C,D,E,H,L})
DEC_A         ; flag-setting alu on A
LD_<R'>_A     ; save dec'd to R' (R' ≠ A; flag-neutral)
LD_A_<R>      ; reload SAME R into A  ← redundant
OR_A          ; test A == 0
JR_Z target   ; or JP_Z
```

Rewrite to:
```mir
LD_A_<R>
SUB_n 1
LD_<R'>_A
JR_C target
```

**Safety conditions:**
1. The two `LD_A_<R>` instructions must reference the same source
   register (or compatible source — same memory address would also
   work, but pure-register form is the only case we've observed).
2. No instruction between the first `LD_A_<R>` and the `LD_<R'>_A`
   modifies R (regalloc invariant; verify by scanning operands).
3. No instruction between `LD_<R'>_A` and the final `JR` modifies
   FLAGS, EXCEPT the second `LD_A_<R>` (a flag-neutral LD) and the
   `OR_A` (which sets flags but is what we're replacing).
4. The result of `DEC_A` (stored in R') must be used later — we
   keep the `LD_<R'>_A` only if R' has live uses post-rewrite.  In
   the gf_alog case R' is the new counter for the loop body, so
   live.

### Peephole #174-B: forward ADD_A_A's carry

Replace P2 with:
```asm
LD_A_D        ; 4 ts
ADD_A_A       ; 4 ts -- CARRY := bit 7 of D
LD_H_A        ; 4 ts
JR_C body     ; 7/12 ts -- use ADD_A_A's carry directly
```
**Total: 4 instructions, ~19 ts.  Saving: -2 inst / -8 ts per iter.**

Size cost: **−2 B per match** (delete `LD A, D` + `RLCA`).

Pattern match:
```mir
LD_A_<R>      ; LD A from R
ADD_A_A       ; A := A << 1; CARRY := bit 7 of R
LD_<R'>_A     ; save shifted to R' (flag-neutral)
LD_A_<R>      ; reload SAME R  ← redundant
RLCA          ; test bit 7 (sets CARRY := bit 7 of (current A) = bit 7 of R)
JR_C/JR_NC target
```

Rewrite to:
```mir
LD_A_<R>
ADD_A_A
LD_<R'>_A
JR_C/JR_NC target
```

**Safety conditions:**
1. Two `LD_A_<R>` reference the same source register.
2. R unchanged between the two loads.
3. Between ADD_A_A and the final JR, no flag-modifying instruction
   other than the deleted `RLCA`.  `LD r, A` and `LD A, r` are
   flag-neutral.
4. Branch condition is C or NC (we're forwarding a single bit; both
   branch directions are equivalent post-rewrite because RLCA was
   identity on CARRY in this construction).

## Per-iter cycle accounting (revised)

| | clang current | P1+P2 fixed | Δ |
|---|---:|---:|---:|
| Counter test sub-block | 27 ts | 22 ts | -5 ts |
| Bit-7 test sub-block | 27 ts | 19 ts | -8 ts |
| Rest of iteration | ~46 ts | ~46 ts | 0 |
| **Per-iter total** | ~100 ts | ~87 ts | **-13 ts** |

**SDCC reference: ~63 ts/iter**.  After P1+P2 closes the redundant
reloads, clang at ~87 ts/iter is **still ~38 % slower than SDCC**.
The residual is the conditional-XOR sub-block (P3, smaller) plus
SDCC's `bit 7, b` test being free-of-carry-side-effects vs clang's
ADD_A_A which sets carry redundantly.

**Estimated workload savings:** 13 ts/iter × ~120 K iters = **~1.5 M ts**
on the AES corpus `09_Oz_prod_like` workload.  Closes ~55 % of the
2.81 M ts gap.

Yield estimate from #174 itself (which framed the savings range at
1.0-2.4 M ts):  this lands close to the **midpoint** of that range.

### Size impact estimate

Per-match: P1 saves 1 B; P2 saves 2 B.  Each gf_alog and gf_log
has exactly one of each → 6 B saved per function × 2 functions =
**−12 B** on AES `09_Oz_prod_like` from the inner loops alone.

Cross-function applicability: this pattern likely appears in any
function doing "test value, then modify it" or "shift value, then
test bit 7".  AES corpus likely sees additional matches outside
gf_log / gf_alog.  Conservative estimate: −20 to −50 B total AES
shrink alongside the cycle win.

cpnos PROM1 (currently 2030 / 2048, 18 B free) likely benefits by
~5-10 B if the pattern fires in any cpnos hot function.

## Implementation steps

### Step 1 — minimal lit test for P1+P2

Add `llvm/test/CodeGen/Z80/issue-174-redundant-reload.ll` covering:
- P1: counter-test idiom with various source registers (B, C, D,
  E, H, L) and both JR_Z + JP_Z branches.
- P2: bit-7-test idiom with various source registers and both
  JR_C + JR_NC.
- Negative cases: pattern should NOT fire when (a) the second LD
  is from a different source, (b) R is modified between the two
  LDs, (c) a flag-modifying instruction interleaves.

### Step 2 — implement P1 (SUB-1-and-test)

Add a new peephole block in `Z80LateOptimization.cpp` modelled on
the existing `#60` redundant-reload-elimination peephole (line 1751).
Helpers `getLDArOpc`, `getLDrAdst60`, and the forward-scan with
ClobbersA / ClobbersReg guards are reusable.

New helpers needed:
- `getDECrFromLDAr` (or inline): identify the DEC_A in the
  expected position.
- `getOrAFromMI`: identify OR_A.

Estimated change: ~80 lines.

### Step 3 — implement P2 (carry forwarding)

Add a second peephole block.  Helpers above are reusable; add:
- `isFlagNeutral`: identify LD r,A / LD A,r instructions that don't
  modify FLAGS.
- `findRedundantRLCAAfterADDAA`: the matcher.

Estimated change: ~80 lines.

### Step 4 — value oracle pass

After each peephole lands:

1. **Z80 lit suite**: 104 + 3 XFAIL must hold.
2. **AES corpus 13-config sweep**: no regression on any config;
   ideally tstate improvement on `09_Oz_prod_like` and size
   shrink on most configs.
3. **z80-utils test-runner clang suite**: 681 PASS baseline.
4. **AES 4-cell verifier**: PASS on K&R + ANSI × clang + zsdcc.

### Step 5 — production target builds

Confirm autoload PROM, cpnos PROM1, and BIOS:
- Build, compare bytes vs pre-peephole.
- If cpnos shrinks, verify polypascal-test still PASS.
- If anything regresses, isolate the cause via per-function
  size diff.

### Step 6 — commit + filing

If all green:
- One commit per peephole (P1, P2).
- Update #174 with measured numbers.
- Add a session summary in `tasks/`.

If a regression appears: revert the offending peephole, document
the regression cause in #174 + the in-place comment.

## Risk analysis

### Risk 1: pattern fires incorrectly on look-alike sequences

The matcher walks forward looking for the exact opcode pattern.  If
some other intermediate instruction emits the same shape but with
different semantics (e.g., the source R was reloaded from memory
not from a kept register), the rewrite would corrupt state.

**Mitigation**: require that the two `LD_A_<R>` instructions
reference the SAME physical register (already in matcher).
Require that no instruction between modifies R (already in
matcher's ClobbersReg check).  Add a lit-test negative case for
each of these conditions.

### Risk 2: P1 changes flags semantics for subsequent code

P1 deletes OR_A.  OR_A sets Z, S, P/V; clears N, C, H.  The
replacement (SUB_n 1 + JR_C) sets Z, S, P/V, N from the dec;
CARRY := pre-was-0; H from bit-4 borrow.

If any instruction AFTER the JR_C consumes a flag that OR_A set but
SUB_n 1 sets differently, the rewrite breaks.

**Mitigation**: at the rewrite point, the only consumer of OR_A's
flags is the immediate JR_Z (which becomes JR_C).  The matcher
already requires no flag-consumer between OR_A and JR.  Verify by
adding a guard: the rewrite is gated on the JR being the only flag
consumer until the next flag-setter.

### Risk 3: SUB-1 isn't the right choice on SM83

Z80 has SUB_n; SM83 also has SUB_n with identical semantics.  No
divergence.  Both `add_a_a` and `dec_a` exist on both targets too.

### Risk 4: Existing peepholes interact

The peephole order in `Z80LateOptimization` matters.  P1+P2 should
run AFTER the existing redundant-LD-A peephole (line 1751) so that
that peephole has already eliminated trivial duplicates.

**Mitigation**: insert P1+P2 immediately after the existing
peephole at line 1830.

### Risk 5: Failure mode resembles session 73p ADD16_tied dead-end

The ADD16_tied attempt catastrophically miscompiled (173/990 test-
runner FAIL) from a regalloc-level mistake.  This work is at a
DIFFERENT layer (post-RA peephole, after regalloc completes), so
the same failure mode is unlikely.

**Mitigation**: verify by running the FULL test-runner clang suite
before committing.  If anything FATALs, revert.

## Comparison with #173 (8-bit BSS spill peephole)

Both are post-RA peepholes in `Z80LateOptimization.cpp`.  Both
estimated to deliver substantial AES gains.  Different cost
centers:

| | #174 (P1+P2) | #173 |
|---|---|---|
| Cost center | gf_log/gf_alog inner loop | aes_mc_inv / aes_mixColumns XOR chains |
| Estimated workload saving | ~1.5 M ts | ~0.4 M ts |
| Per-pattern complexity | Low (linear scan, exact match) | Medium (liveness check on partner half) |
| Production target benefit | Moderate (cpnos has loops) | Strong (cpnos has spill-heavy hot fns) |
| Risk | Low | Low-medium (liveness analysis is the risk) |

**Recommendation**: land #174 P1 first (highest-yield, lowest-risk).
P2 second.  #173 third.

## Next-session execution sketch

The actual implementation is a 1-session task.  Sequence:

1. (15 min) Add the negative-test lit file (`issue-174-*.ll`)
   covering all the safety conditions.
2. (45 min) Implement P1 + verify lit + AES corpus.
3. (45 min) Implement P2 + verify lit + AES corpus.
4. (30 min) Run full value oracle (test-runner clang, MAME
   cpnos polypascal smoke).
5. (15 min) Commit + update #174 + post results.

Total: ~2.5 h estimated.  Result: ~1.5 M ts AES improvement
expected if both peepholes fire and the analysis is correct.

## What this plan deliberately does NOT cover

- **P3 (conditional-XOR sub-block reload)**: smaller win, lower
  priority.  Can be drilled as a follow-up if P1+P2 don't fully
  close the gap.
- **General GISel combiner work**: the `add %, -1` + `icmp eq %, 0`
  pattern could be combined at the IR level into a "dec-and-test"
  pseudo, which the selector could lower as P1.  This is a deeper
  fix but cross-cuts many backends; out of scope.
- **MachineScheduler tuning**: could potentially reorder DEC and
  the test-reload to avoid the redundant reload, but the scheduler
  doesn't see them as independent (both read `$a`).  Out of scope.
- **#172 A-pin liveness-aware**: a separate axis of the same
  general "regalloc keeps loop carriers in fixed registers" problem.
  Even with P1+P2, the residual ~38 % vs-SDCC gap will likely need
  this work.  Tracked separately.
