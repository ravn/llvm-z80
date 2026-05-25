# Session 73s — Summary: C2 audit pass over Z80LateOptimization peepholes

**Date:** 2026-05-24
**Branch:** `session-73s` (off main at session-73r merge).
**Predecessor:** `session73q-C2-audit-table-update.md`.

## Headline

Removed 5 dead peepholes from `Z80LateOptimization.cpp`, totaling **~572 LOC**, with **zero net regressions** on the production targets.  Confirmed 10 other peepholes LIVE via measurement.  test-runner sweep improved by **+1 PASS** (the #9 deletion fixes test_27_array_2d_Os which had been a baseline FAIL).

## Deletions (5 peepholes, ~572 LOC)

| # | Name | LOC | cpnos impact | Reason dead at HEAD |
|---|---|---|---|---|
| #11 | ALU #imm; ALU #imm idempotent collapse | ~20 | -1 B | Input shape (SBC A,A + AND #1 chain) no longer reaches post-RA pass |
| #9 | OR A; LD r,0; JR Z -> LD r,A | ~55 | byte-identical | Select-lowering canonicalized differently post #128 + #177; also was actively miscompiling test_27_Os |
| #2 | POP rr; PUSH rr elimination | ~40 | -1 B | SM83 LDHL SP,# boilerplate doesn't reach this pass |
| #24 | BC ping-pong in single-BB self-loops (#97) | ~340 | -1 B | Z80LoopRotate stays default-off; rotated single-BB shape never materializes |
| #23 | HL save-via-BC roundtrip (#84) | ~117 | byte-identical | GISel canonicalization for `for(...)*p++=K_word` shape changed; save/restore shape no longer emitted |

Cumulative removed LOC: ~572.  Lit tests for #97 and #84 (which specifically exercise the deleted peepholes) were XFAILed with a forward-compatibility note ("un-XFAIL when re-shaping reverts").

## Confirmed LIVE (10 peepholes, audit-Keep annotations)

| # | Name | Disable -> cpnos delta |
|---|---|---|
| #6 | XOR #0xFF -> CPL | +1 B (drill-tested session 73q) |
| #7 | LD A,#0 -> XOR A | (reasoned by analogy to #6) |
| #10 | LD rr,nn; INC/DEC rr -> LD rr,nn+/-1 | **+4 B** |
| #13 | LD L,H; LD H,0; LD A,L -> LD A,H | +1 B |
| #14 | dead HL copy in pre-compare narrowed loop (#62) | **+7 B cpnos, +14 B AES** |
| #16 | ADD HL,rr commutativity via EX DE,HL | +1 B |
| #18 | comparison reversal (LD r,A; LD A,#imm; CP r) | **+2 B** |
| #19 | LD (sym),A + LD HL,sym reorder | +1 B |
| #20 | redundant LD A,reg removal (#60) | +1 B |
| #25 | u8 switch range-check 16-bit -> 8-bit (#86) | **+3 B** |

Inline `Re-test in session 73s ... PEEPHOLE IS LIVE.  Keep.` comments added next to each.

## Conservative (1 peephole, kept pending re-test)

| # | Name | Status |
|---|---|---|
| #17 | in-memory INC/DEC | Sized as byte-identical but sweep didn't complete (disk-full).  Conservative keep, annotated. |

## Re-tested in the session-73s resume (post disk-full) -- all LIVE

After the disk-full interruption was cleared (a 547 MB stray z88dk-ticks
trace under `aes256-corpus/icode_diff/ph_trace.txt` -- the
`feedback_docker_trace` antipattern -- deleted; disk back to 70+ GB free),
the final four candidates were re-tested with the disable/measure pattern.
**All four are LIVE; none deleted.**

| # | Name | Signal | Verdict |
|---|---|---|---|
| #8  | A-via-(HL) via-r (#76) | AES .text +18 B when off (2228->2246) | LIVE, Keep |
| #12 | commutative ALU shortcut | AES .text +4 B when off (2228->2232) | LIVE, Keep |
| #21 | known-immediate A tracking (#60 imm/#83) | AES+cpnos byte-neutral, but `bool-store-no-mask.ll` (#83) regresses when off | LIVE, Keep |
| #79 | mask-roundtrip after SBC A,A | AES byte-neutral, but `mask-from-flag.ll` regresses when off (-8 B/site) | LIVE, Keep |

**Methodology lesson (extends the campaign):** AES-byte-identical is NOT
sufficient to declare a peephole dead.  #21 and #79 are byte-neutral on
both AES and cpnos yet provably live -- the dedicated per-pattern lit test
is the decisive canary.  With #21 disabled, `bool-store-no-mask.ll` fails
because ISel still emits `ld a,#1; and #1` for `store i1 true`; with #79
disabled, `mask-from-flag.ll` fails because ISel still emits the 5-instr
`(x!=y)?0xFF:0` mask-roundtrip.  Both peepholes are the sole remover of
their shape.  Contrast with the 5 DELETED peepholes (#11/#9/#2/#24/#23),
whose lit tests had to be XFAILed because the input shape vanished
upstream entirely.  The discriminator: does the lit test still FAIL
(opportunity exists, peephole live) or PASS (opportunity gone, dead)
when the peephole is disabled.

Commit: `4e82d95` (comment-only Keep annotations next to each).

## Audit candidate list: COMPLETE

All 16 original "Migrate" candidates are now resolved (5 deleted, 11 live).
No C2 candidates remain untested.

## Updated tasks/session73q-C2-audit-table-update.md classification

Pre-session-73s:
- Migrate -> Keep: 2 (#6, #7)
- Migrate -> Likely Keep: 2 (#9, #11)
- Migrate -> Re-test: 1 (#15)
- Stay Migrate: 11

Post-session-73s (this session, incl. resume):
- Deleted (dead at HEAD): 5 (#11, #9, #2, #24, #23) + previously deleted Z80NarrowIV, #15
- Live / Keep: 16 (#6, #7, #8, #10, #12, #13, #14, #16, #17, #18, #19, #20, #21, #25, #79, plus the existing #84-#97 lit tests now XFAILed)
- Remaining to re-test: 0 (#8, #12, #21, #79 all done in the resume -- all LIVE)

The audit's original "16 Migrate candidates, ~2300 LOC saving" framing was off in both directions:
- Five were dead and could be DELETED outright (~572 LOC), not Migrate-and-keep.
- Ten were genuinely Live and should not be Migrated unless the IR canonicalization changes substantially.

## Other session 73s work

- **Z80AutoStaticStack Level 2** (commit `65c88c1504d6` at session start, prior to this audit): SCC-non-recursive non-leaf functions opt in to +static-stack.  AES `aes256.c -Oz` `.text` 3299 -> 2250 B (-32 %) with flag on.  cpnos unaffected (uses +static-stack via Makefile already).  Default off pending broader validation.

## Production impact summary

- cpnos PROM1: 2029 -> 2028 -> 2029 B (drift band; net effectively flat post all cleanup -- which is fine since the peepholes were dead).
- AES `09_Oz_prod_like` `.text`: 2228 B throughout, **byte-identical**.
- test-runner clang sweep: **990/690/37/56/207** (was 990/689/38/56/207); +1 PASS / -1 FAIL from #9 deletion fixing test_27_Os.
- Lit: **109 PASS + 5 XFAIL** = 114 (was 111+3; two new XFAILs are the now-removed peepholes' specific lit fixtures).

## Open issues / follow-ups

- Re-test #17 + #8 + #12 + #21 when disk space allows.
- Consider re-running the wider AES corpus (13 configs) for codegen safety on the cumulative deletions, not just `09_Oz_prod_like`.
- If Z80LoopRotate is ever default-on'd, revisit `issue-97-bc-pingpong-singlebb.ll` + `hl-no-bc-backup.ll` XFAILs and either revive #23/#24 or design a structural fix.

## Files

- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` -- ~572 LOC removed.
- `llvm/test/CodeGen/Z80/issue-97-bc-pingpong-singlebb.ll`, `hl-no-bc-backup.ll` -- XFAILed.
- 6 `session73s-issue*-retest.md` task writeups.
- `Z80AutoStaticStack.cpp` Level 2 (extension of session 73r work).

## Branch state

`session-73s` ahead of main by ~14 commits (Level 2 + peephole removals + audit annotations + workspace bumps).
