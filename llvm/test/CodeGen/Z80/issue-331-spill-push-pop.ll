; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #331: Callee-saved PUSH/POP in SP-relative dynamic stack frames.
; Instead of 5-instruction store + 5-instruction reload:
;   ld hl,#0; add hl,sp; ld (hl),c; inc hl; ld (hl),b  (7 B)
;   ...
;   ld hl,#0; add hl,sp; ld c,(hl); inc hl; ld b,(hl)  (7 B)
; emit direct 1-byte PUSH and POP:
;   push bc  (1 B)
;   ...
;   pop bc   (1 B)
; saving 12 bytes per spill/reload pair.

declare zeroext i8 @callee()
declare void @use_i16(i16)

; Case 1: Loop counter BC preserved across call in loop body (mirrors _fdc_read_result)
; CHECK-LABEL: spill_bc_across_call:
; CHECK-NOT:   add	hl,sp
; CHECK:       push	bc
; CHECK:       call	_callee
; CHECK:       pop	bc
define void @spill_bc_across_call() {
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %v = call zeroext i8 @callee()
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, 7
  br i1 %done, label %exit, label %loop

exit:
  ret void
}

; Case 2: Function argument preserved across call
; CHECK-LABEL: spill_arg_across_call:
; CHECK-NOT:   add	hl,sp
; CHECK:       push	hl
; CHECK:       call	_callee
; CHECK:       pop	hl
define void @spill_arg_across_call(i16 %x) {
entry:
  %v = call zeroext i8 @callee()
  call void @use_i16(i16 %x)
  ret void
}
