; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Test that LD (sym),A followed by LD HL,sym is reordered into
; LD HL,sym + LD (HL),A. Saves 2B (3B store → 1B indirect store).
;
; Pattern arises in BSS-clear via memcpy: store 0 to first byte,
; then memcpy(p+1, p, n-1) where p is the same address.

@buf = external global [256 x i8]
declare void @use_hl(ptr) nounwind

; Store 0 then load HL with same address — should fold to LD HL; LD (HL),A.
; CHECK-LABEL: store_then_load_hl:
; CHECK:      	xor	a
; CHECK:      	ld	(_buf),a
; CHECK:      	ld	hl,#_buf
; CHECK:      	call	_use_hl
; CHECK:      	ret
define void @store_then_load_hl() nounwind {
  store i8 0, ptr @buf
  call void @use_hl(ptr @buf)
  ret void
}

; Negative test: HL is clobbered between store and load — peephole must
; not fire (no "LD (HL),A" before HL is reloaded with the address).
declare ptr @make_ptr() nounwind
; CHECK-LABEL: hl_clobbered_between:
; CHECK:      	xor	a
; CHECK:      	ld	(_buf),a
; CHECK:      	call	_make_ptr
; CHECK:      	ld	hl,#_buf
; CHECK:      	call	_use_hl
; CHECK:      	ret
define void @hl_clobbered_between() nounwind {
; The store should remain as direct LD (sym),A because make_ptr clobbers HL.
  store i8 0, ptr @buf
  %p = call ptr @make_ptr()
  call void @use_hl(ptr @buf)
  ret void
}
