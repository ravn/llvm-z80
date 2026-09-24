; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; Case 1: Loop counter in B -> should fold to DJNZ, NOT ld a,b / dec a / ld b,a / jr nz
; CHECK-LABEL: countdown_djnz:
; CHECK-NOT:   ld	a,b
; CHECK:       djnz
; C source:
;   typedef unsigned char uint8_t;
;   /* Case 1: counter already in B — direct DJNZ, no LD A,B / LD B,A */
;   void countdown_djnz(uint8_t n) { for (; n != 0; n--) {} }
;   /* Case 2: decrement while preserving A */
;   uint8_t dec_preserve_a(uint8_t n) {
;       uint8_t a_val = 42;
;       for (; n != 0; n--) {}
;       return a_val;
;   }
; Counter already in B: DEC B; JR NZ → DJNZ (2 B, no extra copies).

define void @countdown_djnz(i8 zeroext %n) {
entry:
  br label %loop
loop:
  %k = phi i8 [ %n, %entry ], [ %k.next, %loop ]
  %k.next = sub i8 %k, 1
  %c = icmp ne i8 %k.next, 0
  br i1 %c, label %loop, label %exit
exit:
  ret void
}

; Case 2: In-place 8-bit decrement preserving A across operation
; CHECK-LABEL: dec_preserve_a:
; CHECK-NOT:   ld	a,b
; CHECK:       dec	b
define zeroext i8 @dec_preserve_a(i8 zeroext %v) {
  call void asm sideeffect "", "~{a}"()
  %r = sub i8 %v, 1
  call void asm sideeffect "", "~{a}"()
  ret i8 %r
}

; Case 3: In-place 8-bit increment preserving A across operation
; CHECK-LABEL: inc_preserve_a:
; CHECK-NOT:   ld	a,b
; CHECK:       inc	b
define zeroext i8 @inc_preserve_a(i8 zeroext %v) {
  call void asm sideeffect "", "~{a}"()
  %r = add i8 %v, 1
  call void asm sideeffect "", "~{a}"()
  ret i8 %r
}

; Case 4: Nested loop with inner loop folded to djnz
; CHECK-LABEL: nested_countdown:
; CHECK:       djnz
define void @nested_countdown(i8 zeroext %outer, i8 zeroext %inner) {
entry:
  %tobool = icmp eq i8 %outer, 0
  br i1 %tobool, label %exit, label %outer.loop

outer.loop:
  %o = phi i8 [ %outer, %entry ], [ %o.dec, %outer.latch ]
  br label %inner.loop

inner.loop:
  %i = phi i8 [ %inner, %outer.loop ], [ %i.dec, %inner.loop ]
  tail call void asm sideeffect "", ""()
  %i.dec = sub i8 %i, 1
  %i.cmp = icmp ne i8 %i.dec, 0
  br i1 %i.cmp, label %inner.loop, label %outer.latch

outer.latch:
  %o.dec = sub i8 %o, 1
  %o.cmp = icmp ne i8 %o.dec, 0
  br i1 %o.cmp, label %outer.loop, label %exit

exit:
  ret void
}

; Case 5: Nested loop with mid counter C and outer counter B
; CHECK-LABEL: triple_delay:
; CHECK-NOT:   ld	a,c
; CHECK:       dec	c
; CHECK-NOT:   ld	a,b
; CHECK:       dec	b
define void @triple_delay(i8 zeroext %outer, i8 zeroext %inner) {
entry:
  %cmp = icmp eq i8 %outer, 0
  br i1 %cmp, label %exit, label %loop.outer

loop.outer:
  %o = phi i8 [ %outer, %entry ], [ %o.next, %latch.outer ]
  br label %loop.mid

loop.mid:
  %m = phi i8 [ %inner, %loop.outer ], [ %m.next, %latch.mid ]
  br label %loop.inner

loop.inner:
  %k = phi i8 [ 0, %loop.mid ], [ %k.next, %loop.inner ]
  tail call void asm sideeffect "", ""()
  %k.next = add i8 %k, -1
  %k.cmp = icmp ne i8 %k.next, 0
  br i1 %k.cmp, label %latch.mid, label %loop.inner

latch.mid:
  %m.next = add i8 %m, -1
  %m.cmp = icmp ne i8 %m.next, 0
  br i1 %m.cmp, label %latch.outer, label %loop.mid

latch.outer:
  %o.next = add i8 %o, -1
  %o.cmp = icmp ne i8 %o.next, 0
  br i1 %o.cmp, label %exit, label %loop.outer

exit:
  ret void
}
