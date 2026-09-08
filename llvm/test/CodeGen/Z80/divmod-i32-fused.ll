; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 < %s | FileCheck %s

; ravn/llvm-z80#248: an adjacent i32 udiv + urem on identical operands must
; fuse into ONE runtime call (__udivmodsi4, quotient returned + remainder via
; a caller pointer), not two separate __udivsi3 + __umodsi3 calls that each
; re-run the full 32-bit division core.

; CHECK-LABEL: udivmod32:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	(L_udivmod32.frame+6),hl
; CHECK:      	ld	(L_udivmod32.frame),de
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
; CHECK:      	ld	de,(L_udivmod32.frame)
; CHECK:      	call	___udivsi3
; CHECK:      	ld	(L_udivmod32.frame+4),de
; CHECK:      	ld	(L_udivmod32.frame+2),hl
; CHECK:      	pop	af
; CHECK:      	pop	af
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
; CHECK:      	ld	hl,(L_udivmod32.frame+6)
; CHECK:      	ld	de,(L_udivmod32.frame)
; CHECK:      	call	___umodsi3
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	pop	af
; CHECK:      	pop	af
; CHECK:      	ex	de,hl
; CHECK:      	ld	de,(L_udivmod32.frame+4)
; CHECK:      	add	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ex	de,hl
; CHECK:      	ld	hl,(L_udivmod32.frame+2)
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
define i32 @udivmod32(i32 %a, i32 %b) {
  %q = udiv i32 %a, %b
  %r = urem i32 %a, %b
  %s = add i32 %q, %r
  ret i32 %s
}

; Only the low 16 bits of the remainder are used (the pi-spigot shape): still
; one fused call; the dead high half of the remainder may be optimized away.
define i16 @udivmod32_narrow_rem(i32 %a, i32 %b, ptr %out) {
  %q = udiv i32 %a, %b
  %r = urem i32 %a, %b
  store i32 %q, ptr %out
; CHECK-LABEL: udivmod32_narrow_rem:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	(L_udivmod32_narrow_rem.frame+6),hl
; CHECK:      	ld	(L_udivmod32_narrow_rem.frame),de
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
; CHECK:      	ld	de,(L_udivmod32_narrow_rem.frame)
; CHECK:      	call	___udivsi3
; CHECK:      	ld	(L_udivmod32_narrow_rem.frame+4),de
; CHECK:      	ld	(L_udivmod32_narrow_rem.frame+2),hl
; CHECK:      	pop	af
; CHECK:      	pop	af
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
; CHECK:      	ld	hl,(L_udivmod32_narrow_rem.frame+6)
; CHECK:      	ld	de,(L_udivmod32_narrow_rem.frame)
; CHECK:      	call	___umodsi3
; CHECK:      	ld	(L_udivmod32_narrow_rem.frame),de
; CHECK:      	pop	af
; CHECK:      	pop	af
; CHECK:      	ld	de,(L_udivmod32_narrow_rem.frame+4)
; CHECK:      	ld	hl,#6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	inc	bc
; CHECK:      	inc	bc
; CHECK:      	ld	de,(L_udivmod32_narrow_rem.frame+2)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	de,(L_udivmod32_narrow_rem.frame)
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
  %rt = trunc i32 %r to i16
  ret i16 %rt
}

; Signed i32 divrem fuses too, into __divmodsi4 (one call), not separate
; __divsi3 + __modsi3.
define i32 @sdivmod32(i32 %a, i32 %b) {
  %q = sdiv i32 %a, %b
  %r = srem i32 %a, %b
  %s = add i32 %q, %r
  ret i32 %s
}
; CHECK-LABEL: sdivmod32:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	(L_sdivmod32.frame+6),hl
; CHECK:      	ld	(L_sdivmod32.frame),de
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
; CHECK:      	ld	de,(L_sdivmod32.frame)
; CHECK:      	call	___divsi3
; CHECK:      	ld	(L_sdivmod32.frame+4),de
; CHECK:      	ld	(L_sdivmod32.frame+2),hl
; CHECK:      	pop	af
; CHECK:      	pop	af
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
; CHECK:      	ld	hl,(L_sdivmod32.frame+6)
; CHECK:      	ld	de,(L_sdivmod32.frame)
; CHECK:      	call	___modsi3
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	pop	af
; CHECK:      	pop	af
; CHECK:      	ex	de,hl
; CHECK:      	ld	de,(L_sdivmod32.frame+4)
; CHECK:      	add	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ex	de,hl
; CHECK:      	ld	hl,(L_sdivmod32.frame+2)
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	ret
