; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Z80LateOptimization peephole: high-byte extract via 16-bit register
; pair fuses to a single LD A,H.
;
; Pattern (pre-peephole, ~4B):
;     LD L, H        ; copy high byte to L
;     LD H, 0        ; zero high
;     LD A, L        ; final 8-bit consume
;
; Rewrite (post-peephole, 1B):
;     LD A, H
;
; Triggered when ISel zero-extends a high byte into HL (for potential
; 16-bit use) but the result is only consumed as i8.  See
; Z80LateOptimization.cpp `LD L,H; LD H,0; LD A,L -> LD A,H`.

declare void @use_i8(i8 zeroext)

; CHECK-LABEL: _hi_byte_to_caller:
; CHECK:       ld	a,h
; CHECK-NOT:   ld	l,h
; CHECK-NOT:   ld	h,#0x00
; CHECK-NOT:   ld	a,l
; CHECK:       _use_i8
define void @hi_byte_to_caller(i16 %v) nounwind {
  %hi.i16 = lshr i16 %v, 8
  %hi.i8 = trunc i16 %hi.i16 to i8
  call void @use_i8(i8 zeroext %hi.i8)
  ret void
}

; Variant with the high byte fed into an arithmetic op rather than a
; call argument; same fusion shape.
; CHECK-LABEL: _hi_byte_to_local:
; CHECK:       ld	a,h
; CHECK-NOT:   ld	l,h
; CHECK-NOT:   ld	h,#0x00
; CHECK-NOT:   ld	a,l
define i8 @hi_byte_to_local(i16 %v) nounwind {
  %hi.i16 = lshr i16 %v, 8
  %hi.i8 = trunc i16 %hi.i16 to i8
  %r = add i8 %hi.i8, 1
  ret i8 %r
}
