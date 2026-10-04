// RUN: not %clang_cc1 -triple z80 -fdefault-calling-conv=sdcccall0 -fsyntax-only %s 2>&1 | FileCheck %s
// CHECK: error: invalid argument '-fdefault-calling-conv=sdcccall0' not allowed with 'C++'
struct MethodsMustKeepTheirABI {
  int method();
};
