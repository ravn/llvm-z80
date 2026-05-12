# Session 60b (2026-05-12) — #132 cross-MBB BSS-spill: edge-split + first real-world fire

## Context

Session 60 (commit `b127453891b2`) landed the structural foundation
for #132 — the cross-MBB BSS-spill → PUSH/POP peephole with a
conservative single-predecessor-escape gate.  Production-impact
measurement at the time was reported as "byte-identical 1906 B"
on cpnos-rom.  That measurement was contaminated by a stale
`clang` binary (only `llc` was relinked against the updated
`libLLVMZ80CodeGen.a`), so the peephole had not actually been
exercised through the cpnos build path.

## Bug found this session

After a fresh `ninja -C build-macos clang` link, the peephole does
fire on cpnos-rom snios_c.c.  Specifically `_snios_sndmsg_force`'s
inner retry-loop `do { ... if (r != TRANSPORT_TIMEOUT) goto
got_first_ack; } while (--t);` triggers the rewrite: the t-counter
STORE→LOAD pair across `_recv_byte_t` becomes PUSH/POP AF, and a
compensating `inc sp; inc sp` is prepended at the start of the
got_first_ack MBB (single-pred from the loop body).

Production cpnos-rom (clang/pio-irq) payload: **1906 B → 1904 B**
(−2 B).

## Extension landed this session: edge-split for multi-pred escapes

Single-pred-only was strict enough that almost no candidates
would qualify in real code.  The extension adds an edge-split
strategy: when the escape MBB has multiple predecessors but
inserting a new MBB just before it in layout is safe (its
layout-predecessor doesn't already fall through to it AND
MBB_A's terminator has an explicit MBB operand referencing the
escape), create a fresh compensation MBB containing only
`inc sp; inc sp` and rewire MBB_A's terminator to target it.
The new MBB falls through to the original escape, preserving SP
balance only on that path.

Both strategies cost 2 B of compensation per escape edge.  Cost
gate unchanged: fire only when `Save > 2*Nesc`.

The edge-split path is wired but does **not** fire on the
current cpnos-rom build — all production cross-MBB candidates
have single-pred escapes.  No synthetic IR-level lit test
exercises it either, because regalloc + the existing in-MBB
peepholes are aggressive enough that no `.ll` shape I could
construct produced a multi-pred escape with surviving cross-MBB
BSS-spill.  The path is in place as infrastructure; future
regalloc evolution may surface candidates.

## Implementation summary

`Z80LateOptimization.cpp` — the third peephole (added in session
60) was extended:

  - Successor walk now categorises each escape into
    `ESC_PrependInPlace` or `ESC_InsertBefore` based on
    pred-count of the escape and layout safety.
  - The rewrite loop dispatches: `BuildMI` to escape head for
    prepend; or `MF.CreateMachineBasicBlock` + `MF.insert` +
    `MBB_A.ReplaceUsesOfBlockWith` for edge-split.
  - Comment block updated to describe both strategies.

## Verification

  - **lit suite**: 95/95 (93 PASS + 2 XFAIL).  Existing fixture
    `issue-132-bss-spill-cross-mbb.ll` still PASS (covers the
    prepend-in-place path that fires in production).
  - **z80-utils test-runner clang Oz** (165 tests): 113 PASS /
    0 FAIL / 1 FATAL / 51 SKIP.  Baseline match.
  - **cpnos-rom clang/pio-irq**: payload 1904 B (−2 B from
    session-60 baseline of 1906 B).  cpnos.bin diff confined to
    timestamp + the rewritten `_snios_sndmsg_force` region.
  - **cpnos-polypascal-test clang/pio-irq**: PASS (PPAS PRIMES
    completed, 29989 seen).
  - **cpnos-polypascal-test clang/sio**: PASS (PPAS PRIMES
    completed, 29989 seen).  Confirms transport-agnostic
    correctness per `feedback_value_oracle_all_transport_cells`.

## Files touched

  - `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — successor
    classification + edge-split rewrite logic.
  - `llvm/test/CodeGen/Z80/issue-132-bss-spill-cross-mbb.ll` —
    description rewritten to reflect both strategies.
  - `tasks/session60b-132-edge-split.md` — this doc.

## Followups

  - Find or hand-craft a `.mir` test that exercises the
    edge-split path (deferred).
  - Liveness-driven 1 B compensation (`pop af` when A+FLAGS
    dead at escape entry) — would convert −2 B per escape to
    −3 B per escape on the prepend-in-place case.
  - Examine `_snios_rcvmsg_c` — trace flagged BB#3 as
    candidate but the production output didn't gain bytes
    there; investigate why (slot reuse? cost gate at zero net?).

## Rules-checked

  - `feedback_compiler_bug_test`: pre-existing lit test extended
    by description; behavioural coverage of prepend-in-place
    intact.
  - `feedback_no_commit_first_version`: value oracle satisfied
    (lit + test-runner + 2-cell polypascal matrix).
  - `feedback_value_oracle_all_transport_cells`: both pio-irq
    and sio polypascal cells run; both PASS.
  - `feedback_compare_total_section_sizes`: `.payload` total
    (1904 B vs 1906 B).
  - `feedback_extract_rules_from_time_sinks`: see "Lessons" —
    proposing rule below.

## Lessons (proposed memory entry)

When verifying a compiler change against a downstream build that
spawns its own compiler subprocess: **`ninja llc` is not enough**
for any test that goes through `clang` (e.g. the rc700-gensmedet
cpnos build).  The cpnos Makefile invokes
`$(LLVMZ80)/build-macos/bin/clang`, which is a separately-linked
binary against `libLLVMZ80CodeGen.a`.  Without `ninja clang`, the
clang binary continues to use the **previous** `.a` snapshot,
and the build appears to be unchanged.

This caused session 60 to report "byte-identical" cpnos-rom
output for a peephole that DOES fire on production code; the
−2 B saving was only visible after `ninja clang`.

Proposed rule (would slot into §4 "Before any build / compile
/ link flag change"):

> **Always `ninja clang llc` together for llvm-z80 changes** —
> HARD: any change to a pass that downstream builds invoke via
> clang (i.e. anything in `llvm/lib/Target/Z80/`) needs both
> `clang` and `llc` relinked.  `ninja llc` alone leaves the
> `clang` symlink pointing at a stale binary with the old `.a`
> archive linked in.  Symptom: lit + size benchmarks agree
> across before/after but downstream `cpnos-rom` / `rcbios` /
> `autoload-in-c` builds show no change.
