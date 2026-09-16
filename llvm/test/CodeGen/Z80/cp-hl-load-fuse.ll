; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Test: Fold single-use 8-bit memory load into CP (HL).
; Saves loading the byte into a scratch register and preserves accumulator.

@g_val = dso_local global i8 0, align 1

declare void @foo()

; (a) Exact pattern: register vs global load (fused compare-and-branch)
define void @cmp_reg_global(i8 %val) nounwind {
; CHECK-LABEL: _cmp_reg_global:
; CHECK:       ld	hl,#_g_val
; CHECK:       cp	(hl)
; CHECK-NOT:   ld	a,(_g_val)
entry:
  %g = load i8, ptr @g_val, align 1
  %cmp = icmp eq i8 %val, %g
  br i1 %cmp, label %then, label %else

then:
  call void @foo()
  ret void

else:
  ret void
}

; (b1) Structural variation: register vs pointer load
define void @cmp_reg_ptr(i8 %val, ptr %p) nounwind {
; CHECK-LABEL: _cmp_reg_ptr:
; CHECK:       cp	(hl)
entry:
  %loaded = load i8, ptr %p, align 1
  %cmp = icmp ne i8 %val, %loaded
  br i1 %cmp, label %then, label %else

then:
  call void @foo()
  ret void

else:
  ret void
}

; (b2) Structural variation: load on LHS for symmetric predicate (EQ/NE)
define void @cmp_ptr_reg(ptr %p, i8 %val) nounwind {
; CHECK-LABEL: _cmp_ptr_reg:
; CHECK:       cp	(hl)
entry:
  %loaded = load i8, ptr %p, align 1
  %cmp = icmp eq i8 %loaded, %val
  br i1 %cmp, label %then, label %else

then:
  call void @foo()
  ret void

else:
  ret void
}

; (b3) Structural variation: unsigned ordered compare (ULT) with load on RHS
define void @cmp_ult_ptr(i8 %val, ptr %p) nounwind {
; CHECK-LABEL: _cmp_ult_ptr:
; CHECK:       cp	(hl)
entry:
  %loaded = load i8, ptr %p, align 1
  %cmp = icmp ult i8 %val, %loaded
  br i1 %cmp, label %then, label %else

then:
  call void @foo()
  ret void

else:
  ret void
}

; (c) Materialized G_ICMP: boolean 0/1 result
define i8 @icmp_materialize(i8 %val, ptr %p) nounwind {
; CHECK-LABEL: _icmp_materialize:
; CHECK:       cp	(hl)
entry:
  %loaded = load i8, ptr %p, align 1
  %cmp = icmp ult i8 %val, %loaded
  %res = zext i1 %cmp to i8
  ret i8 %res
}

; (d1) Safety / boundary case: multi-use load must NOT be folded into CP (HL)
define i8 @cmp_multi_use(i8 %val, ptr %p) nounwind {
; CHECK-LABEL: _cmp_multi_use:
; CHECK-NOT:   cp	(hl)
; CHECK:       ret
entry:
  %loaded = load i8, ptr %p, align 1
  %cmp = icmp eq i8 %val, %loaded
  br i1 %cmp, label %then, label %else

then:
  ret i8 %loaded

else:
  ret i8 0
}

; (d2) Safety / boundary case: intervening store prevents folding
define void @cmp_intervening_store(i8 %val, ptr %p, ptr %dest) nounwind {
; CHECK-LABEL: _cmp_intervening_store:
; CHECK-NOT:   cp	(hl)
entry:
  %loaded = load i8, ptr %p, align 1
  store i8 42, ptr %dest, align 1
  %cmp = icmp eq i8 %val, %loaded
  br i1 %cmp, label %then, label %else

then:
  call void @foo()
  ret void

else:
  ret void
}
