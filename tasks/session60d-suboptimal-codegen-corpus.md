# Session 60d (2026-05-12) — Suboptimal codegen issues filed from cpnos-rom analysis

## Workflow context

Per user-stated meta-goal this session: real-world cpnos-rom code
is to be used as **evidence corpus** for Z80 backend improvements,
not as a target whose density we're trying to close at the source
level.  When a suboptimal codegen pattern surfaces:

  1. File as issue in `ravn/llvm-z80` with a **minimal test case**
     demonstrating the gap.
  2. Issues accumulate over time.
  3. In due course, batch-derive the actual underlying compiler
     defects from the issue corpus, then report THOSE upstream
     (llvm-z80/llvm-z80, eventually llvm/llvm-project).

This session pivots my work pattern accordingly.  Stopped proposing
cpnos-source-level fixes (e.g. `recv_byte_t` API change in
rc700-gensmedet) for compiler-attributable bloat — that conflated
the two goals.

## Issues filed this session

| # | Title | Pattern | Per-site bloat | cpnos impact estimate |
|---:|---|---|---:|---:|
| 141 | i16 comparison against 0x0100 should fold to high-byte test | `icmp uge i16 r, 256` lowers to 9 B subtract-and-borrow; 4 B achievable via high-byte test | ~5 B | ~45 B in rcvmsg_c alone |
| 142 | Residual i8→i16 zext after `(uint8_t)` cast feeding equality compare | `ld l,a; ld h,0; sub K; or h` after `and K2` (with K2 < 0x80) — 4 B redundant | ~4 B | ~30-50 B across cpnos |
| 143 | #132 multi-fire interaction: subsequent edge-split candidates targeting the same escape MBB bail | Edge-split's NewMBB becomes the escape's layout-predecessor, blocking peer fires | varies | ~10-20 B on multi-loop functions |
| 144 | i16 select-on-equality materializes via i1→shift chain instead of branched conditional return | `(a == K) ? -1 : 0` lowers to 22 B SBC-RLCA-RRCA-SBC chain; 10 B via `ret nz` + `dec de` | ~12 B | ~30-40 B in cpnos |

Plus a substantial root-cause comment on **#139** (the previously-
filed "investigate snios_rcvmsg_c BB#3 candidate" issue):
identified slot coalescing as the underlying cause — slot `$ec09`
in rcvmsg_c has 12 references because regalloc coalesces three
disjoint-lifetime variables (`t`, `hcs`, `cks`) into one BSS slot,
defeating #132's "any other MBB references slot → bail" safety check.
Proposed three approaches (lifetime-aware reference map being the
most concrete).

## Test cases included with each issue

  - **#141**: standalone `.ll` repro (`@check`) — 4 lines of IR
    + expected vs current Z80 output side-by-side.
  - **#142**: standalone `.ll` repro (`@check_soh`).
  - **#143**: synthetic 2-loop `.ll` repro (`@twoloop`) — concrete
    multi-fire CFG that triggers the bug.
  - **#144**: 1-line C source (`select_test`) producing the
    22-byte materialization chain.

All four reduce to ~20 lines or less.  Each is suitable for direct
use as a `llvm-lit` fixture once the underlying fix lands.

## What was NOT filed (and why)

  - **`recv_byte_t` API change** in rc700-gensmedet — earlier draft
    proposed this; withdrawn after the user clarified that cpnos
    source changes for compiler-attributable bloat conflict with
    the corpus-mining workflow.  The `uint16_t r` + 0xFFFF sentinel
    pattern's ~30 B of irreducible-without-API-change overhead is
    documented in `tasks/session60c-analysis-and-followups.md` as
    architectural evidence; not a filed issue.
  - **Tail-merge of common epilogs as RFE** — verified empirically
    that clang ALREADY does this for the simple case (the synthetic
    `multi_exit` repro shows all three `return -1` paths converge
    at `.LBB0_3`).  The remaining "tail-merge" surface area would
    require a real witness where merging fails; not yet identified
    one that's reducible.
  - **`pop h; jmp` inter-procedural unwind** equivalent — no
    realistic compiler implementation path; locked by C structured
    return semantics, documented elsewhere not filed.

## Corpus-building implication

Four issues with concrete test cases — first batch of the
"compiler-research-via-real-code" pipeline.  No `backend-research/`
directory built yet; structure deferred until a second function's
analysis warrants indexing (per session 60c's question about (c)
"just file the issues for now, structure later").

The four issues here all derive from `_snios_rcvmsg_c` analysis;
when the next function (likely `_snios_sndmsg_force` or some
rcbios function) gets the same treatment, the indexing structure
will start paying for itself.

## Pattern observations across the four issues

Two cross-cutting themes:

  1. **i8 vs i16 boundary handling**: #141, #142, #144 all stem
     from how clang lowers `(uint8_t)wide_value`-then-compare or
     `select i16 cmp, ...` patterns.  The Z80 backend is doing
     work that's "correct on i16" but loses the chance to stay
     narrow.  These three issues might fold into a single
     "Z80 i8-narrowing combiner pass" improvement when
     investigated together.
  2. **Peephole local-correctness vs global-interaction**: #143
     is the only issue in this batch that's not about a missed
     idiom — it's a subtle interaction bug in the #132 edge-split
     code path I shipped this session.  Suggests the multi-fire
     interaction class is worth a closer look in other recent
     additions.

## Rules-checked

  - `feedback_compiler_bug_test` — each issue includes a
    minimal test case suitable for converting to lit fixture.
  - `feedback_no_upstream_issues` — all four filed in ravn fork.
  - `feedback_compiler_not_trusted` — each filing inspected
    actual generated asm (not just inferred from source).
  - `feedback_extract_rules_from_time_sinks` — no time-sink
    rule extracted this session; the work was straightforward.

## Files touched

  - `tasks/session60d-suboptimal-codegen-corpus.md` — this doc.
  - ravn/llvm-z80 issues #141, #142, #143, #144 — filed.
  - ravn/llvm-z80 #139 — substantive comment added.
