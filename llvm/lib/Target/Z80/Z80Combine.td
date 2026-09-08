//===-- Z80Combine.td - Describe Z80 combiner -------------*- tablegen -*-===//
//
// Part of LLVM-Z80, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

// Custom rule: wide scalar load+store pair -> block move.
// narrowScalar would split an i32/i64 copy into per-limb loads/stores that
// each re-derive their address ((HL) has no displacement mode) — measured
// 65 instructions for an i64 copy.  Rewriting the pair as G_MEMMOVE
// (load-completes-before-store semantics == memmove) lets the custom
// G_MEMMOVE lowering emit LDIR/LDDR or a _memmove libcall instead.
def z80_wide_copy_matchdata : GIDefMatchData<"MachineInstr *">;
def z80_wide_copy_to_block_move : GICombineRule<
  (defs root:$root, z80_wide_copy_matchdata:$matchinfo),
  (match (G_STORE $val, $ptr):$root,
         [{ return matchWideCopyToBlockMove(*${root}, MRI, ${matchinfo}); }]),
  (apply [{ applyWideCopyToBlockMove(*${root}, MRI, B, ${matchinfo}); }])>;

// Pre-legalization: all standard combines + Z80 wide-copy block move.
def Z80PreLegalizerCombiner : GICombiner<
  "Z80PreLegalizerCombinerImpl", [all_combines, z80_wide_copy_to_block_move]>;

// Post-legalization: selective combines that preserve legality.
def Z80PostLegalizerCombiner : GICombiner<
  "Z80PostLegalizerCombinerImpl",
  [copy_prop, cast_of_cast_combines, identity_combines, merge_combines,
   commute_constant_to_rhs, redundant_and]>;
