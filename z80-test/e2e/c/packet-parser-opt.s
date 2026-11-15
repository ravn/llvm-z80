	.file	"packet-parser-opt.s"
	.text
	.globl	_parse_packet_asm
	.type	_parse_packet_asm,@function

_parse_packet_asm:
	; incoming parameters: HL = base, DE = offset

	; if (rx_status & 0x01)
	ld	a, (_rx_status)                 ; ld bc, _rx_status / ld a, (bc)
	and	1                               ; and 1
	jr	nz, .L_err                      ; jr nz, .LBB0_7

	; uint8_t *ptr = base + offset;
	add	hl, de                          ; add hl, de  [HL = base + offset = ptr]
	ex	de, hl                          ; push hl / ld hl, 6 / add hl, sp / ld c, (hl)  [save ptr in DE]
	ld	hl, 2                           ; inc hl / ld b, (hl) / ld a, (bc)  [HL points to stack arg len]
	add	hl, sp                          ; ld hl, 4 / add hl, sp / ld c, (hl)
	ld	b, (hl)                         ; pop hl / ld b, a  [parameter len -> B]
	inc	hl
	ld	a, (hl)                         ; [parameter delim -> A]
	inc	hl
	ld	c, (hl)                         ; [parameter flags lo -> C]
	inc	hl
	ld	h, (hl)                         ; [parameter flags hi -> H]
	ld	l, c                            ; [HL = flags]
	push	hl
	pop	iy                              ; [parameter flags -> IY]
	ex	de, hl                          ; [restore ptr -> HL]

	; uint8_t matches = 0;
	ld	c, 0                            ; ld d, 0 / ld a, d / ld (L_parse_packet.frame), a  [matches -> C]

	; for (uint8_t i = len; i != 0; --i)
	inc	b                               ; push hl / ld hl, 4 / add hl, sp
	dec	b                               ; ld a, (hl) / pop hl / or a
	jr	z, .L_exit                      ; jr z, .LBB0_8 / jr .LBB0_5

.L_loop:
	; if (*ptr == delim)
	cp	(hl)                            ; ld a, (hl) / push hl / ld hl, 5 / add hl, sp / ld d, (hl) / pop hl / cp d
	jr	nz, .L_else                     ; jr nz, .LBB0_3

	; matches++; *flags |= 0x80;
	inc	c                               ; ld a, (L_parse_packet.frame) / inc d / ld (frame), a
	set	7, (iy+0)                       ; set 7, b
	jr	.L_next                         ; jr .LBB0_4

.L_else:
	; (*flags)++;
	inc	(iy+0)                          ; inc b / ld a, b / push hl / ld hl, 6 / add hl, sp / ld e, (hl) / inc hl / ld d, (hl) / pop hl / ld (de), a

.L_next:
	; ptr++;
	inc	hl                              ; inc hl
	djnz	.L_loop                         ; dec c / inc hl / jr z, .LBB0_9

.L_exit:
	; return matches;
	pop	de                              ; pop bc
	inc	sp                              ; inc sp
	inc	sp                              ; inc sp
	inc	sp                              ; inc sp
	inc	sp                              ; inc sp
	push	de                              ; push bc
	ld	a, c                            ; ld a, (L_parse_packet.frame)  [return matches in A]
	ret                                 ; ret

.L_err:
	; packet_errors++; return 0;
	ld	hl, _packet_errors              ; ld bc, _packet_errors
	inc	(hl)                            ; ld a, (bc) / inc a / ld (bc), a
	ld	c, 0                            ; ld b, 0 / ld a, b / ld (L_parse_packet.frame), a  [return 0]
	jr	.L_exit                         ; jr .LBB0_9

.Lfunc_end0:
	.size	_parse_packet_asm, .Lfunc_end0-_parse_packet_asm
