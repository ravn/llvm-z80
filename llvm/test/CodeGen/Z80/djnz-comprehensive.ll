; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Comprehensive DJNZ regression-lock for the Z80 backend.
;
; The peephole + regalloc-hint chain that produces DJNZ on Z80:
;
;   1.  Z80RegisterInfo::getRegAllocationHints biases the loop
;       counter virtreg toward B when its use chain matches
;       `COPY $a, vreg; DEC_A; ...; JR_NZ`.
;   2.  Z80LateOptimization peepholes:
;         (a) DEC_A; LD_B_A; [OR_A;] JR_NZ → DJNZ
;         (b) DEC_B; JR_NZ → DJNZ
;
; This file pins:
;   - positive cases (DJNZ fires)
;   - negative cases (DJNZ correctly does NOT fire)
;   - sequential loops (each independently uses DJNZ)
;   - nested loops (only one B, only one DJNZ; current backend gives
;     it to the OUTER, which is the opposite of optimal -- pinned as
;     the current behaviour, see ravn/llvm-z80#92)


@port = external global ptr, align 2
@buf = external global [256 x i8], align 1
@out = external global i8, align 1

declare void @sink(i8 zeroext)
declare void @sink_noargs()
declare i8 @produce()


;==============================================================================
; POSITIVE: DJNZ should fire
;==============================================================================

; Argument counter (already in A) -> copied to B -> DJNZ.
; CHECK-LABEL: arg_counter_djnz:
; CHECK:      	ld	b,a
; CHECK:      	ld	hl,#_port
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	xor	a
; CHECK:      	ld	(de),a
; CHECK:      	dec	b
; CHECK:      	jr	nz,.LBB0_1
; CHECK:      	ret
define void @arg_counter_djnz(i8 zeroext %n) {
entry:
  br label %loop
loop:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}


; Pointer-walk + countdown (canonical sum_array shape).
define i8 @sum_walk(ptr %p, i8 zeroext %n) {
entry:
  br label %loop
loop:
; CHECK-LABEL: sum_walk:
; CHECK:      	push	hl
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	c,(hl)
; CHECK:      	pop	hl
; CHECK:      	ld	b,#0
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	a,b
; CHECK:      	add	a,d
; CHECK:      	ld	b,a
; CHECK:      	inc	hl
; CHECK:      	dec	c
; CHECK:      	jr	nz,.LBB1_1
; CHECK:      	ld	a,b
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  %ptr = phi ptr [ %p, %entry ], [ %ptr.next, %loop ]
  %sum = phi i8 [ 0, %entry ], [ %sum.next, %loop ]
  %v = load i8, ptr %ptr
  %sum.next = add i8 %sum, %v
  %ptr.next = getelementptr i8, ptr %ptr, i8 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret i8 %sum.next
}


; Counter is observably 0 after the loop -- DJNZ-clobber-B is fine.
define i8 @counter_used_after() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 10, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
; CHECK-LABEL: counter_used_after:
; CHECK:      	ld	b,#10
; CHECK:      	ld	hl,#_port
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	xor	a
; CHECK:      	ld	(de),a
; CHECK:      	dec	b
; CHECK:      	jr	nz,.LBB2_1
; CHECK:      	ld	a,b
; CHECK:      	ret
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret i8 %i.next
}


;==============================================================================
; NEGATIVE: DJNZ should NOT fire
;==============================================================================

; Loop body has a CALL: B is caller-clobbered per sdcccall.
define void @call_in_body_no_djnz(i8 zeroext %n) {
entry:
  br label %loop
loop:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  call void @sink(i8 zeroext %i)
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
; CHECK-LABEL: call_in_body_no_djnz:
; CHECK:      	dec	sp
; CHECK:      	ld	b,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),b
; CHECK:      	ld	a,b
; CHECK:      	call	_sink
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	a,(hl)
; CHECK:      	dec	a
; CHECK:      	ld	b,a
; CHECK:      	jr	nz,.LBB3_1
; CHECK:      	inc	sp
; CHECK:      	ret
}


