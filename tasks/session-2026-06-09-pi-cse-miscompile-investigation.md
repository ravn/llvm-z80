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

## Where the bug actually lives — NAMED 2026-06-09 continuation

**Bug location:** the outer-loop checksum PHI (`%2:gr16` in the MIR,
`%4` in the IR) gets a split delivery path on the back-edge.  At
`bb.5`'s tail (outer-loop body), `%32` (new checksum = old + c + q)
is computed into HL and stored to memory at `__sfrend_bench_run-16`.
At `bb.2`'s head (loop top), the PHI is consumed from register DE
and written to the same memory slot.  But DE is never set to `%32` —
it still holds `m` (the `umodsi3` leftover from `__umodsi3`'s DE return).
The store-then-overwrite chain means each outer iteration's checksum
slot ends up holding the **previous** iteration's `m`, not the
accumulator.  Trace confirms: each block N's reported checksum equals
*only* that block's contribution (c_old + q), with the accumulator
permanently lost.

**Method of detection:** dumped per-block `checksum` via `OUT (2),A`
in a traced variant of bench_pi.c (`/tmp/bench_pi_traced.c`).  PASS
shows monotone-ish accumulation 3141 → 9067 → 14425 → ... → 28116;
FAIL shows the per-block contribution 3141 → 5926 → 5358 → ... →
2089 (just the latest block, never accumulating).

**Final-asm evidence (CSE-on, `/tmp/pi_red_clang_on.s` lines 115-146):**

    .LBB0_4:                            ; outer-loop body (bb.5)
      ...
      call ___umodsi3                   ; DE = m, HL = (high half, dropped)
      pop af / pop af
      ld   hl,(__sfrend_bench_run-16)   ; HL = checksum_old
      ld   bc,(__sfrend_bench_run-20)   ; BC = c_old
      add  hl,bc                        ; HL = checksum + c
      ld   bc,(__sfrend_bench_run-2)    ; BC = q (saved earlier)
      add  hl,bc                        ; HL = checksum + c + q = NEW CHECKSUM
      ld   (__sfrend_bench_run-16),hl   ; <-- store new checksum to memory
      ld   bc,65522
      ld   hl,(__sfrend_bench_run-18)
      add  hl,bc                        ; HL = new k
      jp   .LBB0_1
    .LBB0_1:                            ; loop top (bb.2)
      ld   (__sfrend_bench_run-16),de   ; <-- !!! overwrites checksum with stale DE (=m)
      ...

The two writes to `-16` are racing; the second wins; DE holds the
wrong value.

**Why MachineCSE triggers it:** pre-CSE, `bb.2` had a fresh
`%109 = LD_r16_nn 0` load for the checksum PHI's "from `bb.1`" arm.
The coalescer saw a clean def at the loop top and assigned the PHI
to a single register slot.  Post-CSE, `%109` is gone — `%2`'s PHI now
takes input `%3:gr16` (entry-block constant 0) from `bb.1` and
`%32:gr16` from `bb.5`.  The coalescer's choice differs and produces
the split delivery (memory store on the back-edge arm, register read
at the loop top), but neither arm writes the register that the other
arm reads.

**This is the kind of correctness bug `-verify-machineinstrs` cannot
catch** — SSA is well-formed; liveness annotations agree; only the
*runtime* semantics are wrong because the back-edge delivery and the
loop-top consumption disagree about where the value lives.

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

## Responsible pass — NAMED 2026-06-09 (final)

**Branch Folder (Control Flow Optimizer, `branch-folder`)** is the pass
that introduces the runtime miscompile.  Confirmed by direct toggle:

    CLANG_EXTRA="-mllvm -z80-enable-cse -mllvm -disable-branch-fold" \
      BENCH=pi ONLY=llvm-z80 ./sweep.sh
    -> pi  llvm-z80  bin=884  text=300  ts=58865925  PASS

With MachineCSE on AND branch-folding disabled, pi computes the correct
checksum (28116).  Disabling MachineCSE alone or disabling
branch-folding alone is sufficient to fix the miscompile.

**MIR delta across `branch-folder`:**

After "Machine Late Instructions Cleanup" (pre-branch-fold), `bb.0`
holds two stores of `$de` to two different BSS slots (the checksum
init and the c init, both `LD_r16_nn 0; LD_nnind_DE <sfrend>`):

    bb.0:
      ...
      $de = LD_r16_nn 0
      LD_nnind_DE <__sfrend_bench_run>   ; store DE=0 to slot A
      LD_nnind_DE <__sfrend_bench_run>   ; store DE=0 to slot B (same DE, different offset)

    bb.1:
      ; predecessors: bb.0, bb.4
      liveins: $hl                       ; <-- $de NOT live-in
      KILL $hl ; LD_A_L ; OR_H
      LD_nnind_HL <__sfrend_bench_run>   ; store HL (k) to slot C
      LD_nnind_HL <__sfrend_bench_run>   ; store HL (k) to slot D
      LD_r16_nn 0 -> $bc
      LD_r16_nn 0 -> $de
      JR_Z bb.5

After Branch Folder:

    bb.0:
      ...
      $de = LD_r16_nn 0
      LD_nnind_DE <__sfrend_bench_run>   ; ONE store remains in bb.0

    bb.1:
      ; predecessors: bb.0, bb.4
      liveins: $hl, $de                  ; <-- $de NOW live-in!
      LD_nnind_DE <__sfrend_bench_run>   ; <-- THE MOVED STORE -- BUG
      KILL $hl ; LD_A_L ; OR_H
      ...

Branch Folder removed one of bb.0's two consecutive `LD_nnind_DE`
stores and placed an equivalent store at the head of bb.1, adding
`$de` to bb.1's liveins.  The hoist is unsound: `bb.4` (the outer
back-edge predecessor of `bb.1`) does NOT end with `$de=0`; `bb.4`'s
last call (`__umodsi3`) leaves DE holding the new `c` (= `m`).  When
control reaches `bb.1` from `bb.4`, the hoisted `LD_nnind_DE` writes
`m` into the checksum BSS slot, overwriting the freshly-computed new
checksum that `bb.4` had just stored.  Each outer iteration's
accumulator is thus replaced by the previous iteration's `m`, and the
trace shows per-block contribution instead of running total.

