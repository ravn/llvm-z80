# Task #17 — peephole audit findings (2026-05-03)

Triggered after the #104 fix surfaced a class of peepholes that
introduce register/flag clobbers without checking liveness.  Audit
covers `llvm/lib/Target/Z80/Z80LateOptimization.cpp` (~5130 LOC, ~30
distinct peephole sections).

Methodology: for each peephole, identified what the rewrite emits new
(register defs / flag defs) vs. what the original sequence emitted,
then checked whether the code verified those side effects were safe
to introduce/elide at the rewrite site.

## Findings

### Severity 1 — register-value clobber (latent functional bug)

| Site                         | Issue   | Status              |
| ---------------------------- | ------- | ------------------- |
| in-memory INC/DEC (l. 1414)  | #104    | **fixed** (3ef14efc) |
| #85 chain rewrite (l. 2916)  | #107    | **fixed** (d7505c8e) |

Both rewrites emit `LD HL, addr` (or chain of HL-walking stores)
without checking that H/L/HL was dead at the rewrite head.  Same
LivePhysRegs-walk fix shape applied to both.

### Severity 2 — FLAGS-state mismatch past branch (latent)

| Site                                                    | Comment            |
| ------------------------------------------------------- | ------------------ |
| DEC A; LD B,A; JR NZ → DJNZ (l. ~735)                  | DJNZ doesn't set FLAGS, no dead-check |
| DEC B; JR NZ → DJNZ (l. ~781)                          | same as above |
| 16-bit increment overflow (l. ~1255)                    | producer changes (AND→OR), no dead-check; HL liveness unchecked |
| comparison reversal (l. ~1554)                          | new CP produces different non-carry flags |
| u8 switch range-check 16→8 (l. ~2559)                   | new CP differs; DE post-state unchecked |
| #93 carry-roundtrip (l. ~2762)                          | explicit "specific enough" admission, no scan back to carry source |

Tracked as **#108** for follow-up.

### Severity 2.5 — comment-aspirational-only safety check

| Site                          | Issue   | Status              |
| ----------------------------- | ------- | ------------------- |
| ADD HL,rr commutativity (l. 1360) | #109 | filed, not fixed |

Comment says "Check that BC is not read..." but code does not.

### Verified safe (audited, no concern)

- POP/PUSH removal (l. 623) — checks isRegDeadAfter on popped reg
- LD A,r; DEC A; ... → DEC r; JR NZ (l. 663) — accidentally
  preserves FLAGS state (DEC A and DEC r set Z identically)
- XOR #0xFF → CPL (l. 806), LD A,#0 → XOR A (l. 826) — both check
  FLAGS dead
- A-via-(HL) → direct (l. 845) — checks A dead, no FLAGS effect on
  either side
- LD rr,nn; INC rr → LD rr,nn+1 (l. 975) — neither side touches
  FLAGS, immediate fold is exact
- ALU #imm; ALU #imm dedup (l. 1025) — idempotent ops only
- redundant LD A,reg removal (l. 1736) — full clobber scan with
  RegMask handling
- known-immediate A tracking (l. 1817) — explicit FLAGS-consumer
  scan-ahead before deleting dead AND/OR/XOR
- BSS spill→PUSH/POP (l. 4645) — three documented safety guards
  (cross-block, cross-register stack depth, MCSymbol offset)

## Open follow-ups

- **#108** — FLAGS-after-branch admissions (DJNZ rewrites + 4 others)
- **#109** — ADD HL,rr commutativity unchecked BC liveness

These are filed but **not** scheduled for immediate fix; the audit's
critical finding (#107) is the only one with a known concrete repro
that compiles to wrong code.  #108/#109 are latent and would require
constructing pathological IR to trigger.

## Process insight

The pattern across all three findings (#104, #107, #109) is the
same: a comment in the source code documents a check that should be
performed but the code below does not perform it.  Greppable
markers: "specific enough", "known producers", "Check that X" with
no corresponding check below.

The #104 fix established the LivePhysRegs walk-back idiom for these
cases.  Future peepholes should use that idiom (or equivalent)
unconditionally when they emit new register defs that the original
sequence did not.
