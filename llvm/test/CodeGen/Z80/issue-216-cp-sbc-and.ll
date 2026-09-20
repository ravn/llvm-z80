; RUN: llc -O2 -mtriple=z80 < %s | FileCheck %s
;
; XFAIL: *
;
; =============================================================================
; KODEDENSITETSOPTIMERING (Code density optimization)
; Issue ravn/llvm-z80#216: `(x < imm) ? value : 0`
;
; Dette er en ren kodedensitetsoptimering (size/speed optimization), ikke en
; miscompile. Clang genererer i dag fuldt funktionel kode (8 B, 33-35 ts), men
; på Z80 kan mønstret udføres grenløst på kun 5 B / 25 ts via `sbc a, a`.
;
; C-KILDETEKST (direkte fra Clang C-frontend):
;
;   unsigned char branch_bot_or_zero(unsigned char line, unsigned char bot) {
;       return (line < 11) ? bot : 0;
;   }
;
;   unsigned char sel_if(unsigned char x, unsigned char val) {
;       if (x < 11)
;           return val;
;       return 0;
;   }
;
;   unsigned char sel_mask(unsigned char x, unsigned char val) {
;       return val & -(x < 11);
;   }
;
; Alle tre C-funktioner oversættes af `clang -O2` til den samme IR-form:
;   %c = icmp ult i8 %x, 11
;   %r = select i1 %c, i8 %val, i8 0
;
; Nuværende Z80-kode (8 B, 33-35 ts):
;   cp   11
;   jr   c, .Lend
;   ld   l, 0
; .Lend:
;   ld   a, l
;   ret
;
; Optimal kodedensitetsoptimeret form (5 B, 25 ts -> -3 B / -37.5%, -8..10 ts):
;   cp   11             ; 2 B, 7 ts   (CF = 1 hvis line < 11, ellers 0)
;   sbc  a, a           ; 1 B, 4 ts   (A = 0xFF hvis CF, ellers 0x00)
;   and  l              ; 1 B, 4 ts   (A = bot hvis line < 11, ellers 0)
;   ret                 ; 1 B, 10 ts
;
; XFAIL-årsag:
;   GlobalISel IRTranslator ekspanderer `select` til en CFG-trekant før
;   instruktionsvalg. At opnå denne optimering kræver enten en GISel pre-legalize
;   combiner eller en post-RA trekants-kollaps i Z80LateOptimization.
;   Da mønstret i dag ikke optræder i aktiv produktionskode, er testen parkeret
;   som XFAIL for at dokumentere det ønskede kodedensitetsmål.
; =============================================================================

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-ve-n8:16"
target triple = "z80"

; C-kilde: return (line < 11) ? bot : 0;
define dso_local noundef zeroext i8 @branch_bot_or_zero(i8 noundef zeroext %line, i8 noundef zeroext %bot) {
  %c = icmp ult i8 %line, 11
  %r = select i1 %c, i8 %bot, i8 0
  ret i8 %r
}

; CHECK-LABEL: _branch_bot_or_zero:
; CHECK-NOT: jr{{[ \t]+}}c
; CHECK-NOT: jr{{[ \t]+}}nc
; CHECK: cp{{[ \t]+}}11
; CHECK: sbc{{[ \t]+}}a, a
; CHECK: and{{[ \t]+}}l
; CHECK: ret

; C-kilde: if (x < 11) return val; return 0;
define dso_local noundef zeroext i8 @sel_if(i8 noundef zeroext %x, i8 noundef zeroext %val) {
  %c = icmp ult i8 %x, 11
  %r = select i1 %c, i8 %val, i8 0
  ret i8 %r
}

; CHECK-LABEL: _sel_if:
; CHECK-NOT: jr{{[ \t]+}}c
; CHECK-NOT: jr{{[ \t]+}}nc
; CHECK: cp{{[ \t]+}}11
; CHECK: sbc{{[ \t]+}}a, a
; CHECK: and{{[ \t]+}}l
; CHECK: ret

; C-kilde: return val & -(x < 11);
define dso_local noundef zeroext i8 @sel_mask(i8 noundef zeroext %x, i8 noundef zeroext %val) {
  %c = icmp ult i8 %x, 11
  %r = select i1 %c, i8 %val, i8 0
  ret i8 %r
}

; CHECK-LABEL: _sel_mask:
; CHECK-NOT: jr{{[ \t]+}}c
; CHECK-NOT: jr{{[ \t]+}}nc
; CHECK: cp{{[ \t]+}}11
; CHECK: sbc{{[ \t]+}}a, a
; CHECK: and{{[ \t]+}}l
; CHECK: ret
