# Session 60c (2026-05-12) — Post-#132 analysis, issues filed, next-session candidates

## Where we are after #132 (commit 15e4eef6e62d, pushed)

  - Z80 backend: third peephole in the BSS-spill→PUSH/POP family
    landed.  In-MBB same-class + in-MBB cross-class + cross-MBB
    (prepend-in-place + edge-split) all now have code paths.
  - cpnos-rom clang/pio-irq payload: **1904 B** (−2 B from
    pre-session-60 baseline of 1906 B).  Single production fire
    in \`_snios_sndmsg_force\`'s t-counter retry loop via the
    prepend-in-place strategy.
  - lit suite: 95/95 (93 PASS + 2 XFAIL).
  - z80-utils test-runner clang Oz: 113 PASS / 0 FAIL.
  - cpnos-polypascal-test 2-cell (clang × pio-irq + clang × sio):
    PASS.

## cpnos-rom resident composition (current HEAD)

Top consumers in \`clang/payload.elf\` (clang/pio-irq, 1904 B
total):

| rank | function | size (B) | notes |
|---:|---|---:|---|
| 1 | \`_snios_rcvmsg_c\` | 384 | largest by 1.86× |
| 2 | \`_netboot_mpm\` | 206 | cold-init path |
| 3 | \`_snios_sndmsg_force\` | 202 | session-60 fired here |
| 4 | \`_scroll_lines\` | 113 | console |
| 5 | \`_port_init\` | 110 | cold-init |
| 6 | \`_specc\` | 96 | console escape |
| 7 | \`_isr_crt\` | 95 | ISR |
| 8 | \`_init_hardware\` | 92 | cold-init |
| 9 | \`_impl_conout\` | 87 | console |
| 10 | \`_cpnos_cold_entry\` | 56 | bootstrap |

Headroom vs the 2 KB PROM-1 ceiling: **2048 − 1904 = 144 B**.
Clear path to fitting fully in PROM-1; remaining squeeze hinges
on \`_snios_rcvmsg_c\` which alone is 20% of resident.

## Cluster A status (Phase 3 of roadmap) after #132

Per session 58 reframe + this session:

| Issue | Mechanism | Status |
|---|---|---|
| #128 | LICM/CSE disable at -Oz | wired in cpnos Makefile; pending default |
| #129 | in-MBB BSS→PUSH/POP | CLOSED 2026-05-11 |
| #131 | \`z80_preserves_regs\` caller-side | CLOSED |
| #132 | cross-MBB BSS→PUSH/POP | substantially implemented this session; left open for #138 + #139 + #140 follow-ups |
| #133 | \`z80_preserves_regs\` callee-side | layer 1 landed (session 58); audit pending |
| #134 | \`d,e,h,l,b,c\` full-set FAIL on polypascal | open, multi-session |

Engagement-mode gate is effectively cleared under both readings
of the roadmap.

## Follow-ups filed this session

Three new issues filed against ravn/llvm-z80:

  - **#138** — Liveness-driven 1 B compensation (\`pop af\` /
    \`pop hl\` / \`pop de\` / \`pop bc\` when dead at escape
    entry, instead of \`inc sp; inc sp\`).  Concrete RFE.
    Estimated saving on cpnos: +1 B per existing fire site
    (the snios_sndmsg_force got_first_ack case is the
    immediate beneficiary; A+FLAGS are dead there).
  - **#139** — Investigation: \`_snios_rcvmsg_c\` BB#3 passes
    the succ-gate diagnostic but produces no observable size
    delta.  Diagnostic needs finer-grained instrumentation to
    identify which downstream gate bails (or whether the rewrite
    fires but produces 0 B net at a later cleanup).
  - **#140** — Add hand-crafted \`.mir\` lit fixture for the
    edge-split path.  IR-level synthesis fails because
    regalloc + in-MBB peepholes are collectively aggressive.

## Candidate next-session work (ranked by leverage)

1. **#138** (liveness comp) — single-session-scoped, concrete
   code change, value oracle ready, +1 B on cpnos in the
   immediate site.  Compounds with future fires.
2. **#139** (rcvmsg_c trace) — diagnostic work, could expose
   an additional fire site worth several B, or close as
   expected.  Touches the same code as #138 so worth scheduling
   together.
3. **#140** (edge-split .mir test) — small task, no code
   change to compiler.  Hardens regression safety.
4. **\`_snios_rcvmsg_c\` density audit** — independent of #132,
   this is the next major lever.  At 384 B it's the largest
   function; SDCC equivalent (per session-58 docs) was 154 B
   hand-written.  Investigation: what fraction is BSS spill
   traffic?  How many cross-MBB candidates does it have post-
   #132?  Where do the remaining bytes go?
5. **#134** (full preserves-regs set FAIL) — multi-session.
   Investigation only; gated on understanding regalloc /
   getCalleeSavedRegs interaction.
6. **#128 → default-on at -Oz** — currently wired via Makefile;
   making it default in the backend would propagate the saving
   to all -Oz consumers.

## Observation for memory (proposed)

#132's experience suggests the in-MBB peepholes + regalloc may
need adjustment to **expose more cross-MBB candidates**.
Specifically: when regalloc places a counter in a callee-saved-
register-equivalent BSS slot but the value is only needed
across a single CALL, the in-MBB peephole captures it via
PUSH/POP.  When the post-CALL test branches out before the
counter is touched, the LOAD lives in a successor MBB —
exactly the #132 case.  But many "candidate" cases get
rewritten in-MBB first because regalloc keeps the dec/test
sequence in the same MBB.

This is not a problem per se — it just means #132's surface
area is narrower than the issue body estimated (which assumed
the SNIOS shape would all be cross-MBB).  Production fires:
1, not the 6 estimated.

Worth tracking in a future "cluster A retrospective" if the
backend matures further.

## Files

  - \`tasks/session60c-analysis-and-followups.md\` — this doc.
  - ravn/llvm-z80#138, #139, #140 — filed this session.
