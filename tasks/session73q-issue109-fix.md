# Session 73q — #109 fix: ADD HL,rr commutativity peephole BC safety check

**Date:** 2026-05-23
**Outcome:** Code change minimal (one `isRegDeadAfter(...BC)` check); doc/code mismatch closed.

## What was wrong

The peephole at `Z80LateOptimization.cpp:1296` rewrites:
```
LD C,L; LD B,H; EX DE,HL; ADD HL,BC   (4 B)
  → ADD HL,DE                          (1 B)
```

The original code's comment said:
> Safety: the original writes BC (clobbers it).  Our replacement does not write BC — safe as long as nobody reads BC expecting the old HL value.  **Check that BC is not read between ADD and the next BC def** (i.e., BC was only a temporary for this ADD).

But the code itself did NO such check.  The peephole proceeded unconditionally on the four-instruction match.  In practice this works (GISel rarely emits a shape that lives BC past the ADD) but is the same class of admitted-but-not-enforced shortcut that surfaced as latent bugs in #104 and #107.

## What was fixed

Added a `isRegDeadAfter(AfterRewrite, MBB, TRI, Z80::BC)` check after the four-instruction match.  Bails if BC is live past the rewrite point (which is after I3 in the no-trailing-EX case, or after I4 in the trailing-EX case).

Comment block updated to match: explicitly says the check is "cheap and matches the original safety comment that was aspirational, not enforced (#109)."

## Verification

- Lit: 109 PASS + 3 XFAIL (unchanged).
- cpnos PROM1 (clang): 2029 B (unchanged from before this fix; the BC-dead-after check passes for all current cpnos fires — empirical confirmation that GISel doesn't produce the unsafe shape).
- test-runner clang sweep: (pending — running in background).
- AES `aes256.c -Oz` `.text`: 3299 B (unchanged).

## Implication

The fix is a **safety hardening with zero observable codegen effect**.  Justification for the change:
1. Comment-doc consistency (the safety condition is now actually checked).
2. Defensive against future regalloc changes that might suddenly produce the unsafe shape.
3. Establishes a pattern for fixing similar admitted-but-not-enforced cases (#108 audit list).

Closes #109.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp`: +13 lines / -6 lines (comment rewrite + check).
- `tasks/session73q-issue109-fix.md`: this writeup.
