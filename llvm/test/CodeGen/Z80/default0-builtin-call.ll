; RUN: llc -mtriple=z80 -verify-machineinstrs < %s | FileCheck %s
;
; C counterpart: clang/test/CodeGen/z80-default-calling-conv-builtins.c.
; puts must receive its pointer on the stack and return its int in HL.

declare z80_sdcccall0 i16 @puts(ptr)

define z80_sdcccall0 i16 @console(ptr %message) {
; CHECK-LABEL: _console:
; CHECK: push hl
; CHECK-NEXT: call _puts
; CHECK-NEXT: pop af
; CHECK-NOT: ex de,hl
; CHECK: ret
  %result = call z80_sdcccall0 i16 @puts(ptr %message)
  ret i16 %result
}
