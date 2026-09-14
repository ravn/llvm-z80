; RUN: llc -mtriple=z80 < %s | FileCheck %s
;
; Wide scalar load+store pairs must become block moves, not per-limb
; traffic.  Without the pre-legalizer combine, narrowScalar splits an i64
; copy into 4 x i16 loads + 4 x i16 stores, each re-deriving its address
; (no displacement mode on (HL)) — measured 65 instructions for @copy8.
; With the combine the pair becomes G_MEMMOVE (load-completes-before-store
; semantics == memmove), which lowers to LDIR/LDDR or a _memmove libcall.
;
; AVR analogy: AVR absorbs the same shape via ldd/std Z+q displacement
; addressing; Z80's idiomatic answer is the block move.

; CHECK-LABEL: copy8:
; CHECK:      	ld	bc,8
; CHECK:      	call	___z80_memmove_builtin
; CHECK:      	ret
define void @copy8(ptr %dst, ptr %src) {
  %v = load i64, ptr %src, align 1
  store i64 %v, ptr %dst, align 1
  ret void
}

; CHECK-LABEL: copy4:
; CHECK:      	ld	bc,4
; CHECK:      	call	___z80_memmove_builtin
; CHECK:      	ret
define void @copy4(ptr %dst, ptr %src) {
  %v = load i32, ptr %src, align 1
  store i32 %v, ptr %dst, align 1
  ret void
}

; i16 is native (register pair) — must NOT become a block move.
define void @keep_i16(ptr %dst, ptr %src) {
  %v = load i16, ptr %src, align 1
  store i16 %v, ptr %dst, align 1
  ret void
}

; CHECK-LABEL: keep_i16:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ex	de,hl
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ret
; Volatile accesses keep their exact memory operations.
define void @keep_volatile(ptr %dst, ptr %src) {
  %v = load volatile i64, ptr %src, align 1
  store volatile i64 %v, ptr %dst, align 1
  ret void
}

; Multi-use loaded value: the value is genuinely needed in registers.
define i8 @keep_multiuse(ptr %dst, ptr %src) {
  %v = load i64, ptr %src, align 1
; CHECK-LABEL: keep_volatile:
; CHECK:      	ld	(L_keep_volatile.frame),hl
; CHECK:      	ld	c,e
; CHECK:      	ld	b,d
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_volatile.frame+2),de
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	inc	hl
; CHECK:      	inc	hl
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_volatile.frame+4),de
; CHECK:      	ld	de,4
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_volatile.frame+6),de
; CHECK:      	ld	de,6
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_volatile.frame+8),de
; CHECK:      	ld	de,(L_keep_volatile.frame+2)
; CHECK:      	ld	bc,(L_keep_volatile.frame)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	inc	hl
; CHECK:      	inc	hl
; CHECK:      	ld	de,(L_keep_volatile.frame+4)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	de,4
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	de,(L_keep_volatile.frame+6)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	de,6
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	de,(L_keep_volatile.frame+8)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ret
  store i64 %v, ptr %dst, align 1
  %t = trunc i64 %v to i8
  ret i8 %t
}

; An intervening may-write between load and store blocks the rewrite
; (the combine moves the read down to the store point).
define void @keep_intervening_store(ptr %dst, ptr %src, ptr %other) {
  %v = load i64, ptr %src, align 1
; CHECK-LABEL: keep_multiuse:
; CHECK:      	ld	(L_keep_multiuse.frame+2),hl
; CHECK:      	ld	c,e
; CHECK:      	ld	b,d
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_multiuse.frame),de
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	inc	hl
; CHECK:      	inc	hl
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_multiuse.frame+4),de
; CHECK:      	ld	de,4
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_multiuse.frame+6),de
; CHECK:      	ld	de,6
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_multiuse.frame+8),de
; CHECK:      	ld	de,(L_keep_multiuse.frame)
; CHECK:      	ld	bc,(L_keep_multiuse.frame+2)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	inc	hl
; CHECK:      	inc	hl
; CHECK:      	ld	de,(L_keep_multiuse.frame+4)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	de,4
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	de,(L_keep_multiuse.frame+6)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	de,6
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	de,(L_keep_multiuse.frame+8)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	bc,(L_keep_multiuse.frame)
; CHECK:      	ld	a,c
; CHECK:      	ret
  store i8 7, ptr %other, align 1
  store i64 %v, ptr %dst, align 1
  ret void
}
; CHECK-LABEL: keep_intervening_store:
; CHECK:      	ld	(L_keep_intervening_store.frame),hl
; CHECK:      	ld	c,e
; CHECK:      	ld	b,d
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_intervening_store.frame+2),de
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	inc	hl
; CHECK:      	inc	hl
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_intervening_store.frame+4),de
; CHECK:      	ld	de,4
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_intervening_store.frame+6),de
; CHECK:      	ld	de,6
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	(L_keep_intervening_store.frame+8),de
; CHECK:      	ld	a,7
; CHECK:      	ld	hl,2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	(bc),a
; CHECK:      	ld	de,(L_keep_intervening_store.frame+2)
; CHECK:      	ld	bc,(L_keep_intervening_store.frame)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	inc	hl
; CHECK:      	inc	hl
; CHECK:      	ld	de,(L_keep_intervening_store.frame+4)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	de,4
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	de,(L_keep_intervening_store.frame+6)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	de,6
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	de,(L_keep_intervening_store.frame+8)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
