; RUN: llc -mtriple=z80 -mattr=+static-stack -z80-asm-format=sdasz80 \
; RUN:     -O2 -disable-lsr < %s | FileCheck %s

; ravn/llvm-z80#114 — EXX-bracket prototype candidate.
;
; Synthetic reproducer for the textbook "outer-counter parked across
; inner no-CALL byte-twiddle loop" pattern observed in rcbios
; `_specc` 0xde19-0xde3c.
;
; The function `render` has:
;   - a u16 outer counter `i` allocated to BC across the outer
;     back-edge (used to compute dp/sp at outer iter start, and
;     INC'd + compared at outer back-edge);
;   - an inner do-while loop with no CALL, holding three pointer-
;     shaped values (dp/sp/stride literal) plus a u8 acc and a u8
;     DJNZ-counter — pressuring `i` out of all three GR16 pairs;
;   - hence the regalloc spills BC to a sframe BSS slot at the
;     entry to the inner loop and reloads it at the outer back-edge.
;
; Today's codegen emits:
;
;     LD (__sfrend_render-9),BC      ; ED 43 nn nn   (4 bytes)
;     ...inner loop body...
;     LD BC,(__sfrend_render-9)      ; ED 4B nn nn   (4 bytes)
;     INC BC
;     JR  outer
;
; The EXX-bracket transform (#114) replaces this 8-byte pair with:
;
;     EXX                            ; D9            (1 byte)
;     ...inner loop body...
;     EXX                            ; D9            (1 byte)
;     INC BC
;     JR  outer
;
; Code savings: 6 bytes per fired loop, plus 2 bytes BSS reclaimed.
; Runtime savings: LD (nn),BC = 20 T-states; EXX = 4 T-states; ~5x
; faster per spill, executed once per outer iteration.
;
; This test asserts the *current* codegen so the candidate doesn't
; silently disappear due to unrelated regalloc changes.  When #114
; lands, flip the CHECK lines to assert the EXX shape and remove
; the BSS-slot symbol assertion.

@out_buf  = external global [0 x i8]
@in_buf   = external global [0 x i8]
@end_idx  = external global i16
@inner_n  = external global i8

; CHECK-LABEL: _render:

; Document-order asm layout (MBB scheduling places the inner
; loop and outer.latch before the outer header):
;   1. inner loop body with DJNZ
;   2. outer.latch:  reload BC from sframe slot, INC BC, exit-test
;   3. outer header: compute dp/sp; spill BC to sframe slot

; CHECK: djnz
; CHECK: ld bc,([[SLOT:__sfrend_render[+-][0-9]+]])
; CHECK: inc bc
; CHECK: ld ([[SLOT]]),bc

define void @render() {
entry:
  %end = load i16, ptr @end_idx, align 1
  %nz  = icmp ne i16 %end, 0
  br i1 %nz, label %outer, label %ret

outer:
  %i = phi i16 [ 0, %entry ], [ %i.next, %outer.latch ]
  %dp.base = getelementptr inbounds i8, ptr @out_buf, i16 %i
  %sp.base = getelementptr inbounds i8, ptr @in_buf,  i16 %i
  %acc0 = trunc i16 %i to i8
  %k0   = load volatile i8, ptr @inner_n, align 1
  br label %inner

inner:
  %dp  = phi ptr [ %dp.base, %outer ], [ %dp.next, %inner ]
  %sp  = phi ptr [ %sp.base, %outer ], [ %sp.next, %inner ]
  %acc = phi i8  [ %acc0,    %outer ], [ %acc.next, %inner ]
  %k   = phi i8  [ %k0,      %outer ], [ %k.next,   %inner ]
  %v   = load i8, ptr %sp, align 1
  %acc.next = xor i8 %acc, %v
  store i8 %acc.next, ptr %dp, align 1
  %sp.next = getelementptr inbounds i8, ptr %sp, i16 80
  %dp.next = getelementptr inbounds i8, ptr %dp, i16 80
  %k.next  = sub i8 %k, 1
  %k.nz    = icmp ne i8 %k.next, 0
  br i1 %k.nz, label %inner, label %outer.latch

outer.latch:
  %i.next = add i16 %i, 1
  %done   = icmp eq i16 %i.next, %end
  br i1 %done, label %ret, label %outer

ret:
  ret void
}
