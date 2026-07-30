; RUN: llc -mtriple=z80 -O2 -z80-asm-format=sdasz80 < %s | FileCheck %s
;
; ravn/llvm-z80#243 FIX: add IX/IY to Uses in all LD_IXd_r/LD_IYd_r/LD_r_IXd/LD_r_IYd
; instruction definitions (Z80InstrInfo.td lines 323-338, 347-362, 370-371).
;
; Without this fix, computeRegisterLiveness(IY, after_end_copy) in the
; IX-transfer peephole (Form 2, Z80LateOptimization.cpp:~4781) returned
; LQR_Dead for IY even when a downstream LD_IYd_A used IY as an address
; register, causing the peephole to drop the `$iy = buf+idx` assignment
; while leaving `ld (iy+0), a` intact.  The store wrote to the caller's IY
; instead of buf+idx -- a miscompile visible only at -Os.
;
; After the fix, IY is declared as a Use of all IX/IY-indexed instructions.
; The liveness check now sees IY as live after the end copy and the peephole
; correctly skips the elimination.  The store was then materialised via IY
; (`push hl;pop iy ... push iy;pop hl ... ld 0(iy),a`).
;
; A FOLLOW-UP peephole (the "(IX/IY+0)->(HL)" fold in Z80LateOptimization.cpp)
; now recognises that HL and IY hold the same address across that window and
; rewrites the store back to (HL), dropping all IY traffic.  The function body
; reduces to `and (hl); ld (hl),a` -- both correct AND optimal.
;
; Smallest C repro (trigger: -Os):
;   static unsigned char buf[16];
;   void test(unsigned char idx, unsigned char tail) {
;       unsigned char mask = (unsigned char)(0xFF << (8 - tail));
;       buf[idx] &= (unsigned char)~mask;
;   }
;
; Wrong output before the #243 fix:
;   add  hl, de          ; HL = buf+idx
;   push hl              ; save HL for AND_HLind
;   cpl                  ; A = ~mask
;   pop  hl              ; restore HL
;   and  (hl)            ; A = buf[idx] & ~mask
;   ld   (iy+0), a       ; WRONG: IY was never set to buf+idx
;
; Correct AND optimal output now (after the #243 fix + the (IX/IY+0)->(HL) fold):
;   add  hl, de          ; HL = buf+idx
;   ld   a, c            ; A = mask
;   cpl                  ; A = ~mask
;   and  (hl)            ; A = buf[idx] & ~mask
;   ld   (hl), a         ; CORRECT: (buf+idx) = A, no IX/IY traffic

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@buf = internal unnamed_addr global [16 x i8] zeroinitializer, align 1

; Function Attrs: optsize
define dso_local void @test(i8 noundef zeroext %0, i8 noundef zeroext %1) local_unnamed_addr #0 {
  %3 = zext i8 %1 to i16
  %4 = sub nsw i16 8, %3
  %5 = shl nuw i16 255, %4
  %6 = zext i8 %0 to i16
  %7 = getelementptr inbounds nuw i8, ptr @buf, i16 %6
  %8 = load i8, ptr %7, align 1
  %9 = trunc i16 %5 to i8
  %10 = xor i8 %9, -1
  %11 = and i8 %8, %10
  store i8 %11, ptr %7, align 1
  ret void
}

attributes #0 = { optsize "target-features"="+z80" }

; CHECK-LABEL: _test:
; No IX/IY traffic: the address lives only in HL, and the load/store both
; use (HL).  The old wrong output stored to the caller's (iy+0); the
; intermediate (correct-but-suboptimal) output round-tripped through IY.
; CHECK-NOT:   iy
; CHECK:       and (hl)
; CHECK-NEXT:  ld (hl),a
; CHECK-NOT:   iy

;
