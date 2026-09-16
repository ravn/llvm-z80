; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

@g_byte = external global i8
@g_array = external global [10 x i8]
@g_word = external global i16

; Test: 8-bit store to global variable
define void @store8_global(i8 %val) {
; CHECK-LABEL: _store8_global:
; CHECK:       ld (_g_byte),a
; CHECK-NEXT:  ret
  store i8 %val, ptr @g_byte
  ret void
}

; Test: 8-bit store to global variable with constant offset
define void @store8_global_offset(i8 %val) {
; CHECK-LABEL: _store8_global_offset:
; CHECK:       ld (_g_array+4),a
; CHECK-NEXT:  ret
  %ptr = getelementptr inbounds [10 x i8], ptr @g_array, i16 0, i16 4
  store i8 %val, ptr %ptr
  ret void
}

; Test: 8-bit load from global variable
define i8 @load8_global() {
; CHECK-LABEL: _load8_global:
; CHECK:       ld a,(_g_byte)
; CHECK-NEXT:  ret
  %v = load i8, ptr @g_byte
  ret i8 %v
}

; Test: 8-bit load from global variable with constant offset
define i8 @load8_global_offset() {
; CHECK-LABEL: _load8_global_offset:
; CHECK:       ld a,(_g_array+3)
; CHECK-NEXT:  ret
  %ptr = getelementptr inbounds [10 x i8], ptr @g_array, i16 0, i16 3
  %v = load i8, ptr %ptr
  ret i8 %v
}

; Test: 16-bit store to global variable (positive control)
define void @store16_global(i16 %val) {
; CHECK-LABEL: _store16_global:
; CHECK:       ld (_g_word),hl
; CHECK-NEXT:  ret
  store i16 %val, ptr @g_word
  ret void
}

; Test: 16-bit load from global variable (positive control)
define i16 @load16_global() {
; CHECK-LABEL: _load16_global:
; CHECK:       ld de,(_g_word)
; CHECK-NEXT:  ret
  %v = load i16, ptr @g_word
  ret i16 %v
}
