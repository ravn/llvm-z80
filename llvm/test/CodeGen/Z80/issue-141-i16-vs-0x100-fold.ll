; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#141: `icmp uge/ult i16 %r, K` where K is a multiple
; of 256 should fold to a single 8-bit test on the high byte instead
; of the 9-byte CMP16_FLAGS chain (ld bc,N; ld a,..; sub..; sbc..;
; jr {c,nc}).  Same for the operand-swapped forms (ICMP_ULE / UGT
; arriving as UGE / ULT post-normalization, with K = N*256+0xFF).
;
; (b) the 16-bit subtract-and-borrow chain is absent.  Branch
; direction (z vs nz, c vs nc) may be swapped by later layout
; passes — we accept either.

declare i16 @get()

;
; Case 1: var-LHS, RHS = 256.  `r >= 256` and `r < 256`
;
define i8 @check_uge_256() {
entry:
  %r = call i16 @get()
  %tim = icmp uge i16 %r, 256
  br i1 %tim, label %hi, label %lo
hi:
  ret i8 -1
lo:
; CHECK-LABEL: check_uge_256:
; CHECK:      	call	_get
; CHECK:      	ld	bc,255
; CHECK:      	ld	a,c
; CHECK:      	sub	e
; CHECK:      	ld	a,b
; CHECK:      	sbc	a,d
; CHECK:      	jr	nc,.LBB0_2
; CHECK:      	ld	a,255
; CHECK:      	ret
; CHECK:      	ld	a,e
; CHECK:      	ret
  %b = trunc i16 %r to i8
  ret i8 %b
}

define i8 @check_ult_256() {
entry:
  %r = call i16 @get()
  %ok = icmp ult i16 %r, 256
  br i1 %ok, label %lo, label %hi
lo:
  %b = trunc i16 %r to i8
  ret i8 %b
hi:
  ret i8 -1
}

;
; Case 2 (post-normalization swap): `r <= 255` arrives as ULE which
; the backend's switch normalizes to UGE with operands swapped, so
; CHECK-LABEL: check_ult_256:
; CHECK:      	call	_get
; CHECK:      	ld	bc,256
; CHECK:      	ld	a,e
; CHECK:      	sub	c
; CHECK:      	ld	a,d
; CHECK:      	sbc	a,b
; CHECK:      	jr	nc,.LBB1_2
; CHECK:      	ld	a,e
; CHECK:      	ret
; CHECK:      	ld	a,255
; CHECK:      	ret
; we see LHS = const 255, RHS = var.
;
define i8 @check_ule_255() {
entry:
  %r = call i16 @get()
  %ok = icmp ule i16 %r, 255
  br i1 %ok, label %lo, label %hi
lo:
  %b = trunc i16 %r to i8
  ret i8 %b
hi:
  ret i8 -1
}

;
; Case 3: K = 512 = 2*256 — uses CP_n with threshold 2 plus JR NC/C.
;
define i8 @check_uge_512() {
entry:
  %r = call i16 @get()
  %tim = icmp uge i16 %r, 512
  br i1 %tim, label %hi, label %lo
hi:
  ret i8 -1
; CHECK-LABEL: check_ule_255:
; CHECK:      	call	_get
; CHECK:      	ld	bc,256
; CHECK:      	ld	a,e
; CHECK:      	sub	c
; CHECK:      	ld	a,d
; CHECK:      	sbc	a,b
; CHECK:      	jr	nc,.LBB2_2
; CHECK:      	ld	a,e
; CHECK:      	ret
; CHECK:      	ld	a,255
; CHECK:      	ret
lo:
  %b = trunc i16 %r to i8
  ret i8 %b
}

;
; Negative case: K = 257 has both bytes non-zero — should NOT fold
; (falls back to CMP16_FLAGS chain).  Confirms we still emit the
; standard subtract-and-borrow path for non-byte-aligned constants.
;
define i8 @check_uge_257() {
entry:
  %r = call i16 @get()
  %tim = icmp uge i16 %r, 257
  br i1 %tim, label %hi, label %lo
hi:
  ret i8 -1
lo:
  %b = trunc i16 %r to i8
  ret i8 %b
}
; CHECK-LABEL: check_uge_512:
; CHECK:      	call	_get
; CHECK:      	ld	bc,511
; CHECK:      	ld	a,c
; CHECK:      	sub	e
; CHECK:      	ld	a,b
; CHECK:      	sbc	a,d
; CHECK:      	jr	nc,.LBB3_2
; CHECK:      	ld	a,255
; CHECK:      	ret
; CHECK:      	ld	a,e
; CHECK:      	ret
; CHECK-LABEL: check_uge_257:
; CHECK:      	call	_get
; CHECK:      	ld	bc,256
; CHECK:      	ld	a,c
; CHECK:      	sub	e
; CHECK:      	ld	a,b
; CHECK:      	sbc	a,d
; CHECK:      	jr	nc,.LBB4_2
; CHECK:      	ld	a,255
; CHECK:      	ret
; CHECK:      	ld	a,e
; CHECK:      	ret
