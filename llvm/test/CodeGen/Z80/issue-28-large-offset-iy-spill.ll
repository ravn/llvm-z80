; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 \
; RUN:     -z80-static-stack-fp-direct-addr=false < %s | FileCheck %s
;
; NOTE: this test guards the large-offset IX/IY *spill* expansion path.  The
; #263 direct-addressing lever (default ON) turns constant-base frame slots
; into direct absolute loads/stores, bypassing that spill path entirely, so we
; pin the lever OFF here to keep exercising the guarded mechanism.

; ravn/llvm-z80#28 — variable-size memcpy bug at -O0 was the dominant
; cause but not the only one.  This test covers the residual: when a
; function has a stack frame so large that some locals sit at IX-offset
; outside the signed-8-bit displacement range (-128..+127), SPILL_GR16
; / RELOAD_GR16 fall through to expandSpillGR16LargeOffset /
; expandReloadGR16LargeOffset.  Pre-fix those helpers only handled
; BC and DE destinations / sources — for IX or IY they silently used
; the DE-pair encoding (Z80::IY != Z80::BC → DE branch), reloading
; into D/E registers and never updating IY at all.  The miscompile
; surfaced in indirect calls because the call lowering puts the
; function pointer in IY: the reload corrupted DE (the second arg
; reg) and left IY pointing wherever it last did.
;
; Repro: a function with enough locals to push an indirect-call
; function pointer outside the IX-indexed range, then call through
; it.  At -O0 the function pointer is spilled (SPILL_GR16 with IY
; src) and reloaded (RELOAD_GR16 with IY dst), both with offsets
; below -128.

target triple = "z80"

declare void @sink(i16, i16)

; CHECK-LABEL: _f:

; The reload that materializes the function pointer back into IY
; before the indirect call must produce a real load into IY.  Pre-fix
; the expansion emitted `LD E,(HL); INC HL; LD D,(HL)` and never
; transferred to IY (silently miscompiling).  Post-fix the expansion
; loads via a temp pair then PUSH/POP into IY.
;
; We don't pin the exact instruction shape; the regression check is
; that *some* PUSH/POP IY pair exists in the function (the only way
; to get a value into IY from a temp pair on Z80).  Without the fix,
; no such PUSH/POP would appear.
;
; CHECK: pop iy

define void @f(ptr %fnptr) {
entry:
  %locals = alloca [200 x i16], align 1
  %p = alloca ptr, align 1
  store ptr %fnptr, ptr %p
  ; Touch many locals so the alloca survives optimization at -O0.
  %a0 = getelementptr [200 x i16], ptr %locals, i32 0, i32 0
  store volatile i16 1, ptr %a0
  %a199 = getelementptr [200 x i16], ptr %locals, i32 0, i32 199
  store volatile i16 2, ptr %a199
  ; Indirect call — at -O0 the callee spills to %p and reloads into IY.
  %fn = load ptr, ptr %p
  call void %fn(i16 99, i16 155)
  ret void
}
