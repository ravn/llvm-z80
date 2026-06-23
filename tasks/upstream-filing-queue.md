# Upstream Filing Queue

**Last updated:** 2026-06-23  
**Filing target:** `llvm/llvm-project` (all four items are generic-LLVM, not Z80-specific)  
**Process rules:** `feedback_explain_before_filing` — root cause explained in chat + explicit per-filing go-ahead before any post. User writes the intro in their own voice. Check for duplicates first.

---

## Ready to file now

### #224 — LiveVariables spurious super-register implicit-def
**Staging tracker:** ravn/llvm-z80#224  
**Fix in this fork:** commit `a32c4f33` (`llvm/lib/CodeGen/LiveVariables.cpp`)  
**Upstream issue:** `llvm/llvm-project#156428` (open since 2025-09-02)  
**Analysis posted:** https://github.com/llvm/llvm-project/issues/156428#issuecomment-4779185912 (2026-06-23)  
**Draft (no-hints):** `tasks/upstream-156428-draft-issue.md`

**Status:** Analysis comment posted. Next step: open a PR at `llvm/llvm-project`
with a clean-room implementation (`Fixes #156428` in body). All information
needed is in the comment — open a fresh session, read the comment, implement.

---

### #226 — RFC: TTI::shouldExpandExperimentalMemSetPattern
**Staging tracker:** ravn/llvm-z80#226  
**POC in this fork:** commit `6839ebc4bcbf` (TTI hook + `PreISelIntrinsicLowering` consult + Z80 consumer)  
**Upstream hook point:** `llvm/lib/CodeGen/PreISelIntrinsicLowering.cpp` line ~427 has a `FIXME` exactly where the hook belongs.

**Root cause (problem statement):** `llvm.experimental.memset.pattern` is always expanded by `PreISelIntrinsicLowering` to a libcall or open-coded loop. There is no way for a backend to claim the intrinsic and lower it natively. Backends with block-fill instructions (Z80 LDIR, …) need a TTI hook to intercept before expansion.

**What to do:**
1. Open an **RFC issue** (not a PR) at `llvm/llvm-project`.
2. The issue body leads with your framing of the problem; the technical detail follows.
3. Goal is to get direction on the API shape before writing the PR.

---

## Needs your summary before filing

### #219 — TruncInstCombine outside-user bail (missed optimisation)
**Staging tracker:** ravn/llvm-z80#219  
**Upstream verified:** reproduces on `de59f9ed` (`opt` only, no Z80).

**Root cause:** `TruncInstCombine` abandons narrowing the entire expression graph when any in-graph value has an outside user (line 274-288, `TruncInstCombine.cpp`). Two cases that ARE safely rewritable are incorrectly rejected: (a) the outside user is a ZExt/SExt back to the original width (a no-op after narrowing), and (b) the outside user is an `icmp` against a constant that fits in the narrower type.

**What to do:**
1. Write 2-3 sentences in your own voice for the issue intro (the staged body already has the full technical detail).
2. Prepend your intro, then open an issue at `llvm/llvm-project`.
3. Framing: missed-optimisation (not a miscompile).

---

## Needs framing work before filing

### #225 — deleteDeadLoop malforms SSA on exit blocks with shared predecessors
**Staging tracker:** ravn/llvm-z80#225  
**Fix in this fork:** commit `6dc359f0` (`llvm/lib/Transforms/Utils/LoopUtils.cpp`)

**Root cause:** `deleteDeadLoop` calls `removeIncomingValueIf` on phi nodes in exit blocks but does not account for exit blocks that have non-loop predecessors. This leaves the phi with a missing incoming value from those predecessors, malforming SSA. The function carries a `hasDedicatedExits()` assertion, so the bug is silent in release builds when a caller violates the precondition.

**Framing concern:** upstream will say "fix the caller, the contract is documented." The correct framing is:  
**(a)** Our fix as **defensive hardening** — even when the assertion fires in debug, the release-build corruption is real and dangerous, so the utility should be safe to call with shared-predecessor exits; OR  
**(b)** File two items: a separate caller-contract issue at the fork (#217 is closed) + the utility hardening at upstream.

**What to do:**
1. Agree on framing (a) or (b) above.
2. Revise the issue body to open with that framing.
3. Then file at `llvm/llvm-project` with your intro.

---

## Process notes

- **Never file without a per-item go-ahead** (`feedback_explain_before_filing`).
- **User writes the intro.** The technical body can be Claude-authored, but the opening paragraph must be in your voice — PR #17 was closed because the maintainer couldn't get answers from the contributor about the content.
- **No fix proposals** in issue bodies (`feedback_file_bugs_not_fixes`). State root cause + repro + current vs expected behaviour. Maintainer decides how to fix.
- **Check for duplicates** at `llvm/llvm-project` before each filing.
- **Routing:** all four items go to `llvm/llvm-project`, not to `llvm-z80/llvm-z80` (which is for Z80-specific bugs only).