**Why MachineCSE makes the difference:** with CSE OFF, `bb.0` ends
with `ld bc,0; ld de,0; ld (-16),de` — only the checksum init is in
the entry path; the other BSS inits live in a separate bb.2 block
reached only on the forward path from bb.1, not via the back-edge.
With CSE ON, the deleted constant-load PHI sources collapse the
forward-only bb.2 prelude, leaving bb.0 with two adjacent
`LD_nnind_DE` stores that Branch Folder then misjudges as
"hoist-equivalent."

## Filing readiness — READY 2026-06-09

Now have a complete, nameable root cause:
- Pass: Branch Folder (`llvm/lib/CodeGen/BranchFolding.cpp`).
- Bug class: unsound cross-block hoist of a store whose source-register
  liveness is single-predecessor but moved into a multi-predecessor
  successor.
- Reducer: 69-line `.ll` (`/tmp/pi_reduce_out.ll`).
- Verification: toggle of `-mllvm -disable-branch-fold` makes the
  miscompile go away.

Per HARD rule `feedback_explain_before_filing` we now have the
"OK which pass produces that" answer.  The bug is in generic LLVM
(Branch Folding is target-agnostic), so the upstream destination would
be `llvm/llvm-project`, NOT `llvm-z80/llvm-z80` — per HARD rule
`feedback_upstream_routing_two_targets`.  Filing requires the user's
explicit per-filing go-ahead.

The mitigation (CSE off by default) stays as-is until the upstream
fix lands.  Production code cost is +21 B autoload / +7 B cpnos /
+8 B BIOS — bounded and documented.

## Pointers

- llvm-z80@34aae512ebc4 — flip MachineCSE default to off.
- `Z80TargetMachine.cpp` lines 86-112 — `EnableMachineCSE` rationale
  comment (mentions this writeup by date).
- `tasks/known-suboptimal-codegen.md` B15 — public-facing entry,
  links here for the details.
- `Z80InstrInfo.cpp:237-268` — the `EX_DE_HL`-as-one-way-copy
  peephole (ruled out as sole carrier; kept as a future candidate
  to refine independently).
