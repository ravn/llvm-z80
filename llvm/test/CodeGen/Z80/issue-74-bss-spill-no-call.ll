; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s

; Issue #74: the BSS-spill→PUSH/POP peephole previously bailed when no CALL
; sat between the store and the matching load.  But pure register-pressure
; spills (no CALL) are exactly where PUSH/POP wins on size and T-states too;
; the StackDepth balance check is the right safety guard.
;
; The test is the #74 issue repro (delete_line shape): a memcpy chain that
; spills two 16-bit address temporaries to the same BSS frame across the
; LDIR setup, with no CALL between the spills and reloads.

@cury = external global i8

declare void @llvm.memcpy.p0.p0.i16(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i16, i1 immarg)
declare void @llvm.memset.p0.i16(ptr nocapture writeonly, i8, i16, i1 immarg)

; CHECK-LABEL: delete_line:
; The peephole should remove at least one of the BSS spill/reload pairs.
; CHECK: push hl
; CHECK: pop hl
define void @delete_line() {
entry:
  %0 = load i8, ptr @cury, align 1
  %1 = zext i8 %0 to i16
  %2 = add i16 %1, 1
  %3 = icmp ult i16 %2, 25
  br i1 %3, label %if.then, label %if.end

if.then:
  %4 = mul i16 %1, 80
  %5 = add i16 %4, -2048             ; 0xF800 = -2048 in i16
  %dst = inttoptr i16 %5 to ptr
  %6 = mul i16 %2, 80
  %7 = add i16 %6, -2048
  %src = inttoptr i16 %7 to ptr
  %ext = zext i8 %0 to i16
  %sub = sub i16 24, %ext
  %len = mul i16 %sub, 80
  call void @llvm.memcpy.p0.p0.i16(ptr %dst, ptr %src, i16 %len, i1 false)
  br label %if.end

if.end:
  call void @llvm.memset.p0.i16(ptr inttoptr (i16 -128 to ptr), i8 32, i16 80, i1 false)
  ret void
}
