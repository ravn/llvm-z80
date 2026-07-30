; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -disable-lsr < %s \
; RUN:   | FileCheck %s --check-prefix=OFF
; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -disable-lsr \
; RUN:   -z80-enable-keep-loop-pointer-in-pair < %s | FileCheck %s --check-prefix=ON
;
; ravn/llvm-z80#249 / #251: a `*p++ = i` i16 store loop parks the loop-carried
; pointer in IY and shuttles it IY<->BC<->HL with three push/pop pairs per
; iteration (the pointer never needs an index register -- greedy just puts it
; there).  Z80KeepLoopPointerInPair constrains the pointer + advanced-next vreg
; to GR16NoIR {DE,HL,BC}; the shuttle collapses to one cheap `ld l,c; ld h,b`
; copy into HL for the store.  Default OFF (corpus-only; B20).
;
; Sibling of #250 (Z80PinLoopPointer): there the byte store lets the pointer LIVE
; in HL; here the 2-byte store walks HL, so pinning to HL is impossible -- the
; correct constraint is "any main pair, just not IX/IY".

; --- Default (flag off): pointer parked in IY, per-iteration push/pop shuttle.
; OFF-LABEL: f:
; OFF: push iy
; OFF: pop iy

; --- Flag on: no index register anywhere; pointer walks a main pair.
; ON-LABEL: f:
; ON-NOT: iy
; ON-NOT: push
; ON-NOT: pop
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
attributes #0 = { minsize optsize nounwind "target-features"="+static-stack,+z80" }
