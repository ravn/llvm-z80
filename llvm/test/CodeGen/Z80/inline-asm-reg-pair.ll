; RUN: llc -mtriple=z80 -verify-machineinstrs < %s | FileCheck %s

; Inline asm binding operands to physical 16-bit register PAIRS (de/hl/bc) and
; to the 8-bit register a. This is exactly the IR clang emits for GCC local
; register variables (`register T x asm("de")`) -- the upstream-standard way to
; pin an operand to a Z80 pair, since clang has no single-token pair CONSTRAINT
; letter and rejects the braced "{de}" form in C source (only register NAMES
; via getGCCRegNames are accepted). The rcbios/cpnos LDIR/LDDR/`ld i,a` helpers
; rely on this lowering. Guards the D/E <-> DE sub-register overlap too: the
; pair constraint must occupy the whole DE, HL, BC pairs.

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; A backward block move: DE=dst_end (src of the move via lddr's implicit HL/DE),
; HL=src_end, BC=count. All three operands are pinned to their pairs.
; CHECK-LABEL: memmove_backwards:
; CHECK:      lddr
; CHECK:      ret
define void @memmove_backwards(ptr %d, ptr %s, i16 %n) {
  %1 = tail call { ptr, ptr, i16 } asm sideeffect "lddr", "={de},={hl},={bc},{de},{hl},{bc},~{memory}"(ptr %d, ptr %s, i16 %n)
  ret void
}

; Forward block move (memcpy shape): the same pair-pinning drives LDIR.
; CHECK-LABEL: memcpy_forward:
; CHECK:      ldir
; CHECK:      ret
define void @memcpy_forward(ptr %d, ptr %s, i16 %n) {
  %1 = tail call { ptr, ptr, i16 } asm sideeffect "ldir", "={de},={hl},={bc},{de},{hl},{bc},~{memory}"(ptr %d, ptr %s, i16 %n)
  ret void
}

; 8-bit register pinning: the page byte must land in A for `ld i, a`.
; CHECK-LABEL: ld_i_a:
; CHECK:      ld i,a
; CHECK:      ret
define void @ld_i_a(i8 %p) {
  tail call void asm sideeffect "ld i, a", "{a}"(i8 %p)
  ret void
}
