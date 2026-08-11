; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-enable-auto-static-stack=true  < %s | FileCheck %s --check-prefix=ON
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 -z80-enable-auto-static-stack=false < %s | FileCheck %s --check-prefix=OFF
;
; ravn/llvm-z80#12 (hasFP=false / static-stack reentrancy): the Z80AutoStaticStack
; IR pass proves "non-recursive" from the MODULE-LOCAL CallGraph, which is blind
; to cross-TU cycles.  Canonical miscompile:
;
;   // TU A (this file):  u32 f(u32 n){ return n ? n + g(n-1) : 0; }   // g extern
;   // TU B (other):      u32 g(u32 n){ return n ? n + f(n-1) : 0; }   // f extern
;
; f and g are MUTUALLY recursive, but g is only a declaration here, so f sits in
; its own single-node SCC and looks non-recursive.  Auto-injecting +static-stack
; puts f's live-across-call spill of `n` in a FIXED BSS slot; the recursive
; re-entry (f -> g -> f) clobbers it and `n + g(n-1)` reads a corrupted `n`
; (observed: 32-bit cross-recursion returned 0x0002 instead of 0x000A).
;
; The soundness gate: auto +static-stack on a non-leaf function is allowed only
; when a cross-TU cycle through it is impossible -- either it is not externally
; visible (local linkage; address-taken is a separate gate), or control never
; leaves the module while it is live (it reaches no opaque/external callee).

@sink = external global i16
declare i16 @g_extern(i16 %n)

; (a) EXTERNAL linkage AND reaches an external callee -> NO static frame.
;     This is the mutually-recursive f above: the fix must fall back to a
;     reentrant (real-stack) frame here.
; ON-NOT: __sfrend_f_calls_external
define i16 @f_calls_external(i16 %n) {
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
  %r = call i16 @g_extern(i16 %m)
  %v = load volatile i16, ptr %p, align 1
  %s = add i16 %r, %v
  ret i16 %s
}

; (b) INTERNAL (local) linkage, same external call -> STILL gets a static frame.
;     A local, non-address-taken function cannot be named or re-entered from
;     another TU, so the external helper (e.g. memcpy) can never call back into
;     it; every cycle through it is intra-module and already covered by the SCC
;     scan.  Density-preserving: keep the win here.
; ON: __sfrend_internal_calls_external
define internal i16 @internal_calls_external(i16 %n) {
entry:
  %buf = alloca [8 x i16], align 1
  %p = getelementptr [8 x i16], ptr %buf, i16 0, i16 0
  store volatile i16 %n, ptr %p, align 1
  %r = call i16 @g_extern(i16 %n)
  %v = load volatile i16, ptr %p, align 1
  %s = add i16 %r, %v
  ret i16 %s
}

; Keep (b) alive (an internal function with no users would be dead).
define i16 @use_internal(i16 %n) {
entry:
  %r = call i16 @internal_calls_external(i16 %n)
  ret i16 %r
}

; (c) EXTERNAL linkage but reaches ONLY an internal, defined callee (control
;     never leaves the module) -> STILL gets a static frame.  This is the
;     whole-program non-recursive case that must keep the density win.
; ON: __sfrend_ext_calls_internal
define i16 @ext_calls_internal(i16 %n) {
entry:
  %buf = alloca [8 x i16], align 1
  %p = getelementptr [8 x i16], ptr %buf, i16 0, i16 0
  store volatile i16 %n, ptr %p, align 1
  %r = call i16 @leaf_callee(i16 %n)
  %v = load volatile i16, ptr %p, align 1
  %s = add i16 %r, %v
  ret i16 %s
}

; Defined internal leaf that does NOT reach any external callee.
define internal i16 @leaf_callee(i16 %n) {
entry:
  %d = add i16 %n, 1
  store volatile i16 %d, ptr @sink, align 1
  ret i16 %d
}

; (e) global opt-out: with the flag off, NObody gets a static frame.
; OFF-NOT: __sfrend
