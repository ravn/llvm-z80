; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-float-sdcccall0 %s -o - | FileCheck %s
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 %s -o - | FileCheck --check-prefix=DEFAULT %s
; XFAIL: *
;
; ravn/llvm-z80 #277 (follow-up to issue-277-f32-libcall-sdcccall0.ll): the
; f32 compare libcalls (__cmpsf2/__gtsf2/__gesf2/__unordsf2) and the f32<->i32
; conversion libcalls (__fixsfsi/__fixunssfsi/__floatsisf/__floatunsisf) are
; ALSO emitted with CallingConv::Z80_SDCCCall0 under the same opt-in
; `-z80-float-sdcccall0` flag, so z88dk's math32 bridge (this time NOT a pure
; alias for compares -- see z88dk's libsrc/l/llvmz80/__cmpsf2.asm -- and a
; pure alias for conversions, libsrc/l/llvmz80/__floatsisf.asm) sees the
; sdcccall(0) stack-args-only / DE:HL(or plain HL for 16-bit results)
; convention consistently across the whole f32 libcall surface.
;
; Compares: BOTH 32-bit float operands are pushed to the stack (4 `push hl`
; total), matching the arithmetic convention exactly (same declared-operand-
; order construction in Z80LegalizerInfo.cpp's G_FCMP case). Conversions:
; ONE 32-bit operand (int, sign/zero-extended by minScalar before reaching
; the libcall, or float) is pushed (2 `push hl`).
;
; DEFAULT (no flag): compares pass only the second float on the stack (the
; first stays register-resident per the default C ABI, same shape as
; issue-277-f32-libcall-sdcccall0.ll's arithmetic DEFAULT case); conversions
; pass no stack argument at all (single register-resident arg fits directly
; in HL:DE under the default ABI, so the call is a plain `jp`/`call` with no
; `push` at all for a 16-bit source int, or an implicit sign-extend then
; direct call for the float source).

; CHECK-LABEL: cmp_lt:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	push	bc
; CHECK:      	ld	hl,#6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	pop	bc
; CHECK:      	push	hl
; CHECK:      	push	bc
; CHECK:      	ld	hl,#6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	pop	bc
; CHECK:      	push	hl
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	call	___cmpsf2
; CHECK:      	ld	a,d
; CHECK:      	rlca
; CHECK:      	and	#1
; CHECK:      	ld	e,a
; CHECK:      	ld	d,#0
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
define i16 @cmp_lt(float %a, float %b) {
;
; DEFAULT-LABEL: _cmp_lt:
; DEFAULT: push hl
; DEFAULT: push hl
; DEFAULT-NOT: push hl
; DEFAULT: call ___cmpsf2
  %c = fcmp olt float %a, %b
  %r = zext i1 %c to i16
  ret i16 %r
}

define i16 @cmp_ueq(float %a, float %b) {
;
; DEFAULT-LABEL: _cmp_ueq:
; DEFAULT: call ___cmpsf2
; DEFAULT: call ___unordsf2
  %c = fcmp ueq float %a, %b
; CHECK-LABEL: cmp_ueq:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	(L_cmp_ueq.frame+4),hl
; CHECK:      	ld	(L_cmp_ueq.frame),de
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	push	bc
; CHECK:      	ld	hl,#6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	pop	bc
; CHECK:      	push	hl
; CHECK:      	ex	de,hl
; CHECK:      	push	hl
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	de,(L_cmp_ueq.frame)
; CHECK:      	call	___cmpsf2
; CHECK:      	ld	(L_cmp_ueq.frame+2),de
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,(L_cmp_ueq.frame+4)
; CHECK:      	ld	de,(L_cmp_ueq.frame)
; CHECK:      	call	___unordsf2
; CHECK:      	ld	bc,(L_cmp_ueq.frame+2)
; CHECK:      	ld	a,c
; CHECK:      	or	b
; CHECK:      	sub	#1
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	c,a
; CHECK:      	ld	a,d
; CHECK:      	ld	b,a
; CHECK:      	ld	a,e
; CHECK:      	or	b
; CHECK:      	add	a,#255
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ld	b,a
; CHECK:      	ld	a,c
; CHECK:      	or	b
; CHECK:      	ld	e,a
; CHECK:      	ld	d,#0
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
  %r = zext i1 %c to i16
  ret i16 %r
}

define i16 @f2i(float %a) {
;
; DEFAULT-LABEL: _f2i:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___fixsfsi
  %r = fptosi float %a to i16
  ret i16 %r
}

; CHECK-LABEL: f2i:
; CHECK:      	call	___fixsfsi
; CHECK:      	ret
define i16 @f2u(float %a) {
;
; DEFAULT-LABEL: _f2u:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___fixunssfsi
  %r = fptoui float %a to i16
  ret i16 %r
}

define float @i2f(i16 %a) {
;
; DEFAULT-LABEL: _i2f:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___floatsisf
; CHECK-LABEL: f2u:
; CHECK:      	call	___fixunssfsi
; CHECK:      	ret
  %r = sitofp i16 %a to float
  ret float %r
}

define float @u2f(i16 %a) {
;
; DEFAULT-LABEL: _u2f:
; DEFAULT-NOT: push hl
; DEFAULT: jp ___floatunsisf
  %r = uitofp i16 %a to float
  ret float %r
}
; CHECK-LABEL: i2f:
; CHECK:      	ld	e,l
; CHECK:      	ld	d,h
; CHECK:      	ld	a,h
; CHECK:      	add	a,a
; CHECK:      	sbc	a,a
; CHECK:      	ld	l,a
; CHECK:      	ld	h,a
; CHECK:      	call	___floatsisf
; CHECK:      	ret
; CHECK-LABEL: u2f:
; CHECK:      	ex	de,hl
; CHECK:      	ld	hl,#0
; CHECK:      	call	___floatunsisf
; CHECK:      	ret
