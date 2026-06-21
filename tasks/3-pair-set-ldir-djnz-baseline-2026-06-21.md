# 3-pair-set LDIR/DJNZ codegen baseline — 2026-06-21

**Branch**: `3-pair-set-ldir-djnz`

**Goal (Phase A)**: empirically check whether the canonical Z80 idioms
(LDIR / LDDR / CPIR / CPDR / DJNZ) get the 3-pair register set
(BC / DE / HL plus B for DJNZ) allocated correctly today on the
production triplet, and categorise any gaps.

**Outcome (Phase A)**: **production is clean**.  All 23 LDIR family
sites and all 6 DJNZ sites on the three production binaries have
correct register allocation.  No extra COPYs, no PUSH/POP extractions,
no missed-DJNZ candidates found.

This is a **surprise finding** that mirrors the #232 stale-Makefile
discovery: the user's framing (the 3-pair set might be allocated
wrong, motivating regalloc work) doesn't match current production
state.

## Methodology

Disassembled the three production ELFs from #232's investigation
(`autoload-baseline.elf`, `cpnos-baseline.elf`, `rcbios-baseline.elf`,
all in `/tmp/issue232/`).  Searched for `ldir`/`lddr`/`cpir`/`cpdr`
and `djnz` occurrences.  For each occurrence, extracted the 6 lines
of context preceding the idiom + 1 line following.  Categorised the
codegen shape by hand against the canonical pattern.

Cross-checked for missed-DJNZ candidates by searching for `dec
[acdehl]; jr nz, label` (8-bit countdown loops using a non-B counter)
and `dec (bc|de|hl); jr nz, label` (16-bit countdowns potentially
narrowable).

## LDIR family — 23/23 sites clean

### Per-target counts

| Target | LDIR | LDDR | CPIR | CPDR | Total |
|--------|------|------|------|------|-------|
| autoload | 10 | 0 | 0 | 0 | 10 |
| cpnos PROM1 | 2 | 0 | 0 | 0 | 2 |
| rcbios | 10 | 1 | 0 | 0 | 11 |
| **Aggregate** | **22** | **1** | **0** | **0** | **23** |

### Canonical pattern (and the only pattern seen)

Every site emits the canonical sequence:

```
ld   hl, <src>      ; LD HL,nn -- 3 B
ld   de, <dst>      ; LD DE,nn -- 3 B
ld   bc, <count>    ; LD BC,nn -- 3 B
ldir                ; ED B0   -- 2 B
```

When HL/DE/BC arrive via calling convention (ZX0 decompressor at
`dzx0s_literals` / `dzx0s_copy` in autoload + cpnos), the operands are
already in the right registers and no further loads are needed.

Several sites prefix the LDIR with the size-zero guard from #105:

```
ld   a, b
or   c
jr   z, .skip       ; skip if BC == 0 (else LDIR loops 65536 times)
ldir
.skip:
```

That's correctness machinery (gating LDIR against the 0 → 65536-iteration
bug), not a regalloc artefact.  4 B per guarded site.  Both autoload
sites at lines 24 and 1087 use this guard; rcbios sites at lines 95,
112, 142, 149 use it.

The one LDDR site (rcbios `_lddr_copy` at line 118) has HL/DE/BC
delivered via caller-stack-and-trampoline pattern -- still clean.

### Conclusion

**The 3-pair set is allocated optimally for LDIR/LDDR on all 23
production sites today.**  No COPY-into-physreg overhead, no
PUSH-IY/POP-HL extraction, no failed hints.

## DJNZ — 6/6 sites clean, 0 missed-opportunity candidates

### Per-target counts

| Target | DJNZ sites | Pattern |
|--------|-----------|---------|
| autoload | 2 | both variable-shift loops |
| cpnos PROM1 | 0 | -- |
| rcbios | 4 | all four variable-shift loops |
| **Aggregate** | **6** | all 6 = variable-shift loops |

### Pattern

