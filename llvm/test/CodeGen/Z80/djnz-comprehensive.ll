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
; CHECK-LABEL: _arg_counter_djnz:
; CHECK:       ld  b,a
; CHECK:       {{[ \t]}}djnz{{[ \t]}}
; CHECK:       ret


; Pointer-walk + countdown (canonical sum_array shape).
define i8 @sum_walk(ptr %p, i8 zeroext %n) {
entry:
  br label %loop
loop:
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
; CHECK-LABEL: _sum_walk:
; CHECK:       {{[ \t]}}djnz{{[ \t]}}
; CHECK:       ret


; Counter is observably 0 after the loop -- DJNZ-clobber-B is fine.
define i8 @counter_used_after() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 10, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret i8 %i.next
}
; CHECK-LABEL: _counter_used_after:
; CHECK:       ld  b,#10
; CHECK:       {{[ \t]}}djnz{{[ \t]}}
; CHECK:       ret


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
}
; CHECK-LABEL: _call_in_body_no_djnz:
; CHECK-NOT:   {{[ \t]}}djnz{{[ \t]}}
; CHECK:       call _sink
; CHECK:       ret


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
; CHECK-LABEL: _i16_counter_no_djnz:
; CHECK-NOT:   {{[ \t]}}djnz{{[ \t]}}
; CHECK:       ret


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
  br i1 %c2, label %loop2, label %exit
exit:
  ret void
}
; CHECK-LABEL: _two_sequential_loops:
; KNOWN ISSUE (#94): only ONE of the two sequential loops uses DJNZ
; today.  Both counters get hinted to B, the greedy regalloc picks
; B for loop2 and forces loop1's counter to D (`dec d; jr nz`).
; Since the loops are sequential, B is free between them and both
; should DJNZ.  Pin the current behaviour; flip the CHECK-NOT to a
; second `djnz` once a regalloc-lifetime-aware fix lands.
; CHECK:       {{[ \t]}}djnz{{[ \t]}}
; CHECK-NOT:   {{[ \t]}}djnz{{[ \t]}}
; CHECK:       ret


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
; CHECK-LABEL: _nested_djnz:
; Today exactly one DJNZ fires (on the outer, sub-optimally).  When
; the regalloc hint is fixed to prefer inner, swap which loop has
; `dec r; jr nz` vs `djnz`.
; CHECK:       {{[ \t]}}djnz{{[ \t]}}
; CHECK-NOT:   {{[ \t]}}djnz{{[ \t]}}
; CHECK:       ret


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
; CHECK-LABEL: _const_trip_inc_jrnz:
; CHECK-NOT:   {{[ \t]}}djnz{{[ \t]}}
; CHECK-NOT:   sbc a,a
; CHECK-NOT:   rrca
; CHECK:       inc d
; CHECK-NEXT:  jr  nz,
; CHECK:       ret
