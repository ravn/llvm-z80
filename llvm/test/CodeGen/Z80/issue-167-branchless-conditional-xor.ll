; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s
;
; XFAIL: *
;
; Issue ravn/llvm-z80#167 (filed session 73l, May 2026): mid-end
; SimplifyCFG `foldTwoEntryPHINode` converts the source-level
; `if (x & 0x80) y ^= 0x1b;` pattern into a `select i1 %cmp, %xor,
; %shl`.  The Z80 backend then lowers the select branchlessly via
; `xor #27; rlca; jr cc; ld _,_`, computing BOTH operands of the
; select unconditionally before picking via the carry flag.
;
; On Z80 -- where branches are cheap (7-12 tstates, no misprediction
; penalty) and selects must be lowered to a branch anyway -- this
; is strictly slower than a conditional branch around the XOR:
; +26 tstates per loop iteration (+38%) in gf_alog / gf_log of
; the AES-256 corpus where this pattern dominates the hot loop.
;
; Reference C source (`aes256.c:269` gf_alog, lines 272-274):
;     atb <<= 1;
;     if (z & 0x80) atb ^= 0x1b;
;     atb ^= z;
;
; This test feeds the select-form IR that clang produces after
; SimplifyCFG (since `llc` doesn't run SimplifyCFG itself) and
; expects the FINAL asm to use a conditional branch around the XOR,
; NOT the branchless compute-both-then-select pattern.
;
; Current (pre-fix) asm:
;     add a,a       ; atb << 1
;     ld  d,a       ; save (atb<<1)
;     xor #27       ; <-- UNCONDITIONAL.  This is the smoking gun.
;     ld  e,a       ; save (atb<<1) ^ 0x1b
;     ld  a,h       ; reload z
;     rlca          ; carry = bit 7 of z
;     jr  c, .Lend  ; if bit 7 set, keep XOR'd version
;     ld  e,d       ; else: pick non-XOR'd
;   .Lend:
;     ld  a,e
;     ret
;
; Desired (post-fix) asm:
;     add a,a       ; atb << 1
;     ld  d,a       ; save (atb<<1)
;     ld  a,h       ; reload z
;     rlca          ; carry = bit 7 of z
;     jr  nc, .L    ; if bit 7 NOT set, skip XOR
;     ld  a,d
;     xor #27       ; <-- CONDITIONAL.  Only runs when needed.
;     ld  d,a
;   .L:
;     ld  a,d
;     ret
;
; Fix candidates: ravn/llvm-z80#168 (mid-end cost-gated bailout in
; foldTwoEntryPHINode -- recommended approach) or a Z80 backend
; peephole that rewrites the branchless select-of-XOR sequence to
; a conditional branch.  Either lands when CHECK lines below pass.
;
; First attempt at `672f24188ca8` (blanket SimplifyCFG bailout) was
; reverted in `4336a23e81b5` because it grew cpnos clang PROM1 by
; +23 B, pushing it over the 2 KB hard cap.  See #168 for the
; per-function evidence (scroll_lines accounted for +21 B alone)
; and the cost-gated design that avoids that regression.

define zeroext i8 @gf_step(i8 zeroext %z, i8 zeroext %atb) {
; CHECK-LABEL: _gf_step:
;
; The shift always happens unconditionally.
; CHECK:      add{{[ \t]+}}a,a
;
; Crucially, no `xor #27` between the shift and the conditional
; branch.  If we see `xor` here, the compiler emitted the branchless
; compute-both-and-select pattern we want to STOP producing.
; CHECK-NOT:  xor
;
; A conditional jump must precede the XOR.  Accept any of Z80's
; carry / zero / sign condition codes (rlca + jr c, bit n + jr z,
; etc.) -- the exact instruction sequence depends on which fix
; approach lands.
; CHECK:      j{{[rp]}}{{[ \t]+}}{{(z|nz|c|nc|p|m|po|pe)}},
;
; And the XOR appears AFTER the conditional jump (i.e. on the
; conditionally-taken path).
; CHECK:      xor{{[ \t]+}}{{#?}}{{(0x)?}}1[Bb]
;
  %shl = shl i8 %atb, 1
  %xor = xor i8 %shl, 27
  %sgn = icmp slt i8 %z, 0
  %sel = select i1 %sgn, i8 %xor, i8 %shl
  ret i8 %sel
}
