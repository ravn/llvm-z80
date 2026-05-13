; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#152: extend #147's SET/RES n,(HL) peephole to fire
; when intervening insns READ A (but don't write it) — preserve A
; via an explicit `LD A,(HL)` after the address load.
;
; Pre-#152:
;   LD A,(cfgtbl)        ; 3 B
;   LD D,A               ;       <-- intervening A-reader: bails
;   OR/AND n             ; 2 B
;   LD (cfgtbl),A        ; 3 B
;
; Post-#152, single-bit (Pop == 1):
;   LD HL,cfgtbl         ; 3 B
;   LD A,(HL)            ; 1 B   <-- preserves A for intervening readers
;   LD D,A               ;       <-- unchanged
;   SET/RES b,(HL)       ; 2 B
;
; Saves 2 B per fire for single-bit ops with A-readers.  Two-bit ops
; break even on size but cost extra T-states (skip).

@cfgtbl = external global i8

declare void @sink(i8)

; Single-bit clear with an intervening A-reader (the `unsigned char
; st = cfgtbl; sink(st)` shape from the issue, but inlined so the
; save and the mutation are in the same MBB).
;
; CHECK-LABEL: clear_one_with_reader:
; CHECK:       ld	hl,{{_?cfgtbl}}
; CHECK-NEXT:  ld	a,(hl)
; CHECK:       res	0,(hl)
; CHECK-NOT:   and	{{-?[0-9]+}}
; CHECK-NOT:   ld	a,({{_?cfgtbl}})
; CHECK-NOT:   ld	({{_?cfgtbl}}),a
define i8 @clear_one_with_reader() {
entry:
  %st = load volatile i8, ptr @cfgtbl
  %new = and i8 %st, -2          ; clear bit 0
  store volatile i8 %new, ptr @cfgtbl
  ret i8 %st                     ; %st is the intervening A-reader
}

; Single-bit set with an A-reader.
;
; CHECK-LABEL: set_one_with_reader:
; CHECK:       ld	hl,{{_?cfgtbl}}
; CHECK-NEXT:  ld	a,(hl)
; CHECK:       set	7,(hl)
; CHECK-NOT:   or	{{-?[0-9]+}}
; CHECK-NOT:   ld	a,({{_?cfgtbl}})
; CHECK-NOT:   ld	({{_?cfgtbl}}),a
define i8 @set_one_with_reader() {
entry:
  %st = load volatile i8, ptr @cfgtbl
  %new = or i8 %st, -128         ; set bit 7
  store volatile i8 %new, ptr @cfgtbl
  ret i8 %st
}

; Negative: two-bit clear with reader — break-even on bytes but
; +14T per fire, skip.
;
; CHECK-LABEL: clear_two_with_reader:
; CHECK:       ld	a,({{_?cfgtbl}})
; CHECK:       and	252
; CHECK:       ld	({{_?cfgtbl}}),a
; CHECK-NOT:   res	{{[0-9]}},(hl)
define i8 @clear_two_with_reader() {
entry:
  %st = load volatile i8, ptr @cfgtbl
  %new = and i8 %st, -4          ; clear bits 0 and 1 (Pop == 2)
  store volatile i8 %new, ptr @cfgtbl
  ret i8 %st
}
