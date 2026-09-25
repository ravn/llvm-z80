// RUN: %clang_cc1 -triple z80 -S -mllvm -z80-asm-format=elf -mllvm -z80-split-quad-directive -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple z80 -S -mllvm -z80-asm-format=elf -o - %s | FileCheck %s --check-prefix=DEFAULT
//
// Some downstream assemblers (e.g. z88dk's z80asm) have no 8-byte directive
// and silently truncate .quad to 4 bytes. -z80-split-quad-directive emits
// two .long halves instead. This test confirms C -> asm produces .long/.long
// with the flag and .quad without it.
//
// ELF asm format used here: GNU-as-compatible assemblers understand .quad
// natively; only non-GNU assemblers need the split.

unsigned long long g_big = 0x4008000000000000ULL;
// CHECK-LABEL: _g_big:
// CHECK:       .long 0
// CHECK-NEXT:  .long 1074266112
// CHECK-NOT:   .quad
//
// DEFAULT-LABEL: _g_big:
// DEFAULT:       .quad 4613937818241073152
// DEFAULT-NOT:   .long
