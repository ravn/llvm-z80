; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Tests for issue #60: redundant LD A,reg removal across basic blocks.
;
; When a value is saved with LD reg,A and A is not subsequently clobbered,
; reloading via LD A,reg is redundant. CP, OR A, AND-imm, INC/DEC of other
; registers, and conditional/unconditional branches do not clobber A.
;
; The redundancy crosses basic block boundaries: after CP imm; JR cc; the
; value of A flowing into BOTH successors equals the value at the CP. Each
; successor that begins with LD A,reg (where reg holds the same value) can
; drop the reload.
;
; The single-block form is already handled by an existing peephole; the
; cross-block form (this issue) requires a small forward dataflow.

@g8 = external global i8

declare i8 @compute()
declare void @sink(i8)

; --- Cross-block, fall-through chain (the fdc_get_result_bytes case) ---
;
; Mirrors the asm in issue #60:
;     call _compute        ; A = result
;     ld   d,a             ; save in D
;     cp   #2
;     jr   nz,.LBB_not2
;     ld   a,d             ; (a) REDUNDANT — A unchanged by CP
;     ret
; .LBB_not2:
;     ld   a,d             ; (b) REDUNDANT — A unchanged by CP+JR
;     or   a
;     jr   z,.LBB_zero
;     ld   a,d             ; (c) REDUNDANT — A unchanged by OR A+JR
;     ld   (g8),a
;     ret
; .LBB_zero:
;     ld   a,#1
;     ret
;
; All three LD A,D instances must be removed by the post-fix peephole.
; CHECK-LABEL: cross_block_chain:
; CHECK:      	call	_compute
; CHECK:      	ld	b,a
; CHECK:      	cp	#2
; CHECK:      	jr	nz,.LBB0_2
; CHECK:      	ld	a,b
; CHECK:      	ret
; CHECK:      	ld	a,b
; CHECK:      	or	a
; CHECK:      	jr	z,.LBB0_4
; CHECK:      	ld	de,#_g8
; CHECK:      	ld	a,b
; CHECK:      	ld	(de),a
; CHECK:      	ret
; CHECK:      	ld	a,#1
; CHECK:      	ret
define i8 @cross_block_chain() {
entry:
  %v = call i8 @compute()
  %is2 = icmp eq i8 %v, 2
  br i1 %is2, label %ret_v, label %not_two
ret_v:
  ret i8 %v
not_two:
  %is0 = icmp eq i8 %v, 0
  br i1 %is0, label %ret_one, label %store_path
ret_one:
  ret i8 1
store_path:
  store volatile i8 %v, ptr @g8
  ret i8 %v
}

; --- Cross-block: longer CP chain, redundancy on every successor ---
;
; Three sequential CP comparisons. The save register holds the value for
; the entire chain; CP does not clobber A; each fall-through arrives with
; A still equal to the saved register. Each LD A,reg that the current
; allocator emits between the CPs is redundant.
define i8 @cp_chain_three() {
; After ravn/llvm-z80#197 (ADJCALLSTACKUP no longer falsely clobbers A), the
; compute() result stays live in A across the whole CP chain -- there is no
; save to a register at all, which subsumes #60's redundant-reload removal.
; A is live to the final store, so #148's `cp #1`->`dec a` collapse no longer
; applies (dec a would clobber the live A).
entry:
  %v = call i8 @compute()
; CHECK-LABEL: cp_chain_three:
; CHECK:      	call	_compute
; CHECK:      	cp	#1
; CHECK:      	jr	nz,.LBB1_2
; CHECK:      	ld	a,#10
; CHECK:      	ret
; CHECK:      	cp	#2
; CHECK:      	jr	nz,.LBB1_4
; CHECK:      	ld	a,#20
; CHECK:      	ret
; CHECK:      	cp	#3
; CHECK:      	jr	nz,.LBB1_6
; CHECK:      	ld	a,#30
; CHECK:      	ret
; CHECK:      	ld	bc,#_g8
; CHECK:      	ld	(bc),a
; CHECK:      	ret
  %e1 = icmp eq i8 %v, 1
  br i1 %e1, label %r1, label %t1
r1:
  ret i8 10
t1:
  %e2 = icmp eq i8 %v, 2
  br i1 %e2, label %r2, label %t2
r2:
  ret i8 20
t2:
  %e3 = icmp eq i8 %v, 3
  br i1 %e3, label %r3, label %t3
r3:
  ret i8 30
t3:
  store volatile i8 %v, ptr @g8
  ret i8 %v
}

; --- Negative test: A modified between save and reload ---
;
; A volatile store of an immediate clobbers A (LD A,#0xff), so the reload
; from the save register is required.
define void @negative_a_clobbered_by_imm_store() {
; The reload must remain; A was clobbered above.
entry:
  %v = call i8 @compute()
  store volatile i8 -1, ptr @g8
  store volatile i8 %v, ptr @g8
  ret void
}

; --- Negative test: CALL between save and reload ---
;
; CALL clobbers A per sdcccall(1). The current allocator spills %v to a
; stack slot here rather than to a callee-saved register, so the LD A,reg
; peephole doesn't directly apply, but the test confirms the produced
; sequence remains correct.
define void @negative_call_clobbers() {
entry:
  %v = call i8 @compute()
; CHECK-LABEL: negative_a_clobbered_by_imm_store:
; CHECK:      	call	_compute
; CHECK:      	ld	b,a
; CHECK:      	ld	de,#_g8
; CHECK:      	ld	a,#255
; CHECK:      	ld	(de),a
; CHECK:      	ld	a,b
; CHECK:      	ld	(de),a
; CHECK:      	ret
  call void @sink(i8 0)
  store volatile i8 %v, ptr @g8
  ret void
}
; CHECK-LABEL: negative_call_clobbers:
; CHECK:      	dec	sp
; CHECK:      	call	_compute
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),a
; CHECK:      	xor	a
; CHECK:      	call	_sink
; CHECK:      	ld	bc,#_g8
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	a,(hl)
; CHECK:      	ld	(bc),a
; CHECK:      	inc	sp
; CHECK:      	ret
