; RUN: opt -passes=instcombine -S < %s | FileCheck %s
;
; Ordinary C declarations retain ordinary LLVM behavior without
; target-specific stamping, including declarations with existing calls.

target triple = "z80-unknown-none-z88dk"

@.str = private unnamed_addr constant [5 x i8] c"foo\0A\00"

declare i16 @printf(ptr, ...)
declare i16 @puts(ptr)

define void @existing_puts(ptr %s) {
; CHECK-LABEL: define void @existing_puts(
; CHECK: call i16 @puts(
  %call = call i16 @puts(ptr %s)
  ret void
}

define void @print_banner() {
; CHECK-LABEL: define void @print_banner()
; CHECK:         call i16 @puts(
  %call = call i16 (ptr, ...) @printf(ptr @.str)
  ret void
}
