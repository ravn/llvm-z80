; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -mattr=+static-stack -z80-idx-addr < %s | FileCheck %s --check-prefix=ON
; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 -mattr=+static-stack < %s | FileCheck %s --check-prefix=OFF

; ravn/llvm-z80#27: a call-free function that dereferences a pointer at a
; constant offset should use IX/IY-displacement addressing (ld r,d(i?) /
; ld d(i?),r) instead of copying the base into HL and adding the offset.
; The base is constrained to the index class (IR16) so it lands in IX/IY and
; Z80ExpandPseudo emits the indexed form.  Flag-gated, default off.

; A read-modify-write at a constant offset: with the flag both the load and
; the store use displacement addressing, so no add-to-HL is needed.
define void @rmw(ptr %p) optsize {
; ON-LABEL: rmw:
; ON:       ld a,{{[0-9]+\(i[xy]\)}}
; ON:       ld {{[0-9]+\(i[xy]\)}},a
; ON-NOT:   add hl,
;
; OFF-LABEL: rmw:
; OFF:       add hl,
; OFF:       ld a,(hl)
entry:
  %a = getelementptr inbounds i8, ptr %p, i16 9
  %v = load i8, ptr %a
  %inc = add i8 %v, 1
  store i8 %inc, ptr %a
  ret void
}

; Several derefs of the same base share one IX/IY setup (the amortising case).
define i8 @sum3(ptr %p) optsize {
; ON-LABEL: sum3:
; ON:       ld {{[a-l]}},{{[0-9]+\(i[xy]\)}}
; ON:       ld {{[a-l]}},{{[0-9]+\(i[xy]\)}}
; ON:       ld {{[a-l]}},{{[0-9]+\(i[xy]\)}}
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

; A function with a call must NOT use the transform: IY is caller-saved
; (Z80_CSR = IX only), so a base parked in IY would not survive the call.
declare void @ext()
define i8 @has_call(ptr %p) optsize {
; ON-LABEL: has_call:
; ON-NOT:   {{[0-9]+\(i[xy]\)}}
entry:
  call void @ext()
  %a = getelementptr inbounds i8, ptr %p, i16 7
  %v = load i8, ptr %a
  ret i8 %v
}
