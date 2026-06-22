# HLReg implementation attempt — structural conflict 2026-06-21

**Branch**: `3-pair-set-ldir-djnz`
**Issue**: ravn/llvm-z80#111 (HLReg for i16 self-loop pointer-arg).

**Outcome**: implementation as designed in the parked #115 sketch and
#111's issue body produces **zero codegen change** on the target XFAIL
test.  Reverted.  Documented the structural reason here so future-me
doesn't re-attempt the same design.

## What was tried

Per the planned Z phase:

1. Added `def HLReg : Z80Reg16Class<(add HL)>;` in
   `Z80RegisterInfo.td`, mirroring the existing `BCReg`.
2. Added `findPointerArgVReg` helper in `Z80SplitDjnzCounters.cpp`
   that finds the `%V:gr16 = COPY $hl` pattern in the entry block.
3. After the existing i16 BCReg constraint fires, called
   `splitCounterAt(MBB, Ptr, Z80::HLRegRegClass, MRI, TII)` to
   constrain the pointer vreg to HL.

Build clean, lit suite 151+5 XFAIL same as baseline.

## What happened on the XFAIL test (`issue-97a-bc-pingpong-i16-counter.ll`)

The constraint was applied correctly.  Pre-greedy MIR shows
`%20:hlreg = COPY %18:gr16` at the entry block, and the loop body
uses `%20:hlreg` for the pointer.

Greedy register allocator decided to **spill** `%20` rather than
allocate it to HL.  Post-greedy MIR shows `%21:gr16` and `%22:gr16`
(fresh vregs without the hlreg constraint) -- the constraint was
effectively dropped via spill+coalesce.

Post-RA asm: **byte-identical to the unpatched case**.  Pointer in
BC (entry `ld c,l; ld b,h`), counter in BSS, both per-iteration
spill/reloads as before.

## Root cause: in-loop physreg HL use conflicts with pointer

The loop body has this physical-register sequence (visible in the
pre-greedy MIR):

```
$hl = COPY %20:hlreg          ; load pointer from constraint into $hl
LD_HLind_E implicit $hl       ; store low byte via (HL)
INC_HL implicit-def $hl       ; ← physically modifies $hl
LD_HLind_D implicit $hl       ; store high byte via (HL+1)
%20:hlreg = INC16 %20:hlreg   ; logical pointer advance (one of two)
%20:hlreg = INC16 %20:hlreg   ; second logical advance (i16 stride = 2)
```

The `INC_HL` inside the i16-store sequence **physically modifies
$hl**, destroying the pointer value.  After `INC_HL`, the value in
$hl is `(pointer + 1)`, not the original pointer.

If greedy allocated `%20:hlreg` to `$hl` (as the constraint
requires), then `%20`'s live value would be **corrupted** by
`INC_HL` mid-loop.  Greedy correctly identifies this conflict and
declines to allocate `%20` to HL -- it spills to stack instead,
where `INC_HL` can't reach it.

So the HLReg constraint, as a single-register class, is
**structurally infeasible** for this loop shape: HL is required
to be both the long-lived pointer (constraint) AND the per-store
mutable temp (i16 store semantics).

## Why the i8 BReg path works but HLReg doesn't

BReg constrains the DJNZ counter vreg to B.  The DJNZ instruction
itself uses B (decrement-and-branch) but **doesn't require any
other physical use of B in the loop body**.  B can stably hold the
counter throughout; no `INC_B` or other B-clobbering op appears
between counter updates.

HLReg on a memory pointer is different: HL is the canonical
Z80 indirection register, and accessing memory via the pointer
requires HL to physically hold the address temporarily.  The
post-access `INC_HL` (part of the store-then-advance sequence)
destroys the value.

## What this rules out

The original #111 design ("HLReg single-register class for pointer-
arg in i16 self-loop") cannot work as-described.  The constraint
mechanism (single-register class + greedy honoring it) does NOT
solve this shape because greedy's spill decision is **correct
given the structural conflict**.

The parked #115 IY-extraction HLReg/DEReg design (from
`issue115-iy-unreserve-investigation-2026-06-21.md`) is also
affected wherever the constrained-to-HL vreg has a use site that
physically clobbers HL.  May still work for sites where HL is just
held as a value (not used as memory pointer) -- those would need
separate verification.

## What would actually fix the XFAIL

The right shape for the post-fix asm is:

```asm
; entry:
;   HL = pointer (delivered by sdcccall, no copy)
;   BC = counter (LD BC, 256)
; loop:
;   LD (HL), e          ; store low
;   INC HL              ; advance pointer
;   LD (HL), d          ; store high
;   INC HL              ; advance pointer (= original + 2, new pointer)
;   DEC BC              ; counter--
;   LD A, c; OR b       ; test counter
;   JR NZ, loop
```

The KEY insight: HL holds the pointer **and** the post-store
advance IS the pointer's new value.  No separate `%20 = INC16`
pseudo needed -- HL post-store IS the next-iteration pointer.

This is **idiom recognition** at the ISel level: the
"store-then-advance" pattern should be recognised and lowered
such that the same vreg flows through the entire sequence
without a separate logical-increment.

This is substantially more work than the HLReg approach
proposed.  It requires:

1. Identifying the `store(p); p += 2` (or stride) pattern in the
   loop body.
2. Lowering it as a single sequence where the pointer vreg's
   tied operand IS updated by the INC_HL inside the store
   sequence.
3. Eliminating the separate `%20 = INC16 %20` pseudos that
   compute the same advance.

Out of scope for this round.

## Status of #111 and #115

- **#111**: HLReg design ruled out for the i16 pointer-arg case.
  The actual fix requires ISel-level idiom recognition.
  Comment with this finding; consider reframing the issue as
  "store-then-advance idiom recognition for pointer-arg i16 loops"
  or close as superseded.
- **#115**: parked.  The HLReg/DEReg design sketch in
  `issue115-iy-unreserve-investigation-2026-06-21.md` is now
  flagged with this caveat: any HLReg-constrained vreg that's used
  as a memory base in the loop body will be spilled by greedy.
  The IY-extraction case (the #115 motivator) needs to be checked
  against this same structural conflict before any implementation
  attempt.

## What stays in tree (post-revert)

Nothing.  Both the .td addition and the pass change were reverted.

## What stays as durable output

- This writeup, as the record of "we tried, here's the structural
  reason it doesn't work, here's what would actually work."
- Updated commentary on #111 (will close or reframe).
- Updated commentary on the parked #115 writeup (HLReg has this
  caveat).

## Cross-references

- ravn/llvm-z80#111 -- the original HLReg-for-pointer-arg
  proposal (this writeup rules out the proposed design).
- ravn/llvm-z80#115 -- parked sister issue; same caveat applies.
- `llvm-z80/tasks/issue115-iy-unreserve-investigation-2026-06-21.md`
  -- to be updated with this caveat.
- ravn/llvm-z80#178 -- "Pseudos with implicit physreg outputs
  (e.g. ADD_HL_rr Defs=[HL]) systematically break rematerialization
  machinery."  Related: INC_HL inside the store sequence is the
  same class of problem -- a physreg side effect that interacts
  badly with vreg constraints.
- `llvm/test/CodeGen/Z80/issue-97a-bc-pingpong-i16-counter.ll` --
  stays as XFAIL; this writeup documents why the obvious fix
  doesn't apply.
