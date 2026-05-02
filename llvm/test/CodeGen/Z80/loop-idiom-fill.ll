; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s

; Issue #88: pattern-fill loops with constant trip count get rewritten
; in the IR as `seed K bytes; memcpy(base+K, base, K*(N-1))`, which the
; backend then lowers as `seed; LDIR`.  K=1 (memset shape), K=2 (word
; fill), K=3 (jump-table / IVT shape), K=4 are all in scope.

@buf1 = external dso_local global [32 x i8]
@buf2 = external dso_local global [16 x i16]
@ivt  = external dso_local global [16 x [3 x i8]]

; --- 1-byte pattern (memset shape) ----------------------------------
define void @fill_byte() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %i16 = zext i8 %i to i16
  %p = getelementptr inbounds nuw [32 x i8], ptr @buf1, i16 0, i16 %i16
  store i8 -1, ptr %p, align 1
  %i.next = add i8 %i, 1
  %done = icmp eq i8 %i.next, 32
  br i1 %done, label %exit, label %loop
exit:
  ret void
}
; CHECK-LABEL: _fill_byte:
; CHECK-NOT:  djnz
; CHECK-NOT:  jr {{.LBB}}
; CHECK:      ld  hl,_buf1
; CHECK:      ld  (hl),
; CHECK:      ld  bc,31
; CHECK-NEXT: ldir
; CHECK-NEXT: ret


; --- 2-byte pattern (word-fill) -------------------------------------
define void @fill_word() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %i16 = zext i8 %i to i16
  %p = getelementptr inbounds nuw [16 x i16], ptr @buf2, i16 0, i16 %i16
  store i16 -13570, ptr %p, align 1
  %i.next = add i8 %i, 1
  %done = icmp eq i8 %i.next, 16
  br i1 %done, label %exit, label %loop
exit:
  ret void
}
; CHECK-LABEL: _fill_word:
; CHECK-NOT:  djnz
; CHECK:      ld  ({{_buf2.*}}),hl
; CHECK:      ld  bc,30
; CHECK-NEXT: ldir
; CHECK-NEXT: ret


; --- 3-byte pattern (jump-table / IVT shape) ------------------------
define void @fill_ivt() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %i16 = zext i8 %i to i16
  %p0 = getelementptr inbounds nuw [16 x [3 x i8]], ptr @ivt, i16 0, i16 %i16, i16 0
  %p1 = getelementptr inbounds nuw [16 x [3 x i8]], ptr @ivt, i16 0, i16 %i16, i16 1
  %p2 = getelementptr inbounds nuw [16 x [3 x i8]], ptr @ivt, i16 0, i16 %i16, i16 2
  store i8 -61, ptr %p0, align 1   ; opcode 0xC3 (JP nn)
  store i8 0,   ptr %p1, align 1   ; lo(default)
  store i8 -13, ptr %p2, align 1   ; hi(default) = 0xF3
  %i.next = add i8 %i, 1
  %done = icmp eq i8 %i.next, 16
  br i1 %done, label %exit, label %loop
exit:
  ret void
}
; CHECK-LABEL: _fill_ivt:
; CHECK-NOT:  djnz
; CHECK:      ld  (_ivt),
; CHECK:      ld  (_ivt+1),
; CHECK:      ld  (_ivt+2),
; CHECK:      ld  bc,45
; CHECK-NEXT: ldir
; CHECK-NEXT: ret


; --- Negative: volatile stores must NOT be rewritten ----------------
define void @fill_volatile_noop() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %i16 = zext i8 %i to i16
  %p = getelementptr inbounds nuw [32 x i8], ptr @buf1, i16 0, i16 %i16
  store volatile i8 -1, ptr %p, align 1
  %i.next = add i8 %i, 1
  %done = icmp eq i8 %i.next, 32
  br i1 %done, label %exit, label %loop
exit:
  ret void
}
; CHECK-LABEL: _fill_volatile_noop:
; CHECK-NOT:  ldir
; CHECK:      ret
