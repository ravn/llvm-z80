; RUN: opt -passes=instcombine -z80-classic-libc-cc -S < %s | FileCheck %s --check-prefix=SMALLC
; RUN: opt -passes=instcombine -S < %s | FileCheck %s --check-prefix=DEFAULT
;
; ravn/llvm-z80 #57 (z88dk classic clib ABI on middle-end-synthesized libcalls).
;
; InstCombine's LibCallSimplifier rewrites printf("foo\n") -> puts("foo") and
; synthesizes the `puts` declaration itself. On the z80 classic (z88dk) path the
; real _puts routine uses the __smallc convention (CallingConv::Z80_SmallC =
; cc132: stack args, caller cleanup), so a call emitted with the default C CC
; reads stack garbage at runtime. With -z80-classic-libc-cc the synthesized decl
; (and therefore the call, which copies the callee CC) is stamped cc132; without
; the flag the behaviour is unchanged (default C CC), so the ELF/sdcccall path is
; unaffected.

target triple = "z80-unknown-unknown"

@.str = private unnamed_addr constant [5 x i8] c"foo\0A\00"

declare i16 @printf(ptr, ...)

define void @print_banner() {
; SMALLC-LABEL: define void @print_banner()
; SMALLC:         call cc132 i16 @puts(
;
; DEFAULT-LABEL: define void @print_banner()
; DEFAULT-NOT:    cc132
; DEFAULT:        call i16 @puts(
  %call = call i16 (ptr, ...) @printf(ptr @.str)
  ret void
}

; The synthesized declaration itself carries the convention under the flag.
; SMALLC: declare cc132 {{.*}}i16 @puts(
; DEFAULT-NOT: declare cc132 {{.*}}@puts(
