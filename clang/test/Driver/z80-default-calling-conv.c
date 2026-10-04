// RUN: %clang -target z80 -fdefault-calling-conv=sdcccall0 -### -c %s 2>&1 | FileCheck %s
// CHECK: "-fdefault-calling-conv=sdcccall0"
int driver_option;
