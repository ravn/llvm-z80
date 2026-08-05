; RUN: llc -mtriple=z80 -z80-asm-format=elf < %s | FileCheck %s

; ravn/z88dk#27: on the ELF/GNU textual path (what z88dk's -compiler=llvmz80
; bridge consumes), a 64-bit global initializer must NOT be emitted as a single
; `.quad`. The Z80 has no native 8-byte data directive and the downstream
; assembler (z88dk z80asm, whose DEFQ/DQ are 4 bytes) would truncate it to 32
; bits. With Data64bitsDirective left null, MCAsmStreamer splits each 8-byte
; value into two little-endian 4-byte `.long` emissions, which round-trips
; losslessly. These CHECK lines fail on the pre-fix (`.quad`) output.

; CHECK-NOT: .quad

; 0x4008000000000000 -> low .long 0, high .long 0x40080000 (1074266112)
@g_big = global i64 4613937818241073152
; CHECK-LABEL: _g_big:
; CHECK-NEXT: .long 0
; CHECK-NEXT: .long 1074266112

; 0x0000000100000000 -> low .long 0, high .long 1
@g_mid = global i64 4294967296
; CHECK-LABEL: _g_mid:
; CHECK-NEXT: .long 0
; CHECK-NEXT: .long 1

; -1 -> both halves 0xFFFFFFFF (4294967295)
@g_neg = global i64 -1
; CHECK-LABEL: _g_neg:
; CHECK-NEXT: .long 4294967295
; CHECK-NEXT: .long 4294967295

; 0x1122334455667788 -> low 0x55667788 (1432778632), high 0x11223344 (287454020)
@g_both = global i64 1234605616436508552
; CHECK-LABEL: _g_both:
; CHECK-NEXT: .long 1432778632
; CHECK-NEXT: .long 287454020
