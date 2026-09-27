; RUN: opt -passes=instcombine -S < %s | FileCheck %s --check-prefix=Z88DK
; RUN: opt -passes=instcombine -z80-classic-libc-cc=false -S < %s | FileCheck %s --check-prefix=OVERRIDE
;
; Under the z80-unknown-none-z88dk triple, middle-end libcall simplification
; (such as printf("foo\n") -> puts("foo")) automatically stamps the synthesized
; declaration with CallingConv::Z80_SmallC (cc129), matching the classic z88dk C
; library calling convention. The -z80-classic-libc-cc flag can explicitly override this.

target triple = "z80-unknown-none-z88dk"

@.str = private unnamed_addr constant [5 x i8] c"foo\0A\00"

declare i16 @printf(ptr, ...)

define void @print_banner() {
; Z88DK-LABEL: define void @print_banner()
; Z88DK:         call cc129 i16 @puts(
;
; OVERRIDE-LABEL: define void @print_banner()
; OVERRIDE-NOT:   cc129
; OVERRIDE:       call i16 @puts(
  %call = call i16 (ptr, ...) @printf(ptr @.str)
  ret void
}

; Z88DK: declare cc129 {{.*}}i16 @puts(
; OVERRIDE-NOT: declare cc129 {{.*}}@puts(
