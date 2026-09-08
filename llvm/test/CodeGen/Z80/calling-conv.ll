; RUN: llc -verify-machineinstrs -mtriple=z80 -z80-asm-format=sdasz80 -O0 < %s | FileCheck %s

; Test: void function returns with just ret
; CHECK-LABEL: void_return:
; CHECK:      	ret
define void @void_return() {
  ret void
}

; Test: 8-bit constant return in A
define i8 @return_const8() {
  ret i8 42
; CHECK-LABEL: return_const8:
; CHECK:      	ld	a,#42
; CHECK:      	ret
}

; Test: 16-bit constant return in DE (SDCC __sdcccall(1))
define i16 @return_const16() {
  ret i16 1234
}

; Test: return second 16-bit arg (DE) - already in return register
; CHECK-LABEL: return_const16:
; CHECK:      	ld	de,#1234
; CHECK:      	ret
define i16 @return_second(i16 %a, i16 %b) {
  ret i16 %b
}

; Test: return third 16-bit arg (stack in SDCC, only 2 reg params)
; At -O0 the compiler uses IX frame pointer to access stack args.
define i16 @return_third(i16 %a, i16 %b, i16 %c) {
  ret i16 %c
; CHECK-LABEL: return_second:
; CHECK:      	ret
}

; Test: 32-bit return identity (HLDE passthrough)
define i32 @return32(i32 %a) {
  ret i32 %a
}

; Test: 4th argument comes from stack (3rd and 4th on stack)
; CHECK-LABEL: return_third:
; CHECK:      	ld	hl,#2
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
define i16 @stack_arg(i16 %a, i16 %b, i16 %c, i16 %d) {
  ret i16 %d
}

; Test: function call
declare void @external_func(i16)

define void @call_func(i16 %x) {
  call void @external_func(i16 %x)
; CHECK-LABEL: return32:
; CHECK:      	ret
  ret void
}
; CHECK-LABEL: stack_arg:
; CHECK:      	ld	hl,#4
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	pop	bc
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	push	bc
; CHECK:      	ret
; CHECK-LABEL: call_func:
; CHECK:      	call	_external_func
; CHECK:      	ret
