; Byte-array/string data must be emitted one `.byte` per value in the default
; (z88dk z80asm-feeding) asm format, NOT as a `.ascii` string.
;
; Why: z88dk-z80asm parses octal escapes greedily with no 3-digit cap, stopping
; only when the value would exceed 255.  LLVM's `.ascii` escaping emits a byte
; such as 0x04 as the 3-digit octal `\004` and a following printable 0x30 as the
; literal digit `0`, producing `\0040`.  z80asm reads that as octal 040 = 0x20,
; swallowing the `0` and dropping a byte -- the array silently shrinks and every
; later element shifts (sizeof, computed by clang, stays correct so the bug is
; invisible until the data is read).  This corrupted `pieces[]` in the z88dk
; chess demo (rook header read h=0 -> 0 sprite cells -> blank/garbage, then a
; cells[] overflow -> `Bdos Err`).  Emitting per-byte removes the ambiguity and
; matches the sccz80 / zsdcc backends.  Only textual `-S` output is affected;
; the integrated-assembler ELF object path writes raw bytes regardless.
;
; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s

; Bytes 0x11, 0x04, 0x30, 0x22, 0x33.  The 0x04 (needs an octal escape) directly
; followed by 0x30 (the literal digit '0') is exactly the ambiguous pattern.
@t = constant [5 x i8] c"\11\04\30\22\33"

; CHECK-LABEL: _t:
; CHECK-NOT:   .ascii
; CHECK:       .byte

; Danish-charset case: on the RC700's ISO-646 DK national variant the bytes 0x5B
; and 0x5D render as the glyphs 'AE' and 'AA' (where a US-ASCII terminal shows
; '[' and ']').  Both are printable, so `.ascii` would keep them literal -- but
; the middle pair 0x04,0x30 is the same greedy-octal trap: `.ascii "[\0040]"`
; assembles to just 3 bytes 0x5B,0x20,0x5D ("AE space AA"), silently losing the
; 0x04 and 0x30.  Per-byte emission pins all four values, so a string carrying
; Danish text plus a low control byte survives the z88dk assembler intact.
@dk = constant [4 x i8] c"\5B\04\30\5D"

; CHECK-LABEL: _dk:
; CHECK-NOT:   .ascii
; CHECK:       .byte 91
; CHECK-NEXT:  .byte 4
; CHECK-NEXT:  .byte 48
; CHECK-NEXT:  .byte 93
