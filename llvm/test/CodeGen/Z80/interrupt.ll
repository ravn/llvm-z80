; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s --check-prefix=Z80
; RUN: llc -mtriple=sm83 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s --check-prefix=SM83

; Test: interrupt handler saves registers and returns with reti
; With direct addressing, only AF is needed for simple ISR (no DE for address)

@g = global i8 0

define void @isr() #0 {
; Z80-LABEL: _isr:
; Z80:       push af
; Z80-NOT:   push de
; Z80:       reti

; SM83-LABEL: _isr:
; SM83:       push af
; SM83:       reti
  store volatile i8 66, ptr @g
  ret void
}

; Test: non-interrupt function uses ret (not reti)
define void @normal() {
; Z80-LABEL: _normal:
; Z80-NOT:   reti
; Z80:       ret

; SM83-LABEL: _normal:
; SM83-NOT:   reti
; SM83:       ret
  store volatile i8 66, ptr @g
  ret void
}

; Test: interrupt handler with more register pressure saves more pairs
; With direct addressing, globals are accessed via LD A,(addr) — only AF used
define void @isr_complex() #0 {
; Z80-LABEL: _isr_complex:
; Z80:       push af
; Z80-NOT:   push de
; Z80:       reti

; SM83-LABEL: _isr_complex:
; SM83:       push af
; SM83:       reti
  %a = load volatile i8, ptr @g
  %b = add i8 %a, 1
  store volatile i8 %b, ptr @g
  %c = load volatile i8, ptr @g
  %d = add i8 %c, 2
  store volatile i8 %d, ptr @g
  %e = load volatile i8, ptr @g
  %f = add i8 %e, 3
  store volatile i8 %f, ptr @g
  ret void
}

attributes #0 = { "interrupt" }
