; RUN: llc -mtriple=z80 -O2 -z80-static-frames -verify-machineinstrs < %s | FileCheck %s

; Test folding of 16-bit register spills/reloads into push/pop across calls
; when using static frames.

@sink = global i16 0

define internal void @callee() {
  ret void
}

; Shape (a): Single 16-bit spill across a call (cross-register: HL spilled, reloaded into BC)
; CHECK-LABEL: single_spill_cross_reg:
; CHECK: push hl
; CHECK-NEXT: call _callee
; CHECK-NEXT: pop bc
; CHECK-NOT: .frame
define internal void @single_spill_cross_reg(i16 %val) {
entry:
  call void @callee()
  %s = add i16 %val, 1
  store volatile i16 %s, ptr @sink
  ret void
}

; Shape (b): Single 16-bit spill across a call (BC spilled and reloaded across call)
; CHECK-LABEL: single_spill_same_reg:
; CHECK: push bc
; CHECK-NEXT: call _callee
; CHECK-NEXT: pop de
define internal void @single_spill_same_reg(ptr %p, i16 %dummy) {
entry:
  %val = load volatile i16, ptr @sink
  call void @callee()
  store volatile i16 %val, ptr %p
  ret void
}

; Shape (c): Positive control - slot read multiple times across calls must NOT be folded to push/pop
; CHECK-LABEL: slot_read_multiple_times:
; CHECK: ld (L_slot_read_multiple_times.frame{{.*}}),hl
; CHECK: call _callee
; CHECK: ld bc,(L_slot_read_multiple_times.frame{{.*}})
; CHECK: call _callee
; CHECK: ld bc,(L_slot_read_multiple_times.frame{{.*}})
define internal void @slot_read_multiple_times(i16 %val) {
entry:
  call void @callee()
  store volatile i16 %val, ptr @sink
  call void @callee()
  store volatile i16 %val, ptr @sink
  ret void
}

; Shape (d): Positive control - stretch containing SP modification must NOT fold to push/pop
; CHECK-LABEL: sp_modified_stretch:
; CHECK: ld (L_sp_modified_stretch.frame{{.*}}),hl
define internal void @sp_modified_stretch(i16 %val) {
entry:
  call void asm sideeffect "inc sp\0Adec sp", ""()
  call void @callee()
  %s = add i16 %val, 2
  store volatile i16 %s, ptr @sink
  ret void
}

; Shape (e): Exact pattern from bug report: 8-bit D register spilled alongside 16-bit BC
; CHECK-LABEL: read_loop:
; CHECK: push bc
; CHECK-NEXT: push de
; CHECK-NEXT: call _fdc_read_when_ready
; CHECK-NEXT: pop de
; CHECK-NEXT: pop bc
; CHECK-NOT: L_read_loop.frame
@sink8 = global i8 0

define internal i8 @fdc_read_when_ready() {
  ret i8 42
}

define internal void @read_loop() {
entry:
  br label %for.cond

for.cond:
  %i = phi i8 [ 7, %entry ], [ %dec, %for.body ]
  %p = phi ptr [ @sink8, %entry ], [ %inc, %for.body ]
  %cmp = icmp eq i8 %i, 0
  br i1 %cmp, label %for.end, label %for.body

for.body:
  %b = call i8 @fdc_read_when_ready()
  store volatile i8 %b, ptr %p
  %inc = getelementptr inbounds i8, ptr %p, i16 1
  %dec = sub i8 %i, 1
  br label %for.cond

for.end:
  ret void
}

; Shape (f): 8-bit slot read multiple times across calls must NOT be folded
; CHECK-LABEL: slot8_read_multiple_times:
; CHECK: ld (L_slot8_read_multiple_times.frame{{.*}}),a
; CHECK: call _callee
; CHECK: ld a,(L_slot8_read_multiple_times.frame{{.*}})
define internal void @slot8_read_multiple_times(i8 %val) {
entry:
  call void @callee()
  store volatile i8 %val, ptr @sink8
  call void @callee()
  store volatile i8 %val, ptr @sink8
  ret void
}

define void @main() {
  call void @single_spill_cross_reg(i16 1)
  call void @single_spill_same_reg(ptr null, i16 2)
  call void @slot_read_multiple_times(i16 3)
  call void @sp_modified_stretch(i16 4)
  call void @read_loop()
  call void @slot8_read_multiple_times(i8 5)
  ret void
}

