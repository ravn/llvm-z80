# Session 40 summary (2026-05-03)

Branch: `session-40-ix-iy-shadow`.  5 commits on top of main
`e801d8974573` (post Phase 3 milestone).

User direction: *"keep an eye on using the ix and iy registers
too, and investigate if the separate bank can be useful"*.

Two distinct strands; investigation-led, with concrete fixes
landed where root causes were identified.

## Strand A — IX/IY un-reservation re-investigation

Re-investigated #38 by un-reserving IY only and running the
full clang -Os runtime suite.  The session-39 framing ("11
runtime FAILs after #28 fix; deeper regalloc / register-class
issue") was incomplete.  True surface:

| Bucket             | IY reserved | IY un-reserved | Δ           |
|--------------------|------------:|---------------:|------------:|
| Total tests×opt    |         972 |            972 |       —     |
| Pass               |         573 |            286 |     -287    |
| Fail (runtime)     |          93 |             58 |     -35     |
| **Fatal (build)**  |           0 |            422 |   **+422**  |

387 distinct test×opt FATALs share the message `Unsupported
instruction : <MCInst 0>`.  Traced to
`Z80InstrInfo::expandPostRAPseudo` — pseudo expansions call
sub-register-keyed opcode lookups (`getSRLOpcode`, etc.) that
return 0 for `IXH`/`IXL`/`IYH`/`IYL`; the 0 is passed straight
to `BuildMI(..., TII.get(0))`, producing bare opcode-0 MIs
(== `TargetOpcode::PHI` so they print as `PHI`) that the encoder
later rejects.

42 sub-register extraction sites audited.  Only **LSHR16 /
ASHR16** produce bare opcode-0 MIs; other sites either route
through PUSH/POP IR16 fallbacks, use opcode lookup tables that
DO have IXH/IXL/IYH/IYL entries (emitting *valid undocumented*
instructions instead of opcode 0), or have `if (!Op) return
false` guards.

Filed two issues with sequential fixes:

| Issue  | Class                                | Status                           |
|--------|--------------------------------------|----------------------------------|
| #112   | Encoder crash on opcode 0            | Partial fix landed (LSHR16/ASHR16 → GR16NoIR) |
| #113   | Policy violation: undocumented half-ops without `+undocumented` | Partial fix landed (CMP16_FLAGS / CMP16_ULT / CMP16_SBC_FLAGS / SM83_CMP_ZERO16 → GR16NoIR) |
| #115   | Regalloc heuristics: greedy picks IY for HL/DE-tied use sites | Filed; gates IY un-reservation |

The single-register-class technique from session 39 (#94
`BReg`, #99 `BCReg`) is reapplied here as **exclusion**:
`def GR16NoIR : Z80Reg16Class<(add DE, HL, BC)>;` — a TableGen-
level guarantee that regalloc cannot pick IX/IY for the affected
pseudo operands, regardless of whether IX/IY are reserved at the
function level.  Same lever, applied as exclusion class.

With IX/IY currently reserved, all this is no-op for the hot
path: clang lit 84 PASS + 1 XFAIL (was 83+1; +1 from new
`issue-112-gr16noir-lshr.ll`); rcbios bios.cim 5932→5933 B
(+1 B regalloc shuffle in `_bios_write_c`); cpnos.bin
byte-exact in code.

#38 (IY un-reservation) is now actionable but blocked on:

  - **#112** finish — no more sites confirmed via audit, but
    keeping the issue open as a regression guard.
  - **#113** finish — extend GR16NoIR to remaining
    XOR_CMP_*/ZEXT/SEXT_GR8_GR16 pseudos that route through
    PUSH/POP IR16 fallbacks (their IR16 path is intentional
    when `+undocumented` is on, so tightening them is a
    feature-flag-gated decision).
  - **#115** — regalloc heuristic fixes (HLReg/DEReg single-
    register exclusion classes for LDIR/LDDR/EX DE,HL and
    similar HL/DE-required use sites).

When all three close, removing `Reserved.set(Z80::IY)` becomes
a size-or-equal change at last.

## Strand B — shadow bank as a second register set

Surveyed current infrastructure:

  - `+shadow-regs` feature flag wires up EXX-based ISR
    save/restore (`Z80FrameLowering.cpp:152-167, 393-412`).
  - Shadow register classes (`AFp`/`BCp`/`DEp`/`HLp`/
    `ShadowGR16`) defined in TableGen but `ShadowGR16` is
    **never referenced** in any `.cpp` — allocatable but
    unreachable.  No vreg ever lands in the shadow bank during
    normal codegen.
  - #102 closure rationale: EXX swaps all three of BC/DE/HL
    atomically, ruling out shadow-as-spill-target.  But it
    does NOT rule out **bracketing a no-CALL inner loop**.

Walked rcbios + cpnos-rom disassemblies.  rcbios has 4 viable
candidates for the EXX-bracket niche (`_specc`, `_scroll`,
`_cursor_left`, `_bios_conin`); cpnos-rom has none significant
(too small).  Spot-check of `_specc` 0xde19-0xde3c shows 8
B/iteration of pure BC spill traffic via `__sfrend_xyadd+0x4`
that an EXX bracket would eliminate — 2 B fixed cost, 8
B/iteration saved over many outer iterations.

Threshold met (3+ candidates with > 3 B win each per the
strand-B acceptance criterion); prototype is justified.

Filed as **#114** with full design (new MIR pass
`Z80ShadowBankBracket`, ~150 LOC, runs after RA, walks
`MachineLoopInfo`, verifies BC/DE/HL liveness preconditions,
inserts EXX brackets).  Implementation deferred to Phase 5
work in `roadmap-to-maturity.md`.

