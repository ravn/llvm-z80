; RUN: llc -mtriple=z80 -mattr=+static-frame -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s
;
; ravn/llvm-z80#135 — #131 caller-side HL/A preservation needs to also
; strip ADJCALLSTACKUP's pessimistic TableGen `Defs = [SP, HL, A]` for
; declared-preserved regs that the pseudo's expansion path won't actually
; clobber.  Without that, regalloc treats HL/A as clobbered across the
; call sequence — silently nullifying HL/A preservation.
;
; This fixture pairs two near-identical functions, one with "d,e"
; preserved and one with "h,l", and CHECKs that both emit equivalent
; spill-free patterns.
;
; Witness: declare two near-identical functions, one with "d,e" preserved,
; one with "h,l".  Without the bug, both should drop the unrelated-pair
; spill across the call.  Currently `f_de` does (saves DE) but `f_hl` does
; not (still BSS-spills DE around the call AND push/pops HL).

declare void @sink_de_only() #0
declare void @sink_hl_only() #1

; %a in HL, %b in DE.  After call, both must be live for the add.
; With "d,e" preserved, DE survives; only HL needs save/restore.
define i16 @f_de(i16 %a, i16 %b) {
  call void @sink_de_only()
  %s = add i16 %a, %b
  ret i16 %s
}

; Symmetric case: with "h,l" preserved, HL should survive across the call.
; CHECK-LABEL: f_de:
; CHECK:      	push	af
; CHECK:      	push	af
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),c
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),b
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	call	_sink_de_only
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,bc
; CHECK:      	ex	de,hl
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	ret
; No push hl / pop hl should be emitted; DE is the one that needs spill.
;
; Today (#135): HL push/pop is emitted AND DE is BSS-spilled, because
; regalloc sees ADJCALLSTACKUP's implicit-def of HL and treats HL as dead.
;
define i16 @f_hl(i16 %a, i16 %b) {
  call void @sink_hl_only()
  %s = add i16 %a, %b
  ret i16 %s
}

attributes #0 = { "z80-preserves-regs"="d,e" }
attributes #1 = { "z80-preserves-regs"="h,l" }
; CHECK-LABEL: f_hl:
; CHECK:      	push	af
; CHECK:      	push	af
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),c
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),b
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	call	_sink_hl_only
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	push	hl
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	b,(hl)
; CHECK:      	pop	hl
; CHECK:      	add	hl,bc
; CHECK:      	ex	de,hl
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	ret
