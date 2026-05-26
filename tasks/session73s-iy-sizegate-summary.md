# Session 73s cont. — IY size-gate (#38) + verify-clean cluster (2026-05-26)

Continuation of `session73s-198-verifier-summary.md` (#198 fix + verifier-surface
triage).  This half: landed the IX/IY density lever and cleared 2 of the 4
verify-clean classes, fixing two more latent miscompiles along the way.

## Landed (all pushed to main)

### #38 — un-reserve IY under size-opt + static-stack (production byte win)
`z80IsIYAllocatable(MF) = Z80UnreserveIY || (hasOptSize && staticStack)`, threaded
through `getReservedRegs`, `getLargestLegalSuperClass` (GR16NoIR no-widen gate), and
`Z80NarrowNoIndex` so the byte-decompose leak-prevention engages together.  IY is a
4th allocatable pair ONLY at -Os/-Oz +static-stack; reserved for speed (-O2/-O3).
- Measured trade: SIZE win (cpnos 2033->2026, autoload 1483->1478, BIOS 5920->5897,
  ~33 B; AES -145 B), small SPEED cost (AES tstates +0.11%: an IY-held value is read
  via `push iy; pop hl` ~25T vs a 3-byte BSS reload ~16T) -> hence the size-opt gate.
- The win is concentrated in hot register-starved functions: BIOS `_specc` (-11),
  `_bios_reader_body` (-7), `_bg_clear_from` (-5); autoload `_main_relocated` (-11
  raw); cpnos `_scroll_lines` (-10).
- 0 undocumented IY ops; gate precise (aes256 +ss: -O2 = 0 IY operands, -Oz = 65);
  cpnos polypascal MAME boot PASS; autoload boots to CP/M A>.

### BSS-spill->PUSH/POP peephole: SP-write guard
The peephole's safety scan tracked PUSH/POP depth but NOT explicit SP writes
(`LD_SP_HL`, the call-arg-cleanup `ld sp,hl`).  A push/pop bracket spanning an
`ld sp,hl` popped the wrong slot.  Both scan loops now bail on a non-push/pop,
non-call SP def.  Fixed the IY-unreserve test_58 blocker AND a PRE-EXISTING latent
miscompile (test_58_fixed_point -O0 +static-stack, 0x002E->0x003F).  MIR test
`bss-spill-pushpop-sp-write-guard.mir`.  Same family as #198.

### ISel: fresh dst vreg for tied INC16 (i16 ==/!= -1 fast path)
The fused compare-branch built `%t = INC16 %t` (dst == tied-src) -> "Multiple
virtual register defs in SSA form" AND a real miscompile (test_38_sort_search at
O1/O1_ss, 0x0007->0x000F).  Fresh dst vreg (two-address re-ties).  lit test
`inc16-tied-ssa-def-issue.ll`.  Cleared the multiple-vreg-defs verify class.

## Verify-clean cluster status (#197)
4 classes that made -O2 `-verify-machineinstrs` RED, now 2 of 4 cleared:
- DONE: "Illegal virtual register" (#201 chokepoint), "Multiple virtual register
  defs in SSA" (tied INC16 fresh-dst).
- OPEN: **#194** undefined-physreg (74, gf_log-style stale liveins -- DELICATE: blanket
  recompute rejected +2B, targeted = multi-block path recompute); **#200** too-few-
  operands (22, SPILL_GR16 array/offset operand count -- cosmetic, in frame-lowering).
When both clear, the test-runner `-verify` flag can flip to a blocking CI lane.

## Three real miscompiles fixed this session
#198 (MachineCSE -O2, via BSS-spill cross-block guard), test_58_fixed_point
(-O0 +static-stack, via SP-write guard), test_38_sort_search (O1, via INC16
fresh-dst).  Two were pre-existing latent production bugs unmasked while pursuing
the verify-clean goal.

## Issues
Filed: ravn/rc700-gensmedet#100 (autoload make-mame stale clang banner check --
harness bug, boot is fine).  Updated #194/#197/#200.  #198/#201 closed.

## Open / next (tasks/todo.md)
- #194 (liveins) + #200 (SPILL_GR16) -> then flip the `-verify` CI lane.
- IX un-reserve: same size/speed analysis as IY (user "do later").
- O0+static-stack test_54 hang: re-check (IY gate keeps O0 reserved now).
- rc700#100 autoload banner-check fix.
