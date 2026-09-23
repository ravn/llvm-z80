; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -mattr=+static-frame < %s | FileCheck %s
;
; Static-frame codegen, post-#82 conservative state (#74 cross-pair
; extension reverted in commit b843d94, 2026-05-04 -- see ravn/llvm-z80#74).
;
; A uint16_t loop counter held in a register pair (BC here) gets *also*
; assigned a static-frame slot, with the call-arg use site loading the
; byte from the slot via a different register pair (HL).
;
; ravn/llvm-z80#82 (commit 87eaf1d, fixed 2026-05-02) ensured the
; BSS-spill->PUSH/POP peephole bails when an orphan reload is into a
; *different* register pair, keeping the slot store/load in BSS rather
; than producing an unbalanced PUSH+POP that would corrupt SP-relative
; data.
;
; ravn/llvm-z80#74 (commit 96dde0c, also 2026-05-02) extended the
; peephole to convert the cross-pair case too: PUSH (storeReg);
; POP (loadReg) preserves value bytes regardless of which 16-bit pair
; is used.  That extension was reverted in b843d94 because it broke
; autoload-in-c boot.  Mechanism unknown -- conservative fix that
; reverted only the cross-pair widening (021d5e5) but kept the LIFO
; collect-and-reverse-apply refactor was *also* insufficient.
;
; This test currently asserts the post-#82 conservative state: the BSS
; slot IS used, the orphan cross-pair LD is the gate that prevents the
; PUSH/POP conversion.  When #74 is re-implemented correctly (see
; ravn/llvm-z80#74 for instructions), update the CHECKs back to
; asserting the cross-pair PUSH/POP shape and remove this preamble.
;
; C source:
;   typedef unsigned char uint8_t;
;   typedef unsigned short uint16_t;
;   void take(uint8_t x);
;   __attribute__((z80_static_frame))
;   void f(void) {
;       for (uint16_t i = 0; i != 4; i++)
;           take((uint8_t)i);
;   }
; The uint16_t counter i is held in BC but also assigned a BSS slot; the
; call-arg reload reads the slot via HL (cross-pair). Post-#82: BSS slot
; store/load survives rather than being rewritten to a mismatched PUSH BC /
; POP HL which would corrupt SP-relative data.

declare void @take(i8 zeroext)

; CHECK-LABEL: f:
; CHECK:      	push	af
; CHECK:      	ld	de,#0
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	ld	a,e
; CHECK:      	call	_take
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	inc	de
; CHECK:      	ld	a,d
; CHECK:      	xor	#4
; CHECK:      	ld	b,a
; CHECK:      	ld	a,e
; CHECK:      	or	b
; CHECK:      	jr	nz,.LBB0_1
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	ret
define void @f() {
; Conservative state: the BSS slot is used because the orphan reload is
; into HL (different pair from the storing BC), so the BSS-spill->PUSH/POP
; peephole bails.
; The PUSH/POP shape from the (currently reverted) #74 cross-pair
; extension must NOT appear -- if it does, the regression #74 was
; reverted to fix has come back.
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %low = trunc i16 %i to i8
  call void @take(i8 zeroext %low)
  %i.next = add nuw nsw i16 %i, 1
  %done = icmp eq i16 %i.next, 1024
  br i1 %done, label %exit, label %loop

exit:
  ret void
}
