# memmove → LDDR lowering: three folds, and why inline can't beat a hand-written helper (2026-07-08)

## Summary

Chased the RC702 screen-scroll idiom `memmove(base + K, base, C - i)` all the
way from "falls back to a runtime call" to "inline LDDR with constant end
pointers".  Landed **three** correct, general compiler improvements on
llvm-z80 main.  Then established — with byte-level measurement — that even
with all three, the compiler-generated inline LDDR **cannot beat a
hand-written shared `lddr_copy` helper** for a *multi-site* scroll, and
explained why (a four-way mismatch between the `memmove` intrinsic and the
`LDDR` instruction, compounded by Z80 register scarcity).  Decision: rcbios
keeps `lddr_copy`; the compiler wins ship because they help single-site and
other constant-address memmoves.

## The three folds (all merged to llvm-z80 main)

### 1. Runtime-base direction fold — `[Z80] memmove direction fold: recognize runtime-base src/dst`

`G_MEMMOVE`'s direction analysis (`Z80LegalizerInfo.cpp`, custom
`legalizeCustom` G_MEMMOVE case) folds an overlapping copy to inline
`LDDR`/`LDIR` when it can prove the copy direction.  Case 1 ("`DstPtr =
SrcPtr + DstOff`") was guarded by `SrcBase == Register()`, meant to say
"SrcPtr is a leaf".  But the `getPtrAddOff()` helper sets `Base` as a side
effect even when `SrcPtr`'s offset is non-constant, so when `SrcPtr` is
itself a `G_PTR_ADD` with a **runtime** offset (`screen + cury`), `SrcBase`
got polluted and the guard spuriously failed → fall back to `__memmove_rt`.

Fix: drop the `SrcBase == Register()` guard on case 1 and the symmetric
`DstBase == Register()` on case 2.  `dst = SrcPtr + const` is a valid
direction relation regardless of `SrcPtr`'s internal structure.  Only the
**relative** "same base + positive constant delta" fact is needed (explicit
in the IR as `%dst = G_PTR_ADD(%src, K)`), NOT absolute address ordering
(overflow-unsafe and unnecessary).

### 2. End-pointer runtime-term cancellation — `[Z80] memmove LDDR: cancel a common runtime term in the end pointer`

LDDR needs *end* pointers: `end = start + Size - 1`.  For `base = p + i;
memmove(base + K, base, C - i)`, `end = (p + i) + (C - i) - 1 = p + C - 1` —
the runtime `i` cancels between the pointer's `+i` and Size's `-i`, leaving a
**constant** offset.  The runtime-`Size` branch built `PtrAdd(start, Size-1)`
with a runtime `add hl,bc`, missing this.

Fix: detect `Size = G_SUB(ConstC, X)` and a matching single `+X` in the
pointer's `G_PTR_ADD` chain; the end pointer then folds to `base + const`.
`X + (C - X) = C` is exact in 16-bit modular arithmetic, so valid regardless
of overflow.

### 3. Constant-address base fold — `[Z80] memmove LDDR: fold constant-address base + const offset into one immediate`

After cancellation the end pointer is `G_PTR_ADD(base, const)`.  When `base`
is a **constant-address** pointer `G_INTTOPTR(G_CONSTANT C1)` — e.g. the fixed
MMIO display base `(byte*)0xF800` — that selected as `LD rr, C1; ADD rr, off`
(7 B) even though both are constants.  (A `G_GLOBAL_VALUE` base already folds
via the ISel `G_PTR_ADD(GV, const)` pattern; only the constant-address case
was missing.  The frontend folds `(byte*)0xF800 + 1919` when both are
literals, but the cancellation introduces the offset in the backend, past the
frontend folder.)

Fix: `finishEndPtr()` folds `G_PTR_ADD(G_INTTOPTR(C1), Total) →
G_INTTOPTR(C1 + Total)`, emitting one `LD HL, 0xFF7F`.  Shared by the
constant-`Size` `buildEndPtr` and the cancellation `tryCancelEnd` paths.

Result on the RC702 scroll: `ld hl,$ff7f; ld de,$ffcf; lddr` — byte-identical
end-pointer materialization to what the hand-written `lddr_copy` caller emits.

All three: lit `memmove-inline.ll` (+cancellation, +negative control,
+constant-address-base) 182 pass + 5 XFAIL; runtime fixtures
`test_memmove_runtime_base.c` / `test_memmove_cancel_term.c` O1-Oz.

## Why inline STILL can't beat the hand-written `lddr_copy` (the core lesson)

Measured on rcbios (56 K BIOS), 2-site screen scroll (`insert_line`):

| variant | size |
|---|---|
| hand-written `lddr_copy` | 5908 B |
| `__builtin_memmove`, all three folds | 5947 B (**+39**) |

Even a hypothetical guard-elision (−8 B) leaves +31.  The gap is structural:
`LDDR` is a **rigid** instruction and `memmove` is a **flexible** contract;
bridging them costs glue, paid **per site** when inlined.

### The four mismatches

1. **Interface: start vs end pointers.**  `memmove(dst, src, n)` gives start
   pointers; LDDR wants `start + n - 1`.  The compiler must synthesize end
   pointers (fixed by folds #2/#3 for the cancellation shape; a runtime add
   otherwise).  `lddr_copy`'s caller passes end pointers directly, which the
   C source wrote as `screen + ROW24_OFFSET - 1` — constant-folded by the
   **frontend** before the backend ever sees it.

2. **Safety: size may be 0.**  `memmove(_, _, 0)` is a legal no-op; `LDDR`
   with `BC = 0` decrements to `0xFFFF` and copies 65536 bytes.  For a
   **runtime** count the compiler MUST emit a guard (ravn/llvm-z80#255) (`LDDR_GUARDED`:
   `ld a,b; or c; jr z`, +4 B/site).  It only drops the guard for a
   compile-time-**constant** Size; it does not propagate runtime
   known-non-zero facts — `if (count)` and even `__builtin_assume(count != 0)`
   do NOT reach the LDDR-vs-LDDR_GUARDED choice (verified empirically).
   `lddr_copy` has the `if(count)` guard **externally** in C, an invariant the
   compiler can't see, so its helper never re-checks.

3. **Register pinning + Z80 scarcity.**  Z80 has only 3 GP 16-bit pairs
   (HL, DE, BC); LDDR pins **all three** at once → **zero** scratch during
   setup.  The three live values (src-end, dst-end, count) must land in
   exactly HL/DE/BC with no spare pair for intermediates, forcing
   `push`/`pop`/`ex de,hl` shuffling.  (Folds #2/#3 largely eliminate this
   *here* by making the end pointers loadable constants.)

4. **No amortization — the dominant remaining cost.**  LDDR is 2 B, but its
   setup (materialize 3 registers + guard) is ~10-12 B.  Inlining pays the
   setup at **every** site.  A shared helper pays guard + count-pop + LDDR +
   return **once**; callers only load the (constant) end pointers + push the
   count.  For 2+ sites of the same shape, the shared helper wins by
   construction.

### Why the hand-written helper wins

`lddr_copy` hard-codes the answer to all four: constant end pointers (interface
bridged at compile time), external `if` (no guard), direct register loads (no
shuffle), and one shared body (amortized).  The compiler must handle the
**general** contract (arbitrary pointers, size maybe 0, arbitrary context)
and insert glue **per site**.  This is the fundamental compiler-vs-hand-tuned
tradeoff: a compiler translates a general, safe contract into a specific,
unsafe instruction, and the bridge costs — especially repeated per call site.

## Decision

rcbios keeps `lddr_copy` (size-optimal for the 2-site scroll).  The three
folds ship on llvm-z80 main as general wins (single-site / other
constant-address memmoves benefit).  Not a compiler defect — a measured
architectural tradeoff.

## Two incidental findings

- **Stale-binary trap (hours lost).**  After the folds I rebuilt `clang llc`
  but NOT `lld`.  rcbios uses `-flto` and links via `ld.lld`; the stale
  `ld.lld` ran the OLD legalizer, emitting `__memmove_rt`.  I diagnosed a
  phantom "LTO backend differs from llc" discrepancy (captured post-LTO IR,
  MIR-before-legalizer — identical!, tested opt levels) before realizing the
  cause was my own un-rebuilt `ld.lld`.  `LLVMZ80CodeGen` is embedded in
  clang + llc + **lld**; LTO/PROM go through `ld.lld`.  Rule saved:
  workspace memory `feedback_rebuild_all_z80_tools` — after any
  `llvm/lib/Target/Z80/` edit, `ninja -C build-macos clang llc lld`.

- **Pre-existing O0 frame miscompile (#192 class).**  The runtime fixture
  surfaced a plain-`-O0` bug: `main`'s fixed frame is allocated too small, so
  a pointer spill to `__sfrend_main-12` underflows BELOW `__sframe_main` into
  the adjacent global buffer (confirmed by symbol addresses:
  `__sfrend_main-12` landed inside `bufb`), corrupting it.  Independent of the
  memmove folds (O0 does zero folds).  Fixtures `SKIP-IF: O0` with this
  rationale; unfixed.
