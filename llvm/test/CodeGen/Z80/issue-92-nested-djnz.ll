; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s

; Issue #92: in nested 8-bit countdown loops, the INNER loop counter must get
; B (DJNZ-eligible).  Before the fix, the outer counter took B and the inner
; emitted "dec r; jr nz" (3 extra bytes per inner iter).
;
; Distinguishing signal: the inner counter's dec/jr_nz lives in a self-looping
; MBB; the outer's dec/jr_nz lives in the latch MBB which branches to the
; outer header (a different MBB).

@port = external global ptr

; CHECK-LABEL: nested_djnz:
; CHECK: djnz [[INNER:\.[A-Za-z0-9_]+]]
; CHECK-NOT: dec{{[ \t]+}}b
; The outer is allowed to use any non-B 8-bit register (anti-hint cluster).
define void @nested_djnz(i8 zeroext %m, i8 zeroext %n) {
entry:
  br label %outer

outer:
  %o = phi i8 [ %m, %entry ], [ %o.next, %outer.latch ]
  br label %inner

inner:
  %i = phi i8 [ %n, %outer ], [ %i.next, %inner ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %i.cont = icmp ne i8 %i.next, 0
  br i1 %i.cont, label %inner, label %outer.latch

outer.latch:
  %o.next = add i8 %o, -1
  %o.cont = icmp ne i8 %o.next, 0
  br i1 %o.cont, label %outer, label %exit

exit:
  ret void
}

; A single self-looping countdown still gets DJNZ (no regression).
; CHECK-LABEL: single_djnz:
; CHECK: djnz
define void @single_djnz(i8 zeroext %n) {
entry:
  br label %loop

loop:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %cont = icmp ne i8 %i.next, 0
  br i1 %cont, label %loop, label %exit

exit:
  ret void
}
