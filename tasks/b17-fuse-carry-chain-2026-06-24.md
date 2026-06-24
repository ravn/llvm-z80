# B17 fixed — multi-byte carry threaded in the flag (Z80FuseCarryChain)

Date: 2026-06-24.  Closes known-suboptimal-codegen.md **B17**.

## The bug

Multi-byte (i32/i64) add/sub selected the inter-limb carry as a *materialized
GR8 register value*: the producer captured the carry flag into A as a 0/1
(`SBC A,A; AND 1`) and the consumer restored it (`LD A,r; RRCA`) before the
next add.  On a CPU with a native carry flag this is pure waste — SDCC and the
in-tree AVR backend emit `add hl,de; adc hl,bc`, keeping the carry in CF.

Witnessed in the dcc-corpus three-compiler comparison: `triangle` 14, `fact`
10, `e` 2 `sbc a,a`; zsdcc 0.  Dominant driver of the 1.5–2.4× int-arith raw
gap vs zsdcc.

Root cause (analysis, no fix proposed at the time, per
`feedback_file_bugs_not_fixes`): ISel models the carry as GR8
(`Z80InstructionSelector.cpp` G_UADDO/G_UADDE/G_USUBO/G_USUBE selection ->
`ADD_HL_rr_CO` / `ADC_HL_rr_CIO` / `SUB_HL_rr_BO` / `SBC_HL_rr_BIO`), and the
pseudo expansion (`Z80InstrInfo.cpp:1591/1638/...`) emits the capture/restore
round-trip.  Design is original to the backend (commit `31997a65c57f`, zlfn
2026-03-12); never the subject of a register-vs-flag evaluation.  AVR
cross-check: same post-legalizer IR (`G_UADDO`->`G_UADDE`) lowers to the
flag-resident chain — so the gap is the Z80 expansion choice, not the mid-end.
Classified NOT-A-BUG-upstream / Z80-backend cost gap.

## The fix

New pass **`Z80FuseCarryChain`** (`llvm/lib/Target/Z80/Z80FuseCarryChain.cpp`),
runs post-RA at the end of `addPostRewrite` (after `Z80FixupImplicitDefs`,
before `ExpandPostRAPseudos`, while the carry pseudos are still intact and the
carry register / dead-ness are concrete).

It recognises a *complete* carry chain — `ADD_HL_rr_CO -> ADC_HL_rr_CIO…`
(or the SUB/SBC analogue) — where every link is flag-safe (nothing between a
producer and its consumer defines FLAGS or touches A; carry single-use) and
the **terminal carry-out is dead**, then rewrites to the flag-resident real
instructions:

```
ADD_HL_rr_CO  -> ADD_HL_BC/ADD_HL_DE            (sets CF, no capture)
ADC_HL_rr_CIO -> ADC_HL_BC/ADC_HL_DE            (reads + sets CF, no A)
SUB_HL_rr_BO  -> AND_A; SBC_HL_BC/SBC_HL_DE     (clear CF, then sub)
SBC_HL_rr_BIO -> SBC_HL_BC/SBC_HL_DE            (reads + sets CF, no A)
```

No new pseudos, no TD changes.  Conservative by construction: any non-flag-safe
link, observed terminal carry, or live terminal FLAGS leaves the whole chain on
the existing register-carry expansion.  SM83 is a no-op (16-bit ADC/SBC HL,rr
are Z80-only).  Gated by `-z80-fuse-carry-chain` (default ON) for no-op control.

## Validation

- **Codegen:** `add32` `sbc a,a` 2->0 (`add hl,de; … ; adc hl,bc`); i64 add
  4->0.  add32 −5 instr (40->35); i64 add −11 instr (95->84).  No-op control
  (`-z80-fuse-carry-chain=false`) restores the round-trip byte-for-byte.
- **lit:** `CodeGen/Z80/` 173 PASS + 6 XFAIL, 0 FAIL.  New
  `fuse-carry-chain.ll` (add32/sub32/add64 positives, `addov` negative =
  observed carry stays unfused, `add32g` with DBG_VALUEs for the -g skip path,
  CTRL no-op-control prefix).  `issue-197-sbc-a-a-undef.ll` pinned with fusion
  disabled (still covers the SBC A,A undef-marking on the register-carry path).
- **runtime (value oracle):** full `cargo run -- clang` = 1128 total / 872 PASS
  / 0 FAIL / 0 FATAL / 256 SKIP.  New `test_224_carry_chain.c` (8 i32/i64
  add/sub carry-propagation cases) PASS at O0–Oz (DE=0x00FF).
- **production no-regression (no-op control):** BIOS byte-identical 5462 B
  ON/OFF; autoload byte-identical 1945 raw / 1481 ZX0 ON/OFF.  Production
  firmware contains no fusible multi-byte arithmetic chains, so the artifacts
  are unchanged -> MAME behaviour unchanged (no re-run needed).

The win lands in arithmetic-heavy code (the dcc corpus), not systems firmware.

## Files

- `llvm/lib/Target/Z80/Z80FuseCarryChain.{cpp,h}` (new)
- `llvm/lib/Target/Z80/CMakeLists.txt`, `Z80TargetMachine.cpp` (plumbing)
- `llvm/test/CodeGen/Z80/fuse-carry-chain.ll` (new)
- `llvm/test/CodeGen/Z80/issue-197-sbc-a-a-undef.ll` (pin fusion off)
- `z80-utils/test-runner/testcases/clang/test_224_carry_chain.c` (new)

Rules-checked: feedback_file_bugs_not_fixes (analysis first, fix is ours/Z80),
feedback_no_op_control_measurement, feedback_revalidate_historical_compiler_claims,
feedback_thorough_tests_for_upstream_bugs, feedback_no_commit_first_version,
feedback_dont_kill_ninja, feedback_peephole_test_with_g.
