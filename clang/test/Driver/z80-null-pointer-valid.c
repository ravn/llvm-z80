// ravn/rc700-gensmedet#49: z80/sm83 are bare-metal targets where address 0 is
// real, addressable memory (CP/M zero page, RST vectors, hardware registers).
// The Z80 toolchain therefore defaults to -fno-delete-null-pointer-checks so
// the optimizer does not delete stores to / through address 0 as UB (which
// silently removed whole functions doing zero-page init).  An explicit
// -fdelete-null-pointer-checks opts back out.

// RUN: %clang -### --target=z80  -c %s 2>&1 | FileCheck %s
// RUN: %clang -### --target=sm83 -c %s 2>&1 | FileCheck %s
// CHECK: "-fno-delete-null-pointer-checks"

// RUN: %clang -### --target=z80 -fdelete-null-pointer-checks -c %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=OPTOUT
// OPTOUT-NOT: "-fno-delete-null-pointer-checks"
