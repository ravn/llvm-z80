; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #77 (regression-lock): u8 countdown loops should compile to
; DJNZ.  Two source shapes:
;
;   do { body; } while (--n);     ← DJNZ already fires (locked here)
;   while (n--) { body; }         ← currently does NOT use DJNZ; the
;                                   counter ends up in C with a per-
;                                   iter ld a,c / dec a / ld c,a round-
;                                   trip.  Future fix; this file pins
;                                   the do-while form so we don't lose
;                                   it during the while-form fix.

@port = external global ptr, align 2

declare void @write_byte(i8 zeroext)

; do { body; } while (--n)  -- DJNZ is the canonical lowering.
; CHECK-LABEL: do_while_dec:
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
define void @do_while_dec(i8 zeroext %n) {
entry:
  br label %loop
loop:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  ; Some side effect that doesn't touch the counter.
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}


; Sanity: a body that calls a function (clobbers B per sdcccall) cannot
; use DJNZ.  Pin the fallback shape.
define void @do_while_dec_with_call(i8 zeroext %n) {
entry:
  br label %loop
loop:
; CHECK-LABEL: do_while_dec_with_call:
; CHECK:      	dec	sp
; CHECK:      	ld	b,a
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),b
; CHECK:      	ld	a,b
; CHECK:      	call	_write_byte
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	a,(hl)
; CHECK:      	dec	a
; CHECK:      	ld	b,a
; CHECK:      	jr	nz,.LBB1_1
; CHECK:      	inc	sp
; CHECK:      	ret
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  call void @write_byte(i8 zeroext %i)
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}

