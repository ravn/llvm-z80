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

let TargetPrefix = "z80" in {  // All intrinsics start with "llvm.z80."

  // Port I/O intrinsics
  // These map directly to Z80 IN and OUT instructions

  // Read a byte from an I/O port
  // i8 @llvm.z80.in(i8 port)
  def int_z80_in : Intrinsic<[llvm_i8_ty], [llvm_i8_ty],
                             [IntrNoMem, IntrHasSideEffects]>;

  // Write a byte to an I/O port
  // void @llvm.z80.out(i8 port, i8 value)
  def int_z80_out : Intrinsic<[], [llvm_i8_ty, llvm_i8_ty],
                              [IntrNoMem, IntrHasSideEffects]>;

  // Halt the CPU (useful for ending programs)
  // void @llvm.z80.halt()
  def int_z80_halt : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;

  // Disable interrupts
  // void @llvm.z80.di()
  def int_z80_di : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;

  // Enable interrupts
  // void @llvm.z80.ei()
  def int_z80_ei : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;

  // No operation (useful for timing loops)
  // void @llvm.z80.nop()
  def int_z80_nop : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;

  // Select interrupt mode 2 (IM 2)
  // void @llvm.z80.im2()
  def int_z80_im2 : Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;

  // Load the interrupt vector register I from A (LD I,A)
  // void @llvm.z80.set_i(i8 val)
  def int_z80_set_i : Intrinsic<[], [llvm_i8_ty], [IntrNoMem, IntrHasSideEffects]>;

  // Fill memory with `count` copies of a K-byte little-endian pattern
  // (ravn/llvm-z80#205).  Emitted by Z80LoopIdiomFill for recognised pattern
  // -fill loops and lowered by the backend to a seed store + forward LDIR
  // (the classic Z80 seed-and-propagate idiom).  This is a *defined* target
  // intrinsic -- unlike the previous overlapping-memcpy representation, which
  // was UB in IR -- and being target-specific it is opaque to generic passes
  // (no InstCombine exploitation, no PreISelIntrinsicLowering loop-expansion).
  // `pattern` is a little-endian integer whose low `k` bytes hold the pattern;
  // its type is a power-of-two container (i8/i16/i32) so the backend never has
  // to store an odd width (e.g. i24 for k==3) -- the real pattern width is the
  // explicit `k` operand (1..4).
  // void @llvm.z80.pattern.fill(ptr dst, iN pattern, i16 k, i16 count)
  // `k` and `count` are passed as ordinary i16 arguments (G_CONSTANT vregs),
  // NOT ImmArg: an ImmArg lowers to a literal MI operand (no .getReg()), which
  // the backend reads via getIConstantVRegSExtVal -- the IR emitter always
  // supplies constants, so a plain vreg arg is both sufficient and uniform.
  def int_z80_pattern_fill
      : Intrinsic<[], [llvm_ptr_ty, llvm_anyint_ty, llvm_i16_ty, llvm_i16_ty],
                  [IntrArgMemOnly, IntrWriteMem, IntrWillReturn,
                   NoCapture<ArgIndex<0>>, WriteOnly<ArgIndex<0>>]>;

}
