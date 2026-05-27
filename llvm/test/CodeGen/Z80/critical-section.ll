; RUN: llc -mtriple=z80 < %s | FileCheck %s
;
; ravn/llvm-z80#4: a function carrying the "z80_critical" IR function
; attribute runs as a critical section -- Z80FrameLowering emits DI at entry
; and EI immediately before the return (EI is delayed one instruction, so no
; interrupt can fire between EI and RET).

@counter = external global i8

; CHECK-LABEL: _atomic_update:
; CHECK:       di
; CHECK:       ei
; CHECK-NEXT:  ret
define void @atomic_update() #0 {
  %v = load i8, ptr @counter
  %inc = add i8 %v, 1
  store i8 %inc, ptr @counter
  ret void
}

; A z80_critical function that is ALSO an interrupt handler must NOT get the
; entry DI (hardware already disabled interrupts on entry); the handler's own
; EI;RETI epilogue is unchanged.
; CHECK-LABEL: _crit_isr:
; CHECK-NOT:   di
; CHECK:       ei
; CHECK-NEXT:  reti
define void @crit_isr() #1 {
  store i8 0, ptr @counter
  ret void
}

attributes #0 = { "z80_critical" }
attributes #1 = { "z80_critical" "interrupt" }
