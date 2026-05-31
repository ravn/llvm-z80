# Upstream-submission writeup — TruncInstCombine: narrow expression graphs through function Arguments (#158)

Status: fix landed in ravn/llvm-z80 main (`a5d49e9378ba`, 2026-05-14; merged
`fa5dcef670d4`).  This doc is the target-agnostic submission text for
llvm/llvm-project.  Per project policy the fix is NOT filed/PR'd upstream by the
agent — a human submits it.

## One-line

`AggressiveInstCombine`'s `TruncInstCombine` refuses to narrow a
`trunc(iN expr)` graph whenever the expression reaches a function `Argument`,
because the DAG walk bails at the first non-`Instruction`, non-`Constant`
operand — leaving the whole expression at the wide type even though it is
provably narrowable.

## Where the bug is

`llvm/lib/Transforms/AggressiveInstCombine/TruncInstCombine.cpp`,
`buildTruncExpressionGraph`:

```cpp
    auto *I = dyn_cast<Instruction>(Curr);
    if (!I)
      return false;   // <-- gives up the moment the graph reaches an Argument
```

The walker treats only `Instruction` and `Constant` nodes as legal interior /
leaf nodes.  A function `Argument` is neither, so any expression whose operand
chain bottoms out at a parameter can never be narrowed.

## Why it matters

The canonical trigger is a small-integer parameter that C int-promotes at the
ABI boundary.  On a target where `int` is 16 bits (e.g. the Z80 backend), a
K&R-style `uint8_t` parameter:

```c
uint8_t rotl(x) uint8_t x;          /* default arg promotion -> int (i16) */
{ return (x << 1) | (x >> 7); }
```

lowers to an i16 expression masked back to 8 bits:

```llvm
%m = and  i16 %arg, 255
%s = shl  i16 %m, 1
%r = lshr i16 %m, 7
%o = or   i16 %s, %r
%t = trunc i16 %o to i8
ret i8 %t
```

`TruncInstCombine` should narrow this to i8 ops so the rest of the mid-end (and
the backend) can recognise the native 8-bit rotate.  The Argument bailout
prevents it, so the target emits a full 16-bit shift/mask/or sequence
(`add hl, hl` + masks on Z80) — measured ~5× the bytes of the native form
(`rj_sb_inv` from a real AES-256 codebase: 147 B vs the ANSI-prototype 16 B).
The same shape appears in any legacy K&R C compiled for a 16-bit-int target.

## The fix

Accept an `Argument` as a narrowable leaf in the two walkers
(`buildTruncExpressionGraph`, `getMinBitWidth`) and, when materialising the
narrowed expression in `getReducedOperand`, emit one explicit `trunc` of the
argument at the function entry so it dominates every narrowed use:

```cpp
// buildTruncExpressionGraph / getMinBitWidth: Arguments impose no width
// requirement on themselves and have no operands — treat them as leaves.
if (isa<Argument>(Curr)) {
  Worklist.pop_back();
  continue;
}

// getReducedOperand: narrow an Argument leaf with an explicit entry trunc.
if (auto *Arg = dyn_cast<Argument>(V)) {
  Function *F = Arg->getParent();
  IRBuilder<> Builder(&*F->getEntryBlock().getFirstInsertionPt());
  return Builder.CreateTrunc(V, Ty);
}
```

## Why it is correct

The Argument's narrowed value is exactly `trunc(arg)` — the same value the wide
expression would compute after its own (now removed) masking — and the
entry-block insertion point strictly dominates every use of the narrowed
expression in the function.  The existing min-bit-width analysis is unchanged:
an Argument simply contributes no lower bound, so narrowing only happens when
the *instructions* in the graph already permit the smaller width (the
pre-existing safety condition).  No new value is observed at a wider-than-legal
width; the transform is the natural extension of the existing leaf handling
(`Constant`) to the one other operand kind a finite expression graph can bottom
out at.

## Test

Target-agnostic regression test (the actual fix):
`llvm/test/Transforms/AggressiveInstCombine/narrow-through-argument.ll`
(`opt -passes=aggressive-instcombine -S`): the i16 rotate-of-argument graph must
narrow to i8 ops fronted by a single `trunc i16 %arg to i8` at entry.  Pre-fix
the function is left entirely at i16 (the regression signal).

End-to-end backend witness (Z80): `clang/test/CodeGen/z80-knr-u8-rotate.c`
(`%clang_cc1 -triple z80 -Oz -S`) — the K&R u8 rotate emits `rlca`, no
`add hl` 16-bit dance.

## Risk

`TruncInstCombine` runs in the optimization pipeline (not codegen).  The change
is conservative — strictly more expressions become eligible, gated by the
unchanged width analysis — and was validated on the Z80 differential value
oracle (no divergences) plus production firmware (BIOS −16 B, cpnos/autoload
byte-identical) when it landed.

## Residual (separate issue, NOT part of this fix)

After narrowing, a *multi-rotate* K&R chain still emits `add a,a`/`srl a`+`or`
instead of `rlca`/`rrca` (the post-narrowing IR is not re-recognised as a rotate
idiom).  That is a distinct rotate-recognition gap (~+15 B on the 3-rotate
`rj_sb_inv`), tracked separately; it does not affect the correctness or the
single-rotate optimality this fix delivers.
