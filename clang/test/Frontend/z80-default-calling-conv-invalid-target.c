// RUN: not %clang_cc1 -triple x86_64-unknown-linux-gnu -fdefault-calling-conv=sdcccall0 -fsyntax-only %s 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -triple sm83 -fdefault-calling-conv=sdcccall0 -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix=SM83
// CHECK: error: invalid argument '-fdefault-calling-conv=sdcccall0' not allowed with 'x86_64-unknown-linux-gnu'
// SM83: error: invalid argument '-fdefault-calling-conv=sdcccall0' not allowed with 'sm83'
int invalid_target;
