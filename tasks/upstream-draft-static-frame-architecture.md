# DRAFT — upstream architectural-decision request (llvm-z80/llvm-z80)

DRAFT, not filed. Route: llvm-z80/llvm-z80. No maintainer name. No PR. Go-ahead required.

---

## Title
[Z80] Static-frame eligibility for freestanding firmware — architectural direction request

## Body

The static-frame machinery (`Z80NonReentrant` + `usesStaticFrame`) introduced in commits `59d8fad4` / `f17c7b76` is provably sound for separate compilation of general libraries. However, on freestanding whole-program firmware, it causes a significant density regression where almost all functions lose static-frame eligibility and spill heavily to the stack. Before submitting any changes, we would appreciate upstream's guidance on the intended architectural direction.

**Observed impact:**
In freestanding firmware (RC700 boot PROM and BIOS), compiled code size increased by ~50-60% (e.g., PROM text grew from 2046 B to 3223 B, exceeding the 2 KB physical EPROM capacity) due to stack-pointer offsets (`add hl, sp` and local spills replacing direct `.bss` access).

**Root cause:**
The synthetic edge `CallsExternalNode -> ExternalCallingNode` added in `Z80NonReentrant::runOnModule` correctly models that calls leaving the translation unit might re-enter any externally visible function. However, in embedded firmware:
- An interrupt service routine (ISR) frequently calls external leaf helpers (e.g., compiler-rt builtins like `memcpy` or port-I/O helpers).
- This creates the reachability path: `ISR -> external helper -> CallsExternalNode -> ExternalCallingNode -> {all global functions in module}`.
- The multi-context reachability walk consequently marks nearly every function in the module as reachable from both main-line code and the ISR context, stripping the `nonreentrant` attribute.

**Why a per-module heuristic is insufficient:**
We investigated whether `CallsExternalNode` in the context walk could be restricted to only locally address-taken functions. While that recovered static frames in firmware, the lit test `static-frames-indirect-isr.ll` correctly rejected it: under separate compilation, a function with external linkage could have its address taken in another translation unit and be called indirectly from an ISR. Because any sound per-module reachability must conservatively treat `{address-taken} ∪ {external-linkage}` as reachable from `ExternalCallingNode`, this cannot be resolved soundly by adjusting per-module call-graph heuristics.

**Architectural options for upstream consideration:**

1. **Closed-world / freestanding mode flag:** A driver or backend option (e.g., `-mllvm -z80-closed-world` or `-ffreestanding` integration) asserting that no external caller invokes module functions unless they are explicitly address-taken or marked as entry points.
2. **Explicit nonreentrant assertion attribute:** Symmetric to `__attribute__((target("no-static-frame")))`, provide a positive attribute (such as `__attribute__((target("static-frame")))` or `__attribute__((nonreentrant))`) allowing developers to explicitly guarantee non-reentrancy on critical functions where static-frame allocation is known to be safe.
3. **LTO and internalization convention:** If upstream prefers strictly relying on LTO, document and support the expected LTO pipeline where all non-entry symbols are internalized so `ExternalCallingNode` naturally leaves them unreached.

Reproducers, lit test cases, and concrete code-size measurements are available. 
---
Internal: tracking issue ravn/llvm-z80#316; technical analysis in `tasks/static-frame-316-options-explained.md`.
