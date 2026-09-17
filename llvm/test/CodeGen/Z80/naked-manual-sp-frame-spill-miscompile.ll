; RUN: llc -mtriple=z80 -O2 -disable-lsr < %s | FileCheck %s
; XFAIL: *
;
; ravn/llvm-z80#318 -- Miscompile: a function that resets SP itself (inline asm
; `ld sp, imm`, as the RC700 autoload `main_relocated` does via SET_SP(ROM_STACK))
; must not keep any compiler-managed frame slot alive across the SP write.
; The register allocator computes spill-slot addresses in the prologue relative
; to the ENTRY SP (`ld hl,K; add hl,sp; ld (hl),...`), then the inline `ld sp`
; moves SP, and the later reload reads the slot relative to the NEW SP -- a
; different, uninitialised address. Observed on autoload: `fdc_cmd.sector = 1`
; reloaded a spilled `&fdc_cmd` pointer of 0x0000 (MAME write-tap), so the store
; went to null and boot failed with DISKETTE ERROR (rc700-gensmedet#128 as
; originally reported; that issue has since been re-diagnosed as a compiler-
; independent FDC emulation problem after the display/ISR fix landed).
;
; History: an earlier reduced repro (single-function autoload shape) was
; incidentally resolved by 37f696f38ee5 -- restoring 8-bit direct global
; addressing eliminated the spill for that specific shape, so the fixture
; started PASSing. This stronger repro forces 16-bit spilling (5 live i16
; values across 5 CALLs before the inline `ld sp`) so the miscompile class
; itself is pinned, not just the one shape that happened to disappear.
;
; Correct behavior: no frame slot may be read or written that spans the manual
; SP reset -- either the frame must be established after the SP write, or the
; peephole/RA must treat an inline-asm SP-clobber as a barrier and materialise
; live values elsewhere (e.g. push/pop pairs on the callee's own frame chain).
; When fixed, the test XPASSes: drop the XFAIL.
;
; Fix directions (documented on ravn/llvm-z80#318):
;   * treat an inline-asm `sp` clobber as a barrier that forbids SP-relative
;     frame access on both sides, OR
;   * make `__naked` on clang actually suppress the frame (currently no-op), OR
;   * require the caller to set SP before entry so the callee never resets it.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

declare void @sink(i16, i16, i16, i16, i16)
declare i16 @src()

define dso_local void @mainrel(i16 %a, i16 %b, i16 %c, i16 %d, i16 %e) {
; CHECK-LABEL: mainrel:
; No frame slot may be materialised (`add hl,sp`) before the manual SP reset,
; because such a slot would be reloaded relative to the WRONG sp afterwards.
; CHECK-NOT: add hl,sp
; CHECK: ld sp,
  %x1 = call i16 @src()
  %x2 = call i16 @src()
  %x3 = call i16 @src()
  %x4 = call i16 @src()
  %x5 = call i16 @src()
  tail call void asm sideeffect "ld sp, 0xbfff", "~{sp}"()
  %s1 = add i16 %a, %x1
  %s2 = add i16 %b, %x2
  %s3 = add i16 %c, %x3
  %s4 = add i16 %d, %x4
  %s5 = add i16 %e, %x5
  call void @sink(i16 %s1, i16 %s2, i16 %s3, i16 %s4, i16 %s5)
  ret void
}