; i16 counter cannot use DJNZ (B is 8-bit only).
define void @i16_counter_no_djnz(i16 %n) {
entry:
  br label %loop
loop:
  %i = phi i16 [ %n, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i16 %i, -1
  %cond = icmp ne i16 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}


; CHECK-LABEL: i16_counter_no_djnz:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	hl,#_port
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	xor	a
; CHECK:      	ld	(de),a
; CHECK:      	dec	bc
; CHECK:      	ld	a,c
; CHECK:      	or	b
; CHECK:      	jr	nz,.LBB4_1
; CHECK:      	ret
;==============================================================================
; SEQUENTIAL LOOPS: each loop should independently use DJNZ.
; B is freed by the first loop (DJNZ leaves B=0) and rehinted for
; the second.
;==============================================================================

define void @two_sequential_loops(i8 zeroext %n, i8 zeroext %m) {
entry:
  br label %loop1
loop1:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop1 ]
  %p1 = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p1, align 1
  %i.next = add i8 %i, -1
  %c1 = icmp ne i8 %i.next, 0
  br i1 %c1, label %loop1, label %between
between:
  br label %loop2
loop2:
  %j = phi i8 [ %m, %between ], [ %j.next, %loop2 ]
  %p2 = load volatile ptr, ptr @port, align 2
  store volatile i8 1, ptr %p2, align 1
  %j.next = add i8 %j, -1
  %c2 = icmp ne i8 %j.next, 0
; CHECK-LABEL: two_sequential_loops:
; CHECK:      	ld	c,a
; CHECK:      	ld	b,l
; CHECK:      	ld	hl,#_port
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	xor	a
; CHECK:      	ld	(de),a
; CHECK:      	dec	c
; CHECK:      	jr	nz,.LBB5_1
; CHECK:      	ld	hl,#_port
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	a,#1
; CHECK:      	ld	(de),a
; CHECK:      	dec	b
; CHECK:      	jr	nz,.LBB5_2
; CHECK:      	ret
  br i1 %c2, label %loop2, label %exit
exit:
  ret void
}
; FIXED (#94): both sequential loops now use DJNZ.  Z80SplitDjnzCounters
; (post-coalesce, pre-greedy) inserts a fresh COPY of each loop counter
; into a BReg single-register class vreg at the loop preheader; the
; class constraint forces greedy past its copy-elimination heuristic
; and B is reused sequentially across the non-overlapping per-loop
; counter ranges.  The `LD B, D` between the loops is the second-loop
; preheader COPY (1 byte; paid back by the DJNZ savings on loop2).


;==============================================================================
; NESTED LOOPS: only one of inner/outer can DJNZ -- they share B.
; Today the backend gives B (DJNZ) to the OUTER loop, leaving the
; INNER as `dec r; jr nz`.  This is the opposite of optimal: the
; inner runs N×M iterations vs the outer's M, so DJNZ on inner saves
; more total bytes per call.  See ravn/llvm-z80#92.
;==============================================================================

define void @nested_djnz(i8 zeroext %m, i8 zeroext %n) {
entry:
  br label %outer
outer:
  %o = phi i8 [ %m, %entry ], [ %o.next, %outer.latch ]
  br label %inner
inner:
  %i = phi i8 [ %n, %outer ], [ %i.next, %inner ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %inner.cond = icmp ne i8 %i.next, 0
  br i1 %inner.cond, label %inner, label %outer.latch
outer.latch:
  %o.next = add i8 %o, -1
  %outer.cond = icmp ne i8 %o.next, 0
  br i1 %outer.cond, label %outer, label %exit
exit:
  ret void
}
; Today exactly one DJNZ fires (on the outer, sub-optimally).  When
; the regalloc hint is fixed to prefer inner, swap which loop has
; CHECK-LABEL: nested_djnz:
; CHECK:      	ld	c,a
; CHECK:      	ld	a,l
; CHECK:      	ld	(L_nested_djnz.frame),a
; CHECK:      	ld	a,(L_nested_djnz.frame)
; CHECK:      	ld	b,a
; CHECK:      	ld	hl,#_port
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	xor	a
; CHECK:      	ld	(de),a
; CHECK:      	dec	b
; CHECK:      	jr	nz,.LBB6_2
; CHECK:      	dec	c
; CHECK:      	jr	nz,.LBB6_1
; CHECK:      	ret
; `dec r; jr nz` vs `djnz`.


;==============================================================================
; FUTURE: nested loops with PUSH BC / POP BC could let BOTH loops use
; DJNZ.  Cost: 2 B per outer iter (push + pop).  Wins iff the inner
; runs enough iterations that its DJNZ-vs-dec/jr saving exceeds 2 B
; per outer iter.  Not implemented; sketch left here for reference:
;
;   ld   b, m         ; outer counter
;   .outer:
;       push bc       ; +1 B
;       ld   b, n     ; inner counter
;       .inner:
;           ... body ...
;           djnz .inner
;       pop  bc       ; +1 B
;       djnz .outer
;==============================================================================


;==============================================================================
; CONSTANT TRIP COUNT: ravn/llvm-z80#93 fix (path b -- post-RA peephole)
;
; Was: 11-byte carry-roundtrip in the loop body
;        ld a,d; add a,#1; ld d,a; sbc a,a; and #1; xor #1; rrca; jr c
; Now: 3-byte INC + jr nz
;        inc d; jr nz
;
; Counter is still in D, not B, so DJNZ doesn't fire here -- that
; needs path (a) (#95) or a count-up -> count-down rewrite + B hint.
;==============================================================================

define void @const_trip_inc_jrnz() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 50, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}
; CHECK-LABEL: const_trip_inc_jrnz:
; CHECK:      	ld	c,#206
; CHECK:      	ld	hl,#_port
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	xor	a
; CHECK:      	ld	(de),a
; CHECK:      	ld	b,#0
; CHECK:      	inc	bc
; CHECK:      	ld	e,c
; CHECK:      	ld	a,c
; CHECK:      	xor	e
; CHECK:      	or	b
; CHECK:      	add	a,#255
; CHECK:      	sbc	a,a
; CHECK:      	and	#1
; CHECK:      	xor	#1
; CHECK:      	jr	nz,.LBB7_1
; CHECK:      	ret
