; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s

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

; --- StoreBack: memcpy(dma, src, 128); dma += 128; (cpnos READ-SEQ) -
define void @read_seq_iter(ptr %src) {
  %p = load ptr, ptr @dma, align 1
  call void @llvm.memcpy.p0.p0.i16(ptr %p, ptr %src, i16 128, i1 false)
  %p2 = getelementptr inbounds nuw i8, ptr %p, i16 128
  store ptr %p2, ptr @dma, align 1
  ret void
}
; CHECK-LABEL: _read_seq_iter:
; CHECK:      ldir
; CHECK-NEXT: ld  ({{.*}}),de
; CHECK-NOT:  add hl,de
; CHECK:      ret


declare void @llvm.memcpy.p0.p0.i16(ptr, ptr, i16, i1 immarg)
