; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -mattr=+static-frame -O2 < %s | FileCheck %s

; Issue #75: CALL nn ; RET → JP nn (tail call) when CALL and RET are
; in different MachineBasicBlocks via fall-through.
;
; The single-MBB version of this peephole already fires when both
; instructions are in the same block.  The cross-MBB case appears in
; if-then-call-without-return shapes where the early-return branch
; produces a separate RET-only MBB.

declare void @helper(i16)

; CHECK-LABEL: cond_call:
; CHECK:      	ld	a,l
; CHECK:      	or	h
; CHECK:      	jr	z,.LBB0_2
; CHECK:      	ex	de,hl
; CHECK:      	call	_helper
; CHECK:      	ret
define void @cond_call(i16 %flag, i16 %x) {
  %t = icmp ne i16 %flag, 0
  br i1 %t, label %call, label %done
call:
  call void @helper(i16 %x)
  br label %done
done:
  ret void
}

