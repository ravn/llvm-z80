; RUN: llc -mtriple=z80 -O1 -verify-machineinstrs < %s | FileCheck %s --check-prefix=Z80
; RUN: llc -mtriple=sm83 -O1 -verify-machineinstrs < %s | FileCheck %s --check-prefix=SM83
; RUN: llc -mtriple=z80 -O0 -verify-machineinstrs < %s -o /dev/null
; RUN: llc -mtriple=sm83 -O0 -verify-machineinstrs < %s -o /dev/null

; A return value too large for registers is stored through a hidden sret
; pointer taken from the stack. Functions with no formal arguments used to
; skip the code that reads that pointer and crashed in lowerReturn.

; The function must read the pointer from the incoming stack slot and store
; the value through it.
; Z80-LABEL: ret_i64:
; Z80: push ix
; Z80: ld c,(ix+4)
; Z80: ld (hl),e
; Z80: ret
; SM83-LABEL: ret_i64:
; SM83: ld bc,(__sfrend_ret_i64+4)
; SM83: ld (hl+),a
; SM83: ret
define i64 @ret_i64() {
  ret i64 1234605616436508552 ; 0x1122334455667788
}

; Z80-LABEL: ret_f64:
; Z80: push ix
; Z80: ld c,(ix+4)
; Z80: ld (hl),e
; Z80: ret
; SM83-LABEL: ret_f64:
; SM83: ld bc,(__sfrend_ret_f64+4)
; SM83: ld (hl+),a
; SM83: ret
define double @ret_f64() {
  ret double 1.5
}
