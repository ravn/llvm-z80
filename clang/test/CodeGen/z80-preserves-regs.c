// RUN: %clang_cc1 -triple z80 -emit-llvm -o - %s | FileCheck %s
//
// ravn/llvm-z80#131 — verify that __attribute__((z80_preserves_regs("d","e")))
// on a function declaration lowers to a "z80-preserves-regs"="d,e" LLVM IR
// function attribute, which the Z80 backend then uses to narrow the call
// site's RegMask.

extern void sink_de(int b) __attribute__((z80_preserves_regs("d", "e")));
extern void sink_pair(int b) __attribute__((z80_preserves_regs("de")));
extern void sink_mix(int b)
    __attribute__((z80_preserves_regs("d", "e", "hl", "b", "c")));

// CHECK: declare void @sink_de(i16{{.*}}) [[ATTR_DE:#[0-9]+]]
void call_sink_de(int b) { sink_de(b); }

// CHECK: declare void @sink_pair(i16{{.*}}) [[ATTR_DE_PAIR:#[0-9]+]]
void call_sink_pair(int b) { sink_pair(b); }

// CHECK: declare void @sink_mix(i16{{.*}}) [[ATTR_MIX:#[0-9]+]]
void call_sink_mix(int b) { sink_mix(b); }

// CHECK-DAG: attributes [[ATTR_DE]]      = { {{.*}}"z80-preserves-regs"="d,e"
// CHECK-DAG: attributes [[ATTR_DE_PAIR]] = { {{.*}}"z80-preserves-regs"="de"
// CHECK-DAG: attributes [[ATTR_MIX]]     = { {{.*}}"z80-preserves-regs"="d,e,hl,b,c"
