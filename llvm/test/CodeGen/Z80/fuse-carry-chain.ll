; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=FUSE
; RUN: llc -mtriple=z80 -O2 -z80-enable-fuse-carry-chain=false < %s | FileCheck %s --check-prefix=CTRL
;
; add32g below carries debug info (#dbg_value -> DBG_VALUE between the carry
; pseudos); the pass skips debug instrs, so fusion must still fire under -g.
;
; B17: multi-byte add/sub should thread the inter-limb carry in the carry FLAG
; (ADD HL,rr; ADC HL,rr), not round-trip it through A (SBC A,A; AND 1 / RRCA).
; The Z80FuseCarryChain pass performs this; -z80-enable-fuse-carry-chain=false is the
; no-op control that restores the original register-carry expansion.

; i32 add: one ADD + one ADC, carry stays in CF.
define i32 @add32(i32 %a, i32 %b) {
  %r = add i32 %a, %b
  ret i32 %r
}
; FUSE-LABEL: add32:
; FUSE:       add hl,
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca
; FUSE:       adc hl,
;
; CTRL-LABEL: add32:
; CTRL:       sbc a,a
; CTRL:       rrca

; i32 sub: AND A (clear borrow) + SBC HL,rr; no borrow round-trip.
define i32 @sub32(i32 %a, i32 %b) {
  %r = sub i32 %a, %b
  ret i32 %r
}
; FUSE-LABEL: sub32:
; FUSE:       sbc hl,
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca

; i64 add: a four-limb chain collapses to add + adc + adc + adc.
define i64 @add64(i64 %a, i64 %b) {
  %r = add i64 %a, %b
  ret i64 %r
}
; FUSE-LABEL: add64:
; CHECK-LABEL: add32:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	push	bc
; CHECK:      	ld	hl,4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	pop	bc
; CHECK:      	add	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ex	de,hl
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ret
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca
; FUSE:       adc hl,
; FUSE:       adc hl,
; FUSE:       adc hl,

; Negative control: when the carry-out is OBSERVED, the terminal cannot be
; fused away, so the capture (SBC A,A; AND 1) must remain.
define i16 @addov(i32 %a, i32 %b, ptr %res) {
  %p = call { i32, i1 } @llvm.uadd.with.overflow.i32(i32 %a, i32 %b)
  %ov = extractvalue { i32, i1 } %p, 1
  %sum = extractvalue { i32, i1 } %p, 0
  store i32 %sum, ptr %res
  %z = zext i1 %ov to i16
  ret i16 %z
}
; FUSE-LABEL: addov:
; FUSE:       sbc a,a

declare { i32, i1 } @llvm.uadd.with.overflow.i32(i32, i32)

; i32 add WITH debug info: DBG_VALUEs sit between the carry pseudos.  The pass
; skips debug instrs, so fusion still fires (guards feedback_peephole_test_with_g).
define i32 @add32g(i32 %0, i32 %1) !dbg !11 {
    #dbg_value(i32 %0, !18, !DIExpression(), !20)
    #dbg_value(i32 %1, !19, !DIExpression(), !20)
  %3 = add nsw i32 %1, %0, !dbg !21
  ret i32 %3, !dbg !22
}
; FUSE-LABEL: add32g:
; FUSE:       add hl,
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca
; FUSE:       adc hl,

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "addg.c", directory: "/tmp")
!3 = !{i32 7, !"Dwarf Version", i32 5}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!11 = distinct !DISubprogram(name: "add32g", scope: !1, file: !1, line: 2, type: !12, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !17)
!12 = !DISubroutineType(types: !13)
!13 = !{!16, !16, !16}
; CHECK-LABEL: sub32:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ex	de,hl
; CHECK:      	push	hl
; CHECK:      	ld	hl,4
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	pop	hl
; CHECK:      	and	a
; CHECK:      	sbc	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ex	de,hl
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	rrca
; CHECK:      	sbc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ret
!16 = !DIBasicType(name: "long", size: 32, encoding: DW_ATE_signed)
!17 = !{!18, !19}
!18 = !DILocalVariable(name: "a", arg: 1, scope: !11, file: !1, line: 2, type: !16)
!19 = !DILocalVariable(name: "b", arg: 2, scope: !11, file: !1, line: 2, type: !16)
!20 = !DILocation(line: 0, scope: !11)
!21 = !DILocation(line: 2, column: 46, scope: !11)
!22 = !DILocation(line: 2, column: 38, scope: !11)
; CHECK-LABEL: add64:
; CHECK:      	ld	hl,4
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	hl,12
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,16
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ex	de,hl
; CHECK:      	push	bc
; CHECK:      	ld	hl,8
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	pop	bc
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ld	(L_add64.frame+4),hl
; CHECK:      	ld	hl,8
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,18
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ld	(L_add64.frame+2),hl
; CHECK:      	ld	hl,10
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,20
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ld	(L_add64.frame),hl
; CHECK:      	ld	hl,2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	hl,2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	inc	hl
; CHECK:      	inc	hl
; CHECK:      	ld	de,(L_add64.frame+4)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	bc,4
; CHECK:      	ld	hl,2
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	add	hl,bc
; CHECK:      	ld	de,(L_add64.frame+2)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	bc,6
; CHECK:      	ld	hl,2
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	add	hl,bc
; CHECK:      	ld	de,(L_add64.frame)
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ret
; CHECK-LABEL: addov:
; CHECK:      	ld	(L_addov.frame),hl
; CHECK:      	ld	hl,2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ex	de,hl
; CHECK:      	ld	hl,(L_addov.frame)
; CHECK:      	push	hl
; CHECK:      	ld	hl,6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ld	(L_addov.frame),hl
; CHECK:      	ld	hl,6
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
; CHECK:      	ld	de,(L_addov.frame)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	e,a
; CHECK:      	ld	d,0
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
; CHECK-LABEL: add32g:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ex	de,hl
; CHECK:      	push	hl
; CHECK:      	ld	hl,4
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,de
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ex	de,hl
; CHECK:      	push	bc
; CHECK:      	ld	hl,6
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	pop	bc
; CHECK:      	rrca
; CHECK:      	adc	hl,bc
; CHECK:      	sbc	a,a
; CHECK:      	and	1
; CHECK:      	ret
