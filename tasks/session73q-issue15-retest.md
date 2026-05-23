# Session 73q — #15 retest (16-bit increment overflow test peephole)

**Date:** 2026-05-23
**Predecessors:** `session73q-C2-audit-table-update.md` (Re-test classification), `session73q-triage.md`.
**Outcome:** PASS.  Peephole #15 removed (~110 LOC).  cpnos PROM1: 2028 -> **2027 B** (-1 B win); behavioral tests: zero per-test diff.

## Background

#180 audit peephole #15 collapsed the 9-instruction "16-bit increment overflow test" idiom into a 4-instruction sequence:

```
Pattern A (16 B -> 5 B, saves 11 B):
  LD HL,1; ADD HL,rr; SBC A,A; AND 1;       INC rr; LD A,hi; OR lo;
  LD lo,L; LD hi,H; XOR 1; AND 1; JR NZ  -> JR NZ

Pattern B (10 B -> 5 B, saves 5 B):
  LD HL,1; ADD HL,rr; SBC A,A; AND 1;       INC rr; LD A,hi; OR lo;
  EX DE,HL; JR Z                          -> JR NZ
```

The audit classified this as "Migrate" with upstream home "IR-level idiom (LSR) / TTI cost."  The C2 reclassification flagged it as **Re-test** because session-73p Phase 2 #128 (LICM/CSE disable) + #177 (TTI hooks) may have made the upstream IR canonicalization produce a different shape that the peephole no longer matches.

## Re-test methodology

1. Probe codegen of a known-target shape (`while (++counter)`).  Result: ISel emits `SBC A,A; RRCA; JR C` (3 instr / 4 B for the overflow test), not the 9-instruction pattern the peephole expects.
2. Scan AES `aes256.c -Oz` binary for the peephole's input pattern (`LD HL,1` followed by `ADD HL,rr`).  Result: **zero hits.**
3. Disable peephole #15 via `if (false && STI.hasZ80())`, rebuild, measure.  Lit clean (109+3 XFAIL), cpnos PROM1 unchanged (initial measurement 2028 B — later corrected to 2027 B; the 2028 was likely a stale sccache hit).
4. Delete peephole #15 entirely.  Re-measure.

## Results

| Surface | Before (with #15) | After (without #15) | Delta |
|---|---|---|---|
| Lit suite | 109 PASS + 3 XFAIL | 109 PASS + 3 XFAIL | identical |
| cpnos PROM1 (clang) | 2028 B (20 B free) | **2027 B (21 B free)** | **−1 B** |
| AES `aes256.c -Oz` `.text` | 3299 B (0xCE3) | 3299 B (0xCE3) | byte-identical |
| LOC removed | n/a | **−96** | from Z80LateOptimization.cpp |
| test-runner clang suite | (pending sweep below) | | |

## The unexpected −1 B win

Removing peephole #15 *saved* 1 B on cpnos PROM1, even though pattern scanning showed zero matches for the peephole's input shape.  This contradicts the simplest model ("peephole doesn't fire => removal is byte-neutral").

Two non-exclusive hypotheses:

1. **Pipeline-ordering side effect** (same class as #187, opposite sign): the peephole's MBB iteration touches the instruction stream in a way that nudges *downstream* peephole decisions.  Removing it gives downstream passes a slightly different starting state.

2. **Pattern matching is incomplete on the asm-disassembly probe**.  The peephole's `if`-chain test on opcodes (`Z80::LD_HL_nn` etc.) might match cases that my regex-on-disassembly grep didn't catch (e.g. different operand spelling, intermediate instruction not in the same MBB).  The peephole could fire in MIR but not be visible as an asm-text match.

Either way, removal is the right call: the source LOC saving (~96) plus the cpnos win plus zero per-test regression dominates any case for keeping the dead code.

## What this means for the #180 audit

The audit's "Migrate" classification for #15 was implicitly correct — the upstream pipeline IS now handling the case directly.  But the actual ACTION was different from the audit's predicted action (migrate to IR-level idiom / TTI):

- The audit predicted: implement a new IR-level / TTI-driven recognizer.
- What actually obsoleted #15: session-73p Phase 2 changes shifted ISel's emission shape entirely.  No new upstream code needed.

This is the second instance in session 73q where an audit "Migrate" candidate turned out to be obsoleted by other upstream changes rather than requiring active migration work (the first was the Z80NarrowIV trio #169/#170/#171 obsoleted by #177 TTI hooks).

**Implication**: when reviewing the audit, "Migrate" entries should be tested for current relevance BEFORE writing migration code.  Some are likely already obsolete.

## Closing entry

This commit and writeup close #15-as-tracked-in-#180.  The peephole's tracking note in `tasks/late-opt-audit-2026-05-02.md` and the C2 reclassification table in `tasks/session73q-C2-audit-table-update.md` are now historical — the peephole is gone.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp`: −96 LOC (the entire peephole block + its match logic).  Replaced with a 9-line comment pointing here.
- `tasks/session73q-issue15-retest.md`: this writeup.
