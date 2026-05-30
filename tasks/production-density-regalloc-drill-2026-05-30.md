# Production-density regalloc drill — 2026-05-30

First instrumented drill on the "production density via regalloc" goal
(#110 / #115 / #100 / #178; CLAUDE.md: "BSS-spill traffic + regalloc churn is
30–48 % of large clang BIOS functions").  Per `feedback_dig_deeper_before_parking`,
instrument the real binaries before assuming.

## Method
Built BIOS (clang, **5897 B** — beats SDCC 6091 B) and cpnos PROM1 (2022 B);
disassembled and classified the dominant instruction patterns.

## BIOS instruction census (2553 insns total)
| pattern | count | ~% | nature |
|---|---|---|---|
| `ld a,(nn)` / `ld (nn),a` — 8-bit BSS via A | 324 | 13 % | **ISA-forced** (8-bit direct memory access is A-only on Z80) |
| A-shuttle moves `ld r,a` / `ld a,r` | ~245 | 10 % | **irreducible** — single-accumulator contention (= #172, proven a wash: conservation of shuttles) |
| 16-bit pair copies `ld c,l; ld b,h` (HL→BC etc.) | ~65 | 3 % | mostly necessary (#27/#110; EX DE,HL path already exists) |
| `push ix` / `ld ix,` (IX-stash of constants, #15) | **0** | — | not a BIOS issue |

cpnos: only 6 `push ix/iy`, near-optimal (confirmed earlier this session).

## Finding — the dominant waste is fundamental, not a recoverable defect
- **BSS-via-A (324)**: Z80 has no `LD r,(nn)` for r≠A; every 8-bit fixed-address
  load/store must go through A.  Not reducible without changing the memory model
  (and clang already prefers direct addressing over IX-indexed where possible).
- **A-shuttle (245)**: the same single-accumulator contention #172 spent five
  approaches on and proved irreducible.
- **Redundant-reload probe**: 11 candidates found, but inspection (`_bios_const`:
  `ld a,($3); ld e,a; and $3; ld d,a; ld a,e`) shows they're A-shuttle *restores*
  from a register copy, not missed memory load-forwards.  ~0 recoverable.
- **No IX-stash (#15) in BIOS**; cpnos near-optimal.

The reason clang BIOS already **beats** SDCC (−194 B) is that both hit the same
ISA walls and clang's direct-addressing + load-forwarding are already good.

## Verdict
There is **no high-leverage production-density regalloc win available.**  The
open issues operate at the margins:
- **#178 (remat)**: real mechanism gap, but remat targets spill-across-CALL /
  IX-stash, and BIOS has **zero** IX-stash and cpnos is tight — so fixing it
  moves the production targets ~nothing (it was an AES `gf_log`/`#166` lever).
- **#110/#115 (greedy hints / IY picks)**: the ~65 pair-copies are the only
  plausibly-recoverable BIOS pattern, but greedy ignores hints (documented
  workaround = single-register classes) and prior attempts were net-harmful;
  expected yield is single-digit-to-low-tens of bytes against a target that
  already beats SDCC.
- **#100 (loop-carrier BSS-spill across CALL)**: gates #77a, narrow.

**Recommendation:** do NOT sink multi-session effort into production-density
regalloc — the wins aren't there (the targets are near the ISA's practical
limit and already beat SDCC).  Higher-value directions: upstream-submission
packaging (both Tier A gates now resolved), or accept the current state.  Revisit
regalloc only if a *specific* function regresses or a new workload (not BIOS/
cpnos/AES) surfaces a different pattern.

## Branch
`z80-178-remat-drill` — analysis only, no code changes (the drill answered the
question before any code was warranted).
