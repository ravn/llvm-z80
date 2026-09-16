; RUN: llc -mtriple=z80 -O1 -z80-static-frames -z80-closed-world < %s | FileCheck %s --check-prefix=CLOSED
; RUN: llc -mtriple=z80 -O1 -z80-static-frames < %s | FileCheck %s --check-prefix=OPEN

; In an open world, an external call from a function with external linkage creates
; a synthetic cycle through CallsExternalNode -> ExternalCallingNode, preventing
; the recursion analysis from proving non-reentrancy.
; In a closed-world / freestanding environment, foreign code does not re-enter
; module symbols, so single-node SCCs are proven non-recursive and receive static frames.

declare void @ext_helper()

; OPEN-LABEL: f:
; OPEN-NOT: L_f.frame
; CLOSED-LABEL: f:
; CLOSED: L_f.frame
define void @f(i16 %x) {
  %buf = alloca [4 x i16], align 1
  store volatile i16 %x, ptr %buf, align 1
  call void @ext_helper()
  ret void
}
