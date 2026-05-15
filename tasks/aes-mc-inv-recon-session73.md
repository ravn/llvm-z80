# aes_mc_inv reconnaissance (post-#165, session 73)

Date: 2026-05-15.  Recon for the regalloc-cluster work after the AES
corpus settled post-#165.

## Sizes at production config (`09_Oz_prod_like` vs SDCC `01_baseline_prod`)

| Function | clang | sdcc | delta |
|---|---:|---:|---:|
| `aes_mc_inv` | 535 | 314 | **+221 B** |
| `aes_mixColumns` | 332 | 241 | +91 |
| `rj_sb_inv` | 121 | 30 | +91 |
| `aes_ar_cpy` | 120 | 98 | +22 |
| `gf_log` | 50 | 32 | +18 |

(`aes_mc_inv` is the single largest clang-loses function in the corpus.)

But the **bin total** is clang-favorable: 2695 B (clang) vs 3604 B
(SDCC) at production config — clang wins by 909 B overall, mainly
on `aes_done`, `aes_expDecKey`, `aes256_init`, `aes_addRoundKey`,
`aes_subBytes`.

## Profile of clang aes_mc_inv (535 B, 311 instructions)

| Metric | clang | sdcc | delta |
|---|---:|---:|---:|
| Total bytes | 535 | 314 | +221 |
| Total instructions | 311 | 172 | +139 |
| 8-bit A spill+reload | 32 ops (× 3 B = 96 B) | (all 8-bit via IX-indexed) | similar |
| 16-bit HL spill+reload | 22 ops (× 3 B = 66 B) | **0** | **+66 B** |
| 16-bit DE/BC spill+reload | 19 ops (× 4 B = 76 B) | **0** | **+76 B** |
| Reg-to-reg 8-bit copies | **90** | **14** | **+76 B** (1 B each) |
| Pair-split 16-bit copies | 8 sequences | 0 | +16 B |
| PUSH/POP | 28 | 53 | −25 B (sdcc uses push for call args) |

## Cost decomposition

The +221 B gap decomposes (approximately) as:

| Cause | Bytes |
|---|---:|
| 16-bit spills (HL+DE+BC) clang has, SDCC doesn't | +142 |
| Extra reg-to-reg copies (90 vs 14) | +76 |
| Pair-split 16-bit copies | +16 |
| Other (push/pop balance, prologue, branches) | −13 |
| **Net** | **+221** |

## Would IX-indexed addressing help?

**No — it would make aes_mc_inv ~100 B LARGER.**

Clang currently uses direct BSS addressing (`+static-stack`).  IX-
indexed would change cost per access:

| Access | BSS direct | IX-indexed | Δ |
|---|---:|---:|---:|
| 8-bit (any reg) | 3 B (A only; +1 for non-A move) | 3 B (any reg) | −0 to −2 |
| 16-bit HL | 3 B | 6 B (2× LD (IX+d),r) | +3 |
| 16-bit DE/BC | 4 B (ED prefix) | 6 B | +2 |

For clang's 41 16-bit spills, IX-indexed adds ~100 B.  For the 32
8-bit accesses, it would save 0-2 B per access.  Net: worse.

SDCC's IX-indexed addressing works for it because **SDCC doesn't
spill 16-bit values** — its iCode allocator keeps 16-bit pointers
in BC/DE/HL across the whole basic block.  Only 8-bit intermediates
go to memory.  The addressing mode is a symptom of that allocator
behavior, not the cause of the size win.

## Real fix surface

1. **Reduce 16-bit spills** (~142 B savings ceiling) — keep `aes_mc_inv`'s
   pointer arithmetic in BC/DE/HL across the GF arithmetic blocks.
   Requires regalloc cost-model work: probably hint pointer types to
   register pairs and increase the cost of spilling them.  Maps onto
   open issue **#115** (regalloc heuristics gap) and **#27** (per-pair
   copy cost).

2. **Reduce A-shuttling** (~76 B savings ceiling) — 70% of clang's
   90 reg copies involve A (move-to-A for arithmetic, move-from-A
   for storage).  Top patterns: 13× `ld d,a`, 11× `ld a,l`, 9× `ld l,a`,
   9× `ld a,c`, 8× `ld a,b`.  SDCC's iCode allocator avoids this by
   structuring computation chains in A.  Would need hint-A-for-binop-
   results allocator changes.

3. **Pair-split avoidance** (~16 B) — 4× `BC→HL` (as `ld l,c; ld h,b`),
   2× `HL→DE`.  Could use `EX DE,HL` (1 B) for the HL↔DE cases when
   source is dead.  Smaller win.

## Recommendation

The largest single lever is **#115/#27** (regalloc cost model so 16-bit
pointer pairs stay in registers).  That's a multi-session
investigation, not a peephole fix.

Given that the **AES corpus is already clang-favorable by 909 B**
overall, this gap-closure work has lower urgency than the user
might expect.  Possible alternative directions:

- Park aes_mc_inv until upstream regalloc work matures
- Focus on `aes_mixColumns` (similar shape, smaller gap, possibly
  same fixes)
- Pivot to non-AES work (engagement-mode gate per the roadmap)

## Files

Reconnaissance scripts: ad-hoc shell+python in session.
Disassemblies: `/tmp/aes_mc_inv_clang.s`, `/tmp/aes_mc_inv_sdcc.s`
(local, not committed).
