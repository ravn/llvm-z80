; RUN: llc -mtriple=z80 -mattr=+static-stack -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s
;
; ravn/llvm-z80#133 layer 1 — callee-side honoring of
; "z80-preserves-regs" function attribute.  A function declared with
; the attribute whose body modifies a declared-preserved register
; must emit push/pop in prologue/epilogue.
;
; Verifies the value oracle protection that, paired with #131
; caller-side narrowing of the call's RegMask, makes the attribute
; safe end-to-end (callers keep values alive; the callee body
; genuinely preserves them).

declare void @sink(i16)

; This function takes its arg in HL (1st i16), calls @sink (which
; clobbers HL per default sdcccall(1)), and returns its arg.
; Without the "z80-preserves-regs"="hl" attribute, regalloc would
; spill HL to BSS around the call.  With the attribute, regalloc
; must keep HL alive across the call — and the FrameLowering must
; emit PUSH HL in the prologue (since the body holds HL across a
; callee-clobbering instruction).
;
; The clean codegen pattern with the attribute:
;
;   _f:
;       push hl       ; save callee-saved HL (the new CSR addition)
;       call _sink    ; clobbers HL per default
;       pop  hl       ; restore HL
;       ex   de,hl    ; result goes in DE per sdcccall(1)
;       ret
;
; CHECK-LABEL: _f:
; CHECK:       push hl
; CHECK:       call _sink
; CHECK:       pop hl
; CHECK:       ret
define i16 @f(i16 %x) #0 {
  call void @sink(i16 0)
  ret i16 %x
}

attributes #0 = { "z80-preserves-regs"="hl" }
