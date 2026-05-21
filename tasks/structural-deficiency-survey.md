# Structural deficiency survey — clang Z80 backend + SDCC

Date: 2026-05-21 (session 73p continued).  Triggered by user prompt:
"I also want you to search memory and project history thoroughly for
identifications of major structural deficiencies in either compiler
and make issues accordingly."

## Method

1. **Existing issues** — enumerated all 176 ravn/llvm-z80 issues + 15
   ravn/z88dk issues (open + closed) to establish baseline.
2. **Memory mining** — read all `feedback_*.md` and `project_*.md`
   entries; surfaced rules + project facts that point at compiler-
   structural concerns.
3. **Roadmap mining** — read
   `llvm-z80/tasks/backend-completion-roadmap.md` (session 36) and
   `llvm-z80/tasks/late-opt-audit-2026-05-02.md` (session 37).
4. **Timeline mining** — grep'd
   `rc700-gensmedet/tasks/timeline.md` for "structural", "parked",
   "deferred", "workaround", "TTI", "cost model".
5. **Issue cross-check** — for each candidate, verified that no
   existing issue already covers it.

## Findings

### NEW clang structural issues filed this survey

| Issue | Title | Source | Priority |
|---|---|---|---|
| **#177** | No Z80-specific TargetTransformInfo | Roadmap area 10; timeline session 73 ("TTI under-predicts spill cost") | **High** |
| **#178** | Pseudos with implicit physreg outputs break rematerializer | Session 73p #166 attempt; roadmap area 1+8 ("isReMaterializable partial") | **High** |
| **#179** | GISel ISel + scheduler don't reorder $a-chained register-independent ops | Session 73p #174 analysis; roadmap area 2 | **High** |
| **#180** | Z80LateOptimization peephole audit — 16 of 38 are upstream stand-ins | `late-opt-audit-2026-05-02.md`; roadmap area 12 | **Medium** (tracker) |
| **#181** | DAGISel vs GISel coexistence audit | Roadmap area 9 | **Low** (upstream-prep) |

### Existing structural-tier issues (already filed)

For completeness, the existing issues that capture structural concerns:

| Issue | Status | Topic | Notes |
|---|---|---|---|
| #7 | Open | DJNZ/LDIR/CPIR/CP(HL) instruction-driven codegen | Umbrella; partly addressed via peepholes |
| #12 | Open | hasFP=false correct but larger | Frame lowering structural |
| #27 | Open | Per-pair 16-bit register copy cost | Subsumed by #177 (TTI) |
| #40 | Open | Evaluate IX frame ptr vs static-stack per-fn | Umbrella for #176 |
| #95 | Closed | IV rewrite countdown→count-up target-aware | Closed; #177 captures the broader TTI gap |
| #115 | Open | Regalloc heuristics: greedy picks IY when un-reserved | Subsumed by #177 |
| #128 | Open | MachineLICM/MachineCSE pessimize at -Oz | Subsumed by #177 |
| #154 | Open | Reg-to-reg copies carry spurious mayLoad/mayStore | TableGen audit |
| #120 | Open | GISel combiners for #79/#93 carry-roundtrip | Narrow combiner work |
| #166 | Open | ADD_HL_rr/LD_HL_a16 rematerialization | Subsumed by #178 |
| #172 | Open | A-shuttle accumulator pin | Regalloc structural |
| #174 | Open | gf_log/gf_alog redundant-reload peepholes | Subsumed (root cause) by #179 |
| #176 | Open | Auto-infer +static-stack safety per-function | Frame structural |

### NEW SDCC structural issues filed this survey

**None.** All major SDCC structural quirks identified in memory and
session notes are already filed:

| Existing z88dk issue | Topic |
|---|---|
| #1 | block-scoped variable undefined in deeply nested code |
| #2 | const-qualified 16-bit pointer uses byte-wise load |
| #3 | --std-sdcc23 missing C23 #embed |
| #4 | const-expression byte-shift miscompile |
| #5 | --nogcse miscompiles AES-256 |
| #6 | sdcc_ix + sdcccall 1 miscompiles AES-256 |
| #7 | block-scope extern doesn't emit GLOBAL symbol |
| #8 | switch dead-jp-block after default ret (closed) |
| #9 | port-IO intrinsic feature (closed) |
| #10 | --fomit-frame-pointer falls back to IX-frame |
| #11 | rematerialization expansion to cross-call spill |
| #12 | jr cc, lbl peep 84 miscompiles |
| #13 | ralloc.c:1190 defensive spillLoc clear |
| #14 | K&R int-promotion penalty (7.8% size, 15% runtime) |
| #15 | wrong host-triple report on macOS aarch64 |

