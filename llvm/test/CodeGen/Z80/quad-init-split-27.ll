; RUN: llc -mtriple=z80 -z80-asm-format=elf -z80-split-quad-directive < %s | FileCheck %s --check-prefix=SPLIT
; RUN: llc -mtriple=z80 -z80-asm-format=elf < %s | FileCheck %s --check-prefix=DEFAULT

; Some downstream assemblers for the ELF/GNU textual path (e.g. z88dk's
; z80asm, DEFQ/DQ = 4 bytes) have no 8-byte directive and truncate `.quad`.
; -z80-split-quad-directive emits two .long halves instead; without it,
; .quad is unchanged (a real GNU-as-compatible assembler handles it fine).
;
; C source reproducing the bug on such an assembler:
;   unsigned long long g_big = 0x4008000000000000ULL;
; truncates to a 4-byte store, so g_big reads back as 0 at runtime.

; 0x4008000000000000 -> low .long 0, high .long 0x40080000 (1074266112)
@g_big = global i64 4613937818241073152
; SPLIT-LABEL: _g_big:
; SPLIT-NEXT: .long 0
; SPLIT-NEXT: .long 1074266112
; DEFAULT-LABEL: _g_big:
; DEFAULT-NEXT: .quad 4613937818241073152

; 0x0000000100000000 -> low .long 0, high .long 1
@g_mid = global i64 4294967296
; SPLIT-LABEL: _g_mid:
; SPLIT-NEXT: .long 0
; SPLIT-NEXT: .long 1
; DEFAULT-LABEL: _g_mid:
; DEFAULT-NEXT: .quad 4294967296

; -1 -> both halves 0xFFFFFFFF (4294967295)
@g_neg = global i64 -1
; SPLIT-LABEL: _g_neg:
; SPLIT-NEXT: .long 4294967295
; SPLIT-NEXT: .long 4294967295
; DEFAULT-LABEL: _g_neg:
; DEFAULT-NEXT: .quad -1

; 0x1122334455667788 -> low 0x55667788 (1432778632), high 0x11223344 (287454020)
@g_both = global i64 1234605616436508552
; SPLIT-LABEL: _g_both:
; SPLIT-NEXT: .long 1432778632
; SPLIT-NEXT: .long 287454020
; DEFAULT-LABEL: _g_both:
; DEFAULT-NEXT: .quad 1234605616436508552
