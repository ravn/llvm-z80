; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#173.  The bare-store + 4-instruction reload-via-A
; pattern was the dominant residual bloat shape in AES `aes_subBytes`,
; `aes_sb_inv`, and `aes_mc_inv` at -Oz +static-stack:
;
;   ld   (slot), a              ; bare store -- A held the value
;   ... call/intermediate ...
;   push af                     ; reload preserving A
;   ld   a, (slot)
;   ld   r, a
;   pop  af
;
; Total 9 B per spill+reload pair.  The Z80LateOptimization peephole
; converts the matched pair into:
;
;   ld   r, a                   ; copy A into r at the store point
;   push rr                     ; spill via stack
;   ... call/intermediate ...
;   pop  rr                     ; at stack-balanced point
;
; Total 3 B per pair, saving 6 B.
;
; Production impact (AES corpus 09_Oz_prod_like, May 2026): -12 B,
; cpnos PROM1: -1 B.

declare i8 @lookup(i8 %x)

define void @aes_subBytes_like(ptr %buf) {
entry:
  br label %loop

loop:
  %i = phi i8 [ 16, %entry ], [ %i.next, %body ]
  %done = icmp eq i8 %i, 0
  br i1 %done, label %exit, label %body

body:
  %i.next = sub i8 %i, 1
  %p = getelementptr i8, ptr %buf, i8 %i.next
  %v = load i8, ptr %p
  %r = call i8 @lookup(i8 %v)
  store i8 %r, ptr %p
  br label %loop

exit:
  ret void
}

; The 4-instruction reload-via-A template (push af; ld a, (sfr);
; ld r, a; pop af) MUST NOT appear after the peephole runs.  We don't
; enforce the exact replacement (which depends on regalloc), only the
; absence of the canonical pre-optimization shape.
;
; CHECK-LABEL: _aes_subBytes_like:
; CHECK-NOT: {{push[ \t]+af[ \t\n]+ld[ \t]+a,\(__sfrend}}