## Files added / modified

New target source classes:

  - `def GR16NoIR : Z80Reg16Class<(add DE, HL, BC)>;` in
    `Z80RegisterInfo.td` (#112).

Pseudo operand-class tightening (`Z80InstrInfo.td`,
`SM83InstrInfo.td`):

  - `LSHR16` / `ASHR16` → GR16NoIR (#112).
  - `CMP16_FLAGS` / `CMP16_ULT` / `CMP16_SBC_FLAGS` →
    GR16NoIR (#113).
  - `SM83_CMP_ZERO16` → GR16NoIR (#113).

In-source documentation update:

  - `Z80RegisterInfo::getReservedRegs` comment block updated
    to point at #112 + remove obsolete "deeper regalloc" framing.

New test:

  - `llvm/test/CodeGen/Z80/issue-112-gr16noir-lshr.ll` —
    regression guard against widening LSHR16/ASHR16 operand
    class back to GR16.

Documentation:

  - `tasks/session40-plan.md`
  - `tasks/session40-strand-a-iy-unreserve.md`
  - `tasks/session40-strand-b-shadow-bank.md`
  - `tasks/session40-summary.md` (this file)

## State at end of session

| Metric                          | Value                                  |
|---------------------------------|----------------------------------------|
| lit                             | 84 PASS + 1 XFAIL                      |
| Per-function size baseline      | zero deltas vs post-Phase-3 baseline   |
| **rcbios bios.cim**             | **5933 B** (was 5932 — regalloc shuffle, +1 B) |
| **cpnos.bin**                   | **byte-exact in code** (date-stamp diff only) |
| clang test runner (-O2/-O0)     | unchanged from session 39 baseline     |

## Issues count

Entering session 40: 22 open.  At end: 25 open (3 newly filed:
#112, #113, #114, #115; -1 because #112 has partial fix landed
but stays open until full audit closes it).

Filed:
  - #112 (encoder crash on IY-half opcode-0)
  - #113 (policy: undocumented half-ops without +undocumented)
  - #114 (shadow-bank EXX-bracket pass for hot no-CALL inner loops)
  - #115 (regalloc heuristics gap: greedy picks IY for HL/DE-tied uses)

## Roadmap progress

  - Phase 1 (foundation): closed earlier sessions.
  - Phase 2 (correctness): complete (session 39).
  - Phase 3 (regalloc Cluster A): complete-modulo-parked
    (session 39).
  - Phase 4 (Cluster B — spill mechanism: #100, #20, #96, #16):
    not started.  Strand B's `_specc` BC-spill finding is a
    direct concrete instance of this cluster's surface.
  - Phase 5 (shadow bank): investigation done; survey confirms
    prototype is justified; #114 filed; implementation deferred.
  - Phases 6-9: not started.

## Recommended next session entry point

Two equally-good options.  User pick:

**Option A — Phase 4 / Cluster B work.**  #100, #20, #96, #16
spill-mechanism cluster.  Direct rcbios/cpnos byte wins.
Strand B's `_specc` 8 B/iter BC spill is one concrete
instance.  Adjacent to Phase 3's recently-closed #74 BSS-
spill-to-PUSH/POP peephole — extends that work.

**Option B — Shadow-bank prototype (#114).**  Phase 5.  Bigger
research-shaped task; ~150 LOC new MIR pass; gated by
`+shadow-regs` feature.  Strand B survey already identifies the
4 BIOS candidate loops.

A is more likely to land bytes; B is more likely to teach us
about the Z80 backend's deeper architecture.  Both are
unblocked by session 40's work.

Phase 3 follow-ups (#111 HLReg, #115 IY heuristics, full #112/
#113 close) are smaller and can land opportunistically when
related work touches their files.
