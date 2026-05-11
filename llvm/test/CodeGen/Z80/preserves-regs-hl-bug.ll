; RUN: llc -mtriple=z80 -mattr=+static-stack -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s
; XFAIL: *
;
; ravn/llvm-z80#135 — #131 caller-side HL/A preservation is silently
; ignored due to ADJCALLSTACKUP's pessimistic TableGen `Defs = [SP, HL, A]`.
; The #131 strip code in Z80CallLowering removes the matching implicit-defs
; from CALL_nn but NOT from the following ADJCALLSTACKUP, so regalloc still
; treats HL (and A) as clobbered across the call sequence.
;
; This file documents the bug as a failing fixture until #135 is fixed.
; When the fix lands, drop the XFAIL directive above and the test should
; PASS: f_hl will mirror f_de with the registers swapped.
;
; Witness: declare two near-identical functions, one with "d,e" preserved,
; one with "h,l".  Without the bug, both should drop the unrelated-pair
; spill across the call.  Currently `f_de` does (saves DE) but `f_hl` does
; not (still BSS-spills DE around the call AND push/pops HL).

declare void @sink_de_only() #0
declare void @sink_hl_only() #1

; %a in HL, %b in DE.  After call, both must be live for the add.
; With "d,e" preserved, DE survives; only HL needs save/restore.
; CHECK-LABEL: _f_de:
; CHECK-NOT:   ld ({{.*}}),de
; CHECK-NOT:   ld de,({{.*}})
; CHECK:       call _sink_de_only
; CHECK-NOT:   ld ({{.*}}),de
; CHECK-NOT:   ld de,({{.*}})
; CHECK:       ret
define i16 @f_de(i16 %a, i16 %b) {
  call void @sink_de_only()
  %s = add i16 %a, %b
  ret i16 %s
}

; Symmetric case: with "h,l" preserved, HL should survive across the call.
; No push hl / pop hl should be emitted; DE is the one that needs spill.
;
; Today (#135): HL push/pop is emitted AND DE is BSS-spilled, because
; regalloc sees ADJCALLSTACKUP's implicit-def of HL and treats HL as dead.
;
; CHECK-LABEL: _f_hl:
; CHECK-NOT:   push hl
; CHECK-NOT:   pop hl
; CHECK:       call _sink_hl_only
; CHECK-NOT:   push hl
; CHECK-NOT:   pop hl
; CHECK:       ret
define i16 @f_hl(i16 %a, i16 %b) {
  call void @sink_hl_only()
  %s = add i16 %a, %b
  ret i16 %s
}

attributes #0 = { "z80-preserves-regs"="d,e" }
attributes #1 = { "z80-preserves-regs"="h,l" }
