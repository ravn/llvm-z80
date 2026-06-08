# Pi MachineCSE miscompile investigation — 2026-06-09

## Summary

Pinned down the structural change MachineCSE makes on `bench_pi.c` that
triggers a runtime miscompile at -Oz, identified the back-edge
codegen difference that follows, ruled out the `EX_DE_HL`-as-one-way-
copy peephole as the sole carrier, and stopped short of a single-line
fix because the bug surfaces only through a downstream regalloc /
coalesce interaction we don't yet have an isolation handle for.

CSE remains disabled by default (Z80TargetMachine.cpp); this writeup
is the prerequisite for the upstream filing per HARD rule
`feedback_explain_before_filing`.

## Reproducer artifacts

- `/tmp/bench_pi.ll` — initial IR (89 lines), `clang -Oz -emit-llvm`
  on `rc700-gensmedet/tasks/compiler-comparison-corpus/bench_pi.c`.
- `/tmp/pi_reduce_out.ll` — `llvm-reduce` output (69 lines).  The
  spigot inner loop with i32 div/mod against zext(i16) is irreducible
  past this point (the divmod pair is the minimal feature set).
- `/tmp/pi_reduce_interesting.sh` — interestingness gate for
  `llvm-reduce`: PASS (exit 0) iff the binary is correct with
  `-z80-enable-cse=false` AND incorrect with `-z80-enable-cse=true`.

## What MachineCSE actually does

Pre-CSE MIR (`bench_run` at outer-loop header `bb.2`):

    %0:gr16        = PHI %37, %bb.1, %36, %bb.5      ; outer-loop i / SCALE
    %1:gr16_bcde   = PHI %3,  %bb.1, %45, %bb.5      ; outer-loop c
    %2:gr16        = PHI %3,  %bb.1, %32, %bb.5      ; outer-loop checksum
    %108:gr16      = LD_r16_nn 0                     ; constant 0
    %109:gr16      = LD_r16_nn 0                     ; constant 0
    %110:gr16      = LD_r16_nn 280                   ; constant SCALE
    ...
    ; inner-loop header bb.3:
    %5:gr16        = PHI %25, %bb.4, %110, %bb.2     ; inner i (=SCALE on entry)
    %94:gr16       = PHI %57, %bb.4, %109, %bb.2     ; d high (=0 on entry)
    %95:gr16_bcde  = PHI %58, %bb.4, %108, %bb.2     ; d low  (=0 on entry)

MachineCSE notes that `%108`, `%109` are `LD 0` (already in `%3`) and
`%110` is `LD 280` (already in `%37`), commones them away, and
re-points the inner-loop PHI back-edges into entry-block vregs:

    ; %108, %109, %110 REMOVED
    %5:gr16        = PHI %25, %bb.4, %37, %bb.2
    %94:gr16       = PHI %57, %bb.4, %3,  %bb.2
    %95:gr16_bcde  = PHI %58, %bb.4, %3,  %bb.2

This is the entire delta MachineCSE introduces in `bench_run`.
Semantically identical at the SSA / IR level — `%3` and `%37` are
constants 0 and 280, their values are immutable across the function.

## Downstream effect

The smaller live range of the now-removed `%108`/`%109`/`%110` shifts
regalloc decisions for the inner loop's back-edge.  Comparing final
asm at the back-edge inside `bb.4`:

**CSE off (PASS, correct):**

    ld   (__sfrend_bench_run-8),de
    ld   (__sfrend_bench_run-6),hl
    jp   .LBB0_3                        ; back to inner loop top

**CSE on (FAIL, miscompile):**

    ld   c,e
    ld   b,d
    ex   de,hl                          ; <-- destructive swap as 1-way copy
    pop  af
    pop  af
    jp   .LBB0_2

The CSE-on version uses `EX DE,HL` as a one-way `DE ← HL` copy
(known-fragile pattern; CLAUDE.md flags it).  But disabling the
`EX_DE_HL` shortcut in `Z80InstrInfo::copyPhysReg` (forced fall-through
to the 2 × `LD r,r` path) does **not** fix the miscompile — pi still
returns the wrong checksum (881 B / 58.88 M ts FAIL vs 880 / 58.87 M).
So `EX_DE_HL` is at most one symptom, not the root cause.

## Where the bug actually lives (hypothesis)

The MachineCSE change widens the live range of `%3` / `%37` from the
entry block all the way through the outer + inner loops to the
inner-loop PHI back-edges.  That extra liveness changes:

- which vregs get coalesced (the inner-loop PHIs no longer have
  fresh in-block defs to coalesce against);
- which SSA values get spilled to BSS vs kept in registers across
  the inner-loop body;
- which back-edge values get assigned to which physical pairs.

The miscompile shows up in the **back-edge value transport** of the
i32 `d` accumulator (`d *= (uint32_t)i` followed by the back-edge).
Specifically the `mulsi3` result HL:DE flows into the back-edge as
two 16-bit pieces, and the post-call sequence the regalloc emits
gets one of those pieces into the wrong physical register on at least
one iteration — but the verifier is silent (`-verify-machineinstrs`
clean) and the SSA / liveness graph is internally consistent.

The simplest hypothesis that fits: the register coalescer or the
RegisterCoalescer's interaction with PHI elimination makes a
locally-correct choice that depends on a transitive invariant
MachineCSE has broken.

## Things ruled out

- **EX_DE_HL one-way copy in copyPhysReg** — disabling it shifts
  the symptom by 1 B and 12 k ts but the checksum is still wrong.
- **Machine verifier** — silent.  No SSA / liveness violations.
- **llc -O2 vs clang -Oz** — same IR through llc -O2 produces
  identical asm with CSE on vs off; the divergence shows up only
  in the clang -Oz pipeline (size attribute + size-specific passes).
  This means a clang-only post-IR pass is part of the chain.
- **Z80NarrowNoIndex / Z80FixupImplicitDefs / Z80LateOptimization** —
  not investigated yet; possible carriers.

## Why we stopped here

Further isolation needs either:

1. A binary-search through llvm's pass list with
   `-mllvm -print-after=<pass>` to spot the first pass that produces
   different post-CSE-on vs post-CSE-off MIR with semantic divergence;
2. or a programmatic differential MIR-equivalence checker (rebuild
   semantic interpretation) — out of session scope.

The mitigation (CSE off by default) is stable; lit + runtime + the
five compiler-comparison-corpus benches all PASS.  Production code
cost is +21 B autoload / +7 B cpnos / +8 B BIOS — bounded and
documented.  Future session can pick up with the
`/tmp/pi_reduce_out.ll` + `/tmp/pi_reduce_interesting.sh` pair.

## Filing readiness

NOT YET READY to file at llvm-z80/llvm-z80.  Per HARD rule
`feedback_explain_before_filing`, we need a root cause we can name,
not "MachineCSE exposes a downstream bug we haven't found."  The
reducer is good, the MIR-delta is clean, but the "what's the actual
bug" is missing — filing as-is would be the same misroute that got
PR #17 retracted in session #77.

## Pointers

- llvm-z80@34aae512ebc4 — flip MachineCSE default to off.
- `Z80TargetMachine.cpp` lines 86-112 — `EnableMachineCSE` rationale
  comment (mentions this writeup by date).
- `tasks/known-suboptimal-codegen.md` B15 — public-facing entry,
  links here for the details.
- `Z80InstrInfo.cpp:237-268` — the `EX_DE_HL`-as-one-way-copy
  peephole (ruled out as sole carrier; kept as a future candidate
  to refine independently).
