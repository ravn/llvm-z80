; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Test that LD (sym),A followed by LD HL,sym is reordered into
; LD HL,sym + LD (HL),A. Saves 2B (3B store → 1B indirect store).
;
; Pattern arises in BSS-clear via memcpy: store 0 to first byte,
; then memcpy(p+1, p, n-1) where p is the same address.

@buf = external global [256 x i8]
declare void @use_hl(ptr) nounwind

; Store 0 then load HL with same address — should fold to LD HL; LD (HL),A.
define void @store_then_load_hl() nounwind {
; CHECK-LABEL: _store_then_load_hl:
; CHECK:       ld hl,#_buf
; CHECK-NEXT:  ld (hl),a
; CHECK-NOT:   ld (_buf),a
  store i8 0, ptr @buf
  call void @use_hl(ptr @buf)
  ret void
}

; Negative test: HL is clobbered between store and load — peephole must
; not fire (no "LD (HL),A" before HL is reloaded with the address).
declare ptr @make_ptr() nounwind
define void @hl_clobbered_between() nounwind {
; CHECK-LABEL: _hl_clobbered_between:
; The store should remain as direct LD (sym),A because make_ptr clobbers HL.
; CHECK:       ld (_buf),a
  store i8 0, ptr @buf
  %p = call ptr @make_ptr()
  call void @use_hl(ptr @buf)
  ret void
}
