; RUN: llc -mtriple=z80 -mattr=+static-frame < %s | FileCheck %s
;
; ravn/llvm-z80#18: when A already holds a constant (from `xor a` / `sub a`
; → 0, or `ld a, n`), a subsequent immediate load of the SAME constant into
; another 8-bit register is 1 B shorter as `ld r, a` (1 B) than `ld r, n`
; (2 B).  A is only read, so the value and flags survive and the tracked
; constant stays valid across consecutive fires.

declare void @take2(i8 zeroext, i8 zeroext)

; A is zeroed via `xor a` for the first argument; the second argument's
; `ld l,0` becomes `ld l,a`.
define void @f_zero(i8 zeroext %x) {
entry:
  %c = icmp eq i8 %x, 0
  br i1 %c, label %done, label %call
; CHECK-LABEL: f_zero:
; CHECK:      	or	a
; CHECK:      	jr	z,.LBB0_2
; CHECK:      	xor	a
; CHECK:      	ld	l,0
; CHECK:      	call	_take2
; CHECK:      	ret
call:
  tail call void @take2(i8 zeroext 0, i8 zeroext 0)
  br label %done
done:
  ret void
}

; Non-zero constant: `ld a,5` then `ld l,a` (not a second `ld l,5`).
define void @f_const(i8 zeroext %x) {
entry:
  %c = icmp eq i8 %x, 0
  br i1 %c, label %done, label %call
call:
  tail call void @take2(i8 zeroext 5, i8 zeroext 5)
  br label %done
done:
; CHECK-LABEL: f_const:
; CHECK:      	or	a
; CHECK:      	jr	z,.LBB1_2
; CHECK:      	ld	a,5
; CHECK:      	ld	l,5
; CHECK:      	call	_take2
; CHECK:      	ret
  ret void
}
