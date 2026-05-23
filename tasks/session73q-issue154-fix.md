# Session 73q — #154 fix: spurious mayLoad/mayStore on reg-to-reg LD opcodes

**Date:** 2026-05-23
**Outcome:** Fixed.  cpnos PROM1 +2 B (2027 -> 2029 B); behavioral correctness preserved; existing memory-rule workaround (`feedback_z80_copy_spurious_mem_flags`) can be relaxed in future peephole work.

## What was wrong

`Z80Inst` (base class for Z80 instructions) defaults `mayLoad = mayStore = hasSideEffects = true` as a conservative MC-layer default.  This applied to register-to-register `LD r,r'` opcodes (e.g. `LD_D_A`, `LD_A_D`, `LD_IXH_*`, `LD_*_IYH`, etc.), even though these instructions touch no memory and produce no side effects.

The spurious flags broke peepholes that filter by `mayLoad()` / `mayStore()` as a proxy for "this instruction may alias a memory access."  Concrete case noted on #154: the SET/RES peephole's `if (OpIt->isTerminator() || ... || OpIt->mayLoad() || OpIt->mayStore()) BailIntervening = true;` bailed at every intervening `LD_D_A`.  The local workaround was `!OpIt->memoperands_empty()` (a more honest test).

## What was fixed

Wrapped three TableGen blocks with `let mayLoad = 0, mayStore = 0, hasSideEffects = 0 in { ... }`:

1. `Z80InstrCommon.td` lines 22-78: documented `LD r,r'` family (LD_B_B through LD_A_A, 49 opcodes).
2. `Z80InstrInfo.td` lines 1067-1097: `+undocumented` IX 8-bit half-register moves (LD_IXH_*, LD_IXL_*, LD_*_IXH, LD_*_IXL).
3. `Z80InstrInfo.td` lines 1124-1157: same for IY half-register moves.

LD immediate-loads (`LD_B_n` etc.), INC/DEC, ALU-with-half-reg etc. are NOT in this fix — they have their own flag semantics (`INC` sets FLAGS, `LD_r_n` has an immediate operand stream).  Scope kept minimal.

## Verification

- Lit: 109 PASS + 3 XFAIL (unchanged).
- test-runner clang sweep: 990/689/38/56/207, **zero per-test diff** vs the post-#15-removal baseline.
- AES `aes256.c -Oz` `.text`: 3299 B (unchanged).
- cpnos PROM1 (clang): 2027 -> **2029 B** (+2 B).

## The +2 B cpnos regression

Same class as #187: a TableGen-level change to instruction flags shifted downstream generic-pass decisions (MachineSink / MachineLICM / MachineCSE / MachineScheduler use `mayLoad/mayStore` for movability analysis).  AES `.text` is unaffected, so the regression is localized to cpnos.

The fix is semantically correct — the previous flags were lies.  Accepting the +2 B for now; documented in #187 as another data point.  cpnos PROM1 is at 19 B free under the 2 KB cap, well within budget.

## Implication for the memory rule

The memory rule `feedback_z80_copy_spurious_mem_flags` (in MEMORY.md) currently says:

> "don't use MI.mayLoad()/mayStore() in Z80 peepholes; use !MI.memoperands_empty() (LD_D_A flag-noise, #154)."

After this fix, the spurious flags are gone for reg-reg LDs.  Future peepholes CAN now use `MI.mayLoad()/mayStore()` as the more idiomatic check.  The rule should be updated to reflect that the workaround is no longer needed for these instructions (though it may still apply for other instructions where the conservative defaults haven't been relaxed yet).

Not updating MEMORY.md in this session — separate task.

## Files

- `llvm/lib/Target/Z80/Z80InstrCommon.td`: 6-line `let` block + 3-line comment.
- `llvm/lib/Target/Z80/Z80InstrInfo.td`: two 4-line `let` blocks (IX and IY).
- `tasks/session73q-issue154-fix.md`: this writeup.
