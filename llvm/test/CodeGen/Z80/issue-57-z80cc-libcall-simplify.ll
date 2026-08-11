; RUN: opt -passes=instcombine -z80-classic-libc-cc -mtriple=z80-unknown-unknown -S < %s | FileCheck %s --check-prefix=Z80
; RUN: opt -passes=instcombine -z80-classic-libc-cc -mtriple=x86_64-- -S < %s | FileCheck %s --check-prefix=OTHER
;
; ravn/llvm-z80 #57 (z88dk#57): libcall simplification must fire for z88dk
; classic-clib functions even though they carry an explicit Z80 calling
; convention rather than plain C.
;
; The z88dk headers declare printf with __attribute__((sdcccall(0))) ->
; CallingConv::Z80_SDCCCall0 (its real clib ABI: count returned in HL, varargs
; pushed right-to-left).  Before the fix, TargetLibraryInfoImpl::
; isCallingConvCCompatible() returned false for that CC, so InstCombine's
; LibCallSimplifier refused to rewrite printf("foo\n") -> puts("foo") and the
; ~20% banner-code size win was lost whenever the classic ABI was honored.
;
; With the fix, on a z80 target the Z80 clib CCs are treated as C-compatible for
; the purpose of libcall simplification, so the transform fires; the synthesized
; replacement (puts/putchar/...) is stamped cc132 (Z80_SmallC) by
; -z80-classic-libc-cc.  On any other target the Z80 CC stays non-C-compatible,
; so the transform must NOT fire.
;
; target triple supplied on the RUN line (-mtriple) so both runs share this body.

@.str      = private unnamed_addr constant [5 x i8] c"foo\0A\00"
@.str.1    = private unnamed_addr constant [2 x i8] c"x\00"
@.str.2    = private unnamed_addr constant [3 x i8] c"%d\00"
@.str.only = private unnamed_addr constant [4 x i8] c"abc\00"

declare z80_sdcccall0 i16 @printf(ptr, ...)
declare cc132 i16 @puts(ptr)

;======================================================================
; (a) EXACT bug pattern: printf("foo\n"), sdcccall(0) -> puts, cc132.
;======================================================================
define void @print_banner() {
; Z80-LABEL: define void @print_banner()
; Z80:         call cc132 i16 @puts(
; Z80-NOT:     ) @printf(
;
; OTHER-LABEL: define void @print_banner()
; OTHER:         call z80_sdcccall0 i16 (ptr, ...) @printf(
; OTHER-NOT:     @puts
; OTHER:         ret void
  %call = call z80_sdcccall0 i16 (ptr, ...) @printf(ptr @.str)
  ret void
}

;======================================================================
; (b) STRUCTURAL VARIATIONS.
;======================================================================

; printf of a single-char string "x" folds to putchar; the synthesized putchar
; must also carry cc132.
define void @print_char() {
; Z80-LABEL: define void @print_char()
; Z80:         call cc132 i16 @putchar(i16 120)
; Z80-NOT:     ) @printf(
  %call = call z80_sdcccall0 i16 (ptr, ...) @printf(ptr @.str.1)
  ret void
}

; A direct puts declared __smallc (z80_smallc, cc132) must survive
; simplification with its cc132 intact (isCallingConvCCompatible must accept
; z80_smallc so the call is not left in an inconsistent state).
define i16 @keep_puts() {
; Z80-LABEL: define i16 @keep_puts()
; Z80:         call cc132 i16 @puts(
  %r = call cc132 i16 @puts(ptr @.str.only)
  ret i16 %r
}

;======================================================================
; (c) POSITIVE CONTROL: the fix is target-gated -- on a non-Z80 target the
; identical IR must NOT be transformed (OTHER prefix on print_banner above).
;======================================================================

;======================================================================
; (d) SAFETY / BOUNDARY: a printf that actually consumes a vararg
; (printf("%d", x)) is NOT reducible to puts/putchar, so it must stay a printf
; call and keep sdcccall(0) -- the transform must neither fire nor corrupt the
; surviving call's CC.
;======================================================================
define void @printf_with_arg(i16 %x) {
; Z80-LABEL: define void @printf_with_arg(i16 %x)
; Z80:         call z80_sdcccall0 i16 (ptr, ...) @printf(ptr {{.*}}@.str.2
; Z80-NOT:     @puts
; Z80-NOT:     @putchar
; Z80:         ret void
  %call = call z80_sdcccall0 i16 (ptr, ...) @printf(ptr @.str.2, i16 %x)
  ret void
}
