; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s

; Issue #93 (path b -- post-RA peephole).  GISel + LSR rewrites a
; constant-trip-count countdown to a count-up-from-(-N) form, where
; the wrap-to-zero exit test is materialised as a 4-instruction
; carry-roundtrip + a bigger-than-needed `add a,1`:
;
;     ld   a, r           ; reload counter
;     add  a, 1           ; counter++ (sets carry on wrap to 0)
;     ld   r, a           ; save back
;     sbc  a, a           ; A = 0xFF if no carry, 0x00 if carry
;     and  1              ; A = (was no-carry ? 1 : 0)
;     xor  1              ; A = (was no-carry ? 0 : 1)
;     rrca                ; rotate bit 0 into carry
;     jr   c, .loop       ; loop iff old C was 0
;
; That's 11 bytes of post-body code per loop site.  Two peepholes in
; Z80LateOptimization fix this:
;
;   1. SBC A,A; AND 1; XOR 1; RRCA; JR C target  →  JR NC target
;      (the 4-inst chain inverts carry; flipping the branch condition
;       achieves the same exit decision)
;   2. LD A,r; ADD A,1; LD r,A; JR NC target     →  INC r; JR NZ target
;      (after step 1, the only consumer of ADD's carry is the JR; INC
;       sets Z and the only way ADD A,1 wraps is when result is 0, so
;       JR NC ≡ JR NZ here)
;
; Combined: 11 B → 3 B per loop site.

@port = external dso_local global ptr, align 2

define void @const_trip_50() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 50, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}
; CHECK-LABEL: _const_trip_50:
; The carry-roundtrip chain must be gone.
; CHECK-NOT:   sbc a,a
; CHECK-NOT:   rrca
; CHECK-NOT:   add a,1
; The replacement is INC + JR NZ.
; CHECK:       inc d
; CHECK-NEXT:  jr nz,
; CHECK:       ret


; A larger constant trip count (255) should also fold.
define void @const_trip_255() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 255, %entry ], [ %i.next, %loop ]
  %p = load volatile ptr, ptr @port, align 2
  store volatile i8 0, ptr %p, align 1
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}
; CHECK-LABEL: _const_trip_255:
; CHECK-NOT:   sbc a,a
; CHECK:       inc d
; CHECK-NEXT:  jr nz,
; CHECK:       ret
