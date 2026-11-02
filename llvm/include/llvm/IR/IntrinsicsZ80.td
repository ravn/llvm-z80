//===- IntrinsicsZ80.td - Defines Z80 intrinsics -----------*- tablegen -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the Z80-specific intrinsics.
//
//===----------------------------------------------------------------------===//

// None of these instructions can transfer control into code of the module:
// IN/OUT/NOP/HALT/DI/EI do not call anything, and interrupts are asynchronous
// (the "interrupt" function attribute models them as separate contexts).
// IntrNoCallback says so, which keeps the call graph from treating the
// declaration as "could call anything" (CallGraph.cpp adds an edge to
// CallsExternalNode for every declaration without nocallback), and so lets
// Z80NonReentrant keep static frames in functions that use them.
//
// IntrNoMem + IntrHasSideEffects is deliberately kept: it yields
// memory(unknown) in IR, so these stay full compiler barriers (no CSE,
// hoisting or reordering around volatile accesses).  willreturn is left off
// HALT/DI/EI (di; halt never returns; DI/EI synchronise with interrupts).
let TargetPrefix = "z80" in {  // All intrinsics start with "llvm.z80."

  // Port I/O intrinsics
  // These map directly to Z80 IN and OUT instructions.  The port number is
  // 8 bits; what is driven on the upper address lines A15-A8 is unspecified
  // (a constant port selects IN A,(n)/OUT (n),A, which put A there; a
  // variable port selects IN r,(C)/OUT (C),r, which put B there).

  // Read a byte from an I/O port
  // i8 @llvm.z80.in(i8 port)
  def int_z80_in : Intrinsic<[llvm_i8_ty], [llvm_i8_ty],
                             [IntrNoMem, IntrHasSideEffects, IntrNoCallback,
                              IntrWillReturn]>;

  // Write a byte to an I/O port
  // void @llvm.z80.out(i8 port, i8 value)
  def int_z80_out : Intrinsic<[], [llvm_i8_ty, llvm_i8_ty],
                              [IntrNoMem, IntrHasSideEffects, IntrNoCallback,
                               IntrWillReturn]>;

  // Halt the CPU (useful for ending programs)
  // void @llvm.z80.halt()
  def int_z80_halt : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects,
                                        IntrNoCallback]>;

  // Disable interrupts
  // void @llvm.z80.di()
  def int_z80_di : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects,
                                      IntrNoCallback]>;

  // Enable interrupts
  // void @llvm.z80.ei()
  def int_z80_ei : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects,
                                      IntrNoCallback]>;

  // No operation (useful for timing loops)
  // void @llvm.z80.nop()
  def int_z80_nop : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects,
                                       IntrNoCallback, IntrWillReturn]>;

}
