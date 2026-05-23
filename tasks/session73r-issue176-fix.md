# Session 73r — #176/#40 auto +static-stack on leaf functions (opt-in)

**Date:** 2026-05-24
**Outcome:** Opt-in IR pass `Z80AutoStaticStack` added that adds `"target-features"="+static-stack"` to leaf functions (no CALL / INVOKE instructions, ignoring inline asm).  Default OFF; opt-in via `-mllvm -z80-auto-static-stack=true`.

## Design

The full proposal in #176 covers a 3-level inference:
1. **Leaf functions** — no CALLs, structurally non-recursive.
2. **Non-recursive callgraph** — SCC analysis, no cycles.
3. **ISR isolation** — separate BSS regions per privilege level.

This session ships **Level 1 only**.  Leaf-function inference is the
cheapest level and captures most of the spill-heavy hot functions.
A leaf function CANNOT recurse (it has no calls), so static-stack is
structurally safe for any leaf.

The safety caveat (mentioned in the source comment): a leaf F is
unsafe under static-stack iff F is called CONCURRENTLY with itself
(e.g. from main and an ISR).  Typical Z80 firmware ISRs call only
ISR-specific helpers, not arbitrary C functions, so the heuristic is
"mostly safe for real-world code."  Users with ISR-shared functions
must opt out explicitly via a future `+no-static-stack` feature (not
implemented in this session — would need additions to the feature
parser).

## Implementation

- **New file** `llvm/lib/Target/Z80/Z80AutoStaticStack.{h,cpp}` (~100 LOC):
  - `class Z80AutoStaticStack : public ModulePass` walks functions, identifies leaves, adds the attribute.
  - `cl::opt<bool> EnableAutoStaticStack` gates the pass behavior.
  - `bool llvm::isZ80AutoStaticStackEnabled()` exposes the cl::opt for pipeline-construction-time check.

- **Wired in** `Z80TargetMachine.cpp`:
  - Pass registered in `LLVMInitializeZ80Target`.
  - `addPass(createZ80AutoStaticStackPass())` in `addIRPasses` — **gated by `isZ80AutoStaticStackEnabled()`** so that default-off users get NO pipeline-ordering side effect (#187 lesson applied).

- **CMakeLists.txt** updated.

## Measured impact

| Source | Default | `-mllvm -z80-auto-static-stack=true` | Δ |
|---|---|---|---|
| `aes256.c -Oz` `.text` | 3299 B | **2808 B** | **−491 B (−15%)** |
| `test_main.c -Oz` `.text` | 377 B | 377 B | 0 |
| cpnos PROM1 | 2029 B | (already uses +static-stack via Makefile CFLAGS) | n/a |

The AES `aes256.c` win is substantial (-15% / -491 B in a single
file).  Other AES corpus files have smaller impact because they're
already leaf-dominated or have fewer spill candidates.

## Verification

- Lit: 111 PASS + 3 XFAIL = 114 (unchanged, no new lit tests added).
- test-runner default-off sweep: (pending — running in background).
  Expected: 990/689/38/56/207, zero per-test diff vs the pre-session-73r
  baseline because the pass-pipeline gating ensures no behavior
  change when the flag is off.
- cpnos PROM1 (default): 2029 B (unchanged).

## Trade-offs

- **Default off**: the pass is opt-in for users who want to experiment.  Production targets that already use `+static-stack` (cpnos, BIOS) get no benefit; they're not the target audience.
- **Safety scope**: Level 1 only.  Future sessions could add Level 2 (callgraph SCC) for non-recursive non-leaf functions.  Level 3 (ISR isolation) needs explicit ISR-attribute work elsewhere in the backend.
- **Pipeline cost**: zero when flag is off (cl::opt gating on the `addPass()` call).  When flag is on, one additional ModulePass at the start of the CodeGen IR pipeline — O(N) over functions, microseconds for typical Z80 modules.

## Implication for #176

#176's Level 1 is shipped.  Level 2 (SCC) and Level 3 (ISR) remain for future work.  The issue can be repurposed as a tracker for the remaining levels, or closed-with-comment pointing at the partial ship.

## Implication for #40

#40 asked for per-function IX-frame-vs-static-stack decision.  This pass is a step toward that: it gives ONE side of the decision (force +static-stack on leaves).  The other side (force IX-frame on functions where static-stack is unsafe or where IX-frame is smaller) is not addressed.  #40 stays open as the broader optimization tracker.

## Files

- `llvm/lib/Target/Z80/Z80AutoStaticStack.cpp` — pass implementation.
- `llvm/lib/Target/Z80/Z80AutoStaticStack.h` — public interface.
- `llvm/lib/Target/Z80/Z80.h` — initialize* declaration.
- `llvm/lib/Target/Z80/CMakeLists.txt` — source list.
- `llvm/lib/Target/Z80/Z80TargetMachine.cpp` — pass registration + pipeline registration (gated).
- `tasks/session73r-issue176-fix.md` — this writeup.