Every DJNZ site implements a variable-width right-shift via DJNZ as
the shift counter.  Canonical shape:

```
ld   b, <count>     ; B = shift count
inc  b              ; ┐ zero-trip guard:
dec  b              ; │ flags Z if B was 0
jr   z, .skip       ; ┘ skip the loop body
.body:
srl  h              ; right-shift body (variable per site)
rr   l
djnz .body
.skip:
```

The 4-byte `inc b; dec b; jr z` guard is necessary because DJNZ
with B=0 loops 256 times instead of 0.  IndVarSimplify could
sometimes prove B > 0 and elide the guard, but that's not the
3-pair-set question.

### Searched for missed-DJNZ candidates: zero found

- `dec [acdehl]; jr nz, label` (8-bit countdown using a non-B
  counter that should DJNZ): **0 production candidates** found by
  pattern search.  One near-miss in rcbios at line 744 (`dec a; jr
  nz, .label`) is a one-shot `if` test (forward branch), not a loop --
  that's the #222 "DJNZ as ==1 test" territory, off the 3-pair-set
  scope.
- `dec (bc|de|hl); jr nz, label` (16-bit countdown potentially
  narrowable): **0 production candidates**.

### Conclusion

**DJNZ already fires on every production loop where it can.**  The
`Z80SplitDjnzCounters` pass + the existing pre-RA BReg/BCReg
machinery is doing its job.

## Surprise interpretation

The user's request -- "make LLVM allocate the correct registers so
memcpy maps directly to LDIR and B is used as a <255 counter by
default" -- is **already satisfied on production today**.

Mechanism:

- For LDIR: GISel emits the `LD HL,nn; LD DE,nn; LD BC,nn; LDIR`
  sequence directly (the source vregs are physreg-allocated by the
  COPY-into-physreg pseudo expansion).  No COPY-elimination heuristic
  has anything to override.
- For DJNZ: `Z80SplitDjnzCounters` + BReg/BCReg single-register classes
  catch every shape that fires on production today.  Greedy can't
  override the class constraint (this is the #110 workaround that
  works correctly here).

This is the **inverse of the #232 finding**: where #232 found that
production needed `-disable-lsr` for a reason outside the cost model
(ZX0 compressibility), Phase A finds that production **doesn't need
the proposed 3-pair-set fix** because the fix is effectively already
in tree.

## What's left in the original scope

Three buckets from the original plan:

- **(P1) LDIR source-vreg constraints** -- **moot today on production**.
  LDIR's HL/DE/BC come from direct immediate loads, not from vregs that
  needed constraint.  When LDIR receives vregs (the ZX0 calling
  convention sites), they're already in HL/DE/BC by ABI.  Cases where
  this could matter (LDIR with vreg sources that aren't naturally in
  HL/DE/BC) **don't appear in production today**.
- **(P2) DJNZ detector coverage** -- **moot today on production**.
  Every loop that could DJNZ does DJNZ.  No missed-opportunity loops
  found.
- **(P3) #110 hint priority generic-fix** -- **still open in principle**
  but the workaround (single-register classes) is functionally
  complete for the production codebase.

## Where the gap might still live (out of Phase A scope)

Production is one workload.  There are several workloads where 3-pair
allocation might still misbehave:

1. **AES-256 corpus** (`rc700-gensmedet/tasks/aes256-corpus`).  Per
   CLAUDE.md, AES is the main testbed for LSR-active configs and has
   shown #177 wins (-8..-124 B at LSR-active).  Has AES's LDIR/DJNZ
   regalloc been checked separately?  Possibly some misallocations
   that don't appear in production because production disables LSR.
2. **compiler-comparison-corpus** (`rc700-gensmedet/tasks/compiler-
   comparison-corpus`).  General C benchmarks (pi, sieve, fannkuch,
   gf_log, etc.).  Different optimization mix; possibly different
   regalloc behaviour.
