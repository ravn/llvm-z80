; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=NORMAL
; RUN: llc -mtriple=z80 -O2 -mattr=+shadow-isr < %s | FileCheck %s --check-prefix=SHADOW

; With +shadow-isr, interrupt functions save AF/BC/DE/HL via EXX/EX AF,AF'
; instead of explicit PUSH/POP.  Only IY needs an explicit PUSH/POP.
; IX frame pointer is never emitted for shadow-isr ISRs.

; NORMAL-LABEL: _isr_no_shadow:
; NORMAL-NOT:   exx
; NORMAL-NOT:   ex af,af'
; NORMAL:       push {{af|bc|de|hl}}
; NORMAL:       pop {{af|bc|de|hl}}
; NORMAL:       reti

; SHADOW-LABEL: _isr_shadow:
; SHADOW:       exx
; SHADOW:       ex af,af'
; SHADOW-NOT:   push ix
; SHADOW-NOT:   push af
; SHADOW-NOT:   push bc
; SHADOW-NOT:   push de
; SHADOW-NOT:   push hl
; SHADOW:       ex af,af'
; SHADOW:       exx
; SHADOW:       reti

define void @isr_no_shadow() #0 {
  store volatile i8 42, ptr inttoptr(i16 1234 to ptr)
  ret void
}

define void @isr_shadow() #1 {
  store volatile i8 42, ptr inttoptr(i16 1234 to ptr)
  ret void
}

attributes #0 = { "interrupt" }
attributes #1 = { "interrupt" "target-features"="+shadow-isr" }
