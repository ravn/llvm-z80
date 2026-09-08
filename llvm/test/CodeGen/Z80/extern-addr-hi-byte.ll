; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #90: `(uint8_t)(extern_addr >> 8)` byte-arg call previously
; routed through DE -> L -> H -> A in 10 bytes:
;
;   ld   de, sym
;   ld   l, d        ; L = high byte
;   ld   h, $0       ; H = 0  (zero-extend the i16 lshr result)
;   ld   a, l        ; A = L  (trunc-to-i8)
;
; The fold in Z80InstructionSelector::G_TRUNC recognises a one-use
; G_LSHR-by-8 producer and emits a single sub_hi extract instead:
;
;   ld   de, sym     ; (or hl, depending on regalloc)
;   ld   a, d        ; A = sub_hi(DE)
;
; Saves 3 B per call site / store site.

@external_data = external global [1 x i8], align 1

declare void @take_byte(i8 zeroext)

; CHECK-LABEL: call_with_extern_high:
; CHECK:      	ld	a,#>(_external_data)
; CHECK:      	call	_take_byte
; CHECK:      	ret
define void @call_with_extern_high() {
  %1 = ptrtoint ptr @external_data to i16
  %2 = lshr i16 %1, 8
  %3 = trunc i16 %2 to i8
  tail call void @take_byte(i8 zeroext %3)
  ret void
}


define void @store_extern_high() {
  %1 = ptrtoint ptr @external_data to i16
  %2 = lshr i16 %1, 8
  %3 = trunc i16 %2 to i8
  store volatile i8 %3, ptr inttoptr (i16 -3072 to ptr), align 1024
  ret void
; CHECK-LABEL: store_extern_high:
; CHECK:      	ld	bc,#62464
; CHECK:      	ld	a,#>(_external_data)
; CHECK:      	ld	(bc),a
; CHECK:      	ret
}

