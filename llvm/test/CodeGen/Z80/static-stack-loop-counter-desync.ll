; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -mattr=+static-stack < %s | FileCheck %s
; XFAIL: *
;
; Static-stack codegen bug (ravn/llvm-z80#82, found 2026-04-27 during
; rc700-gensmedet PIO-B speed-test bring-up):
;
; A uint16_t loop counter held in a register pair (BC here) is *also*
; assigned a static-stack slot, but the slot is never written to by the
; loop body — only read from at the call-arg use site.  The live counter
; (BC) is incremented each iteration; the static-stack slot stays at its
; uninitialised BSS value (0).  Result: every call inside the loop passes
; 0 instead of the running counter.
;
; Observed asm with -mattr=+static-stack (this test):
;
;   _f:
;       ld    bc,#0                  ; init i = 0 in BC
;   .LBB0_1:
;       ld    a,b                    ; loop-test reads BC ✓
;       xor   #4
;       or    c
;       ret   z
;       push  bc
;       ld    hl,(__sfrend_f-2)      ; ← BUG: load 'i' from static-stack slot
;                                    ;   that nobody ever writes to
;       ld    a,l
;       call  _take
;       pop   bc
;       inc   bc                     ; live counter advances ...
;       jr    .LBB0_1                ; ... but the slot at __sfrend_f-2 doesn't
;
; Two valid fixes (either is acceptable):
;   1. Don't materialise the slot — keep the counter in BC, source the
;      argument as `ld a,c` (low byte of BC).
;   2. Spill BC to the slot whenever it changes, so reads from the slot
;       see the live value.
;
; Workaround at source level: split the uint16_t counter into nested
; uint8_t loops — clang then keeps the counters in registers and the
; slot/register split doesn't occur.

declare void @take(i8 zeroext)

define void @f() {
; CHECK-LABEL: _f:
; The argument byte to `take` must be derived from the live loop
; counter — NOT from a static-stack slot that no instruction stores
; into.  The buggy form was `ld hl,(__sfrend_f-...)` followed by
; `ld a,l; call _take`.  Forbid that exact shape.
;
; CHECK-NOT:   ld{{[ \t]+}}hl,(__sfr{{[a-z_]*}}_f
; CHECK:       call{{[ \t]+}}_take
entry:
  br label %loop

loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %low = trunc i16 %i to i8
  call void @take(i8 zeroext %low)
  %i.next = add nuw nsw i16 %i, 1
  %done = icmp eq i16 %i.next, 1024
  br i1 %done, label %exit, label %loop

exit:
  ret void
}
