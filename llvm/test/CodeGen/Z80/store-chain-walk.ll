; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -mattr=+static-stack -O2 < %s | FileCheck %s

; Issue #85: ≥3 consecutive byte stores to consecutive addresses
; should compile to a single `LD HL, base` followed by repeated
; `LD (HL), n; INC HL` (omitting the trailing INC HL).
;
; Per-pair cost in the old form:  LD A,n; LD (addr),a  =  5 B
; Per-pair cost in the new form:  LD (HL),n; INC HL    =  3 B
; Plus a one-time 3 B `LD HL, base` setup.
;
;  N=2:  10 B → 11 B  (loss; skip)
;  N=3:  15 B → 11 B  (-4 B)
;  N=4:  20 B → 14 B  (-6 B)
;
; Z80LateOptimization peephole accumulates a maximal run of
; { LD A,imm; LD (addr),A } pairs whose addresses are arithmetic-1
; consecutive (matching on global symbol + offset, or on numeric
; immediate), and rewrites when the run length is ≥3.

@buf = external global [8 x i8], align 1

define void @seed_buf() {
  store i8 16, ptr @buf, align 1
  store i8 32, ptr getelementptr inbounds ([8 x i8], ptr @buf, i16 0, i16 1), align 1
  store i8 48, ptr getelementptr inbounds ([8 x i8], ptr @buf, i16 0, i16 2), align 1
  store i8 64, ptr getelementptr inbounds ([8 x i8], ptr @buf, i16 0, i16 3), align 1
  ret void
}

; CHECK-LABEL: _seed_buf:
; CHECK:      ld  hl,#_buf
; CHECK-NEXT: ld  (hl),#16
; CHECK-NEXT: inc  hl
; CHECK-NEXT: ld  (hl),#32
; CHECK-NEXT: inc  hl
; CHECK-NEXT: ld  (hl),#48
; CHECK-NEXT: inc  hl
; CHECK-NEXT: ld  (hl),#64
; CHECK-NEXT: ret

; Sanity: only 2 consecutive stores -- run too short, fall back to
; the old `LD A,n; LD (addr),a` form (no HL setup overhead win).
@buf2 = external global [8 x i8], align 1

define void @seed_two() {
  store i8 1, ptr @buf2, align 1
  store i8 2, ptr getelementptr inbounds ([8 x i8], ptr @buf2, i16 0, i16 1), align 1
  ret void
}

; CHECK-LABEL: _seed_two:
; CHECK:      ld  a,#1
; CHECK-NEXT: ld  (_buf2),a
; CHECK:      ld  a,#2
; CHECK-NEXT: ld  (_buf2+1),a
; CHECK-NOT:  ld  hl,
