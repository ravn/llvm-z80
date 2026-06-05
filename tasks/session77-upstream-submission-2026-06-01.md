# Session 77 — Curated upstream submission to llvm-z80/llvm-z80 (2026-06-01)

> **RETRACTION (2026-06-05).** PR #17 was **closed by @zlfn** with: *"I can't
> merge code contributions that contributors can't explain themselves.
> Especially if it's unclear whether they should be submitted to upstream or
> Z80 fork like this."* The rejection was correct on two grounds:
>
> 1. **Misroute.** 5 of the 6 XFAIL demonstrations are target-agnostic
>    generic-LLVM bugs (#18 deleteDeadLoop, #19/#21 TruncInstCombine, #20
>    SimplifyCFG cost gate, #22 InstCombine memcpy-fold) that belong at
>    `llvm/llvm-project`, not at the Z80 fork. This session misread "z80
>    upstream only" as a routing directive ("file at the Z80 fork") when it
>    was actually a velocity directive ("don't fan out to multiple
>    campaigns").
> 2. **Undefendable bundle.** Six bugs in one PR, AI-generated rationale,
>    no per-bug user-defendability check. The user said upthread: *"I have
>    not understood these things well... trust that [the AI] got it right"*
>    — exactly what zlfn rightly rejected.
>
> **Cleanup actions (option D, user-confirmed 2026-06-05):**
> - PR #17 closed (by maintainer). No re-file.
> - Issues #18-#25 to be withdrawn pending per-bug re-evaluation under the
>   new explain-before-filing rule.
> - PR #27 (test-runner) and Issue #26 stay — they were correctly scoped
>   to genuine Z80-backend infrastructure and PR #27 was merged upstream.
>
> **Memory-rule updates:** `tasks/memory/feedback_upstream_routing_two_targets.md`,
> `tasks/memory/feedback_no_upstream_issues.md`, and the new
> `tasks/memory/feedback_explain_before_filing.md` codify the corrected policy.
>
> What follows below is the original session writeup, preserved as-is for
> history. It documents what was *attempted*, not what was *correct*.

---

Continuation of session 77. After the #158 packaging + upstream-readiness assessment
(recorded in the workspace CLAUDE.md #77 entry), the user directed a **curated upstream
submission** to the z80 fork-of-record **`llvm-z80/llvm-z80`** (@zlfn). Directive:
**"z80 upstream only"**, "do not create a PR per bug — one tests PR + issues", "discuss
each issue before creating it", and the fixes described may differ from our workarounds.

## What was submitted (all at `llvm-z80/llvm-z80`)

**PR #17 — tests-only**, branched off `upstream/main` (tests-only diff), 6 `XFAIL`
bug-demonstration tests (mergeable; XFAIL keeps upstream CI green; un-XFAIL as each fix
lands):
- `CodeGen/Z80/issue-156428-livevars-independent-subregs.mir` (A1)
- `CodeGen/Z80/issue-182-deletedeadloop-phi.ll` (A2)
- `Transforms/AggressiveInstCombine/narrow-through-argument.ll` (A3)
- `Transforms/AggressiveInstCombine/issue-165-trunc-outside-user.ll` (A5)
- `Transforms/InstCombine/memcpy-fold-nonlegal-int.ll` (the audit's missed bug)
- `CodeGen/z80-issue-168-conditional-xor-no-fold.c` (A6)

**Issues — 5 fixed generic-LLVM bugs** (each links its failing test + frames the fix as a
proposal; cross-linked to the downstream ravn issue):
- #18 deleteDeadLoop SSA-malform on shared exit (ravn#182)
- #19 TruncInstCombine narrow-through-Argument (ravn#158)
- #20 SimplifyCFG cost-gate foldTwoEntryPHINode (ravn#168)
- #21 TruncInstCombine icmp/and outside-user allowlist (ravn#160+#165)
- #22 InstCombine memcpy-fold legality gate (ravn#87+#73)
- A1 LiveVariables is **already open upstream** as `llvm/llvm-project#156428` (independent
  DSP-backend repro; introduced by `llvm/llvm-project#119446`) → referenced, not duplicated.

**Issues — 3 unfixed bugs** (explicitly "real bug, no fix found"):
- #23 MachineLICM/MachineCSE pessimize tiny-register-file targets (workaround: `disablePass`)
- #24 MachineScheduler reload-after-test reorder gap (workaround: `Z80ReorderTestDec`)
- #25 `opt -mtriple=z80` stack-overflow on datalayout-less IR (root cause not isolated)

**Issue #26 + PR #27 — test-suite enhancement**: bring the enhanced `z80-utils/test-runner`
(multi-oracle: `expect` / `-diff-opt` / `-native` / `-verify-machineinstrs`) + the CI
workflow to upstream. PR #27 is **infrastructure only** (no new testcases; `cargo check`
green; fork-fix-dependent gates omitted so upstream CI stays green). Fork issue refs in the
brought files were fully qualified `ravn/llvm-z80#NNN`.

## Completeness audit (the rigorous part)

Verified coverage via the **ground truth**: the generic-code diff `ravn/main` vs
`upstream/main` (everything outside `Target/Z80` + Z80-clang). Only **5 generic
transform/codegen files** differ — all mapped. The audit caught one **missed** fix
(`InstCombineCalls.cpp` memcpy-fold, ravn#87/#73) → filed as #22 with a test. Every clang
change is the intrinsics/attributes/CC **feature** work (not bugs). De-dup finding: the
whole `-verify-machineinstrs` family (ravn#194/#199/#200/#209/#210) is downstream of the
single LiveVariables bug (#156428).

## Intentionally deferred (documented, not gaps)
- A4 ravn#163 (and-mask synthetic trunc root) — **inert on the Z80-only build** (x86-only
  effect; x86 not compiled in) → no Z80-observable demonstration.
- ravn#164 (TruncInstCombine zext re-insertion cost model) — design-needed.
- ravn#162 (call-arg body-peek) — speculative, needs a frontend tag.

## Follow-on (future)
- As fixes land upstream: bring the expanded corpus (ravn 181 vs upstream 59) + re-enable
  the production-verify / generic-fix CI gates in the PR #27 workflow.
- PR #27's CI first actually runs on `llvm-z80/llvm-z80` only once merged.

## Policy recorded (user-confirmed, session 77)
"z80 upstream only" engagement-mode: user-directed curated submission to `llvm-z80/llvm-z80`
(issues + a tests-only PR + infra PR) is now an approved workflow — never per-bug PRs, never
fix PRs, never official `llvm/llvm-project` directly, never unsolicited. Memories updated:
`feedback_no_upstream_issues`, `feedback_no_pull_requests`, `feedback_upstream_routing_two_targets`.
