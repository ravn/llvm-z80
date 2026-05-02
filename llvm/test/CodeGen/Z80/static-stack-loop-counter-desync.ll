; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -mattr=+static-stack < %s | FileCheck %s
;
; Static-stack codegen bug history (ravn/llvm-z80#82, fixed 2026-05-02,
; refined 2026-05-02 as part of #74's cross-pair extension):
;
; A uint16_t loop counter held in a register pair (BC here) gets *also*
; assigned a static-stack slot, with the call-arg use site loading the
; byte from the slot via a different register pair (HL).  The original
; BSS-spill→PUSH/POP peephole converted only the spill+matching-pair
; reload; an orphan `LD HL,(slot)` was left reading from a never-written
; BSS slot (=0).
;
; The first fix (#82) bailed when an orphan reload to a different
; register pair was seen — keeping the slot store/load in BSS.  That
; was correct but conservative.  The cross-pair extension for #74
; converts the orphan TOO: PUSH (storeReg); POP (loadReg) preserves
; value bytes regardless of which 16-bit pair is used.
;
; Now the loop body is:
;
;   push bc     ; was: ld (slot),bc
;   pop  hl     ; was: ld hl,(slot)   ← orphan, now the cross-pair POP
;   push hl     ; re-PUSH so the next pop can grab it
;   ld a,l
;   call _take
;   pop  bc     ; was: ld bc,(slot)
;   inc bc
;   jr ...
;
; This is correct (LIFO preserved, BSS slot unused) AND smaller: 4 B of
; PUSH/POP vs 11 B of BSS store+two loads.

declare void @take(i8 zeroext)

define void @f() {
; CHECK-LABEL: _f:
; CHECK:       ld{{[ \t]+}}bc,#0
; The slot store should now be a PUSH BC, and both reloads should be
; POPs (with a re-PUSH between them so the second POP sees the value).
; CHECK:       push{{[ \t]+}}bc
; CHECK-NEXT:  pop{{[ \t]+}}hl
; CHECK-NEXT:  push{{[ \t]+}}hl
; CHECK:       call{{[ \t]+}}_take
; CHECK:       pop{{[ \t]+}}bc
; CHECK:       inc{{[ \t]+}}bc
; The BSS slot must be unused after conversion.
; CHECK-NOT:   ld{{[ \t]+}}({{[^,)]*}}__sfr{{[a-z_]*}}_f-2),bc
; CHECK-NOT:   ld{{[ \t]+}}hl,({{[^,)]*}}__sfr{{[a-z_]*}}_f-2)
; CHECK-NOT:   ld{{[ \t]+}}bc,({{[^,)]*}}__sfr{{[a-z_]*}}_f-2)
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %low = trunc i16 %i to i8
  call void @take(i8 zeroext %low)
  %i.next = add nuw nsw i16 %i, 1
  %done = icmp eq i16 %i.next, 1024
  br i1 %done, label %exit, label %loop

exit:
  ret void
}
