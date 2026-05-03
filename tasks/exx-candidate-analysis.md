# EXX-bracket candidate: synthetic test + savings (#114)

## Files

  - `tasks/exx-candidate-synthetic.c` — C reproducer.  Builds with the
    same flags as rcbios (`-Oz +static-stack -disable-lsr`).
  - `llvm/test/CodeGen/Z80/issue-114-exx-bracket-candidate.ll` — IR
    fixture, locks in the current spill shape so the candidate
    doesn't silently disappear under unrelated regalloc churn.

## What the test models

The textbook `_specc` 0xde19-0xde3c shape: a u16 outer counter
parked across an inner no-CALL byte-twiddle loop.  Three GR16
pairs are pressured by inner work, forcing the outer counter
(in BC) to BSS-spill at the inner-loop preheader and reload at
the outer back-edge.

C version uses `extern volatile uint8_t inner_n` so the optimizer
can't unroll the inner loop; all inputs are globals so the
function gets `+static-stack` BSS slots instead of IX-frame.

## Current codegen (IR-driven, `llc -O2 +static-stack -disable-lsr`)

Function `_render` lays out as three MBBs (after entry):

```
LBB0_2 (inner loop body):
    ld   l,e         ; sp into HL for byte-load
    ld   h,d
    ld   a,c         ; acc into A
    xor  (hl)        ; acc ^= *sp
    ld   c,a         ; back to C
    ld   hl,(__sfrend_render-2)   ; reload dp
    ld   (hl),a                   ; *dp = acc
    ex   de,hl
    ld   de,#80
    add  hl,de       ; sp += 80
    push hl
    ld   hl,(__sfrend_render-2)
    add  hl,de       ; dp += 80
    pop  de
    ld   (__sfrend_render-2),hl   ; spill dp
    djnz .LBB0_2     ; inner back-edge

LBB0_3 (outer.latch):
    ld   bc,(__sfrend_render-6)   ; <- BC RELOAD (4 bytes)
    inc  bc                        ; ++i
    ld   a,b
    ld   de,(__sfrend_render-8)   ; reload end_idx
    xor  d
    ld   d,a
    ld   a,c
    xor  e
    or   d
    ret  z

LBB0_4 (outer header):
    ld   hl,#_out_buf
    add  hl,bc
    ld   (__sfrend_render-2),hl   ; spill dp
    ld   hl,#_in_buf
    add  hl,bc
    ex   de,hl
    ld   (__sfrend_render-6),bc   ; <- BC SPILL (4 bytes)
    ld   l,c
    ld   h,b
    ld   c,l
    ld   a,(_inner_n)
    ld   b,a
    jr   .LBB0_2
```

The two BC <-> sframe slot-6 instructions are the spill pair.
Both encode as ED 43/4B nn nn = 4 bytes each, 20 T-states each.

## Predicted post-#114 codegen

```
LBB0_3 (outer.latch):
    exx                           ; <- 1 byte, restore main bank
    inc  bc
    ...exit-test...

LBB0_4 (outer header):
    ld   hl,#_out_buf
    add  hl,bc
    ld   (__sfrend_render-2),hl
    ld   hl,#_in_buf
    add  hl,bc
    ex   de,hl
    exx                           ; <- 1 byte, switch to shadow bank
    ld   l,c                      ; (these now operate on BC' — no more
    ld   h,b                      ;  conflict with the parked BC value)
    ld   c,l
    ld   a,(_inner_n)
    ld   b,a
    jr   .LBB0_2
```

Note: the EXX in LBB0_4 lands AFTER the `add hl,bc` uses BC (because
BC = i is needed there to compute dp/sp).  The EXX in LBB0_3 lands
BEFORE `inc bc` (so BC is back to the main bank, holding `i`).

## Savings

| Metric                   | Today | Post-#114 | Δ        |
|--------------------------|------:|----------:|---------:|
| Spill code (bytes)       |     8 |         2 |     **-6** |
| BSS slot (bytes)         |     2 |         0 |     **-2** |
| **Total static**         |  **10**|     **2** |    **-8** |
| Spill code (T-states/iter) |    40 |         8 |    **-32** |

Static byte win: 6 B of code + 2 B of BSS = **8 B per fired loop**.
Runtime: ~80 % faster per spill, executed once per outer iteration.

Per session-40 strand-B survey, the BIOS has 4 viable candidate
loops in this niche (`_specc`, `_scroll`, `_cursor_left`,
`_bios_conin`).  A 6-B-of-code-each-fires win across 4 sites would
shave roughly **24 B from rcbios bios.cim** plus reclaim 8 B of BSS
— tractable and matches the strand-B threshold for prototype
justification.

## Status

  - Synthetic C reproducer + lit fixture committed.
  - `_render` byte-precise disassembly captured above as the
    pre-transform baseline.
  - #114 prototype work itself is deferred — see
    `tasks/session40-strand-b-shadow-bank.md` for the
    `Z80ShadowBankBracket` MIR pass design.

When the prototype lands:
  1. Flip the lit CHECK lines from `ld (slot),bc` / `ld bc,(slot)`
     to `exx` / `exx`.
  2. Drop the `inc bc` between them (it must stay AFTER the second
     EXX so BC holds `i` again).
  3. Confirm `__sframe_render` shrinks by 2 bytes and rcbios
     bios.cim byte size drops on the four candidate functions.
