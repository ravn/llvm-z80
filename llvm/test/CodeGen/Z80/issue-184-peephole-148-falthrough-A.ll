; RUN: llc -mtriple=z80 -mattr=+static-frame < %s | FileCheck %s
;
; ravn/llvm-z80#184: peephole #148 (`CP/XOR with 1 or 0xFF → DEC_A/INC_A`)
; had a too-loose safety check.  Before the fix, it would rewrite
; `CP 255; JP_Z target` to `INC A; JP_Z target` even when the
; fall-through MBB read A (e.g. via `push af` to save A across a
; CALL).  The `inc a` left A as `c+1`, so the post-pop `dec a; ld c, a`
; updated C to `c+1-1 = c` (no actual decrement), creating an
; infinite loop.
;
; The fix: also walk the fall-through MBB's instructions explicitly,
; not just check Succ->liveins() (which may be stale post-regalloc).
;
; This test reproduces the AES `aes_sb_inv` shape: a post-decrement
; loop that spills the counter A around a call.
;
; C source (AES aes_sb_inv countdown shape):
;   typedef unsigned char uint8_t;
;   uint8_t lookup(uint8_t x);
;   __attribute__((z80_static_frame))
;   void aes_sb_inv_like(uint8_t *buf) {
;       for (uint8_t i = 16; i != 0; i--)
;           buf[i-1] = lookup(buf[i-1]);
;   }
; Counter i ends in A; fall-through MBB saves A via PUSH AF across lookup().
; Peephole #148 mistakenly rewrote `CP 0xFF; JR Z` → `INC A; JR Z` without
; seeing the PUSH AF: A became i+1 instead of i → counter never reached 0
; → infinite loop.

declare i8 @lookup(i8 %x)

define void @aes_sb_inv_like(ptr %buf) {
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

; The miscompile pattern we want to AVOID emitting:
;   inc  a            ; left A = c+1
;   ret  z            ; or jr/jp z
;   ...               ; body uses C as index
;   push af           ; saves A = c+1 (bug!)
;   call _lookup
;   pop  af           ; A = c+1
;   dec  a            ; A = c  (NOT c-1!)
;   ld   c, a         ; C := c (unchanged → infinite loop)
;
; CHECK-LABEL: _aes_sb_inv_like:
; The negative pattern below would only appear if peephole #148
; mis-fires.  The fix routes the test through `cp` (or some other
; safe sequence) when A is read by the fall-through MBB.
; CHECK-NOT: {{inc[ \t]+a[ \t\n]+(ret|jp|jr)[ \t]+z[^a-z]}}
