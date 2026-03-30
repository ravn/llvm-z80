	.file	"hello_libc.c"
	.text
	.globl	_main                           ; -- Begin function main
	.type	_main,@function
_main:                                  ; @main
; %bb.0:
	ld	hl,L_.str
	push	hl
	call	_puts
	pop	af
	ld	hl,3
	push	hl
	ld	hl,L_.str.1
	push	hl
	call	_printf
	pop	af
	pop	af
	ld	de,0
	ret
.Lfunc_end0:
	.size	_main, .Lfunc_end0-_main
                                        ; -- End function
	.type	L_.str,@object                  ; @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
L_.str:
	.asciz	"Hello World!"
	.size	L_.str, 13

	.type	L_.str.1,@object                ; @.str.1
L_.str.1:
	.asciz	"1 + 2 = %d\n"
	.size	L_.str.1, 12

	.ident	"clang version 23.0.0git"
	.section	".note.GNU-stack","",@progbits
	.addrsig
