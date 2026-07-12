; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-enable-auto-static-stack=true  < %s | FileCheck %s --check-prefix=ON
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-enable-auto-static-stack=false < %s | FileCheck %s --check-prefix=OFF
;
; ravn/llvm-z80#176: the Z80AutoStaticStack IR pass auto-injects
; "target-features"="+static-stack" on provably-non-recursive functions, so
; their locals land in a fixed BSS frame (__sframe_<f>..__sfrend_<f>) instead
; of on the real stack.  This test pins the pass's safety gate:
;
;   (a) a leaf function GETS a static frame;
;   (b) a self-recursive function does NOT (CallGraph self-edge skip);
;   (c) an "interrupt"-attributed function AND a helper it calls do NOT --
;       fixed BSS slots would be clobbered if a shared helper re-enters from
;       an ISR while main flow is mid-call (the ISR-concurrency gate).  The
;       helper is otherwise leaf-qualifying, so taint is the only reason;
;   (d) an address-taken function does NOT -- it could be installed as a
;       runtime interrupt vector or be the target of an opaque indirect call
;       the CallGraph cannot resolve;
;   (e) -z80-enable-auto-static-stack=false disables the pass entirely (global
;       opt-out): NO function gets a static frame.

@sink = external global i16
@vec = global ptr @handler

; (a) leaf -> static frame.
; ON-LABEL: _leaf:
; ON: __sfrend_leaf
define void @leaf(i16 %n) {
entry:
  %buf = alloca [8 x i16], align 1
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %p = getelementptr [8 x i16], ptr %buf, i16 0, i16 %i
  store volatile i16 %i, ptr %p, align 1
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, %n
  br i1 %done, label %exit, label %loop
exit:
  %v = load volatile i16, ptr %buf, align 1
  store i16 %v, ptr @sink, align 1
  ret void
}

; (b) self-recursive -> NO static frame.
; ON-NOT: __sfrend_recur
define i16 @recur(i16 %n) {
entry:
  %buf = alloca [8 x i16], align 1
  %p = getelementptr [8 x i16], ptr %buf, i16 0, i16 0
  store volatile i16 %n, ptr %p, align 1
  %z = icmp eq i16 %n, 0
  br i1 %z, label %base, label %rec
base:
  ret i16 0
rec:
  %m = sub i16 %n, 1
  %r = call i16 @recur(i16 %m)
  %s = add i16 %r, %n
  ret i16 %s
}

; (c) interrupt handler and the helper it calls -> NEITHER gets a static frame.
; ON-NOT: __sfrend_isr
; ON-NOT: __sfrend_isr_helper
define void @isr() #0 {
entry:
  call void @isr_helper()
  ret void
}

; isr_helper is a leaf (would qualify for Level 1) but is ISR-reachable.
define void @isr_helper() {
entry:
  %buf = alloca [8 x i16], align 1
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %p = getelementptr [8 x i16], ptr %buf, i16 0, i16 %i
  store volatile i16 %i, ptr %p, align 1
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, 8
  br i1 %done, label %exit, label %loop
exit:
  %v = load volatile i16, ptr %buf, align 1
  store i16 %v, ptr @sink, align 1
  ret void
}

; (d) address-taken (its pointer is stored into @vec) -> NO static frame,
; even though it is a leaf.
; ON-NOT: __sfrend_handler
define void @handler() {
entry:
  %buf = alloca [8 x i16], align 1
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %p = getelementptr [8 x i16], ptr %buf, i16 0, i16 %i
  store volatile i16 %i, ptr %p, align 1
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, 8
  br i1 %done, label %exit, label %loop
exit:
  %v = load volatile i16, ptr %buf, align 1
  store i16 %v, ptr @sink, align 1
  ret void
}

; (e) global opt-out: with the flag off, NObody gets a static frame.
; OFF: _leaf:
; OFF-NOT: __sfrend

attributes #0 = { "interrupt" }
