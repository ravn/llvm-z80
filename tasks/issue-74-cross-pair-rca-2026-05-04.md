# RCA: BSS spill→PUSH/POP cross-pair (#74) regression

**Bisect identified:** commit `96dde0c` ([Z80] BSS spill→PUSH/POP:
cross-register-pair (#74, refines #82), 2026-05-02).

**Reported symptom:** autoload-in-c PROM hangs in
`_fdc_detect_sector_size_and_density` (PC sample stuck at LMA `0x02D6`
= VMA `0x626E`).  rcbios standalone boot via assembly roa375 PROM is
unaffected.

## Initial framing (wrong)

I assumed autoload-in-c was being miscompiled because that was where
the symptom appeared.  Wrote a fix that restricts the BSS-spill
peephole to same-register-pair only.  The fix worked: with cross-pair
disabled, autoload-in-c boots end-to-end.

## What I found later (correct framing)

After the fix landed, I A/B-compared autoload-in-c's compiled
`prom.lis` between the broken-compiler and fixed-compiler states.
**They are byte-identical.**  Only the embedded build timestamp
differs.  No cross-pair PUSH/POP fires anywhere in autoload-in-c.

So the fix worked NOT by changing autoload-in-c's code, but by
changing **rcbios**'s code (which DOES have cross-pair fire-sites:
BIOS at 5929 B without the fix, 5949 B with the fix).  rcbios is the
binary loaded from floppy by the autoload PROM.

But: the SAME 5929 B rcbios works fine when loaded by the
hand-assembled `roa375.rom` autoload — `make mame-test` with the
assembly PROM shows the BIOS banner and `DISK=<hex> ERR=0`.  Only
autoload-in-c's loading path fails.

## Mechanism (refined hypothesis)

Two independent factors compose to produce the failure:

1. **rcbios at 5929 B contains a cross-pair PUSH/POP sequence
   somewhere** in its early-boot path (before the BIOS banner is
   written).  That sequence is structurally valid as a same-pair
   spill substitute (PUSH H,L bytes; POP into D,E preserves bytes
   identically) — but is fragile to register-state assumptions at
   the moment the sequence executes.

2. **autoload-in-c hands off to BIOS with a different register state
   than the assembly roa375 does.**  Specifically: I-register value,
   IX/IY, shadow-register state, FDC port residual values.  The
   assembly autoload happens to leave registers in a state that the
   cross-pair sequence tolerates; autoload-in-c does not.

Neither factor alone is wrong:
  - rcbios passes lit + size oracle + boots via assembly.
  - autoload-in-c is byte-identical between broken/fixed compiler.

The **composition** breaks the boot.  The cross-pair feature
(#74) introduced the latent fragility into rcbios; autoload-in-c
exposes it by happening to enter BIOS with the un-tolerated
register state.

## Why my fix works

The fix re-restricts the BSS-spill peephole to same-pair only.
This prevents rcbios from having any cross-pair PUSH/POPs.  rcbios
is now back to its pre-#74 codegen shape (5949 B vs 5920 B at #74's
peak; the -29 B savings were real but unsafe in this composition).

It is a CONSERVATIVE fix — it reverts the optimization broadly
rather than identifying the specific problematic site in rcbios and
rewriting the rcbios source to be tolerant of cross-pair conversion.
The narrower fix would require:
  1. Diff broken-rcbios `bios.cim` against fixed-rcbios `bios.cim`
     to identify which function gained a cross-pair PUSH/POP.
  2. Inspect that function's register-state expectations at the
     PUSH/POP boundary.
  3. Either rewrite the function in C to be cross-pair-tolerant, OR
     add a more selective gate to the peephole.

Filed as a follow-up investigation under ravn/llvm-z80#74.

## Lessons

  1. **Symptom-where ≠ bug-where.**  The PC sample showed autoload-
     in-c hanging in fdc_detect; the actual miscompile was in rcbios
     (not autoload-in-c).  When the same binary works in one
     environment but not another, suspect the OTHER component, not
     the one displaying the symptom.

  2. **Bisect identified the commit; A/B asm diff identified the
     binary.**  The bisect alone would have led me to "rewrite #74
     differently" or "narrow #74 to specific sites."  The A/B diff
     showed neither was needed — autoload-in-c is unchanged; only
     rcbios changed.  That distinction shapes the right fix scope.

  3. **Latent fragility from optimization.**  The cross-pair feature
     was correct in isolation (PUSH HL ; POP DE === LD (slot),HL ;
     LD DE,(slot) for value bytes).  It introduced a sequence whose
     correctness depends on register state at execution time.  When
     a downstream consumer (autoload-in-c boot path) doesn't satisfy
     that implicit precondition, the optimization breaks the
     composition.

  4. **The autoload-in-c MAME test is the right value-oracle
     addition.**  Even though the bug was technically in rcbios, the
     ONLY caller that exposed it was autoload-in-c.  Without
     autoload-in-c in the test matrix, this would have stayed
     hidden indefinitely.
