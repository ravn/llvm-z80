; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s --check-prefix=Z80
; RUN: llc -verify-machineinstrs -mtriple=sm83 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s --check-prefix=SM83
;
; ravn/llvm-z80#322: Machine Outliner is explicitly disabled in
; Z80TargetMachine::createPassConfig (EnableMachineOutliner = false) due to
; 4B call/ret overhead — a run of repeating instructions is worth 3B of CALL
; and 1B of RET, so on Z80 the call is smaller AND slower than what it
; replaces (only minsize would take the trade, and even then reaching locals
; through IX gets tricky because the CALL's 2-byte return-address push moves
; SP but not IX; on SM83 all local access goes through SP so it's worse).
;
; This test pins the design decision: no _OUTLINED_FUNCTION_ symbol may be
; emitted regardless of `minsize` / `optsize` / any function attribute. If
; someone re-enables MachineOutliner without the Z80-specific cost analysis
; that motivated the disable, this test will fire.

declare zeroext i16 @g(i16 zeroext)

define zeroext i16 @f1(i16 zeroext %a, i16 zeroext %b) minsize "frame-pointer"="all" {
entry:
  %xor = xor i16 %a, 4660
  %call = tail call zeroext i16 @g(i16 zeroext %xor)
  %xor1 = xor i16 %b, 4660
  %call2 = tail call zeroext i16 @g(i16 zeroext %xor1)
  %and = and i16 %a, 3855
  %and4 = and i16 %b, 3855
  %add = add i16 %and4, %and
  %add3 = add i16 %add, %call
  %add5 = add i16 %add3, %call2
  ret i16 %add5
}

define zeroext i16 @f2(i16 zeroext %a, i16 zeroext %b) minsize "frame-pointer"="all" {
entry:
  %xor = xor i16 %b, 4660
  %call = tail call zeroext i16 @g(i16 zeroext %xor)
  %xor1 = xor i16 %a, 4660
  %call2 = tail call zeroext i16 @g(i16 zeroext %xor1)
  %and = and i16 %b, 3855
  %and4 = and i16 %a, 3855
  %add = add i16 %and, %and4
  %add3 = add i16 %add, %call
  %add5 = add i16 %add3, %call2
  ret i16 %add5
}

define zeroext i16 @f3(i16 zeroext %a, i16 zeroext %b) optsize "frame-pointer"="all" {
entry:
  %or = or i16 %a, 21845
  %call = tail call zeroext i16 @g(i16 zeroext %or)
  %or1 = or i16 %b, 21845
  %call2 = tail call zeroext i16 @g(i16 zeroext %or1)
  %sub = sub i16 %call, %call2
  ret i16 %sub
}

define zeroext i16 @f4(i16 zeroext %a, i16 zeroext %b) optsize "frame-pointer"="all" {
entry:
  %or = or i16 %b, 21845
  %call = tail call zeroext i16 @g(i16 zeroext %or)
  %or1 = or i16 %a, 21845
  %call2 = tail call zeroext i16 @g(i16 zeroext %or1)
  %sub = sub i16 %call, %call2
  ret i16 %sub
}

; No outlined helper symbol may appear anywhere in the output.
; Z80-NOT:    _OUTLINED_FUNCTION_
; SM83-NOT:   _OUTLINED_FUNCTION_
; Z80-NOT:    call _OUTLINED
; SM83-NOT:   call _OUTLINED
