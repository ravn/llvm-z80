# IX as an allocatable register — full measurement & why it doesn't pay (yet)

**Issue:** ravn/llvm-z80#12 ("hasFP=false correct but larger: IX CSR overhead
exceeds frame savings"), with ravn/llvm-z80#38 (un-reserve index registers).
**Date:** 2026-05-26 (session-ix).
**Outcome:** all IX-allocatable work **reverted**; IX stays reserved by default
(unchanged from before this session).  This document records *why*, so the
next person who looks at #12 does not re-run the dead ends.

> TL;DR — In the current (callee-saved-IX) ABI there is **no profitable way to
> use IX**: every configuration that makes IX allocatable is a size regression,
> and the greedy CSR cost model cannot even reach break-even.  The **only** path
> to an IX size win is making IX **caller-saved**, and that is correct **only
> after #12 is fixed** (frame-pointer elimination for stack-arg functions).
> Revisit this then.

---

## 0. What was tried, one line each

| Approach | cpnos PROM1 | BIOS | autoload | verdict |
|---|---|---|---|---|
| **IX reserved** (today's default) | 2027 | 5897 | 1657 | baseline |
| Un-reserve, callee-saved (size-opt+static-stack gate) | 2048 (**+21**) | 5988 (**+91**) | 1657 (0) | regression |
| Un-reserve + `getCSRCost()` cost model | — | 5915–5988 (**+18 floor**) | — | regression; can't reach neutral |
| **IX caller-saved** (drop from CSR, force hasFP=false) | 2019 (**−8**) | 5896 (**−1**) | 1657 (0) | **win — but breaks 526/1044 tests; needs #12** |

(All numbers are clean rebuilds — see §6 on the staleness trap.)

---

## 1. The question

IY was un-reserved this session (#38, size-gated) for a clean ~33 B production
win because IY is **caller-saved** (free to clobber; no prologue cost).  The
natural follow-up: do the same for IX.  IX is the other 16-bit index pair, so
"a 4th/5th allocatable pair" ought to help register-starved functions the same
way.

It does not, and the reason is structural.

## 2. Why IX is callee-saved (the root constraint)

From `Z80CallingConv.td`:
```
// - IX: Frame pointer (callee-saved)
def Z80_CSR : CalleeSavedRegs<(add IX)>;   // SDCC __sdcccall(1)
```

Two intertwined reasons:

1. **Frame-pointer role.**  A function that uses IX as its frame pointer
   (`push ix; ld ix,0; add ix,sp`) calls other functions and needs IX intact
   afterward.  The only way to guarantee that is to require *callees* to
   preserve IX → callee-saved.  Callee-saved is a *consequence* of the
   frame-pointer role.
2. **SDCC `__sdcccall(1)` ABI compatibility** for clang↔SDCC cross-linking
   (elf2rel/rel2elf).  Not active for the firmware (pure `--target=z80` ELF, no
   SDCC objects), but it is the documented contract.

Consequence: **any** function that lets the allocator place a value in IX must
emit `PUSH_IX`/`POP_IX` (4 B) to honor the contract — *even if no caller
actually relies on IX surviving a call*.  That is the tax IY never pays.

## 3. Measured fact: the firmware has ZERO frame-pointer functions

Disassembly of all three production images (count of `add ix,sp`):

| image | funcs | IX-as-frame-pointer | IX-as-GPR (push/pop ix, no FP) |
|---|---|---|---|
| autoload | 51 | **0** | 0 |
| BIOS | 96 | **0** | 8 (under blanket unreserve) |
| cpnos | 105 | **0** | 9 (under blanket unreserve) |

This is the pivotal finding.  **IX is already free to allocate in every
firmware function** — it is never pinned as a frame pointer.  So:

- The frame-pointer-elimination work (#12) is **moot for the firmware as it
  stands** — there are no IX frame pointers to eliminate.
- Yet using IX still costs the `PUSH_IX`/`POP_IX` tax, purely because IX is
  *designated* callee-saved.  **The tax protects a contract nothing in the
  firmware uses.**

## 4. Why blanket un-reserve (keeping callee-saved) regresses

With IX allocatable and callee-saved, greedy sees a 4th/5th pair and grabs it
for short scratch ranges, paying 4 B it cannot amortize.  BIOS used IX in 8
functions → **+91 B**.  cpnos 9 functions → **+10 B payload / +21 B PROM1**.

## 5. Why the `getCSRCost()` cost model cannot fix it

`RAGreedy::tryAssignCSRFirstTime` already implements the right idea — *spill or
split instead of using a callee-saved register when the avoided spill is cheaper
than `CSRCost`* — gated on `TRI->getCSRCost()` (default 0 → CSR is "free").
Overriding `getCSRCost()` (or sweeping the equivalent
`-regalloc-csr-first-time-cost=N`) does steer greedy away from IX, **but**:

- It is a **block-frequency** heuristic (cost compared against `BlockFrequency`,
  entry = 1<<14).  It cannot see "4 bytes."  Under `-Oz` the frequencies are
  near-uniform, so the behaviour is **bimodal**: IX used everywhere until a very
  large cost (~2e5–5e6), then abruptly not at all.  There is no middle value
  that keeps only the *size-profitable* IX uses, because the model has no notion
  of size profitability.
- **Worse:** even with IX suppressed to a single use (cost = 5e6), BIOS is still
  **+18 B** over the reserved baseline.  That residual is pure **availability
  perturbation** — merely adding IX to the allocatable set changes greedy's
  *other* allocation/splitting/eviction decisions, and that cost survives even
  when IX goes unused.  The only way back to 5897 B is keeping IX **reserved**.

So the cost-model lever's best achievable state is **+18 B**, i.e. strictly
worse than reserving IX.  It is the wrong tool for a size problem.

(Sweep evidence, BIOS, `-regalloc-csr-first-time-cost`: 0 → 5988 B / 57 ix;
2e5 → 5951 B / 23 ix; 5e6 → 5915 B / 1 ix.  Reserved baseline = 5897 B / 0 ix.)

## 6. The only winning path: IX caller-saved — and why it needs #12

Making IX **caller-saved** removes the tax entirely (no prologue PUSH/POP; the
allocator saves IX only where a value is genuinely live across a call, which it
handles via the call RegMask).  Measured: cpnos **−8 B**, BIOS **−1 B**,
autoload 0 — a real, correct win on the FP-free firmware, and `-verify-
machineinstrs`-clean, with cross-call liveness handled correctly (the allocator
parks cross-call values on the stack, not in IX).

**But it is not generally correct.**  The experimental patch made IX
caller-saved program-wide by:
1. returning `Z80_AllReg_CSR_SaveList` from `getCalleeSavedRegs` and
   `Z80_AllReg_CSR_RegMask` from `getCallPreservedMask` (IX → caller-saved), and
2. forcing `hasFPImpl` to return `false` (IX is never a frame pointer; stack
   args go SP-relative via `emitSPRelativeAddr`).

Run against the full clang `-static-stack` suite this **broke 526 / 1044 tests**
(vs 56 baseline fatals).  The dominant failure is a **link error**:

```
ld.lld: error: undefined symbol: __sfrend_add3
```

i.e. forcing `hasFP=false` for functions that *do* need a frame (stack
arguments, `alloca`) breaks the **static-stack frame-symbol emission**
(`__sframe_X` / `__sfrend_X`) — the SP-relative path is incomplete for those
functions.  **That incompleteness is exactly #12.**  The firmware survives only
because it has zero such functions (§3).

### So the #12 connection is precise:

> Caller-saved IX is the correct, winning design.  It is **blocked solely by the
> incomplete frame-pointer elimination** that #12 tracks: until a function with
> stack args / alloca can emit a valid static-stack frame **without** an IX
> frame pointer (fix the `__sfrend_X` emission under forced `hasFP=false`),
> caller-saved IX can only be a program-wide opt-in for guaranteed-FP-free
> programs.

When #12 is fixed, the path is:
1. Add the program-wide caller-saved-IX feature (drop IX from `Z80_CSR` +
   `getCallPreservedMask`; force `hasFP=false`).  ~30 lines; the reverted patch
   is in this session's history if useful.
2. The suite should then pass (the `__sfrend_X` link errors were the bulk of the
   526 failures; confirm with `cargo run -- clang -static-stack`).
3. Re-measure the firmware; expect the −8 / −1 wins (or better, once IX is used
   more freely without the tax).

## 7. Reproduction

```bash
# baseline (clean builds are mandatory — see staleness note below)
cd rc700-gensmedet/rcbios-in-c && make clean && make bios          # 5897 B
# sweep the CSR cost model without rebuilding clang:
#   add `$(EXTRA_MLLVM)` to the clang CFLAGS line, then:
make clean && make bios EXTRA_MLLVM="-mllvm -regalloc-csr-first-time-cost=5000000"
# count IX usage in any image:
llvm-objdump -D clang/bios.clang.elf | grep -ic '\bix\b'
# count frame-pointer functions:
llvm-objdump -d <img>.elf | grep -c 'add[[:space:]]*ix,sp'
```

**Staleness trap (cost me real time):** the firmware Makefiles do **not** list
the clang *binary* as a prerequisite of the `.o` files, so an incremental
`make` after rebuilding clang silently reuses stale objects.  Every
compiler-vs-compiler size comparison **must** `make clean` first.  An early
"IX-on = neutral" reading here was entirely a stale `payload.elf`.

## 8. What was kept vs reverted

- **Reverted:** the `Z80UnreserveIX` flag + `z80IsIXAllocatable` gate,
  `getReservedRegs`/`getLargestLegalSuperClass`/`Z80NarrowNoIndex` IX wiring,
  `getCSRCost()`, the caller-saved patch, the `prom1.ld` 4 KB region loosening,
  the firmware `$(EXTRA_MLLVM)` hooks, and the test-runner `CLANG_EXTRA_MLLVM`
  passthrough.  Tree is back to IX-reserved.  lit 120+5.
- **Kept:** AGENTS.md "Baseline before you change" rule (unrelated, added this
  session).

## 9. Bottom line for #12

#12's framing ("hasFP=false correct but larger") is confirmed and sharpened:
the largeness is the **callee-saved CSR tax**, and the cost model **cannot**
recover it (it bottoms out at +18 B from allocation perturbation alone).  The
real prize is **caller-saved IX**, worth a small but real win on the firmware
(−8 / −1 B) and potentially more once IX is used freely.  It is gated on
finishing the SP-relative frame-object path so `hasFP=false` works for
stack-arg functions — **that is the actual deliverable of #12.**  Do that, then
flip IX to caller-saved.
