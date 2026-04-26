; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Verify __attribute__((naked)) emits no compiler-inserted prologue or
; epilogue.  The function body must be exactly the inline asm — no
; push/pop ix, no SP fiddling, no implicit ret.
;
; cpnos-rom Phase 3 (2026-04-26) depends on this property to migrate
; bios_shims.s, isr.s, and other ABI-translation shims into naked C
; co-located with their C bodies.

; CHECK-LABEL: _bare_naked:
; CHECK-NOT:   push ix
; CHECK-NOT:   pop ix
; CHECK-NOT:   ld ix,
; CHECK:       ld a,#66
; CHECK-NEXT:  out (24),a
; CHECK-NEXT:  ret
; CHECK-NOT:   push ix
; CHECK-NOT:   pop ix
define void @bare_naked() naked {
  call void asm sideeffect "ld a, #0x42 \0A\09 out (0x18), a \0A\09 ret", ""()
  unreachable
}

; sdcccall(1) passes uint8_t in A.  A naked function reading that arg
; via inline asm must see A unmodified — no spill, no register move.

; CHECK-LABEL: _naked_with_arg:
; CHECK-NOT:   push ix
; CHECK-NOT:   pop ix
; CHECK:       out (24),a
; CHECK-NEXT:  ret
; CHECK-NOT:   push ix
; CHECK-NOT:   pop ix
define void @naked_with_arg(i8 %c) naked {
  call void asm sideeffect "out (0x18), a \0A\09 ret", ""()
  unreachable
}
