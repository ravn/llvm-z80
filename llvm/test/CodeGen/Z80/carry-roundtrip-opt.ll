; RUN: llc -mtriple=z80 -mattr=+static-frame -disable-lsr < %s | FileCheck %s --check-prefixes=CHECK,NOLSR
; RUN: llc -mtriple=z80 -mattr=+static-frame -O2 < %s | FileCheck %s --check-prefixes=CHECK,O2

; Test 1: 16-bit add loop where carry is roundtripped through A and inverted.
; Intervening register copies (ld c,l; ld b,h) are flag-neutral.
; The SBC A,A; AND 1; XOR 1; RRCA; JR C chain must fold to JR NC.
define dso_local void @test_u16_loop_overflow(i8 noundef zeroext %val) {
; CHECK-LABEL: test_u16_loop_overflow:
; NOLSR:       .LBB0_1:
; NOLSR:       	ld	hl,1
; NOLSR-NEXT:  	add	hl,de
; NOLSR-NEXT:  	ex	de,hl
; NOLSR-NEXT:  	jr	nc,.LBB0_1
; NOLSR-NOT:   	sbc	a,a
; NOLSR-NOT:   	rrca
entry:
  br label %loop

loop:
  %t = phi i16 [ 0, %entry ], [ %t.next, %body ]
  %msr = load volatile i8, ptr addrspace(2) inttoptr (i16 4 to ptr addrspace(2)), align 4
  %ready = icmp slt i8 %msr, -64
  br i1 %ready, label %ready_exit, label %body

ready_exit:
  store volatile i8 %val, ptr addrspace(2) inttoptr (i16 5 to ptr addrspace(2)), align 1
  br label %exit

body:
  %t.next = add i16 %t, 1
  %cmp = icmp eq i16 %t.next, 0
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

; Test 2: Direct issue #93 pattern: SBC A,A; AND 1; XOR 1; JR NZ -> JR NC
@port = external dso_local global ptr, align 2

define void @test_issue93_jr_nz() {
; CHECK-LABEL: test_issue93_jr_nz:
; O2:       .LBB1_1:
; O2:       	add	a,255
; O2-NEXT:  	jr	nc,.LBB1_1
; O2-NOT:   	sbc	a,a
; O2-NOT:   	and	1
; O2-NOT:   	xor	1
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

; Test 3: Positive control: A is live after the comparison, must preserve SBC A,A.
define i8 @test_negative_a_live(i16 %a, i16 %b) {
; CHECK-LABEL: test_negative_a_live:
; CHECK:       	sbc	a,a
; CHECK:       	and	1
entry:
  %sum = add i16 %a, %b
  %cmp = icmp eq i16 %sum, 0
  %res = zext i1 %cmp to i8
  ret i8 %res
}
