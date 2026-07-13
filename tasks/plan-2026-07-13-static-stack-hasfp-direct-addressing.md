# Plan — static-stack + hasFP: local frame slots use slow IX-relative addressing instead of direct absolute addressing

Date: 2026-07-13. Owner: (unassigned). Tracks the register/BSS-spill investigation
requested after retitling #244 (the `e` benchmark BSS-spill gap).

## 1. Root cause (VERIFIED this session)

Under `+static-stack`, a local frame slot lives at the **link-time-constant** address
`__sfrend_<fn> + offset`. `eliminateFrameIndex` already knows how to emit **direct
absolute addressing** for these — `ld (__sfrend_f-4),de` (SPILL_GR16), `ld de,(nn)`
(RELOAD_GR16), `ld hl,__sfrend_f-4` (LEA_IX_FI) — via the `addBSSAddr` helper. BUT
that whole direct-BSS block is gated `if (STI2.staticStack() && !UseFP)`
(`Z80RegisterInfo.cpp:1480`).

`Z80FrameLowering::hasFPImpl` returns **true** for any function containing an
`alloca` (`Z80FrameLowering.cpp:88-90`). A fixed-size C local array — `int a[200]`
in `dcc/tests/e.c` — lowers to `alloca [200 x i32]` and survives to -O2, so
`hasFP=true`, so `UseFP=true`, so the direct-BSS path is **skipped**. Every frame
access then degrades to the IX-frame form:

```
; MIR after prologepilog (llc -print-after=prologepilog on near_e.ll):
LD_IX_nn __sfrend_f            ; prologue: IX = frame base (a CONSTANT)
...
PUSH_IX; POP_HL; LD_DE_nn off; ADD_HL_DE; LD_HLind_C; INC_HL; LD_HLind_B
```

A later peephole rematerializes `PUSH_IX;POP_HL` back to `ld hl,__sfrend_f`
(constant reload), so the emitted asm per 16-bit access is:

```
ld hl,__sfrend_f   ; 10T
ld de,off          ; 10T
add hl,de          ; 11T
ld e,(hl)/ld (hl),e; 7T
inc hl             ; 6T
ld d,(hl)/ld (hl),d; 7T   = 51T   (store or load)
```

Direct absolute addressing of the SAME constant address would be **one** instruction:
`ld de,(__sfrend_f+off)` / `ld (__sfrend_f+off),de` (ED 5B/53 nn nn) = **20T**, or
`ld hl,(nn)` / `ld (nn),hl` (2A/22 nn nn) = 16T. So ~51T -> ~20T, **~60% per access**,
and a **size** win too (~9 B -> 4 B) which helps -Oz production.

### Known vs guessed
- KNOWN: alloca -> hasFP=true -> UseFP path -> 51T/access; a no-array volatile repro
  gets hasFP=false -> direct `ld (nn),de`. Verified via `-print-after=prologepilog`
  MIR + emitted asm (`/tmp/near_e.*`, `/tmp/fixed.*`).
- KNOWN: `IX = __sfrend_f` is a link-time constant; every `IX+off` local access is a
  constant address; direct-addressing opcodes for HL/DE/BC all exist and are already
  used by the `!UseFP` path.
- GUESSED (must verify): that redirecting the constant-base LOCAL accesses to direct
  addressing while leaving IX otherwise untouched is (a) correct, (b) net-positive on
  the corpus + production, (c) does NOT trigger the B2 static-stack runtime hang.

## 2. Why this is distinct from parked work

- **B2 / #12 (hasFP=false hangs the PROM)** is about making IX **allocatable** (freeing
  it). This lever does NOT free IX — IX stays reserved and still loaded with
  `__sfrend_f` at runtime; we only stop *using* it for constant-address local accesses,
  emitting direct absolute addressing instead. The runtime IX state is unchanged, so
  the B2 timing/ISR hang should not be reachable. (Must still MAME-verify — listed
  below.)
