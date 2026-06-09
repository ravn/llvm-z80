; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s
;
; Direct exercise of the Z80 backend's claim of
; `llvm.experimental.memset.pattern`.  This bypasses Z80LoopIdiomFill so we
; verify the legalizer arm in isolation: PreISelIntrinsicLowering must NOT
; expand the intrinsic (TTI hook returns false for i8/i16/i32 patterns), and
; Z80LegalizerInfo must lower it to the seed + LDIR idiom.
;
; The lowering for count >= 2 emits, in order:
;   1. LD DE,dst+K           ; LDIR destination
;   2. <seed store(s)>       ; K-byte pattern at dst
;   3. LD HL,dst             ; LDIR source
;   4. LD BC,K*(count-1)     ; LDIR length
;   5. LDIR
; This is the same shape as the existing z80_pattern_fill lowering (the K=3
; / fork-local case in loop-idiom-fill.ll continues to use that arm; K in
; {1,2,4} routes through the upstream intrinsic).

declare void @llvm.experimental.memset.pattern.p0.i8.i16(ptr nocapture writeonly, i8, i16, i1)
declare void @llvm.experimental.memset.pattern.p0.i16.i16(ptr nocapture writeonly, i16, i16, i1)
declare void @llvm.experimental.memset.pattern.p0.i32.i16(ptr nocapture writeonly, i32, i16, i1)

; ---- K = 1 (byte fill, same shape as memset) --------------------------------
; Pattern 0xAA = 170; count 10 ; BC = 1 * (10 - 1) = 9.

; CHECK-LABEL: fill_byte:
; CHECK:       ld a,170
; CHECK:       ld (hl),a
; CHECK:       ld bc,9
; CHECK:       ldir
; CHECK:       ret
define void @fill_byte(ptr %dst) {
  call void @llvm.experimental.memset.pattern.p0.i8.i16(ptr %dst, i8 -86, i16 10, i1 false)
  ret void
}

; ---- K = 2 (word fill, the cpnos IVT-init shape) ----------------------------
; Pattern 0xAA55 = 43605; count 10 ; BC = 2 * (10 - 1) = 18.

; CHECK-LABEL: fill_word:
; CHECK:       ld de,43605
; CHECK:       ld bc,18
; CHECK:       ldir
; CHECK:       ret
define void @fill_word(ptr %dst) {
  call void @llvm.experimental.memset.pattern.p0.i16.i16(ptr %dst, i16 -21931, i16 10, i1 false)
  ret void
}

; ---- K = 4 (dword fill) -----------------------------------------------------
; Pattern 0xAAAAAAAA = -1431655766; count 10 ; BC = 4 * (10 - 1) = 36.

; CHECK-LABEL: fill_dword:
; CHECK:       ld bc,36
; CHECK:       ldir
; CHECK:       ret
define void @fill_dword(ptr %dst) {
  call void @llvm.experimental.memset.pattern.p0.i32.i16(ptr %dst, i32 -1431655766, i16 10, i1 false)
  ret void
}
