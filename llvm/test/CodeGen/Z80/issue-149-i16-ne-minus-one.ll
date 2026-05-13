; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#149: `icmp {eq,ne} i16 r, -1` (≡ ne/eq with constant
; 0xFFFF) was lowering via the XOR-based EQ/NE path:
;   ld a, lhs_hi; cpl; ld tmp, a; ld a, lhs_lo; cpl; or tmp; jr cc
;   (8 B for the test+branch)
; The equivalent `(r + 1) != 0` test using INC rr + byte-OR is 5 B:
;   inc de; ld a, e; or d; jr cc
;
; Constraint: INC rr mutates the value, so the fix is only safe when
; r has a single use (the icmp itself).  Multi-use values like the
; cpnos `_snios_rcvmsg_c` `r = recv_byte_t(); if (r != -1) check(r);`
; pattern need r preserved across the test — those bail to the
; standard XOR/CPL path.

declare i16 @get()
declare void @action()
declare void @timeout_fn()

;
; Single-use case: r is consumed ONLY by the icmp.  Fix fires.
;
; CHECK-LABEL: ne_minus_one_single:
; CHECK:       call	_get
; CHECK:       inc	de
; CHECK-NEXT:  ld	a,e
; CHECK-NEXT:  or	d
; CHECK-NEXT:  jr	{{n?z}},
; CHECK-NOT:   cpl
define void @ne_minus_one_single() {
  %r = call i16 @get()
  %ok = icmp ne i16 %r, -1
  br i1 %ok, label %valid, label %timeout
valid:
  call void @action()
  ret void
timeout:
  call void @timeout_fn()
  ret void
}

;
; EQ form, also single-use.
;
; CHECK-LABEL: eq_minus_one_single:
; CHECK:       call	_get
; CHECK:       inc	de
; CHECK-NEXT:  ld	a,e
; CHECK-NEXT:  or	d
; CHECK-NEXT:  jr	{{n?z}},
; CHECK-NOT:   cpl
define void @eq_minus_one_single() {
  %r = call i16 @get()
  %tim = icmp eq i16 %r, -1
  br i1 %tim, label %timeout, label %valid
valid:
  call void @action()
  ret void
timeout:
  call void @timeout_fn()
  ret void
}

;
; Multi-use case: r is used both by the icmp AND in the valid path.
; INC would clobber r, so the fix must bail.  Falls back to the
; XOR/CPL path (8 B).
;
; CHECK-LABEL: ne_minus_one_multi:
; CHECK:       call	_get
; CHECK:       cpl
; CHECK-NOT:   inc	de
; CHECK-NOT:   inc	hl
; CHECK-NOT:   inc	bc
define i16 @ne_minus_one_multi() {
  %r = call i16 @get()
  %ok = icmp ne i16 %r, -1
  br i1 %ok, label %valid, label %timeout
valid:
  ret i16 %r                ; <-- second use of %r
timeout:
  ret i16 0
}

;
; Negative: K = 0xFFFE (one less than -1) — must NOT fold via INC
; (the inc-then-test trick only works for K = -1).  Falls back to
; standard XOR/CPL or whatever the existing path emits.
;
; CHECK-LABEL: ne_fffe:
; CHECK:       call	_get
; CHECK-NOT:   inc	de
; CHECK-NOT:   inc	hl
; CHECK-NOT:   inc	bc
define void @ne_fffe() {
  %r = call i16 @get()
  %ok = icmp ne i16 %r, -2          ; 0xFFFE
  br i1 %ok, label %valid, label %timeout
valid:
  call void @action()
  ret void
timeout:
  call void @timeout_fn()
  ret void
}
