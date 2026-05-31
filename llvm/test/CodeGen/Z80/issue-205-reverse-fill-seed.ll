; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 < %s | FileCheck %s --check-prefix=OFF
; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -z80-reverse-fill-seed -verify-machineinstrs < %s | FileCheck %s --check-prefix=ON
;
; ravn/llvm-z80#205 follow-up (experimental, default OFF): the K=2 LDIR-fill
; seed `LD HL,VAL; LD (nn),HL; ...; LD HL,nn; LDIR` can be rewritten as a
; reversed (HL) byte seed that lands HL on the fill base (= the LDIR source),
; folding away the separate source load and saving 1 byte/site.  Only fires for
; a constant VAL (a symbol VAL would need MO_LO/MO_HI byte halves).
;
; VAL = 51966 = 0xCAFE -> lo 0xFE (254), hi 0xCA (202).
;
; Default (flag off): the absolute seed store + separate source load.
; OFF-LABEL: _fill_word:
; OFF:       ld hl,51966
; OFF:       ld ({{_buf2.*}}),hl
; OFF:       ld hl,_buf2
; OFF:       ldir
;
; With the flag: reversed (HL) seed; no `ld (nn),hl`, no separate `ld hl,_buf2`.
; ON-LABEL:  _fill_word:
; ON-NOT:    ld ({{_buf2[^+].*}}),hl
; ON:        ld hl,_buf2+1
; ON-NEXT:   ld (hl),202
; ON-NEXT:   dec hl
; ON-NEXT:   ld (hl),254
; ON:        ldir

@buf2 = external dso_local global [16 x i16]

define void @fill_word() {
entry:
  br label %loop
loop:
  %i = phi i8 [ 0, %entry ], [ %i.next, %loop ]
  %i16 = zext i8 %i to i16
  %p = getelementptr inbounds nuw [16 x i16], ptr @buf2, i16 0, i16 %i16
  store i16 -13570, ptr %p, align 1
  %i.next = add i8 %i, 1
  %done = icmp eq i8 %i.next, 16
  br i1 %done, label %exit, label %loop
exit:
  ret void
}
