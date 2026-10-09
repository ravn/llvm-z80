; RUN: llc -mtriple=z80 -O1 -z80-static-frames -verify-machineinstrs < %s | FileCheck %s

; A function marked no-recurse asserts it never re-enters. The non-reentrant
; analysis marks it doesNotRecurse unconditionally (bypassing the call graph)
; and treats it as a context barrier: context walks do not propagate through it
; into its subtree.

; --- Case 1: no-recurse bypasses SCC analysis for a simple function ---
; no_recurse_fn is internal with no indirect calls. The attribute marks it
; doesNotRecurse directly and it gets a static frame.
; CHECK-LABEL: no_recurse_fn:
; CHECK: L_no_recurse_fn.frame
define internal i16 @no_recurse_fn(i16 %x) #0 {
  %buf = alloca i16, align 1
  store volatile i16 %x, ptr %buf, align 1
  %v = load volatile i16, ptr %buf, align 1
  ret i16 %v
}

; CHECK-LABEL: main:
; CHECK: L_main.frame
define i16 @main() {
  %buf = alloca [4 x i16], align 1
  store volatile i16 1, ptr %buf, align 1
  %r = call i16 @no_recurse_fn(i16 2)
  ret i16 %r
}

; --- Case 2: context barrier isolates a shared callee ---
; isr calls barrier (no-recurse). barrier calls shared. Without the barrier,
; shared is reachable from both the ISR context (via barrier) and the main
; context, making it reentrant. With the barrier the ISR context walk stops at
; barrier, so shared is only seen from the main context and gets a static frame.

; shared is only reachable from main (ISR context stopped at barrier).
; CHECK-LABEL: shared:
; CHECK: L_shared.frame
define internal i16 @shared(i16 %x) {
  %buf = alloca i16, align 1
  store volatile i16 %x, ptr %buf, align 1
  %v = load volatile i16, ptr %buf, align 1
  ret i16 %v
}

; barrier is a no-recurse context barrier — no static frame (ISR visits it).
; CHECK-LABEL: barrier:
; CHECK-NOT: L_barrier.frame
define internal void @barrier() #0 {
  %r = call i16 @shared(i16 1)
  ret void
}

; isr is an interrupt root — no static frame.
; CHECK-LABEL: isr:
; CHECK-NOT: L_isr.frame
define void @isr() #1 {
  call void @barrier()
  ret void
}

attributes #0 = { "target-features"="-recurse" }
attributes #1 = { "interrupt" }