The SDCC ecosystem has fewer surface structural issues partly because
**SDCC is a finished compiler** (mature codebase, ~25 years of
maintenance) and partly because we use it as a reference oracle
rather than developing it.  The structural quirks we hit (e.g.,
iCode allocator's design per `feedback_dont_fight_sdcc_icode.md`)
are documented as known designs, not deficiencies to file.

If SDCC structural issues surface during future work, they'll go to
ravn/z88dk per `feedback_file_issues_in_forks.md`.

## What this survey deliberately did NOT identify as structural

- **Specific missed peephole opportunities** (e.g., one missing
  `inc r` form in some context).  These are mid-tier optimization
  issues; the Z80 backend's late-opt is already crowded with them.
  New ones get filed as encountered, not preemptively.

- **K&R-mode SDCC quirks beyond what's filed**.  Most cross-compiler
  K&R issues are covered by z88dk#14 (the umbrella).

- **AES-corpus-specific patterns** that don't generalize.  E.g.,
  the rj_xtime shift+bit7-test pattern (~17 K ts on AES) is too
  narrow to be a structural issue; covered as a follow-up to #179.

- **Hardware-emulator concerns** (MAME bugs, z88dk-ticks limits).
  Those go in ravn/mame or upstream z88dk repos, not the compiler
  trackers.

## Structural map after this survey

The clang Z80 backend has these major structural issues now tracked:

```
                Z80 backend structural debt
                        │
        ┌───────────────┼───────────────┐
        │               │               │
   Cost models     ISel/MIR        Late-opt
   (#177 TTI)     (#178 pseudos)   (#180 audit)
        │         (#179 reorder)         │
        │         (#181 DAGISel)         │
        │               │                │
   #128, #95,      #166, #174,      Per-peephole
   #27, #115,      #172, #100,      migration sessions
   #92, #93,       #166...
   #94, #95,
   #126, #130
```

Closing #177 + #178 + #179 + #181 cascades closures of ~15
already-filed issues.  Closing #180 (after the above) cleans up
the late-opt pass.

## Recommended Phase-1 execution order

After the user agrees to start:

1. **#179** first — captures the dominant per-iteration cost universally.
   ~1-2 weeks (combiner rules + lit tests + corpus measurement).
2. **#177** in parallel — implementing TTI's individual hooks is
   roughly per-hook-per-week of effort; can be incremental.
3. **#178** when #166 work resumes — Path A (SSA-shaped pseudos)
   is invasive but unlocks remat across the backend.
4. **#181** as upstream-prep cleanup (low immediate value).
5. **#180** sub-issues opened only AFTER #177-#179 land.

## Files referenced

- `llvm-z80/tasks/backend-completion-roadmap.md` — Session 36 audit
- `llvm-z80/tasks/late-opt-audit-2026-05-02.md` — Session 37 audit
- `llvm-z80/tasks/aes-speed-gap-analysis.md` — Session 73p
- `llvm-z80/tasks/issue174-implementation-plan.md` — Session 73p
- `llvm-z80/tasks/all-modes-competitive-plan.md` — Session 73p
- `memory/feedback_root_cause_over_peephole.md`
- `memory/project_z80_backend_unfinished.md`
- `memory/project_z80_upstream_goal.md`
- `rc700-gensmedet/tasks/timeline.md`

## What "structural" meant for this survey

Restricted to compiler-infrastructure-level issues:

- **Yes**: missing target hooks (#177 TTI), broken type-system invariants
  (#178 pseudos), pipeline ordering bugs (#179), accumulated tech debt
  (#180), uncertain coexistence (#181).
- **No**: individual missed optimizations, ABI fine-points already
  filed, specific miscompiles for unusual IR shapes.

The dividing line is roughly "would an upstream LLVM reviewer say
'this is structurally wrong, the backend isn't done here'" vs
"this is a tactical optimization opportunity."
