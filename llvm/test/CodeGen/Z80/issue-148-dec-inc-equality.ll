; RUN: llc -mtriple=z80 -mattr=+static-frame < %s | FileCheck %s
;
; ravn/llvm-z80#148: `XOR $1; J{Z,NZ}` and `CP $FF; J{Z,NZ}` for
; the equality tests A == 1 and A == 0xFF compile in 4 bytes but
; can use the 1-byte forms `DEC A` (for A == 1) or `INC A` (for
; A == 0xFF) when A's modified value is dead afterwards.  Z80 has
; `OR A` (1 B) for the analogous `A == 0` test already — this
; closes the K ∈ {1, 0xFF} gap.
;
; Post-RA peephole in Z80PreEmitPeephole.cpp.  Pattern:
;   {XOR_n, CP_n} K (K ∈ {1, 0xFF})
;   J{Z,NZ,C,NC}_e <target>
;   (then A redefined or dead along both paths)

; C source:
;   typedef unsigned char uint8_t;
;   void test_eq1(uint8_t a)    { if (a == 1)    {} }   /* DEC A; JR Z */
;   void test_eq_ff(uint8_t a)  { if (a == 0xFF) {} }   /* INC A; JR Z */
; XOR 1; JZ / CP 0xFF; JZ (4 B each) → DEC A; JZ / INC A; JZ (2 B each)
; when A's modified value is dead after the branch.

declare void @sink(i8)
declare i8 @getbyte()

;
; `if (x == 1) sink(0);` — A's value after the test isn't used.
; Pre-fix: ld a,..; xor $1; jr nz, .skip; ld a,0; call sink; .skip: ret
; Post-fix: ld a,..; dec a; jr nz, .skip; ld a,0; call sink; .skip: ret
;
define void @eq_1() {
  %x = call i8 @getbyte()
  %is1 = icmp eq i8 %x, 1
  br i1 %is1, label %act, label %skip
act:
  call void @sink(i8 0)
; CHECK-LABEL: eq_1:
; CHECK:      	call	_getbyte
; CHECK:      	dec	a
; CHECK-NEXT: 	ret	nz
; CHECK:      	xor	a
; CHECK:      	call	_sink
; CHECK:      	ret
  br label %skip
skip:
  ret void
}

;
; `if (x == 0xFF) sink(0);` — same pattern with INC A.
;
define void @eq_ff() {
  %x = call i8 @getbyte()
  %is_ff = icmp eq i8 %x, -1
  br i1 %is_ff, label %act, label %skip
act:
  call void @sink(i8 0)
  br label %skip
skip:
  ret void
}

;
; CHECK-LABEL: eq_ff:
; CHECK:      	call	_getbyte
; CHECK:      	inc	a
; CHECK-NEXT: 	jr	z,.LBB1_2
; CHECK:      	ret
