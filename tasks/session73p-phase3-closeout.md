# Session 73p Phase 3 closeout (2026-05-22, late)

Phase 3 = the work after the Phase 2 summary doc was written.  Two
correctness fixes shipped; three optimization candidates closed by
survey/diagnosis (verified zero-yield); methodology rule extracted.

## Codegen shipped during Phase 3

### #184 root cause 1 fix (commit `07716e582d44`, merged via `a6c77dae2e95`)

`Z80LateOpt` peephole #148 fall-through MBB safety check.  The
`CP/XOR with 1 or 0xFF → DEC_A/INC_A` peephole relied on
`Succ->liveins()` which was stale post-regalloc, allowing the
rewrite when A was actually live (via `push af` in the fall-through
body).  Fix: walk the fall-through MBB's instructions explicitly
via `targetDeadA` + recognize `XOR_A` self-clear idiom as a full
def.

  Production delta: cpnos PROM1 −1 B (catches previously-conservative
  cases); AES baseline byte-identical; lit 106 → 107.

### #185 root cause 2 fix (commit `fcce7b2c83e8`, merged via `ea3e3a0eff46`)

`Z80LateOpt` peephole `DEC A; LD B, A; [OR A;] JR NZ → DJNZ`
B-clobber safety check.  Body clobbered B (via `ld c, l; ld b, h`
for parallel pointer arithmetic in aes_done at -Os with i16=2);
the `LD B, A` reload was essential.  Fix: refuse the rewrite when
B is defined elsewhere in the MBB.

  Production delta: byte-neutral at baseline.  With i16=2 enabled,
  all 13 AES configs PASS with real tstate counts (previously:
  02_Os/04_O2 silent ts=28 false-positive PASS).  Lit 107 → 108.

  Methodology footnote: initially declared "regalloc-level multi-
  week fix".  Drill-down showed it was a **5-line peephole fix**.

### i16=2 measurement (`session-73p-i16eq2-trial` -- not landed)

With #184+#185 both fixed, i16=2 TTI cost is now correctness-safe.
Measured production targets:

| Target | Without i16=2 | With i16=2 | Δ |
|---|---:|---:|---:|
| cpnos PROM1 | 2028 B | 2037 B | **+9 B (eats hard cap)** |
| autoload PROM | 1652 B | 1668 B | +16 B |
| AES `09_Oz_prod_like` | 2562 B | 2606 B | +44 B |
| BIOS | 5922 B | 5916 B | −6 B |

Decision: keep OFF.  Now a tuning question, not correctness.

## Optimization candidates closed by survey/diagnosis

### #173 cross-MBB multi-reload extension — VERIFIED ZERO-YIELD

Surveyed AES + BIOS + cpnos for cross-MBB candidates of the bare-
store + 4-instr-reload pattern:

- AES: every cross-MBB candidate has all GR16 pair halves modified
  on the path (heavy XOR + pointer arithmetic) — no safe pair.
- BIOS: remaining 5 reload templates are absolute hardware memory
  accesses (`$f38d-f`), not BSS spills.  My peephole's isSfr
  correctly excludes them.
- cpnos: 0 reload templates remain post Phase-2 ship.

Implementation cost: ~200 LOC.  Verified yield: 0 B.  Closed.

### #175 8-bit ALU memory operand — VERIFIED ZERO-YIELD

Surveyed for fusable `ld r, (hl|ix+|iy+); <alu> r` patterns:

- HL-indirect ALU ops (ADD_A_HLind, AND_HLind, etc.) ALREADY
  defined in `Z80InstrCommon.td` and used optimally by ISel
  (36 `xor (hl)` instances in AES).
- IX/IY-indexed ALU ops not defined, but production targets all
  use `+static-stack` (no IX-frame).  Zero `(ix+d)` accesses in
  BIOS / cpnos / autoload / AES production.
- Fusable patterns remaining in any production binary: ZERO.

The issue's "entirely absent" claim for HL-indirect was incorrect.
Implementation would yield 0 production bytes until IX-frame
becomes production-relevant.  Deferred until #40 strategic
decision flips.

### #172 A-register pinning default-on — STRUCTURALLY BLOCKED

Z80PinAluAccumulator pass landed default-off in 73o due to
materializing-COPY regressions.  Attempted PHI-cycle filter
extension; investigation revealed deeper issue:

