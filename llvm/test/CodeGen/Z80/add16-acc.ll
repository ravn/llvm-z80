; RUN: llc -mtriple=z80 -mattr=+static-stack -z80-add16-acc < %s | FileCheck %s
;
; ravn/llvm-z80#178: the non-tied ADD16_acc pseudo lowers G_PTR_ADD without
; the tied-operand coalescer trap that miscompiled ADD16_tied (session 73s).
; In the base-reuse shape (p[i] + p[j]) the base must survive the first add.
; Regression guard: BOTH indexed adds must use the HL accumulator
; (`add hl,de`) -- NOT the BC-accumulator fallback
; (`push bc; pop hl; add hl,de; push hl; pop bc`) whose `pop hl` clobbered
; the first load result and corrupted an unrelated value.
;
; This path is default-OFF (it pins pointer-arithmetic results to HL, a size
; regression under the IX/IY-reserved allocator); the flag exercises the
; lowering for correctness.  See session73s-issue178-add16-tied-rootcause.md.

define dso_local zeroext i8 @two_idx(ptr noundef readonly %p, i16 noundef %i, i16 noundef %j) {
entry:
  %a = getelementptr inbounds nuw i8, ptr %p, i16 %i
  %va = load i8, ptr %a, align 1
  %b = getelementptr inbounds nuw i8, ptr %p, i16 %j
  %vb = load i8, ptr %b, align 1
  %sum = add i8 %vb, %va
  ret i8 %sum
}

; CHECK-LABEL: two_idx:
; Base is held in BC and re-copied into HL for each add; both adds use HL.
; CHECK:      add hl,de
; CHECK:      add hl,de
; Epilog: i8 return -> HL dead -> EX (SP),HL retcleanup trick (ravn/llvm-z80#146).
; The BC-fallback pop hl between the adds is gone; one pop hl in the epilog is OK.
; CHECK:      pop ix
; CHECK-NEXT: pop hl
; CHECK-NEXT: ex (sp),hl
; CHECK-NEXT: ret
