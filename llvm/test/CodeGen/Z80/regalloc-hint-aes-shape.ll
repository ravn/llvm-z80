; RUN: llc -mtriple=z80 -z80-log-regalloc-hints %s -o /dev/null 2>&1 | FileCheck %s

; ravn/llvm-z80#115 + #27 S1 wiring test (session 73).
;
; Stripped-down kernel mimicking `aes_mc_inv`'s inner block: two 16-bit
; pointer vregs both used as `(HL)`-source via indirect load.  This is
; the shape where greedy regalloc currently spills both pointers to BSS,
; producing the +142 B 16-bit-spill cost on aes_mc_inv vs SDCC.
;
; The test does NOT check the `.s` output.  It verifies that the
; instrumentation hook `-z80-log-regalloc-hints` fires for the relevant
; vregs with the expected register class and use opcodes.  Future
; sessions (S2-S5 per `tasks/plan-115-27-regalloc-cluster.md`) will
; build on top of this knowing the hook reaches the right operands.
;
; Decision-point reference: per the plan, "pause if RC is Anyi16/Ptr16
; not GR16 at hint time".  This test asserts the negative — the
; pointer-vreg RC IS GR16, so the design proceeds along the planned
; path.

target datalayout = "e-m:o-p:16:8-i16:8-n8:16"
target triple = "z80"

@table_a = external global [256 x i8]
@table_b = external global [256 x i8]

define i8 @kernel(i8 %idx) {
; A pointer-arith chain feeding an indirect load is the shape that
; later stages will target.  Log entry must show the load-pointer
; vreg in class GR16, used by LOAD8_IND.
;
; CHECK: z80-regalloc-hint: VReg={{[^ ]+}} RC=GR16 in kernel uses=[LOAD8_IND]
;
; And the i8 idx, after SEXT, lives in a GR16_BCDE-constrained vreg
; (because clang lowers the GEP through ADD_HL_rr where the rr-side
; must be BC or DE — never IX/IY).  This proves the existing hint
; path (which already narrows to BCDE) still fires.
;
; CHECK: z80-regalloc-hint: VReg={{[^ ]+}} RC=GR16_BCDE in kernel uses=[ADD_HL_rr{{(,ADD_HL_rr)?}}]
entry:
  %pa = getelementptr [256 x i8], ptr @table_a, i16 0, i8 %idx
  %pb = getelementptr [256 x i8], ptr @table_b, i16 0, i8 %idx
  %va = load i8, ptr %pa
  %vb = load i8, ptr %pb
  %xor = xor i8 %va, %vb
  ret i8 %xor
}
