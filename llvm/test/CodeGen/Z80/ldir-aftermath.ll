; RUN: llc -mtriple=z80 -mattr=+static-frame -O2 < %s | FileCheck %s

; Issue #78: after LDIR, DE = dst+count and HL = src+count.  The
; reconstruction triple `LD HL,(slot); LD DE,N; ADD HL,DE` reads the
; pre-LDIR slot value back, adds N, and lands on (DE post-LDIR).  The
; #78 peephole rewrites it three ways:
;   - StoreBack: trailing LD (target),HL → LD (target),DE
;   - DropEx:    trailing EX DE,HL       → drop everything (DE wins)
;   - Other:     unknown sink            → LD H,D; LD L,E
; Plus a ±1 fixup (INC/DEC DE or INC/DEC HL) when the reload count
; differs from LDIR's BC by exactly 1.

@dma = external dso_local global ptr

; --- StoreBack, Diff=0: memcpy(dma, src, 128); dma += 128; -----------
; (cpnos READ-SEQ pattern.)  Triple+store collapses to LD (target),DE.
; CHECK-LABEL: read_seq_iter:
; CHECK:      	ld	de,(_dma)
; CHECK:      	ld	(L_read_seq_iter.frame),de
; CHECK:      	ld	bc,128
; CHECK:      	ldir
; CHECK:      	ld	bc,128
; CHECK:      	ld	hl,(L_read_seq_iter.frame)
; CHECK:      	add	hl,bc
; CHECK:      	ld	(_dma),hl
; CHECK:      	ret
define void @read_seq_iter(ptr %src) {
  %p = load ptr, ptr @dma, align 1
  call void @llvm.memcpy.p0.p0.i16(ptr %p, ptr %src, i16 128, i1 false)
  %p2 = getelementptr inbounds nuw i8, ptr %p, i16 128
  store ptr %p2, ptr @dma, align 1
  ret void
}

; --- StoreBack, Diff=+1: GEP one byte past end of memcpy ------------
; LDIR count is 128, GEP offset is 129.  Post-LDIR DE = dst+128, so
; INC DE before the store gives dst+129.
; CHECK-LABEL: plus_one:
; CHECK:      	ld	de,(_dma)
; CHECK:      	ld	(L_plus_one.frame),de
; CHECK:      	ld	bc,128
; CHECK:      	ldir
; CHECK:      	ld	bc,129
; CHECK:      	ld	hl,(L_plus_one.frame)
; CHECK:      	add	hl,bc
; CHECK:      	ld	(_dma),hl
; CHECK:      	ret
define void @plus_one(ptr %src) {
  %p = load ptr, ptr @dma, align 1
  call void @llvm.memcpy.p0.p0.i16(ptr %p, ptr %src, i16 128, i1 false)
  %p2 = getelementptr inbounds nuw i8, ptr %p, i16 129
  store ptr %p2, ptr @dma, align 1
  ret void
}

; --- StoreBack, Diff=-1: GEP one byte short of memcpy end -----------
; LDIR count is 128, GEP offset is 127.  Post-LDIR DE = dst+128, so
; DEC DE before the store gives dst+127.
; CHECK-LABEL: minus_one:
; CHECK:      	ld	de,(_dma)
; CHECK:      	ld	(L_minus_one.frame),de
; CHECK:      	ld	bc,128
; CHECK:      	ldir
; CHECK:      	ld	bc,127
; CHECK:      	ld	hl,(L_minus_one.frame)
; CHECK:      	add	hl,bc
; CHECK:      	ld	(_dma),hl
; CHECK:      	ret
define void @minus_one(ptr %src) {
  %p = load ptr, ptr @dma, align 1
  call void @llvm.memcpy.p0.p0.i16(ptr %p, ptr %src, i16 128, i1 false)
  %p2 = getelementptr inbounds nuw i8, ptr %p, i16 127
  store ptr %p2, ptr @dma, align 1
  ret void
}

declare void @llvm.memcpy.p0.p0.i16(ptr, ptr, i16, i1 immarg)
