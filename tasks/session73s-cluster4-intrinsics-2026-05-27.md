# Session 73s (cont.) — Cluster 4: compiler intrinsics + attributes (2026-05-27)

Directive: "all, keep going" on Cluster 4 (#42, #4, #133).  The defining
requirement, given mid-stream: **the same rcbios source must compile under both
clang and SDCC with no `#ifdef`, and the intrinsic header must live in the
compiler, not the project.**

## Result: 3 issues closed

| # | outcome | detail |
|---|---------|--------|
| **#42** | **FIXED** (main `81b46fe`) | clang ships `<intrinsic.h>` + `__builtin_z80_*` privileged builtins |
| **#4** | **FIXED** (main `736f83f`) | `__attribute__((z80_critical))` -> DI/EI; rcbios `__critical` now real |
| **#133** | CLOSED (verify) | callee-side `z80_preserves_regs` save/restore already implemented + tested |

## #42 — the header lives in the compiler

Verify-first found the LLVM side was largely present (intrinsics `z80_di/ei/halt/
nop/in/out` defined + selected).  Missing: clang builtins to reach them, plus
`im2`/`set_i`.  Built:

- **llvm**: `int_z80_im2` / `int_z80_set_i` intrinsics + selection (IM 2 /
  move-to-A + LD I,A) + legalizer whitelist.
- **clang frontend**: `BuiltinsZ80.td` (`__builtin_z80_di/ei/halt/nop/im2/set_i`),
  tablegen wiring (CMakeLists + `TargetBuiltins.h` Z80 namespace),
  `Z80TargetInfo::getTargetBuiltins`, `EmitZ80BuiltinExpr` (TargetBuiltins/Z80.cpp),
  CGBuiltin dispatch.
- **resource header** `clang/lib/Headers/intrinsic.h` (installed to
  `lib/clang/<v>/include/`), mirroring z88dk's API (di/ei/halt/nop + im_2 z80-only).
  `set_i` (LD I,A) stays a clang-only builtin (z88dk has no such intrinsic).

End-to-end: `clang --target=z80 -S` with **no `-I`** emits `di; im 2; halt; ei`.
`#include <intrinsic.h>` resolves to clang's copy under clang and z88dk's under
SDCC -> same source, both toolchains, no ifdef.

**rcbios adoption**: `clang/intrinsic.h` now `#include_next <intrinsic.h>` (under
`__z80__`) instead of defining the intrinsics with inline asm.  BIOS **5922 ->
5897 B (-25 B)** — the intrinsics let the optimizer treat DI/EI as precise side
effects vs the opaque `__asm__ volatile` barrier.  MAME boot: signon + A> +
disk ERR=0 across 77 tracks.  SDCC BIOS 6091 B builds clean (same source).

## #4 — z80_critical

The backend's `Z80FrameLowering` already emitted DI(entry)/EI(before ret) for the
`"z80_critical"` IR fn attribute, but nothing set it (unreachable from C,
untested).  Added the clang attribute (Attr.td/AttrDocs/SemaDeclAttr/CGCall),
mirroring `z80_preserves_regs`.  On `__attribute__((interrupt))` functions the
entry DI is suppressed (HW already disabled).  rcbios `__critical` (a silent
`#define` no-op) now maps to it: BIOS 5897 B unchanged, di count 21 (no spurious
DI on the `__critical __interrupt` ISRs), MAME boot OK.

## #133 — verify-and-close

Layer 1 (callee-side save/restore) was **already implemented + tested**
(`Z80RegisterInfo::getCalleeSavedRegs` extends the CSR list with pair-coalescing;
`preserves-regs-callee.ll` proves push/pop fires).  With caller-side (#131) and
callee-side now agreeing, a body clobber is no longer a miscompile.  Closed on
substance; Part B (the violation *warning*) deferred — Layer 1 demoted it from a
miscompile guard to a marginal cost-advisory.

## Verification (all)
- lit: intrinsics.ll, z80-builtins.c, z80-intrinsic.c, z80-critical.c,
  critical-section.ll; Z80 suite **123+5**; clang z80 attribute tests 5/5.
- differential oracles 0 DIFFOPT / 0 NATIVE (default + static-stack) on the #42
  clang.  (#4 is a frontend attribute -> N/A for corpus programs.)
- cpnos PROM1 codegen-neutral: uncompressed payload byte-identical; the
  2026<->2027 B final wobble is the embedded cold-init build date+githash string
  (byte 585) compressing differently, not codegen.  polypascal PASS 51.87 s.
- rcbios BIOS (clang 5897 B + SDCC 6091 B) — same source, MAME boot to A>.

## Method notes
- verify-first kept paying off: #42 LLVM half present, #4 backend present, #133
  Layer 1 present.  The work was the missing clang frontend, not new codegen.
- chased a cpnos +1 B "regression" to an embedded build-timestamp (state-certainty),
  not codegen — the compiled payload was byte-identical throughout.
