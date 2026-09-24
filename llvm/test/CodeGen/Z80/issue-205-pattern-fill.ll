; RUN: llc -mtriple=z80  -verify-machineinstrs < %s | FileCheck %s --check-prefix=Z80
; RUN: llc -mtriple=sm83 -verify-machineinstrs < %s | FileCheck %s --check-prefix=SM83
;
; ravn/llvm-z80#205: the defined target intrinsic llvm.z80.pattern.fill replaces
; Z80PatternFillRecognize's old UB-in-IR overlapping-memcpy representation of a
; pattern-fill loop.  It carries the "intentional forward overlap" contract
; explicitly (so it is opaque to generic IR passes -- no UB, no
; PreISelIntrinsicLowering loop-expansion) and the backend lowers it directly:
;
;   Z80  : store the K-byte pattern once (the seed), then LDIR with HL=dst (src),
;          DE=dst+K, BC=K*(count-1) -- the forward overlapping copy propagates
;          the seed.  The overlap is a post-legalize MIR construct, so no
;          optimizer can exploit it.
;   SM83 : no LDIR, so the seed-and-propagate trick is unavailable -> unrolled
;          per-pattern stores.
;
; All must be -verify-machineinstrs clean.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; Signature: llvm.z80.pattern.fill(ptr dst, iN pattern, i16 K, i16 count).
; K and count must be constants; pattern may be a runtime value.
; C source:
;   // ravn/llvm-z80#205: llvm.z80.pattern.fill target intrinsic replaces
;   // Z80PatternFillRecognize's old UB-in-IR overlapping-memcpy representation.
;   // It carries the "intentional forward overlap" contract explicitly and
;   // lowers to: store K-byte seed once, then LDIR(src=dst, dst=dst+K, BC=K*(n-1)).
;   //
;   // Pattern-fill: repeat K-byte pattern p[] across dst[0..K*n-1].
;   void pattern_fill_u16(uint16_t *dst, uint16_t pat, uint16_t n) {
;       dst[0] = pat;
;       // backend lowers the rest to LDIR(src=dst, dst=dst+1, BC=2*(n-1))
;       for (uint16_t i = 1; i < n; i++) dst[i] = pat;
;   }
declare void @llvm.z80.pattern.fill.i16(ptr, i16, i16, i16)
declare void @llvm.z80.pattern.fill.i8(ptr, i8, i16, i16)

; K=2, count=10: seed store + LDIR of K*(count-1) = 18 bytes (NOT 20 -- a
; one-pattern over-run would set BC=20).
; Z80-LABEL: word10:
; Z80:       ld {{bc, ?18}}
; Z80:       ldir
; Z80:       ret
; SM83-LABEL: word10:
; SM83-NOT:   ldir
; SM83:       ret
define void @word10(ptr %p, i16 %v) {
  call void @llvm.z80.pattern.fill.i16(ptr %p, i16 %v, i16 2, i16 10)
  ret void
}

; K=1, count=20: seed byte + LDIR of K*(count-1) = 19 bytes.
; Z80-LABEL: byte20:
; Z80:       ld {{.*}}{{bc, ?19}}
; Z80:       ldir
define void @byte20(ptr %p, i8 %v) {
  call void @llvm.z80.pattern.fill.i8(ptr %p, i8 %v, i16 1, i16 20)
; CHECK-LABEL: word10:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	hl,10
; CHECK:      	push	hl
; CHECK:      	ld	hl,2
; CHECK:      	push	hl
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	call	_llvm.z80.pattern.fill.i16
; CHECK:      	ret
  ret void
}

; count=1: the seed IS the whole fill -- no LDIR at all.
; Z80-LABEL: one:
; Z80-NOT:    ldir
; Z80:        ret
define void @one(ptr %p, i16 %v) {
  call void @llvm.z80.pattern.fill.i16(ptr %p, i16 %v, i16 2, i16 1)
  ret void
}
; CHECK-LABEL: byte20:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	hl,4
; CHECK:      	push	af
; CHECK:      	add	hl,sp
; CHECK:      	pop	af
; CHECK:      	ld	a,(hl)
; CHECK:      	ld	hl,20
; CHECK:      	push	hl
; CHECK:      	ld	hl,1
; CHECK:      	push	hl
; CHECK:      	push	af
; CHECK:      	inc	sp
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	call	_llvm.z80.pattern.fill.i8
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
; CHECK-LABEL: one:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	hl,1
; CHECK:      	push	hl
; CHECK:      	ld	hl,2
; CHECK:      	push	hl
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	call	_llvm.z80.pattern.fill.i16
; CHECK:      	ret
