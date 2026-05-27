# Session 73s cont. — differential oracles + 3 miscompiles closed (2026-05-26/27)

Continuation of `session73s-iy-sizegate-summary.md`.  Started from "are the
oracles good enough?"; ended with two differential test oracles, three real
miscompiles closed, and a green oracle baseline.

## Headline

| # | bug | how found | fix |
|---|-----|-----------|-----|
| **#202** | cross-block BSS-spill->PUSH/POP dropped a loop-carried store-back | manual (verify-cluster) | unify loop-carried guard across all 4 spill peepholes |
| **#204** | address-taken slot's store converted to PUSH (double-ptr swap read it via a pointer) | **native oracle** | unify address-taken guard across all 4 peepholes |
| **#136** | "38 mystery O1 fixtures" — Z80LoopIdiomFill emits an *overlapping* memcpy that InstCombine inlines to a wide load+store at -O1, breaking LDIR forward-propagation | **diff-opt oracle + opt-bisect** | one-line `volatile` on the memcpy (forces LDIR lowering) |

All three **production byte-identical** (cpnos/autoload/BIOS unchanged); lit
121+5 throughout.

## Two differential oracles (`z80-utils/test-runner`)

- **`-diff-opt`** — every opt level of a program must return the same value
  (optimization is semantics-preserving); a disagreement is a miscompile
  independent of the hand-written `expect`.  Caught #202/#15; root-caused #136.
- **`-native-oracle`** — compiles + runs each test with the host C compiler and
  compares (a *computed* reference, not a hand-written expect); catches
  *consistently-wrong* values `-diff-opt` can't.  **Found #204** on its first run;
  pinned #136's wrong side (O1).  One UB false-positive (test_94 overlapping
  memcpy) excluded via a new `NATIVE-SKIP` directive.

Both now sit at a **clean baseline** (0 DIFFOPT / 0 NATIVE) in default AND
+static-stack configs (sm83 isn't emulatable; omit-fp clean).  SKIP-IF gained
`+feature` support; `test_36` (mutual recursion) skipped under non-reentrant
+static-stack.

## #136 root-cause method (worth remembering)

diff-opt flagged O1-only divergence -> first-failing-line probe (rewrite the
`CHECK` macro to record the line) isolated a 3-element `buf[j]=v` fill ->
extracted a 5-line repro -> the optimized IR showed `store buf[0]; load i16 buf;
store i16 buf+1` (reads uninitialized buf[1]) -> opt-bisect (`-opt-bisect-limit`)
pinned pass #65 `Z80LoopIdiomFill` (the memcpy creator) and pass #75 instcombine
(the mis-inliner).  The overlapping memcpy is UB; the fix makes it volatile so no
generic pass touches it.  (A `memset`-for-K=1 attempt regressed -Oz across the
fixtures and was backed off -- the value oracle caught it pre-commit.)

## Also this session

- **IX un-reserve** investigated + **reverted**: a regression in the callee-saved
  ABI (BIOS +91 B); the cost model can't reach neutral; all 3 firmware images have
  ZERO frame-pointer functions, so the only IX win is caller-saved IX, gated on #12
  (FP-elimination).  Full write-up: `tasks/issue12-ix-unreserve-measurement-2026-05-26.md`.
- **rc700#100** (autoload banner check) fixed + CLOSED.
- **#203** advanced (2 of the cleanly-extractable guards unified; OPEN for the rest).
- **#205** filed (LDIR-fill UB-in-IR follow-up to #136).
- **CI differential gate** designed + PARKED (`tasks/ci-test-runner-differential-gate-PARKED.md`).

## Discipline hardened (AGENTS.md + memory, all repos)

- **Certainty in filed issues** — a root cause is a hypothesis until checked (#202
  was filed with a wrong "accumulator-aliasing" cause).
- **Baseline before you change** — capture the control on the unmodified system first.
- **A bug found by luck is a bug in your oracle** — audit the *detector*, not just
  the cause (the differential oracle came only after being asked "are the oracles
  good enough?").

## Open (see tasks/todo.md)

#203 (structural guard unification), #205 (non-UB LDIR-fill), the parked CI gate,
the ~56 test-runner FATAL triage (50 = O0 big-fixture emulator no-result, 6 =
test_48 alloca.h), plus the carried #194/#200 (verify-gate) and #12 (caller-saved IX).
