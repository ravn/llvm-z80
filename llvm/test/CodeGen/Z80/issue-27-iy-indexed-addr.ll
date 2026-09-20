; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -z80-idx-addr -z80-verify-inline-runtime-size < %s | FileCheck %s --check-prefix=ON
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s --check-prefix=OFF
; XFAIL: *

; ravn/llvm-z80#27: a call-free function that dereferences a pointer at a
; constant offset should use IX/IY-displacement addressing (ld r,d(i?) /
; ld d(i?),r) instead of copying the base into HL and adding the offset.
; The base is constrained to the index class (IR16) so it lands in IX/IY and
; Z80ExpandPseudo emits the indexed form.  Flag-gated, default off, and only
; when the base has >=2 constant-offset access sites (so the one-time
; `push hl; pop iy` setup amortises) and the function is call-free (IY is
; caller-saved, Z80_CSR = IX only).

; Several derefs of the same base share one IX/IY setup (the amortising case).
define i8 @sum3(ptr %p) optsize {
; ON-LABEL: sum3:
; ON:       ld {{[a-l]}},{{[0-9]+\(i[xy]\)}}
; ON:       ld {{[a-l]}},{{[0-9]+\(i[xy]\)}}
; ON:       ld {{[a-l]}},{{[0-9]+\(i[xy]\)}}
;
; Without the flag the same code copies the base into HL and adds offsets.
; OFF-LABEL: sum3:
; OFF:       add hl,
entry:
  %a1 = getelementptr inbounds i8, ptr %p, i16 1
  %a5 = getelementptr inbounds i8, ptr %p, i16 5
  %a13 = getelementptr inbounds i8, ptr %p, i16 13
  %v1 = load i8, ptr %a1
  %v5 = load i8, ptr %a5
  %v13 = load i8, ptr %a13
  %s = add i8 %v1, %v5
  %t = add i8 %s, %v13
  ret i8 %t
}

; Read-modify-write at two sites: both the loads and the stores use
; displacement addressing, so there is no add-to-HL at all.
define void @rmw2(ptr %p) optsize {
; ON-LABEL: rmw2:
; CHECK-LABEL: sum3:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	ld	e,l
; CHECK:      	ld	d,h
; CHECK:      	inc	de
; CHECK:      	ld	(L_sum3.frame+2),de
; CHECK:      	ld	de,#5
; CHECK:      	add	hl,de
; CHECK:      	ld	(L_sum3.frame),hl
; CHECK:      	ld	de,#13
; CHECK:      	ld	l,c
; CHECK:      	ld	h,b
; CHECK:      	add	hl,de
; CHECK:      	ld	bc,(L_sum3.frame+2)
; CHECK:      	ld	a,(bc)
; CHECK:      	ld	b,a
; CHECK:      	ld	de,(L_sum3.frame)
; CHECK:      	ld	a,(de)
; CHECK:      	ld	c,a
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	a,b
; CHECK:      	add	a,c
; CHECK:      	add	a,d
; CHECK:      	ret
; ON:       ld a,{{[0-9]+\(i[xy]\)}}
; ON:       ld {{[0-9]+\(i[xy]\)}},a
; ON:       ld a,{{[0-9]+\(i[xy]\)}}
; ON:       ld {{[0-9]+\(i[xy]\)}},a
; ON-NOT:   add hl,
entry:
  %a2 = getelementptr inbounds i8, ptr %p, i16 2
  %a6 = getelementptr inbounds i8, ptr %p, i16 6
  %v2 = load i8, ptr %a2
  %i2 = add i8 %v2, 1
  store i8 %i2, ptr %a2
  %v6 = load i8, ptr %a6
  %i6 = add i8 %v6, 1
  store i8 %i6, ptr %a6
  ret void
}

; A single access site does NOT amortise the IX/IY setup, so the profitability
; gate (>=2 sites) leaves it on the add-to-HL path even with the flag on.
define void @single(ptr %p) optsize {
; ON-LABEL: single:
; ON:       add hl,
; ON-NOT:   {{[0-9]+\(i[xy]\)}}
entry:
  %a = getelementptr inbounds i8, ptr %p, i16 9
  %v = load i8, ptr %a
  %i = add i8 %v, 1
  store i8 %i, ptr %a
  ret void
}

; A function with a call must NOT use the transform: IY is caller-saved, so a
; base parked in IY would not survive the call.
declare void @ext()
define i8 @has_call(ptr %p) optsize {
; ON-LABEL: has_call:
; ON:       add hl,
; CHECK-LABEL: rmw2:
; CHECK:      	ld	c,l
; CHECK:      	ld	b,h
; CHECK:      	inc	bc
; CHECK:      	inc	bc
; CHECK:      	ld	de,#6
; CHECK:      	add	hl,de
; CHECK:      	ld	a,(bc)
; CHECK:      	inc	a
; CHECK:      	ld	(bc),a
; CHECK:      	ld	a,(hl)
; CHECK:      	inc	a
; CHECK:      	ld	(hl),a
; CHECK:      	ret
; ON-NOT:   {{[0-9]+\(i[xy]\)}}
entry:
  call void @ext()
  %a = getelementptr inbounds i8, ptr %p, i16 7
  %v = load i8, ptr %a
  ret i8 %v
}
; CHECK-LABEL: single:
; CHECK:      	ld	bc,#9
; CHECK:      	add	hl,bc
; CHECK:      	ld	a,(hl)
; CHECK:      	inc	a
; CHECK:      	ld	(hl),a
; CHECK:      	ret
; CHECK-LABEL: has_call:
; CHECK:      	push	af
; CHECK:      	ld	e,l
; CHECK:      	ld	d,h
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	(hl),e
; CHECK:      	inc	hl
; CHECK:      	ld	(hl),d
; CHECK:      	call	_ext
; CHECK:      	ld	bc,#7
; CHECK:      	ld	hl,#0
; CHECK:      	add	hl,sp
; CHECK:      	ld	e,(hl)
; CHECK:      	inc	hl
; CHECK:      	ld	d,(hl)
; CHECK:      	ld	l,e
; CHECK:      	ld	h,d
; CHECK:      	add	hl,bc
; CHECK:      	ld	a,(hl)
; CHECK:      	inc	sp
; CHECK:      	inc	sp
; CHECK:      	ret
