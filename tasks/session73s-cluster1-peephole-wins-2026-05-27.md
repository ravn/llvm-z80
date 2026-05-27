# Session 73s (cont.) — Cluster 1 small-peephole wins (2026-05-27)

Continuation of the issue-closeout push (`tasks/issue-closeout-plan-2026-05-27.md`).
Directive: "start cluster 1 fresh — small peephole wins."  Method: verify-first
per issue, then land the genuinely-clean fixes behind the full value oracle.

## Result: 3 issues closed, 1 reclassified, 1 follow-up filed

| # | outcome | detail |
|---|---------|--------|
| **#18** | **FIXED** (main `bfce0fff25db`) | new peephole `LD r,n` -> `LD r,A` when A holds the constant |
| **#151** | CLOSED (verify-only) | already fixed — clean `sub 1; sbc a,a`, no `and 1; rrca` residual |
| **#152** | CLOSED (verify-only) | already implemented + lit-tested — SET/RES through A-readers via `LD A,(HL)` |
| **#146** | RECLASSIFIED | `ex (sp),hl` clobbers the live HL return value; narrow + zero production impact |
| **#206** | FILED | #18 follow-up: extend known-constant copy to non-A registers (low priority) |

## #18 — the fix

New per-MBB peephole in `Z80LateOptimization.cpp`.  After `xor a` / `sub a`
(A=0) or `ld a, n`, an immediate load of the SAME constant into another 8-bit
register is rewritten from `ld r, n` (2 B) to `ld r, a` (1 B).  A is only READ,
so its value and the flags survive and the tracked constant stays valid across
consecutive fires:

```
xor a; ld l,0; ld h,0   ->   xor a; ld l,a; ld h,a      (-2 B)
ld a,5; ld l,5          ->   ld a,5; ld l,a             (-1 B)
```

Safety: tracking is strictly within one basic block (reset at entry).  Any def
of A invalidates the known value — including a CALL's RegMask clobber, which has
no explicit A def operand (checked via `clobbersPhysReg` + `regsOverlap`, not
just operand scan).

### Value oracle (all green)
- lit `issue-18-ld-r-a-known-const.ll` (zero + non-zero const); Z80 suite **122+5**.
- test-runner differential oracles `-diff-opt` + `-native-oracle`: **0 DIFFOPT /
  0 NATIVE** in BOTH default and `+static-stack` configs.
- production: cpnos PROM1 **2028 -> 2026 B** (-2 B, 22 B free under the 2 KB cap);
  cpnos-polypascal-test **PASS 51.17 s** (clears the #150 boot-regression precedent).

## Negative / ruled-out findings (analysis)

- **No double `and 1`.**  An earlier note suspected a redundant `and 1; and 1`
  in the bool-result path; re-scan of eq16/ne16/eq8/_Bool/chained shows each
  comparison ends with exactly the needed `sub 1; sbc a,a; and 1` (EQ) or
  `sbc a,a; and 1` (NE) to produce an exact 0/1.  Correct, not redundant.
- **#122 `and 254` is NOT a miscompile.**  For `(v & 0xFF) < 10` clang folds the
  mask to `and 254` — value-preserving demanded-bits folding (the even boundary
  makes bit 0 irrelevant).  At threshold 11 the mask is dropped entirely.  The
  differential oracle would catch a real miscompile here; it is clean.
- **#146 is not a small win.**  The `pop bc; inc sp; inc sp; push bc; ret` ->
  `pop hl; ex (sp),hl; ret` rewrite clobbers HL, which is LIVE (the return value)
  at the epilog of value-returning functions — that is precisely why codegen
  emits `pop bc` and not HL there.  Safe only for void / non-HL-return + one
  stack word + HL-dead, needs a liveness guard, 1 B, and ZERO production impact
  (`+static-stack` functions take no stack args).

## Cluster-1 remainder (not clean small wins)

- **#117** i16 EQ/NE neither-in-HL: LIVE, ~1 B, risky `emitFusedCompareAndBranch` area.
- **#122** 8-bit `CP n` fast path: LIVE but marginal — the 8-bit load already
  happens; only the 16-bit subtract tail is suboptimal.  Same risky area.
- **#173** 8-bit BSS spill via A: the only remainder with real (AES) value, but
  MEDIUM and the obvious repro produced the already-optimal `push af; call; pop af`
  rather than the `push af; ld a,r; ld (nn),a; pop af` shape it targets.  Needs a
  fresh repro drill before fixing — promote to a focused session.

## Standing-rule adherence
Branch-first (on main), `--no-ff` merge, branch deleted, not pushed.  Codegen
commit carries `Rules-checked:`.  Issues filed in ravn fork only.  Yield claims
stated as unverified where not measured.
