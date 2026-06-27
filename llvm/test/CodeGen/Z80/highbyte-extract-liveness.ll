; RUN: llc --mtriple=z80 -O0 < %s | FileCheck %s
;
; ravn/llvm-z80#242: the `LD L,H; LD H,0; LD A,L -> LD A,H` peephole
; (Z80LateOptimization) must NOT fire when H is read after the LD A,L.
; This IR (clang -O0 output: a 16-bit unsigned compare with a zext-from-i8
; operand reloaded into H) expands to LD L,H; LD H,0; LD A,L; SUB E; LD A,H;
; SBC A,D.  The trailing LD A,H reads the zeroed high byte, so the LD H,0
; MUST survive.  Before the liveness guard, the peephole deleted it and
; 255u > 1u computed false.
;
; CHECK-LABEL: _f:
; CHECK:      ld   l,h
; CHECK-NEXT: ld   h,0
; CHECK-NEXT: ld   a,l
; CHECK:      ld   a,h
; CHECK-NEXT: sbc  a,d

@v8 = internal global i8 0, align 1

define dso_local i16 @f() #0 {
  %1 = alloca i16, align 1
  %2 = alloca i8, align 1
  store volatile i8 -1, ptr @v8, align 1
  %3 = load volatile i8, ptr @v8, align 1
  %4 = zext i8 %3 to i16
  store i16 %4, ptr %1, align 1
  store i8 1, ptr %2, align 1
  %5 = load i16, ptr %1, align 1
  %6 = load i8, ptr %2, align 1
  %7 = zext i8 %6 to i16
  %8 = icmp ugt i16 %5, %7
  %9 = zext i1 %8 to i16
  ret i16 %9
}

attributes #0 = { noinline nounwind null_pointer_is_valid optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+z80" }
