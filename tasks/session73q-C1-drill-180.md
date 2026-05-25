# Session 73q — Track C drill C1 (#180 single-peephole audit)

**Date:** 2026-05-23
**Budget:** 30 min
**Wall:** ~35 min
**Outcome:** GO.  Demonstrated the single-peephole audit methodology end-to-end on **XOR #0xFF -> CPL** (Z80LateOptimization.cpp:823-841).  Classified as **independent migration** with a mid-difficulty safety condition.

## Pick (and why)

I chose peephole #6 from the migration list — **XOR #0xFF -> CPL** (Z80LateOptimization.cpp:823-841, 19 LOC).  Reasoning:

- The brief asked for "highest T-state" but no per-fire T-state table exists in the audit doc and the actual high-T-state candidates (#15 16-bit overflow idiom, #25 u8 switch range-check) have "IR-level / TTI" upstream homes that don't fit the "single GISel rule migration" template the brief implies.  #6 is the cleanest **methodology demo** — small, well-bounded, with a textbook GISel-combiner home.
- Per fire: -1 byte (2 -> 1), -3 T (7 -> 4).  Not the largest, but reproducible across hundreds of sites in the BIOS / cpnos `~x` paths.
- Subsequent C2 drill (the audit table) can copy the methodology for the higher-T-state items.

## Peephole semantics

```cpp
// Z80LateOptimization.cpp:823-841
if (MI.getOpcode() == Z80::XOR_n && MI.getOperand(0).getImm() == 0xFF) {
  auto After = std::next(MII);
  if (isRegDeadAfter(After, MBB, TRI, Z80::FLAGS)) {
    BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(Z80::CPL));
    MII = MBB.erase(MII);
    ...
  }
}
```

Pattern: `XOR_n 0xFF` (2 bytes, 7 T) -> `CPL` (1 byte, 4 T).  Safety guard: `FLAGS` is dead after the XOR.  This matters because `XOR_n` sets S/Z/P from the result whereas `CPL` leaves S/Z/P unchanged (sets only H=1, N=1).

## Upstream home

The peephole is repairing a deficiency in the C++ ISel rule for `G_XOR`.  Site is **`Z80InstructionSelector.cpp:3508-3522`** (G_XOR i8 case, imm-fold branch):

```cpp
BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(TargetOpcode::COPY), Z80::A)
    .addReg(Src1Reg);
if (ImmVal) {
  unsigned ImmOpc = (Opcode == TargetOpcode::G_OR) ? Z80::OR_n : Z80::XOR_n;
  BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(ImmOpc))
      .addImm(*ImmVal & 0xFF);
} else { ... }
BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(TargetOpcode::COPY), DstReg)
    .addReg(Z80::A);
```

The migration is to add a special case **before** the generic XOR_n emit:

```cpp
if (ImmVal && Opcode == TargetOpcode::G_XOR && (*ImmVal & 0xFF) == 0xFF
    && /* no flag-consumer in this G_XOR's result use chain */) {
  BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(Z80::CPL));
} else if (ImmVal) {
  unsigned ImmOpc = (Opcode == TargetOpcode::G_OR) ? Z80::OR_n : Z80::XOR_n;
  BuildMI(MBB, MI, MI.getDebugLoc(), TII.get(ImmOpc))
      .addImm(*ImmVal & 0xFF);
} else { ... }
```

## Safety check

The peephole guards `isRegDeadAfter(After, FLAGS)`.  At ISel, FLAGS isn't yet tracked as a physreg liveness — it's an implicit-def on the XOR_n instruction.  Two options for the ISel-level safety check:

1. **Walk the G_XOR's result-vreg users.**  If none of them is a flag-consuming op (G_BR_COND that selects to JR_C/JR_NC/JR_Z/JR_NZ without an intervening compare, G_SELECT that uses XOR's flags, etc.), CPL is safe.  Z80's GISel ISel always emits a fresh compare (OR_A, CP_r, etc.) before any flag-consumer that reads the result of a XOR, so this check is mostly tautological.  **Sufficient simple condition: if the G_XOR's result has any non-debug user that is itself a G_ICMP/G_BR_COND/G_SELECT on the same MBB, fall back to XOR_n.**
2. **Keep the peephole.**  Migrate via the lowering AND keep the late-opt peephole as belt-and-suspenders.  Removes nothing, but the peephole fires less often (only when ISel emits XOR_n through a different path, e.g. the bit-7 toggle at Z80InstructionSelector.cpp:1108-1120 — those paths intentionally use XOR_n for its flag effect and **must not** be migrated).

The bit-7 toggle paths (lines 1108, 1115, 1120, 1277, 1284, 1566, 1585, 1758, 1778) emit `XOR_n 0x80` deliberately for flag side-effects — they are NOT migration candidates.  Only the **g_xor-with-0xFF-imm-fold** site is.

