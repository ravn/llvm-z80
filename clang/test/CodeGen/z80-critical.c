// RUN: %clang_cc1 -triple z80 -emit-llvm -o - %s | FileCheck %s --check-prefix=IR
// RUN: %clang_cc1 -triple z80 -S -o - %s | FileCheck %s --check-prefix=ASM
//
// ravn/llvm-z80#4: __attribute__((z80_critical)) marks a function as a
// critical section.  Clang emits the "z80_critical" IR fn attribute; the Z80
// backend then wraps the body with DI at entry and EI before the return.

extern unsigned char counter;

// IR: define{{.*}}void @atomic_update(){{[^#]*}}#[[A:[0-9]+]]
// IR: attributes #[[A]] = {{.*}}"z80_critical"

// ASM-LABEL: atomic_update:
// ASM: di
// ASM: ei
// ASM-NEXT: ret
__attribute__((z80_critical)) void atomic_update(void) { counter++; }
