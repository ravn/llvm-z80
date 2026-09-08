; RUN: llc -mtriple=z80 < %s | FileCheck %s
;
; ravn/llvm-z80#4: a function carrying the "z80_critical" IR function
; attribute runs as a critical section -- Z80FrameLowering emits DI at entry
; and EI immediately before the return (EI is delayed one instruction, so no
; interrupt can fire between EI and RET).

@counter = external global i8

define void @atomic_update() #0 {
  %v = load i8, ptr @counter
  %inc = add i8 %v, 1
  store i8 %inc, ptr @counter
; CHECK-LABEL: atomic_update:
; CHECK:      	ld	bc,_counter
; CHECK:      	ld	a,(bc)
; CHECK:      	inc	a
; CHECK:      	ld	(bc),a
; CHECK:      	ret
  ret void
}

; A z80_critical function that is ALSO an interrupt handler must NOT get the
; entry DI (hardware already disabled interrupts on entry); the handler's own
; EI;RETI epilogue is unchanged.
define void @crit_isr() #1 {
  store i8 0, ptr @counter
  ret void
}

attributes #0 = { "z80_critical" }
attributes #1 = { "z80_critical" "interrupt" }
; CHECK-LABEL: crit_isr:
; CHECK:      	push	af
; CHECK:      	push	bc
; CHECK:      	ld	bc,_counter
; CHECK:      	xor	a
; CHECK:      	ld	(bc),a
; CHECK:      	pop	bc
; CHECK:      	pop	af
; CHECK:      	reti
