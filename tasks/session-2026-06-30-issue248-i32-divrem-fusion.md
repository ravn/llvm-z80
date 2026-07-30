# Session 2026-06-30 — i32 divrem fusion (ravn/llvm-z80#248, closes B19)

## Problem

compiler-comparison-corpus `pi` (uint32_t spigot) was **+39 % slower than
dcc** (clang 58.75 M vs dcc 42.2 M t-states) despite being 2.6× smaller.
An adjacent `x / y` and `x % y` on identical i32 operands lowered to **two
separate runtime calls** (`__udivsi3` + `__umodsi3`), each re-running the full
32-iteration `__udiv32_core`, so the most expensive operation ran twice where
one pass produces both quotient and remainder.

## Mechanism (why it happened)

`Z80TTIImpl::hasDivRemOp` returns `true` for all widths, so `DivRemPairs` hoists
the pair and the pre-legalizer combiner (`all_combines`; pre-legalize
short-circuits the legality gate in `CombinerHelper.cpp:1597`) fuses it into one
i32 `G_UDIVREM`. The legalizer then **un-fused** it: `Z80LegalizerInfo`'s
`{G_UDIVREM, G_SDIVREM}` rule was `customFor({S16})` — only i16 was custom; i32
fell through to `.lower()` → separate `G_UDIV` + `G_UREM` → two libcalls. (The
original #248/B19 text mis-stated `hasDivRemOp(i32)` as false; corrected.)

## Fix

Lower i32 (and i64-not-yet) fused divrem through the conventional compiler-rt
ABI: **quotient returned in registers, remainder written through a caller
pointer** (the existing CallLowering stack-arg path already drives the i32
divisor, and the runtime core already leaves quotient in HLDE + remainder in the
shadow set).

- **Runtime** (`compiler-rt/lib/builtins/z80/`):
  - `divmodsi3.asm`: new `___udivmodsi4` — one `__udiv32_core` pass; quotient in
    HLDE (return), remainder stored through the `IX+8..9` pointer arg.
  - `divmodsi4.asm` (new, **own object**): `___divmodsi4` — signed counterpart,
    sign rules via the existing `__neg32_hlde`/`__neg32_divisor` helpers. Kept in
    a separate object so unsigned-only code (which links `__udivmodsi4` + the
    core from `divmodsi3.o`) does not drag the signed routine in via
    whole-object linking — that bundling cost pi +74 B before the split.
- **Legalizer** (`Z80LegalizerInfo.cpp`): `{G_UDIVREM, G_SDIVREM}` →
  `customFor({S16, S32})`; `legalizeCustom` routes i32 to a `createLibcall` of
  `__udivmodsi4`/`__divmodsi4` with a 4-byte stack slot for the remainder, then
  `G_LOAD`s the remainder back. i16 still selected directly in ISel.

## Side effect: B15 cleanup

`branch-folder-unsound-hoist-pi-cse-miscompile.ll` (the pi CSE miscompile,
known-suboptimal **B15**) went XFAIL→XPASS: the fused lowering loads the
remainder from a stack slot instead of returning it in `$de` from `__umodsi3`,
which removes the "two consecutive DE stores" MIR shape Branch Folder unsoundly
hoisted. The predicted "Z80-specific mitigation breaks the trigger shape" cleanup
signal fired — XFAIL removed, test kept as a regression guard. B15's underlying
Branch Folder unsoundness is mitigated-for-this-shape, not root-fixed.

## Results

| | before | after |
|---|---|---|
| pi t-states | 58.75 M | **38.06 M (−35 %)** — now faster than dcc (42.2 M) |
| pi size (clang -Oz) | 880 B | 888 B (+8 B; the +74 B signed-bundle avoided by the object split) |
| Lit suite | 179 PASS + 6 XFAIL | **180 PASS + 5 XFAIL** (B15 cleaned up) |
| Runtime suite | — | **890 PASS, 0 FAIL** (new `test_248_i32_divmod_fused`, all opt levels) |
| Production (autoload/cpnos/BIOS) | — | **0 B impact** (no 32-bit divide linked) |

Proofs: lit `llvm/test/CodeGen/Z80/divmod-i32-fused.ll` (unsigned + signed pin
`call ___[u]divmodsi4`, `CHECK-NOT` the split calls); runtime fixture
`z80-utils/test-runner/testcases/clang/test_248_i32_divmod_fused.c`.

## Follow-ups (not done)

- i64 fused divrem (`__udivmoddi4`/`__divmoddi4`) — i64 stays on `.lower()`.
- B15's Branch Folder unsoundness remains latent (only this witness's trigger is
  broken). Revisit if another shape resurrects it.
