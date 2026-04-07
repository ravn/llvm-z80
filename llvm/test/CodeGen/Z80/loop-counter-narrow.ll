; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test that i16 loop counter with provably-zero high byte uses
; 8-bit comparison (CP) instead of 16-bit (SUB+OR H).
;
; This pattern arises when LLVM's IndVarSimplify widens an i8 loop
; counter to i16 for GEP pointer arithmetic. The high byte is always
; zero because the counter starts at 0 and increments with nuw nsw
; up to a small constant (<= 255).

@buf = external global [8 x i8]

declare void @use_byte(i8 zeroext) nounwind

; Loop 0..6 with i16 counter used as GEP index.
; The counter is i16 but range is [0,7), so high byte is always 0.
; Fused branch: should emit CP 7, not SUB 7; OR H.
; Issue #62: also verify the dead HL copy (ld l,e; ld h,d) is eliminated
; before the compare. The peephole detects HL is dead-stored (reassigned
; before any use) and replaces `ld l,e; ld h,d; ld a,l` with `ld a,e`.
define void @loop_counter_narrow() nounwind {
; CHECK-LABEL: _loop_counter_narrow:
; CHECK:       ld a,e
; CHECK-NEXT:  cp #7
; CHECK-NOT:   or h
; CHECK-NOT:   or b
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %next, %body ]
  %cmp = icmp eq i16 %i, 7
  br i1 %cmp, label %exit, label %body

body:
  %ptr = getelementptr inbounds i8, ptr @buf, i16 %i
  %val = load i8, ptr %ptr
  call void @use_byte(i8 %val)
  %next = add nuw nsw i16 %i, 1
  br label %loop

exit:
  ret void
}

; Same pattern but with NE predicate.
define void @loop_counter_narrow_ne() nounwind {
; CHECK-LABEL: _loop_counter_narrow_ne:
; CHECK:       ld a,e
; CHECK-NEXT:  cp #7
; CHECK-NOT:   or h
; CHECK-NOT:   or b
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %next, %body ]
  %cmp = icmp ne i16 %i, 7
  br i1 %cmp, label %body, label %exit

body:
  %ptr = getelementptr inbounds i8, ptr @buf, i16 %i
  %val = load i8, ptr %ptr
  call void @use_byte(i8 %val)
  %next = add nuw nsw i16 %i, 1
  br label %loop

exit:
  ret void
}

; Counter without nuw — INC16 still preserves high-byte-zero since
; the loop structure guarantees range [0,7].
define void @loop_counter_no_nuw() nounwind {
; CHECK-LABEL: _loop_counter_no_nuw:
; CHECK:       cp #7
; CHECK-NOT:   or h
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %next, %body ]
  %cmp = icmp eq i16 %i, 7
  br i1 %cmp, label %exit, label %body

body:
  %ptr = getelementptr inbounds i8, ptr @buf, i16 %i
  %val = load i8, ptr %ptr
  call void @use_byte(i8 %val)
  %next = add i16 %i, 1
  br label %loop

exit:
  ret void
}

; Negative test: parameter-based start value — high byte may not be zero.
; Should use OR H because the PHI incoming from entry is not provably [0,255].
define void @loop_counter_param_start(i16 %start) nounwind {
; CHECK-LABEL: _loop_counter_param_start:
; CHECK:       or h
entry:
  br label %loop

loop:
  %i = phi i16 [ %start, %entry ], [ %next, %body ]
  %cmp = icmp eq i16 %i, 7
  br i1 %cmp, label %exit, label %body

body:
  %ptr = getelementptr inbounds i8, ptr @buf, i16 %i
  %val = load i8, ptr %ptr
  call void @use_byte(i8 %val)
  %next = add nuw nsw i16 %i, 1
  br label %loop

exit:
  ret void
}