- **M2 (BSS traffic ISA-fundamental)** is about 8-bit A-only memory access. This is a
  16-bit-addressing-mode miss, orthogonal, and NOT ISA-fundamental — the instruction
  exists, it just isn't selected under hasFP.

## 3. The lever (two candidate implementations)

### Option A (RECOMMENDED first — lowest risk): post-RA peephole
Add a peephole in `Z80LateOptimization.cpp` that folds the constant-base access:

```
ld hl,__sfrend_f ; ld de,off ; add hl,de ; ld (hl),lo ; inc hl ; ld (hl),hi
    ->  ld (__sfrend_f+off), de              (LD_nnind_DE / _BC / _HL)
ld hl,__sfrend_f ; ld de,off ; add hl,de ; ld lo,(hl) ; inc hl ; ld hi,(hl)
    ->  ld de,(__sfrend_f+off)               (LD_DE_nnind / _BC_ / _HL_)
```

Guards: HL must be dead after the sequence (it is a throwaway address here); the
value pair must be BC/DE/HL (all have nnind forms); flags liveness (the `add hl,de`
sets flags — confirm no consumer, or the direct form preserves them, which it does).
Pattern-local, easy to gate behind a flag, easy to A/B, no offset-math or IX-liveness
reasoning. Downside: relies on the exact rematerialized shape; may miss variants.

### Option B (more thorough): enter the direct-BSS path under UseFP
In `eliminateFrameIndex`, allow the `staticStack()` direct-BSS emission when `UseFP`
AND the object is a **local BSS slot** (constant `__sfrend_<fn>+off` base), keeping IX
for **stack-argument / fixed** objects only (those need the real runtime IX/SP base).
Requires: split the offset formula (static-stack local: `-= getOffsetOfLocalArea();
+= CalleeSavedFrameSize`, NOT the `UseFP` `+= 2`) per object kind; confirm no other
reader of IX depends on the removed accesses. Higher blast radius; do only if Option A
leaves measurable gaps.

## 4. Implementation + validation steps (red-green, per project discipline)

1. **Baseline first.** Capture unmodified: corpus sweep (all lanes, SIZE+SPEED) and
   the `e` T-states (`dcc/scripts/compare3.sh e` or the corpus harness), plus rcbios /
   cpnos / autoload byte sizes. Record the fail-set / sizes BEFORE any change.
2. **Red test.** Add a lit test (`llvm/test/CodeGen/Z80/static-stack-array-direct-addr.ll`)
   from `near_e`-shape IR (a static-stack function with a local array + i16 loop
   scalars) with FileCheck lines asserting `ld de,(` / `ld (`+`),de` direct forms and
   `CHECK-NOT: add\thl,de` in the inner loop. Confirm it FAILS on current HEAD.
3. **Implement Option A** behind `-z80-static-stack-direct-addr` (default OFF first).
4. **Green.** Lit test passes; run the full Z80 lit suite (must stay PASS/XFAIL clean)
   and the test-runner runtime oracle (correctness).
5. **Measure.** Corpus sweep: expect `e` big speed win, sieve/others neutral-or-better,
   production (rcbios/cpnos/autoload) SIZE neutral-or-smaller and byte-diff reviewed.
6. **B2-hang gate.** Build autoload PROM + `make floppy-boot-test` / `make sw1-test`;
   build cpnos prom1-lineprog + `polypascal-test`; boot BIOS in MAME. Any hang => stop,
   the IX-untouched hypothesis is wrong.
7. **Default-on decision** only after 5+6 are clean across the corpus + all three
   production targets. Add a runtime fixture if correctness is only observable at
   runtime.
8. **Speed-check protocol.** Re-run `tasks/tools/m5-loop-reload-scan.py` (M5) after the
   change to confirm no new GLOBAL-BASE reloads were introduced.

## 5. Expected impact (estimate, not yet measured)

