//===-- Z80Combine.td - Describe Z80 combiner -------------*- tablegen -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

// Pre-legalization: use all standard combines.
def Z80PreLegalizerCombiner : GICombiner<
  "Z80PreLegalizerCombinerImpl", [all_combines]>;

// Custom rule: canonicalize cross-size COPYs to G_TRUNC/G_ANYEXT.
// The legalizer's narrowScalar can produce COPYs between different-sized
// virtual registers (e.g. s16 -> s8). ISel requires G_TRUNC/G_ANYEXT instead.
def z80_cross_size_copy_matchdata : GIDefMatchData<"unsigned">;
def z80_cross_size_copy : GICombineRule<
  (defs root:$dst, z80_cross_size_copy_matchdata:$matchinfo),
  (match (COPY $dst, $src):$mi,
         [{ return matchCrossSizeCopy(*${mi}, MRI, ${matchinfo}); }]),
  (apply [{ applyCrossSizeCopy(*${mi}, ${matchinfo}); }])>;

// Custom rule: G_SEXT (G_ICMP ...) -> G_ANYEXT (G_ICMP ...).
// The Z80 lowering of G_ICMP already produces a full-width i8 mask in A
// (via the `add a,$ff; sbc a,a` sequence).  Sign-extending the i1 result
// to i8 via the canonical `(shl 7; ashr 7)` idiom would then add a
// redundant 5-instruction tail.  Rewriting G_SEXT to G_ANYEXT lets the
// downstream cast_of_cast / identity_combines absorb the no-op widen.
// See ravn/llvm-z80#79 and #120.
def z80_sext_from_icmp : GICombineRule<
  (defs root:$dst),
  (match (G_SEXT $dst, $src):$mi,
         [{ return matchSextFromIcmp(*${mi}, MRI); }]),
  (apply [{ applySextFromIcmp(*${mi}); }])>;

// Post-legalization: selective combines that preserve legality, plus
// Z80-specific cross-size COPY canonicalization.
def Z80PostLegalizerCombiner : GICombiner<
  "Z80PostLegalizerCombinerImpl",
  [copy_prop, cast_of_cast_combines, identity_combines, merge_combines,
   commute_constant_to_rhs, z80_cross_size_copy, z80_sext_from_icmp]>;