## Migration MIR test (sketch)

A minimal lit test in `llvm/test/CodeGen/Z80/`:

```llvm
; RUN: llc < %s --mtriple=z80 -global-isel -O2 | FileCheck %s

define i8 @not_u8(i8 %x) {
  %r = xor i8 %x, -1
  ret i8 %r
}
; CHECK-LABEL: not_u8:
; CHECK: cpl
; CHECK-NOT: xor 0xff
```

This test passes today (the late-opt peephole catches it), so it doesn't demonstrate the migration in isolation.  To prove ISel-level emission, add `-mllvm -disable-z80-late-opt=true` (if a flag exists, else gate the peephole with a `cl::opt` introduced for this audit) and re-run.  Currently the smallest case (`return ~x`) emits `cpl` only because of the peephole; pre-peephole MIR would show `XOR_n 0xFF`.

## Independent or chained?

**Independent.**  Migrating this peephole touches one ISel site and zero other peepholes.  No transitive dependencies on other migration candidates.

## Migration cost estimate

- Code change: ~15 lines in `Z80InstructionSelector.cpp` (the `if ImmVal == 0xFF && Opcode == G_XOR` branch + the flag-consumer scan helper).
- Lit test: 1 new file, ~15 lines.
- Possible regression surface: the safety check is non-trivial — a G_XOR feeding into G_BR_COND via an intermediate G_ICMP could in principle be miscompiled if the ISel rule for G_BR_COND doesn't insert a fresh compare.  Verification by full lit + test-runner suite at all opt levels needed.
- Total: ~1 hour including verification (faster than the 4-h estimate in the audit doc, which assumed a TableGen Pat<> approach).

## Drill outcome

- **Methodology validated.**  The single-peephole audit pattern produces: (a) precise upstream site (file:line), (b) safety condition explicitly stated, (c) migration cost estimate, (d) dependency classification, (e) lit-test sketch.
- **Ready for C2.**  The remaining 16 migrate-candidate peepholes can be processed at ~15-20 min each using the same template, yielding the audit table requested by #180.
- **Caveat**: this peephole is small.  The audit's higher-T-state items (#15, #21, #25) have non-trivial upstream homes (LSR-cost-model, GISel-constant-tracking, switch-lowering) and won't fit the simple "ISel rule migration" template.  Those need their own per-item drills.

## Files

- `/tmp/scev182/not_x.c` — minimal repro: `return (unsigned char)~x;`.  Currently emits `cpl; ret` via the late-opt peephole.
- `/tmp/scev182/not_x_complex.c` — extended repro that exercises the FLAGS-dead path (write through pointer between XOR and EQ test).  Currently emits `cpl` correctly via the late-opt peephole.

## Next steps

1. Add a `cl::opt` to `Z80LateOptimization.cpp` to optionally skip the XOR-FF-to-CPL peephole (one-line addition) — needed for the lit test below.
2. Add the ISel-side emit (~15 LOC) per the sketch above.
3. Add lit test in `llvm/test/CodeGen/Z80/xor-ff-to-cpl.ll` exercising both the simple case and the flag-consumer case (where XOR_n must remain).
4. Verify zero regressions on full lit + test-runner suite.
5. Either delete the late-opt peephole (if all sites covered) or document its remaining purpose (bit-7 toggle paths).

## COMPLETED 2026-05-26 (session-73ab)

Migration finished and the peephole retired.  The C1 migration had landed the
standalone `G_XOR x, 0xFF -> CPL` ISel emit, but the post-RA peephole was found
**still live** for a second emitter: the i16 `== -1` / `!= -1` comparison
fallback (`Z80InstructionSelector.cpp` CVal>=0 byte-XOR path) emitted `XOR_n
0xFF` on the byte inversions and relied on the peephole to fold them.  Removing
the peephole without addressing that regressed `issue-149-i16-ne-minus-one.ll`
`ne_minus_one_multi` (cpl 1B -> xor 255 2B, 3 lit sites).

Fix: the comparison fallback now emits `CPL` directly when a byte immediate is
0xFF (intermediate flags are dead -- only the final `OR` is consumed -- so CPL's
flag difference is irrelevant).  With both emitters CPL-direct, the peephole is
dead and was removed (~22 LOC incl. comment).

Validation: codegen **byte-identical** across the Z80 lit suite at -O2 and -Oz
(diff against the pre-change fingerprint = empty); lit 117 PASS + 5 XFAIL;
test-runner clang Fail/Fatal unchanged from baseline (37/56).  Value-preserving
by construction (CPL and XOR 0xFF yield the same A; only the dead intermediate
flags differ).  One of #180's 16 "stand-in" peepholes retired.