`e` inner loop: 6-8 16-bit accesses/iter x ~31T saved = ~200-250T/iter. `e` gap vs dcc
is currently +50-65% and is dominated by exactly this access cost, so this lever is the
primary candidate to close most of the `e` gap. Production -Oz: a size win wherever a
function with a local array/alloca exists (BIOS `_specc`, `_rwoper`, `_isr_crt` etc.
carry large BSS locals) — measure, don't assume.

## 6. Filing

Per user rule (issues not fixes; own repo, no permission needed): file a tracker issue
on ravn/llvm-z80 describing the mechanism (alloca->hasFP->slow constant-base
addressing) with the `near_e` repro + the 51T-vs-20T comparison, cross-linked to #244
(symptom) and B2/#12 (the distinct hasFP-free hang). Keep #244 as the `e`-witness.

## 7. Pointers

- `llvm/lib/Target/Z80/Z80RegisterInfo.cpp:1480` (`staticStack() && !UseFP` gate),
  :1557-1600 (SPILL/RELOAD_GR16 direct-BSS emission via `addBSSAddr`).
- `llvm/lib/Target/Z80/Z80FrameLowering.cpp:88-90` (alloca -> hasFP=true).
- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` (peephole home for Option A).
- Repro: `/tmp/near_e.c` (array + i16 loop, slow form), `/tmp/fixed.c` (no array,
  fast direct form) — regenerate with the corpus `+static-stack -O2` flags.
- Symptom tracker: #244. Distinct parked hang: B2 / #12.
- known-suboptimal-codegen.md: add a Bnn entry once filed.

## 8. Outcome (2026-07-13, branch ravn/static-stack-hasfp-direct-addr)

IMPLEMENTED as the eliminateFrameIndex broadening (not a peephole): when the
UseStaticFrame invariant holds (IX == __sfrend_<fn>), the fixed-offset accesses
take the existing direct-BSS emission path with the identical displacement, so
the absolute address is byte-identical to the IX-relative access. Gated behind
`-z80-static-stack-fp-direct-addr` (Hidden, default OFF).

The runtime oracle (clang suite, 912 cases) FALSIFIED the initial
"byte-identical by construction is sufficient" assumption: with the flag ON it
flagged 4 miscompiles, which localised TWO pre-existing latent backend bugs the
lever merely makes common (both now fixed unconditionally, red-green):

  1. Z80LateOptimization RMW->bit-set peephole: `addSym(sym, getOffset())` --
     addSym's 2nd arg is TargetFlags, so a non-zero MCSymbol offset
     (__sfrend_f-3) was dropped -> wrong-address write. Fixed by setting the
     offset on the operand. Test: late-opt-bitset-mcsymbol-offset.ll.
  2. Z80PostRACompareMerge::setsZForA treated POP_AF as Z-reflects-A, deleting a
     needed `or a` after a reload-via-A that preserved A with push af/pop af.
     POP_AF restores SAVED flags, not flags-of-A. Fixed by excluding POP_AF.
     Test: postra-compare-merge-pop-af.mir.

Validation (all green):
  - Full Z80 lit: 196 PASS + 5 XFAIL (3 new tests incl. the #263 feature test).
  - clang runtime oracle: 912/0 with the flag ON AND with it OFF (default).
  - Corpus sweep: byte-identical to baseline (inert -- corpus uses file-scope
    arrays -> hasFP=false).
  - Production density (unconditional fixes, flag OFF): cpnos prom1 clang
    2014/2048 B, payload 1986/1384 -- byte-identical to baseline. Fixes never
    fire in shipping code.
  - Address identity: __sfrend_f+65334 == -202 mod 2^16 == ld (__sfrend_f-202).
  - `e`-shape proxy: 43 add-hl -> 7, ~57% fewer asm lines with the flag on.

Runtime-oracle A/B hook added: test-runner `Z80_TR_EXTRA_MLLVM` env var injects
extra -mllvm flags into the clang suite (commit).

REMAINING before default-ON (separate user decision): MAME B2/#12 hang gate with
the flag enabled on autoload/cpnos/BIOS; a production-density measurement of the
WIN (build production WITH the flag) since production functions with local
arrays/allocas are where the lever pays off.
