; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-float-sdcccall0 %s -o - | FileCheck %s
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 %s -o - | FileCheck --check-prefix=DEFAULT %s
;
; ravn/llvm-z80 #277: under the opt-in flag `-z80-float-sdcccall0`, the f32
; arithmetic libcalls (__addsf3/__subsf3/__mulsf3/__divsf3, and their _fast
; nnan/ninf/nsz variants) are emitted with CallingConv::Z80_SDCCCall0 instead
; of the default CallingConv::C.
;
; The flag is OFF by default and must stay that way: TWO different float
; runtimes exist for this target -- the ELF/standalone path's own compiler-rt
; __addsf3 (written for the default sdcccall(1) ABI) vs z88dk math32's
; sdcccall(0)-ABI wrappers (only linked in by `zcc -compiler=llvmz80`). Only
; the z88dk build passes this flag (via `-mllvm -z80-float-sdcccall0`); the
; DEFAULT run below pins that the ELF path's ABI is unchanged when the flag
; is absent.
;
; WHY: z88dk's math32 float runtime already ships SDCC-ABI wrappers written
; for exactly this convention (cm32_sdcc_fsadd/fssub/fsmul/fsdiv --
; libsrc/math/float/math32/c/sdcc/ in z88dk).  sdcccall(0) pushes ALL
; arguments on the stack in declared order (no register-passed args), and
; returns a 32-bit result in DE:HL (D=MSB).  That is a byte-for-byte match to
; what cm32_sdcc_fsadd expects, so the z88dk bridge can be a pure `JP` alias
; with no operand-reordering glue code (verified end-to-end at runtime under
; ntvcm with clang's double==float32 config, ravn/llvm-z80#277).
;
; BEFORE this change (default CallingConv::C / sdcccall(1)): the first f32
; argument came back in HL:DE (H=MSB) and only the second argument was pushed
; on the stack -- see the (now superseded) measured-ABI comment at the top of
; z88dk's libsrc/l/llvmz80/__addsf3.asm, which required a word-swap shim.
;
; This test pins the NEW convention: BOTH operands of `fadd float %a, %b` are
; pushed to the stack (4 `push hl` total, one per 16-bit half of each 32-bit
; float) before `call ___addsf3`, and the 32-bit result comes back through an
; `ex de,hl` (i.e. DE:HL with D=MSB, not the caller's native HL:DE).

define float @add(float %a, float %b) {
; CHECK-LABEL: _add:
; CHECK: push hl
; CHECK: push hl
; CHECK: push hl
; CHECK: push hl
; CHECK: call ___addsf3
; CHECK: ex de,hl
;
; DEFAULT-LABEL: _add:
; DEFAULT: push hl
; DEFAULT: push hl
; DEFAULT-NOT: push hl
; DEFAULT: call ___addsf3
  %r = fadd float %a, %b
  ret float %r
}
