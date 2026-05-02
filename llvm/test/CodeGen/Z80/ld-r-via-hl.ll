; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s

; Issue #76: GISel sometimes emits the 2-instruction A-via path
;   LD A,(HL); LD r,A     (2 B / 11 T)
;   LD A,r;     LD (HL),A (2 B /  8 T)
; when the direct forms `LD r,(HL)` and `LD (HL),r` (1 B / 7 T) exist
; for every 8-bit GP register.  Z80LateOptimization detects the pair
; with A dead after and rewrites to the direct form.

@v = external dso_local global i8
@port_table = external dso_local global ptr


; --- Motivating case: cpnos-rom port-init shape ----------------------
; Byte from *port_table → C; B=0; A=v; OUT (C),A.  Pre-#76 the byte
; went via A: `ld a,(hl); ld c,a` (2 B).  Post-#76: `ld c,(hl)` (1 B).
define void @port_out_byte() {
  %t = load ptr, ptr @port_table, align 1
  %port_lo = load i8, ptr %t, align 1
  %v = load i8, ptr @v, align 1
  %port16 = zext i8 %port_lo to i16
  call void asm sideeffect "out (c), a", "{bc},{a},~{memory}"(i16 %port16, i8 %v)
  ret void
}
; CHECK-LABEL: _port_out_byte:
; CHECK-NOT:   ld  a,(hl)
; CHECK-NOT:   ld  c,a
; CHECK:       ld  c,(hl)
; CHECK:       out (c),a
; CHECK:       ret
