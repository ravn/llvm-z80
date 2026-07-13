; RUN: llc -mtriple=z80 -mattr=+static-stack -O0 -verify-machineinstrs \
; RUN:     -z80-static-stack-fp-direct-addr < %s | FileCheck %s
;
; ravn/llvm-z80#263 (latent bug exposed by the static-stack direct-addressing
; lever): the RMW->bit-set peephole in Z80LateOptimization rewrites
;   ld a,(mem) ; or <1<<n> ; ld (mem),a   ==>   ld hl,mem ; set n,(hl)
; but it built the `ld hl,mem` address with
;   NewLd.addSym(Addr.getMCSymbol(), Addr.getOffset())
; -- and addSym's SECOND argument is TargetFlags, NOT an offset.  For an
; MCSymbol operand carrying a non-zero offset (a static-stack frame slot such
; as __sfrend_f-3) the offset was silently dropped, so the bit-set hit
; __sfrend_f+0 instead of __sfrend_f-3: a wrong-address miscompile.  (The
; GlobalAddress branch was unaffected: addGlobalAddress's 2nd arg IS the
; offset.)
;
; With the fix the `ld hl,...` retains the frame-slot offset.

target triple = "z80"

define dso_local zeroext i8 @f() {
; CHECK-LABEL: f:
; The bit-set must load the SLOT address (non-zero offset), not the bare base:
; CHECK: ld hl,__sfrend_f{{[-+][0-9]+}}
; CHECK-NEXT: set {{[0-9]}},(hl)
; CHECK-NOT: ld hl,__sfrend_f{{$}}
  %x = alloca i16, align 1
  %r = alloca i8, align 1
  store volatile i16 100, ptr %x, align 1
  store i8 0, ptr %r, align 1
  %1 = load volatile i16, ptr %x, align 1
  %2 = icmp uge i16 %1, 100
  br i1 %2, label %set, label %end

set:
  %3 = load i8, ptr %r, align 1
  %4 = or i8 %3, 1
  store i8 %4, ptr %r, align 1
  br label %end

end:
  %5 = load i8, ptr %r, align 1
  ret i8 %5
}
