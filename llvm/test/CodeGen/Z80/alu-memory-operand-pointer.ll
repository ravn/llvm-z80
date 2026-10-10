; RUN: llc -mtriple=z80 -verify-machineinstrs %s -o - | FileCheck %s

target triple = "z80"

define i8 @or_from_ptr(ptr %p, i8 %x) {
entry:
  %mem = load i8, ptr %p, align 1
  %result = or i8 %mem, %x
  ret i8 %result
}

; CHECK-LABEL: _or_from_ptr:
; CHECK:       or (hl)
