; RUN: llc -verify-machineinstrs -mtriple=z80-unknown-none-z88dk < %s | FileCheck %s
; RUN: llc -verify-machineinstrs -mtriple=z80 < %s | FileCheck %s --check-prefix=ELF
;
; C regression:
; int test_counter = 1, L4_test7_counter = 2;
; int test(void) { static int counter = 3;
;   return ++counter + test_counter + L4_test7_counter; }
; Encode the already-prefixed name, not the raw LLVM name: otherwise
; test.counter and the ordinary C identifier L4_test7_counter collide.

@test_counter = global i16 1
@L4_test7_counter = global i16 2
@test.counter = internal global i16 3
@.str.1 = private constant [2 x i8] c"x\00"
@a..b = internal global i16 4
@a_1.b2 = internal global i16 5
@abcdefghijkl.x = internal global i16 6
@end. = internal global i16 7
@plain = private global i16 8
@g.frame = internal alias i16, ptr @test.counter

; CHECK-LABEL: _addresses:
; CHECK: L5__test7_counter
; CHECK: L2_L_3_str1_1
; CHECK: L2__g5_frame
; ELF-LABEL: _addresses:
; ELF: _test.counter
; ELF: L_.str.1
define void @addresses(ptr %out) {
  store ptr @test.counter, ptr %out
  %p1 = getelementptr ptr, ptr %out, i16 1
  store ptr @.str.1, ptr %p1
  %p2 = getelementptr ptr, ptr %out, i16 2
  store ptr @g.frame, ptr %p2
  ret void
}

; CHECK: GLOBAL _test_counter
; CHECK-NEXT: _test_counter:
; CHECK-NEXT: DEFW 1
; CHECK: GLOBAL _L4_test7_counter
; CHECK-NEXT: _L4_test7_counter:
; CHECK-NEXT: DEFW 2
; CHECK: L5__test7_counter:
; CHECK-NEXT: DEFW 3
; CHECK: L2_L_3_str1_1:
; CHECK-NEXT: DEFM "x\000"
; CHECK: L2__a0_1_b:
; CHECK-NEXT: DEFW 4
; CHECK: L4__a_12_b2:
; CHECK-NEXT: DEFW 5
; CHECK: L13__abcdefghijkl1_x:
; CHECK-NEXT: DEFW 6
; CHECK: L4__end0_:
; CHECK-NEXT: DEFW 7
; CHECK: L_plain:
; CHECK-NEXT: DEFW 8
; CHECK: L2__g5_frame = L5__test7_counter
