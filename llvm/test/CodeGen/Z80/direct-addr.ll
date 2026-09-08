; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test direct addressing for stores/loads to constant addresses.
; Issue #45: LD (addr),rr for 16-bit stores to known addresses.

; CHECK-LABEL: store8_const_addr:
; CHECK:      	ld	bc,#5
; CHECK:      	ld	(bc),a
; CHECK:      	ret
define void @store8_const_addr(i8 %val) {
  store volatile i8 %val, ptr inttoptr (i16 5 to ptr)
  ret void
}

define void @store16_const_addr(i16 %val) {
  store volatile i16 %val, ptr inttoptr (i16 1 to ptr)
; CHECK-LABEL: store16_const_addr:
; CHECK:      	ex	de,hl
; CHECK:      	ld	hl,#1
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ret
  ret void
}

define i8 @load8_const_addr() {
  %val = load volatile i8, ptr inttoptr (i16 3 to ptr)
  ret i8 %val
}
; CHECK-LABEL: load8_const_addr:
; CHECK:      	ld	bc,#3
; CHECK:      	ld	a,(bc)
; CHECK:      	ret

define i16 @load16_const_addr() {
  %val = load volatile i16, ptr inttoptr (i16 6 to ptr)
  ret i16 %val
}
; CHECK-LABEL: load16_const_addr:
; CHECK:      	ld	hl,#6
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ret
