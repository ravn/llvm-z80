; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s

; Regression test for ravn/llvm-z80#159 — Z80LateOptimization's
; "Commutative ALU shortcut" peephole (LD r,A; LD A,r2; ALU r → ALU r2)
; used to fire even when the temp register r was needed AFTER the ALU
; instruction, producing silent miscompiles via reads of an
; uninitialised register.
;
; This testcase is a u8 rotate chain (rj_sb_inv from aes256.c) where the
; intermediate rotated value must be preserved across an XOR for the next
; iteration of the chain. The mid-end lowers it to three fshl.i8 calls
; with the same input %2 and rotation amounts 1, 3, 6.
;
; The buggy codegen was:
;   xor 99 ; rlca ; ld d,a ; rlca ; rlca ; xor d ; ld d,a ;
;   ld a,e         <-- E never written
;   rlca ; rlca ; rlca ; xor d ; ret
;
; The fixed codegen MUST save the rotated value to E (or another temp)
; BEFORE the destructive XOR, then reload it for the next rotation.

define zeroext i8 @rj_sb_inv(i8 zeroext %x) {
; CHECK-LABEL: rj_sb_inv:
; CHECK:        xor 99
; CHECK-NEXT:   rlca
; CHECK-NEXT:   ld d,a
; CHECK-NEXT:   rlca
; CHECK-NEXT:   rlca
; CHECK-NEXT:   ld e,a
; CHECK-NEXT:   ld a,d
; CHECK-NEXT:   xor e
; CHECK-NEXT:   ld d,a
; CHECK-NEXT:   ld a,e
; CHECK-NEXT:   rlca
; CHECK-NEXT:   rlca
; CHECK-NEXT:   rlca
; CHECK-NEXT:   ld e,a
; CHECK-NEXT:   ld a,d
; CHECK-NEXT:   xor e
; CHECK-NEXT:   ret
  %y0 = xor i8 %x, 99
  %y1 = tail call i8 @llvm.fshl.i8(i8 %y0, i8 %y0, i8 1)
  %y2 = tail call i8 @llvm.fshl.i8(i8 %y0, i8 %y0, i8 3)
  %sb2 = xor i8 %y1, %y2
  %y3 = tail call i8 @llvm.fshl.i8(i8 %y0, i8 %y0, i8 6)
  %sb3 = xor i8 %sb2, %y3
  ret i8 %sb3
}

declare i8 @llvm.fshl.i8(i8, i8, i8)
