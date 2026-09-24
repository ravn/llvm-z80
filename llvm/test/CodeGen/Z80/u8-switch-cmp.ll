; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #86 (regression-lock): switch on u8 discriminant uses 8-bit
; CP, NOT 16-bit SUB/SBC.  These tests pin the current (good) behavior
; so future codegen changes can't silently regress to 16-bit cmp
; widening.

; C source:
;   // ravn/llvm-z80#86 regression guard: switch on uint8_t discriminant must
;   // use 8-bit CP, NOT 16-bit SUB/SBC after zero-extension.
;   //
;   extern void f0(void), f1(void), f2(void), fdef(void);
;   void switch_3(uint8_t v) {
;       switch (v) { case 0: f0(); break; case 1: f1(); break;
;                    case 2: f2(); break; default: fdef(); }
;   }
declare void @f0()
declare void @f1()
declare void @f2()
declare void @def()

; Three-case switch: should emit a chain of `cp #imm; jr z` then
; final `or a; jr nz` for the case-0 vs default discrimination.
; CHECK-LABEL: switch_3:
; CHECK:      	cp	#2
; CHECK:      	jr	z,.LBB0_5
; CHECK:      	cp	#1
; CHECK:      	jr	z,.LBB0_4
; CHECK:      	or	a
; CHECK:      	jr	nz,.LBB0_6
; CHECK:      	call	_f0
; CHECK:      	ret
; CHECK:      	call	_f1
; CHECK:      	ret
; CHECK:      	call	_f2
; CHECK:      	ret
; CHECK:      	call	_def
; CHECK:      	ret
define void @switch_3(i8 zeroext %v) {
  switch i8 %v, label %d [
    i8 0, label %c0
    i8 1, label %c1
    i8 2, label %c2
  ]
c0: tail call void @f0() ret void
c1: tail call void @f1() ret void
c2: tail call void @f2() ret void
d:  tail call void @def() ret void
}


; Many-case switch with a contiguous range -- should emit `cp #N`
; for the upper-bound check, NOT a 16-bit `sbc hl, BC` after widening.
@jt = external global [16 x ptr]
declare void @callee()

define void @switch_range(i8 zeroext %v) {
  switch i8 %v, label %d [
    i8 0, label %c i8 1, label %c i8 2, label %c
    i8 3, label %c i8 4, label %c i8 5, label %c
    i8 6, label %c i8 7, label %c i8 8, label %c
    i8 9, label %c i8 10, label %c i8 11, label %c
  ]
c:  tail call void @callee() ret void
d:  tail call void @def() ret void
; CHECK-LABEL: switch_range:
; CHECK:      	cp	#12
; CHECK:      	jr	nc,.LBB1_2
; CHECK:      	call	_callee
; CHECK:      	ret
; CHECK:      	call	_def
; CHECK:      	ret
}

