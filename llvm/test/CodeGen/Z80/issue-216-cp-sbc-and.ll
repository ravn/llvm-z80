; RUN: llc -O2 -mtriple=z80 < %s | FileCheck %s
;
; XFAIL: *
;
; ravn/llvm-z80#216 -- the `(x < imm) ? value : 0` idiom should lower
; to the Z80-canonical `cp imm; sbc a, a; and value` (5 B) instead of
; the branchy `cp imm; jr c; ld r, 0` form (8 B).
;
; XFAIL because the 2026-06-21 implementation attempt found the
; Z80LowerSelect approach unworkable: IRTranslator lowers
; `select i1 cond, x, const0` directly to a triangle (G_BRCOND in
; head, empty fallthrough, G_PHI in tail) -- there is no G_SELECT
; for Z80LowerSelect to intercept.  See #216 closing comment for
; alternative fix paths (pre-legalize combiner OR post-RA cross-BB
; peephole); both are heavier than the ~3 B production win warrants
; today.  Test stays as the canonical RED reproducer; flip to PASS
; when one of the alternative paths is implemented.
;
; `sbc a, a` broadcasts CF to A: A = 0xFF if CF (line<imm), 0x00 else.
; `and value` then masks the value by that mask.
;
; Pattern requires:
;   * i8 type (the `sbc a, a` trick only produces 0xFF/0x00 in A)
;   * unsigned less-than (`cp` sets CF when A < operand)
;   * one select arm is constant 0 (the "and value" gets free zero)

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; Minimal reproducer per the issue body.
define i8 @branch_bot_or_zero(i8 %line, i8 %bot) {
  %c = icmp ult i8 %line, 11
  %r = select i1 %c, i8 %bot, i8 0
  ret i8 %r
}

; CHECK-LABEL: branch_bot_or_zero:
; Post-fix expected shape: cp 11; sbc a,a; and l; ret (or with a's
; sub-register convention -- the value arrives in L per sdcccall second
; i8 arg, or A if the convention pre-shuffles).
; Forbid the branchy form:
; CHECK-NOT: jr{{[[:space:]]+}}c
; CHECK-NOT: jr{{[[:space:]]+}}nc
; CHECK: sbc{{[[:space:]]+}}a, a
; CHECK: and
; CHECK: ret
