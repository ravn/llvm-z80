; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -mattr=+static-stack < %s | FileCheck %s
;
; Static-stack codegen bug (ravn/llvm-z80#82, fixed 2026-05-02):
;
; A uint16_t loop counter held in a register pair (BC here) is *also*
; assigned a static-stack slot, with the call-arg use site loading
; the byte from the slot via a different register pair (HL).  The
; BSS-spill → PUSH/POP peephole then converted the spill+matching
; reload to PUSH/POP without noticing the orphan `LD HL,(slot)`,
; leaving that load reading from a never-written BSS slot (=0).
;
; Buggy asm (pre-fix):
;
;   _f:
;       ld    bc,#0                  ; init i = 0 in BC
;   .LBB0_1:
;       ld    a,b
;       xor   #4
;       or    c
;       ret   z
;       push  bc                     ; was: ld (__sfrend_f-2),bc (spill)
;       ld    hl,(__sfrend_f-2)      ; ← BUG: orphan reload, slot is 0
;       ld    a,l
;       call  _take                  ; passes 0 every iteration
;       pop   bc                     ; was: ld bc,(__sfrend_f-2) (reload)
;       inc   bc
;       jr    .LBB0_1
;
; Fix: the BSS-spill peephole now bails when an intervening BSS load
; from the same slot targets a different register pair than the one
; being spilled.  The slot store stays in place; the orphan reload
; reads the live value.  Code is 2 B bigger for this shape but
; correct.

declare void @take(i8 zeroext)

define void @f() {
; CHECK-LABEL: _f:
; The slot must be stored to inside the loop before any reload from
; it.  The peephole now bails on the PUSH/POP rewrite when an orphan
; reload to a different register pair targets the same slot — so the
; spill `LD (slot),BC` survives.
;
; CHECK:       ld{{[ \t]+}}(__sfr{{[a-z_]*}}_f-2),bc
; CHECK-NEXT:  ld{{[ \t]+}}hl,(__sfr{{[a-z_]*}}_f-2)
; CHECK:       call{{[ \t]+}}_take
; CHECK:       ld{{[ \t]+}}bc,(__sfr{{[a-z_]*}}_f-2)
; CHECK:       inc{{[ \t]+}}bc
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
