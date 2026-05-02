# TODO: Audit closed compiler issues for source-side cleanup opportunities

User flag (2026-05-02, late session 33): for each closed issue in
ravn/llvm-z80, check whether the rcbios / cpnos-rom / autoload-in-c
sources still carry workarounds (inline asm, manual register hints,
unrolled-by-hand idioms, structural contortions) that the now-fixed
compiler would handle.  If yes, revert the workaround to plain C --
the source becomes clearer, more generic, and ready to benefit from
future improvements in the same area.

Companion to `tasks/peephole-vs-root-cause.md` (audit peepholes that
mask earlier-pass bugs).  The two are inverse: peepholes audit looks
upstream from generated code; this audit looks downstream from
closed issues into our hand-written sources.

## How to do it

Walk `gh issue list --repo ravn/llvm-z80 --state closed --limit 200`
and for each closed issue, ask:

1. **Did we change source code to work around the bug?**  Look in:
   - `rc700-gensmedet/rcbios-in-c/clang/` and the .c sources
   - `rc700-gensmedet/cpnos-rom/` (resident.c, transport_pio.c, etc.)
   - `rc700-gensmedet/autoload-in-c/`
   - any `__asm__` block, any `register` keyword, any pragma, any
     "// XXX clang gen suboptimal" comment.
2. **Is the workaround still needed?**  Repro the original bug shape
   on the current backend.  If clean, the workaround can go.
3. **Would removing the workaround make the source clearer or more
   portable across compilers (clang AND z88dk SDCC)?**  Per the user's
   `feedback_dual_compiler_test.md` rule, the rcbios sources must
   still build on both — so cleanup is OK only if both compilers
   accept the cleaner form.

## Likely candidates (first pass — needs verification)

These closed issues in particular touched source-side patterns:

- **#82 BSS-spill orphan-reload** (closed session 32, refined session 33):
  rcbios may have `register` annotations or local pointer aliasing
  hacks added when this bug was hot.  Check if any survived.
- **#76 LD A,(HL); LD r,A → LD r,(HL)**: sources may have explicitly
  routed byte-loads through A or via temporaries.  Plain `r = *p`
  should now compile clean.
- **#88 Z80LoopIdiomFill** (K-byte constant-trip pattern fills): any
  hand-rolled "store + LDIR copy from base" in C source could revert
  to a plain `for (i=0;i<N;i++) buf[i]=v;` shape.
- **#74 BSS spill→PUSH/POP** (closed session 33): rcbios functions
  with `register` annotations to avoid spills, or split-into-helpers
  to reduce live-range overlap, can probably be re-merged.
- **#86 u8 switch range-check 16→8 bit**: any switch where the cases
  were widened to int to keep clang from emitting the 16-bit chain
  could go back to plain `switch (u8_val)`.
- **#64 memmove inline (LDIR/LDDR)**: any `memcpy` vs `memmove` case
  differentiation in source -- may be unnecessary now.
- **#45 Direct addressing for constant-address loads/stores**: any
  hand-cast `*(volatile T*)0xNNNN = v;` that was wrapped in helper
  inline asm is now redundant.
- **#46 Ptrtoint(GV+const) fold**: similar.
- **#71 SRL A → RRCA on AND-mask**: any inline asm that did this by
  hand can revert.
- **#62 Dead HL copy**: if any hand-coded "extract two bytes, then
  re-assemble" was structured to avoid the dead copy, may be
  un-needed.
- **#58 JP→JR**: irrelevant to source (link-time).
- **#60 Cross-block known-A**: any "force-zero-A here" patterns in
  source could go.

## Bar for action

Convert a workaround back to plain C ONLY when:
- The original bug is verified resolved on current backend (write a
  small repro in `/tmp/check_$N.c`, compile, inspect asm).
- The cleaner C compiles to equal-or-smaller code in BOTH clang and
  zsdcc (or the source is clang-only, like cpnos-rom).
- The cleaner C has no functional regression (the rcbios test set
  should still pass).

Don't remove a workaround that:
- Has a comment explaining "this MUST stay because of <constraint
  not visible from the bug fix>" (e.g. relocate_bios's BSS-clear
  ordering).
- Touches an interrupt service routine or other safety-critical
  path -- those need MAME validation, not just code review.

## Output

When closing this TODO: produce a list "in source X, line Y was a
workaround for closed issue #Z; verified resolved; clean form is
{...}; size delta {...}; tested on {clang|both}".  Then a follow-up
PR (or set of commits) lands the cleanups.

Estimated yield: a few B more on rcbios (each cleanup tends to
shave 2-8 B), but the bigger payoff is source readability and
ease of future maintenance.

## Audit log

### 2026-05-02 (post session-33 merge)

- **cpnos-rom/init.c:119-125** (IVT setup loop, pointer-walk + uint8_t
  countdown): A/B'd against `for (uint8_t i = 0; i < N; i++) ivt[i]=…`
  and `for (int i = …)` at -Oz with current backend.  Current form
  wins by 7 B (no BSS spill); idiomatic up-counter forms incur a 2-B
  BSS slot + push hl/pop hl in the inner loop.  **Keep.**
- **cpnos-rom/init.c:144-150** (port_init dispatch, same shape):
  same A/B, current form wins by ~6 B.  **Keep.**

Root cause for both: IR-level countdown→count-up IV rewrite at -Oz
(#95) prevents DJNZ for either form.  Until #95 fixed, the hand-
written countdown idiom is the path to smallest code at these sites.
