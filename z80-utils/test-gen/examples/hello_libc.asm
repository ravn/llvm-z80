	EXTERN _printf
	EXTERN _puts
	SECTION CODE
	PUBLIC _main
_main:                                  ; @main
	ld	hl,L_str
	push	hl
	call	_puts
	pop	af
	ld	hl,3
	push	hl
	ld	hl,L_str_1
	push	hl
	call	_printf
	pop	af
	pop	af
	ld	de,0
	ret
	SECTION RODATA
L_str:
	defm "Hello World!"
	defb 0
L_str_1:
	defm "1 + 2 = %d\n"
	defb 0