3. **lit tests** (`llvm/test/CodeGen/Z80/`).  Some are XFAIL'd
   because regalloc misallocates -- e.g. `issue-97a-bc-pingpong-i16-
   counter.ll` (referenced by #111).  These known gaps are documented
   but don't ship to production.

## Recommendation

The original 3-phase plan (Phase A audit + Phase B inventory + Phase C
fix) was designed for a world where production had gaps to close.
**It doesn't.**  Two honest options:

### Option 1: Close the 3-pair-set work as already-done on production

Document Phase A's finding, retire the branch, mark the work as
"production-clean."  Any future LDIR/DJNZ regression would surface as
a specific issue and get addressed individually.

Cost: this writeup + a comment on #7 (the meta tracker) noting the
production state.  No further work this round.

### Option 2: Widen scope to AES corpus + compiler-comparison-corpus

Re-run the same Phase A audit on the AES corpus binaries (which build
with LSR active) and the compiler-comparison-corpus.  Find LDIR/DJNZ
sites; categorise; identify gaps that don't appear on production
because of `-disable-lsr` etc.

Cost: ~60 minutes for the audit; another 30 for the writeup.

### Option 3: Close known XFAIL tests as the gap targets

Look at the lit-test XFAILs that relate to 3-pair-set allocation
(e.g. `issue-97a-bc-pingpong-i16-counter.ll`).  Pick the highest-
leverage one (probably the one that would fix #111 -- HLReg for
i16-counter pointer-arg in self-loops).  Implement HLReg per #111's
proposal (which is the same class as #115's HLReg, just for a
different consumer).

Cost: ~1-2 sessions for the actual implementation; immediate
deliverable is closing #111 and dropping the XFAIL.

**Recommend Option 1 + Option 3 in sequence**:

- First, document the surprise finding (Phase A complete; production is
  clean).  Commit to the branch.
- Then, consider Option 3 as the next concrete delivery once you've
  digested Phase A.  Option 3's HLReg implementation closes #111 and
  partially closes #115 (the parked one) -- so the HLReg infrastructure
  arrives, with both motivating consumers ready to opt in.

## Decision point for Phase C

Pause here.  The Phase A finding fundamentally changes the framing:
production is fine.  The original plan's Phase D (implement the top
gap) was conditional on Phase A finding gaps; it didn't.

I'll wait for direction:

- Accept Option 1 (close as already-done) and we're done.
- Accept Option 2 (widen to AES / comparison corpus) and I continue
  the empirical work.
- Accept Option 3 (jump to HLReg for #111 / partial-#115) and we begin
  TableGen + ISel work test-first.
- Other direction -- let me know.

## Files produced (Phase A)

Under `/tmp/3-pair-set/`:
- `autoload-baseline.disas.s` -- disassembled production ELF.
- `cpnos-baseline.disas.s`
- `rcbios-baseline.disas.s`

These are transient; can be regenerated with:

```bash
build-macos/bin/llvm-objdump -d --triple=z80 <elf> > <output>.s
```

## Cross-references

- ravn/llvm-z80#7 -- the meta tracker; production status confirms the
  existing implementation matches the goal.
- ravn/llvm-z80#111 -- HLReg for i16-counter pointer-arg (Option 3).
- ravn/llvm-z80#115 -- HLReg/DEReg for IY-extraction (parked; same
  HLReg infrastructure).
- ravn/llvm-z80#110 -- greedy heuristic; workaround machinery is
  effective on production.
- ravn/llvm-z80#94 / #98 / #99 / #112 -- the BReg / BCReg / GR16NoIR
  precedents that made the production state clean.
- `llvm-z80/tasks/issue115-iy-unreserve-investigation-2026-06-21.md`
  -- the parked HLReg/DEReg design sketch (re-usable for Option 3).
- `llvm-z80/tasks/issue232-lsr-sledgehammer-investigation-2026-06-21.md`
  -- the parallel "Makefile claim was stale" finding from earlier today.
