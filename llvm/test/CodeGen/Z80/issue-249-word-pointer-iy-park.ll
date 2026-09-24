; RUN: llc -mtriple=z80 -O2 -disable-lsr < %s | FileCheck %s
; RUN: llc -mtriple=z80 -O2 -disable-lsr \
; RUN:   -z80-enable-keep-loop-pointer-in-pair < %s | FileCheck %s
;
; ravn/llvm-z80#249 / #251: a `*p++ = i` i16 store loop must not park the
; loop-carried pointer in IY (which would shuttle it IY<->BC<->HL with push/pop
; pairs per iteration). The pointer must walk in main register pairs {DE, HL, BC}.
;
; CHECK-LABEL: _f:
; CHECK-NOT: iy
; CHECK-NOT: push
; CHECK-NOT: pop
; CHECK:     ld (hl),e
; CHECK:     inc hl
; CHECK:     ld (hl),d
; CHECK:     jr
; C source:
;   // ravn/llvm-z80#249/#251: a *p++=i i16-store loop must not park the
;   // walking pointer in IY (shuttle IY<->BC<->HL with push/pop per iteration).
;   // The pointer must stay in main GR16 pairs {BC,DE,HL}.
;   //
;   void fill_i16(uint16_t *p, uint16_t n) {
;       for (uint16_t i = n; i != 0; i--) *p++ = i;
;   }
define dso_local void @f(ptr noundef captures(address) %p, i16 noundef %n) #0 {
entry:
  br label %loop

loop:
  %ptr = phi ptr [ %p, %entry ], [ %ptr.next, %body ]
  %i = phi i16 [ %n, %entry ], [ %i.next, %body ]
  %done = icmp eq i16 %i, 0
  br i1 %done, label %exit, label %body

exit:
  ret void

body:
  %ptr.next = getelementptr inbounds nuw i8, ptr %ptr, i16 2
  store volatile i16 %i, ptr %ptr, align 1
  %i.next = add i16 %i, -1
  br label %loop
}

; minsize/optsize are what shift greedy's copy/spill weighting into parking the
; pointer in IY (the bug only appears at -Oz/-Os, not -O2 without minsize).
attributes #0 = { minsize optsize nounwind "target-features"="+static-frame,+z80" }
