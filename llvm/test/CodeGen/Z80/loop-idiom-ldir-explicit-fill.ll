; RUN: llc -mtriple=z80 -O2 -z80-asm-format=sdasz80 < %s | FileCheck %s
;
; Explicit-loop fill -> LoopIdiomRecognize -> LDIR.
;
; C source (fw_bitops.c / rcbios bg_clear_from idiom):
;
;   for (j = 0; j < whole; j++)          // variable size u8
;       bgbuf[byteoff + j] = 0;
;
;   for (j = 0; j < 64; j++)             // constant size
;       bgbuf[j] = 0xFF;
;
; clang's LoopIdiomRecognize converts both loops to llvm.memset at -Os/-O2.
; The Z80 legalizer then lowers llvm.memset to the seed-store + LDIR sequence:
;
;   LD (HL), val     ; write first byte (seed)
;   LD DE, HL+1      ; destination = source + 1 (overlap by 1)
;   LD BC, n-1       ; copy remaining n-1 bytes
;   LDIR             ; forward overlapping fill: propagates seed across buffer
;
; For variable size, a BC==0 guard wraps the LDIR (LDIR with BC=0 would
; run 65536 times — ravn/llvm-z80#105).  For the constant-64 case the guard
; is elided since the size is statically non-zero.
;
; This test pins the LDIR lowering so regressions are caught immediately if
; LoopIdiomRecognize stops firing or the memset lowering path regresses.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@bgbuf = external dso_local local_unnamed_addr global [64 x i8], align 1

declare void @llvm.memset.p0.i16(ptr writeonly captures(none), i8, i16, i1 immarg)

; Variable-size zero fill (fwbitops bitclear inner loop, u8 byteoff + u8 whole).
; Expected: guarded LDIR (size may be 0 -> must skip LDIR when BC=0).
; CHECK-LABEL: fill_zero_slice:
; CHECK:      	ld	c,a
; CHECK:      	ld	a,l
; CHECK:      	or	a
; CHECK:      	ret	z
; CHECK:      	ld	b,l
; CHECK:      	ld	e,c
; CHECK:      	ld	d,#0
; CHECK:      	ld	hl,#_bgbuf
; CHECK:      	add	hl,de
; CHECK:      	ld	c,b
; CHECK:      	ld	b,d
; CHECK:      	ld	e,b
; CHECK:      	ld	a,b
; CHECK:      	or	c
; CHECK:      	jr	z,[[EXIT:\.LBB[0-9_]+]]
; CHECK:      	ld	(hl),e
; CHECK:      	dec	bc
; CHECK:      	ld	a,b
; CHECK:      	or	c
; CHECK:      	jr	z,[[EXIT]]
; CHECK:      	ld	d,h
; CHECK:      	ld	e,l
; CHECK:      	inc	de
; CHECK:      	ldir
; CHECK:      [[EXIT]]:
; CHECK:      	ret
define dso_local void @fill_zero_slice(i8 noundef zeroext %byteoff, i8 noundef zeroext %whole) local_unnamed_addr {
  %skip = icmp eq i8 %whole, 0
  br i1 %skip, label %done, label %do_fill

do_fill:
  %off16 = zext i8 %byteoff to i16
  %ptr = getelementptr inbounds nuw i8, ptr @bgbuf, i16 %off16
  %n16 = zext i8 %whole to i16
  call void @llvm.memset.p0.i16(ptr nonnull align 1 %ptr, i8 0, i16 %n16, i1 false)
  br label %done

done:
  ret void
}

; Constant-size 0xFF fill (fwbitops main init loop, 64 bytes).
; Expected: direct LDIR without guard (size == 64, statically nonzero).
; CHECK-LABEL: fill_ones:
; CHECK:      	ld	hl,#_bgbuf
; CHECK:      	ld	a,#255
; CHECK:      	ld	(hl),a
; CHECK:      	ld	de,#_bgbuf
; CHECK:      	inc	de
; CHECK:      	ld	bc,#63
; CHECK:      	ldir
; CHECK:      	ret
define dso_local void @fill_ones() local_unnamed_addr {
  call void @llvm.memset.p0.i16(ptr nonnull align 1 @bgbuf, i8 -1, i16 64, i1 false)
  ret void
}

; Variable-size: the branch before LDIR skips when whole==0 (BC=0 guard).

; Constant 64: seed the first byte, then LDIR for the remaining 63.

