; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#148: `XOR $1; J{Z,NZ}` and `CP $FF; J{Z,NZ}` for
; the equality tests A == 1 and A == 0xFF compile in 4 bytes but
; can use the 1-byte forms `DEC A` (for A == 1) or `INC A` (for
; A == 0xFF) when A's modified value is dead afterwards.  Z80 has
; `OR A` (1 B) for the analogous `A == 0` test already — this
; closes the K ∈ {1, 0xFF} gap.
;
; Post-RA peephole in Z80LateOptimization.cpp.  Pattern:
;   {XOR_n, CP_n} K (K ∈ {1, 0xFF})
;   J{Z,NZ,C,NC}_e <target>
;   (then A redefined or dead along both paths)

declare void @sink(i8)
declare i8 @getbyte()

;
; `if (x == 1) sink(0);` — A's value after the test isn't used.
; Pre-fix: ld a,..; xor $1; jr nz, .skip; ld a,0; call sink; .skip: ret
; Post-fix: ld a,..; dec a; jr nz, .skip; ld a,0; call sink; .skip: ret
;
; CHECK-LABEL: eq_1:
; CHECK:       call	_getbyte
; CHECK:       dec	a
; CHECK-NEXT:  {{(jr|ret)}}	{{n?z}}
; CHECK-NOT:   xor	1
; CHECK-NOT:   xor	$1
define void @eq_1() {
  %x = call i8 @getbyte()
  %is1 = icmp eq i8 %x, 1
  br i1 %is1, label %act, label %skip
act:
  call void @sink(i8 0)
  br label %skip
skip:
  ret void
}

;
; `if (x == 0xFF) sink(0);` — same pattern with INC A.
;
; CHECK-LABEL: eq_ff:
; CHECK:       call	_getbyte
; CHECK:       inc	a
; CHECK-NEXT:  {{(jr|ret)}}	{{n?z}}
; CHECK-NOT:   cp	255
; CHECK-NOT:   cp	$ff
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
; Negative: K=2 doesn't have a 1-byte equivalent — fall back to CP.
;
; CHECK-LABEL: eq_2:
; CHECK:       call	_getbyte
; CHECK:       cp	2
; CHECK-NOT:   dec	a
; CHECK-NOT:   inc	a
define void @eq_2() {
  %x = call i8 @getbyte()
  %is2 = icmp eq i8 %x, 2
  br i1 %is2, label %act, label %skip
act:
  call void @sink(i8 0)
  br label %skip
skip:
  ret void
}
