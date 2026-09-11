# DRAFT — upstream architectural-decision request (llvm-z80/llvm-z80)

DRAFT, not filed. Route: llvm-z80/llvm-z80. No maintainer name. No PR. Go-ahead required.

---

## Title
[Z80] Static-frame eligibility for freestanding firmware — architectural decision needed

## Body

The static-frame machinery (`Z80NonReentrant` + `usesStaticFrame`) is sound for
separate compilation, but on freestanding whole-program firmware it costs most of the
static-frame density, and there is no sound per-module fix — so I'd like your call on
the intended direction before implementing anything.

**Impact (RC700):** autoload code +57% (2046 → 3223 B); rcbios `.bss` now overflows its
region and won't link.

**Why:** the synthetic `CallsExternalNode -> ExternalCallingNode` edge means an ISR that
calls *any* named external (a runtime leaf helper like `memcpy`) is treated as reaching
every externally-callable function. Nearly every global is then "reachable from two
contexts" and loses its static frame — including functions the ISR never calls.

**Key finding:** tightening the context walk to only *locally* address-taken functions
recovers the density but is unsound — `static-frames-indirect-isr.ll` correctly rejects
it (an external-linkage function may be address-taken in another TU). Any *sound*
per-module reach is `{address-taken} ∪ {external-linkage}` = today's behavior. So
recovery needs a whole-program / closed-world signal or an explicit programmer
assertion, not an analysis tweak.

**Decision — which direction?**
- a whole-program / closed-world signal (drop the external edge when nothing is
  actually externally reachable);
- a per-function non-reentrancy assertion, symmetric to the `no-static-frame` opt-out;
- rely on LTO + internalization end-to-end;
- accept the cost.

Reproducers and per-function measurements ready; happy to implement whichever you pick.

---
Internal: our tracking issue ravn/llvm-z80#316; full analysis in
`tasks/static-frame-316-options-explained.md`. Confirm before filing: I file or you?
new issue vs comment on an existing thread?