The MIR has `XOR_r %loop_carrier` where the carrier IS the
explicit source operand of `xor r` (A := A XOR r).  Pinning
%carrier to A would emit `xor a, a` = zero A = **miscompile**.

SDCC's optimal shape rotates A as accumulator + B as snapshot;
clang's ISel emits opposite shape (carrier as XOR source).
Fix requires ISel-level pattern change to emit snapshot-rotate
XOR chains — multi-week work, not regalloc-level.

Pass stays default-OFF.  Re-evaluate after ISel snapshot-rotate
work.

## Methodology rule extracted

`feedback_dig_deeper_before_parking.md`:

> When about to declare "deferred / multi-week / regalloc-level",
> first instrument+bisect for 30 min.  Surface estimates were
> wrong 5× in session 73p; deeper drill collapsed "multi-week"
> into 5-line peephole fixes.

Cross-referenced in MEMORY.md §1 (always-on).

## Session 73p Phase 3 estimate accuracy

| Investigation | Estimate | Actual | Note |
|---|---|---|---|
| #184 r/c 1 fix | "regalloc-level work" | 17-line targetDeadA extension | ✗ overestimated |
| #185 r/c 2 fix | "multi-week regalloc work" | 5-line peephole safety check | ✗ overestimated 10×+ |
| i16=2 ship/don't-ship | "agonizing tradeoff" | 5-minute production measurement | ✗ overestimated |
| #173 cross-MBB | "200 LOC, 0-20 B yield" | 30-min catalog showed 0 B | ✓ already revised |
| #175 ALU mem operand | "5-10 B static-stack" | 30-min survey showed 0 B | ✗ overestimated |
| #172 A-pin default-on | "structural pass + selector" | Diagnosed as ISel-level blocker | ✓ multi-week (real) |

Pattern: **estimates that survived the drill were accurate;
estimates that didn't survive were almost always optimistic on
implementation cost OR pessimistic on yield**.  The methodology
rule from this session (`feedback_dig_deeper_before_parking`)
captures the lesson.

## Final session 73p totals (Phases 1 + 2 + 3)

| Target | Pre-session | Post-session | Δ |
|---|---:|---:|---:|
| AES `09_Oz_prod_like` (bin) | 2667 B | **2562 B** | **−105 B** |
| AES `09_Oz_prod_like` (tstates) | 14.89 M | 10.74 M | **−27.8 %** |
| cpnos PROM1 (clang) | 2030 B | **2028 B** | −2 B |
| BIOS (clang) | 5925 B | **5922 B** | −3 B (−169 B vs SDCC) |
| autoload PROM (clang) | 1652 B | 1652 B | unchanged |
| Z80 lit suite | 104+3 | **108+3** | +4 tests |

### Issues progressed in ravn/llvm-z80

- Closed/fixed: #128, #145, #167, #174, #179 (Phase 1); **#184**, **#185** (Phase 2/3)
- Diagnosed with precise root cause + survey, closed at "deferred":
  #173, #175, #172
- Filed for follow-up: #182 (SCEV crash; workaround in place)

### Branches merged to main

- `session-73p-phase1` (Phase 1 wins)
- `session-73p-phase2-issue177` (TTI partial ship)
- `session-73p-issue173` (#173 same-MBB peephole)
- `session-73p-issue184` (peephole #148 safety)
- `session-73p-issue185-fix-v2` (DJNZ B-clobber safety)

### Branches deleted without landing

- `session-73p-issue173` (initial MVP) — scoped wrong, recreated
- `session-73p-issue184` (early attempt) — fix iteration  
- `session-73p-issue185` (initial parking) — diagnosis-only branch
- `session-73p-i16eq2-trial` (i16=2 trial) — measurement, kept off
- `session-73p-issue185-fix-v2` initial commit — fix was reverted from working tree, amended

## What's truly remaining for future sessions

Each multi-hour, fresh context required:

1. **#180** peephole audit (16/38 stand-ins for missing upstream)
2. **#181** DAGISel vs GISel coexistence audit
3. **#178** ADD_HL_rr remat (pseudos with implicit physreg outputs)
4. **#172** ISel-level snapshot-rotate XOR chain (if #172 is ever to ship)

None offer "30-minute wins".  All require focused multi-hour
sessions on backend internals.

Sessions 73p+ work is genuinely complete.
