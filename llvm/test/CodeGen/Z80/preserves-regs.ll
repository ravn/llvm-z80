; RUN: llc -mtriple=z80 -mattr=+static-stack -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s
;
; ravn/llvm-z80#131 — caller-side honoring of "z80-preserves-regs" function
; attribute.  Functions declared with this attribute tell the regalloc that
; the callee preserves the listed registers; the caller can then keep values
; alive in those registers across the call without spilling.
;
; This is the clang/LLVM-IR analog of SDCC's `__preserves_regs(...)`.
;
; Z80 sdcccall(1) returns i16 in DE.  Callees that take an i16 arg and
; return void can plausibly preserve DE (it carries their incoming arg but
; is not in use after the function returns).  This test exercises that
; case: a caller holds a uint16_t in DE across the call and reads it back
; after.

declare void @sink_de_preserves(i16) #0
declare void @sink_de_plain(i16)

; Baseline: callee without attribute clobbers DE, so the caller must spill
; the second arg (in DE) across the call.  Returning the second arg is the
; simplest way to force regalloc to keep it alive across the call.
;
; CHECK-LABEL: _baseline_no_attr:
; CHECK:       push de
; CHECK:       call _sink_de_plain
; CHECK:       pop de
; CHECK:       ret
define i16 @baseline_no_attr(i16 %a, i16 %b) {
  call void @sink_de_plain(i16 %a)
  ret i16 %b
}

; With "z80-preserves-regs"="d,e": DE survives the call, so no BSS spill of
; DE is needed.  The function reduces to a single CALL + RET (or tail jump).
;
; CHECK-LABEL: _preserves_de:
; CHECK-NOT:   ld ({{.*}}),de
; CHECK-NOT:   ld de,({{.*}})
; CHECK-NOT:   push de
; CHECK-NOT:   pop de
; CHECK:       {{jp|call}} _sink_de_preserves
define i16 @preserves_de(i16 %a, i16 %b) {
  call void @sink_de_preserves(i16 %a)
  ret i16 %b
}

attributes #0 = { "z80-preserves-regs"="d,e" }
