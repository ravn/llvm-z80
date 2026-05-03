; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O1 < %s | FileCheck %s

; ravn/llvm-z80#107 — the consecutive `LD A,n; LD (addr),A` chain
; peephole (issue #85) rewrites a run of byte-immediate stores into
;   LD HL, base; LD (HL),n0; INC HL; LD (HL),n1; INC HL; LD (HL),n2
; The rewrite clobbers H and L via the new LD HL,base.  Without a
; liveness check, the peephole fires even when the surrounding code
; held a live value in H or L — corrupting that value.  Same anti-
; pattern as #104.
;
; Reproducer: %ptr arrives in HL (sdcccall passes i16 ptr in HL); a
; chain of three byte stores follows, then *%ptr is loaded.  Pre-fix,
; the chain peephole rewrote the three stores to use HL, destroying
; the parameter, and the final load read from buf[2] instead of *%ptr.

target triple = "z80"

%struct.three = type { i8, i8, i8 }
@buf = dso_local global %struct.three zeroinitializer, align 1

; CHECK-LABEL: _hold_hl_across_chain:

; The chain rewrite must NOT fire here because HL is live across the
; chain (it holds the %ptr parameter, used by the trailing load).  We
; assert that the original `LD A,n; LD (addr),A` form is preserved
; rather than the LD HL,#_buf rewrite.
;
; CHECK:        ld      a,#17
; CHECK:        ld      ({{.*}}buf{{.*}}),a
; CHECK:        ld      a,#34
; CHECK:        ld      a,(hl)
; CHECK-NOT:    ld      hl,#_buf
; CHECK:        ret

define i8 @hold_hl_across_chain(ptr %ptr) {
entry:
  %p0 = getelementptr inbounds %struct.three, ptr @buf, i32 0, i32 0
  %p1 = getelementptr inbounds %struct.three, ptr @buf, i32 0, i32 1
  %p2 = getelementptr inbounds %struct.three, ptr @buf, i32 0, i32 2
  store volatile i8 17, ptr %p0, align 1
  store volatile i8 34, ptr %p1, align 1
  store volatile i8 51, ptr %p2, align 1
  %v = load volatile i8, ptr %ptr, align 1
  ret i8 %v
}
