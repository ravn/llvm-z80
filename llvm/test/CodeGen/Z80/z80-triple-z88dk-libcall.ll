; RUN: opt -passes=instcombine -S < %s | FileCheck %s
;
; Under the z80-unknown-none-z88dk triple, middle-end libcall simplification
; (such as printf("foo\n") -> puts("foo")) automatically stamps the synthesized
; declaration with CallingConv::Z80_SmallC (cc129), matching the classic z88dk C
; library calling convention. No flag needed — the triple drives this.

target triple = "z80-unknown-none-z88dk"

@.str = private unnamed_addr constant [5 x i8] c"foo\0A\00"

declare i16 @printf(ptr, ...)

define void @print_banner() {
; CHECK-LABEL: define void @print_banner()
; CHECK:         call cc129 i16 @puts(
  %call = call i16 (ptr, ...) @printf(ptr @.str)
  ret void
}

; CHECK: declare cc129 {{.*}}i16 @puts(
