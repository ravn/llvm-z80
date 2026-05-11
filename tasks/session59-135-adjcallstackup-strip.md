# Session 59 (2026-05-11) — #135 ADJCALLSTACKUP implicit-def strip + #136/#137 filed

## What landed

**ravn/llvm-z80#135 — completes #131 caller-side `z80_preserves_regs`** (commit
`c910320`).  Strips `ADJCALLSTACKUP`'s pessimistic TableGen
`Defs = [SP, HL, A]` implicit-defs for declared-preserved registers that the
predicted expansion path won't actually clobber.  HL and A preservation now
work end-to-end alongside DE (previously only DE worked; HL/A were silently
nullified by the surviving implicit-defs).

### Mechanism

A `stripAdjCallStackUp` lambda in `Z80CallLowering::lowerCall` predicts the
actual clobber based on the adj amount + SM83 mode (mirrors
`adjCallStackUpClobbersReg` in `Z80RegisterInfo.cpp`):

| Condition                | Clobbered |
|---|---|
| `AdjAmount == 0`         | none (ADJCALLSTACKUP erased) |
| `SM83 && AdjAmount ≤ 127`| SP/flags only (ADD SP,e) |
| `AdjAmount ≤ 16`         | A/flags (POP AF × N) |
| `AdjAmount >  16`        | HL/flags (LD HL,n; ADD HL,SP; LD SP,HL) |

Called at both `ADJCALLSTACKUP` emission sites (callee-cleanup early-emit + standard
post-CALL).

### Witness

```llvm
declare void @sink_hl_only() #1
attributes #1 = { "z80-preserves-regs"="h,l" }

define i16 @f_hl(i16 %a, i16 %b) {
  call void @sink_hl_only()
  %s = add i16 %a, %b
  ret i16 %s
}
```

Before:
```asm
_f_hl:
    push hl                    ; <-- redundant: HL is "declared" preserved
    ld (__sfrend_f_hl-4),de
    call _sink_hl_only
    pop hl                     ; <-- redundant
    ld de,(__sfrend_f_hl-4)
    add hl,de
    ex de,hl
    ret
```

After:
```asm
_f_hl:
    ld (__sfrend_f_hl-4),de    ; unrelated-pair DE spill (no attr) still emitted
    call _sink_hl_only
    ld de,(__sfrend_f_hl-4)
    add hl,de
    ex de,hl
    ret
```

The HL push/pop drops completely.  Body is now symmetric with `_f_de` (same
fixture, "d,e" attribute) modulo register swap.

## Verification

- `llvm/test/CodeGen/Z80/preserves-regs-hl-bug.ll`: XFAIL lifted; positive
  CHECKs added (`CHECK-NOT: push hl`, `CHECK-NOT: pop hl` across the call).
  **PASS.**
- Full Z80 lit (94 tests): **92 PASS + 2 XFAIL, 0 regressions.**
- `z80-utils/test-runner` clang suite: 690 PASS / 37 FAIL / 56 FATAL / 207 SKIP.
  All 37 FAILs A/B'd against pre-#135 `llc` — pre-existing O1 noise on
  `test_90/91 edge_*` (see #136), independent of this change.
- cpnos-rom impact: clang+pio-irq production 1906 B unchanged.  HL preservation
  is honest end-to-end now, but no current SNIOS caller routes state through HL
  across `xport_send_byte`, so caller-side savings are 0 B today.  Future
  regalloc improvements (or sources that DO hold HL state) will newly benefit.

## Cross-references

- **#131** — the caller-side base.  Strips CALL_nn's implicit-defs for
  preserved regs.  Worked for DE but not HL/A because of ADJCALLSTACKUP's
  surviving implicit-defs.  #135 is the completing fix.
- **#133** — callee-side honoring (layer 1 shipped session 58, layer 2 + 3
  open).  Independent of #135.
- **rc700-gensmedet `feedback_diff_binaries_before_blaming_codegen.md`** —
  the binary-identity observation from session 58 (full `("d","e","h","l","b","c")`
  set vs `("d","e","b","c")` produced byte-identical cpnos.bin) was the
  inadvertent witness that HL preservation was a no-op.  At the time we attributed
  it to "no caller has HL state to preserve."  Both turned out to be true
  simultaneously: caller-side HL was being silently dropped AND no current source
  exercises it.  #135 fixes the former; the latter is just current source.

## Tasks completed in this session

- #135 fix: implemented, lit fixture green, full lit green, integration green, cpnos rebuild clean
- Session 58's latent SIO correctness gap (rc700-gensmedet#97 Part C) closed in the same day under sessions 59 + 59b — see `rc700-gensmedet/tasks/timeline.md`
- Smoke harness rehabilitation: rc700-gensmedet#43, #96, #98, #99 all closed in 59b
- Memory rules extracted (in `/Users/ravn/z80/memory/`):
  - `feedback_ab_before_blaming_test_runner` — stash + rebuild + rerun on test-runner FAILs
  - `feedback_value_oracle_all_transport_cells` — runtime-test every linking TRANSPORT cell on shared-source changes

## Tasks/issues filed this session

- **ravn/llvm-z80#136** — pre-existing O1 noise on `test_90/91 edge_*` (38 fixtures).
  Independent of #135 but surfaced during verification.  Includes investigation
  comment ruling out the obvious literal-overflow hypothesis.
- **ravn/llvm-z80#137** — test-runner should capture port-1 stdout text alongside
  DE register, so multi-block fixture failures point at the specific failing CHECK
  line.  Motivated by #136 bisection difficulty.

## Next session candidates

- **#133 layer 2** (regalloc allocation-order tweak) — biases callee body register
  selection to prefer declared-preserved regs, so the push/pop introduced by
  #133 layer 1 drops out when the body has flexibility.  Could recover the +2 B
  SIO correctness cost from session 59 and shave PIO similarly.
- **#133 layer 3** — `-Wz80-preserves-regs-violation` clang diagnostic.  Safety
  net for the exact class of bug session 59 caught at runtime (declaration says
  "preserves D", definition body clobbers D, no #133 layer 1 annotation on the
  def).  Frontend Sema work.
- **#132** — cross-MBB BSS-spill→PUSH/POP peephole (needs MachineDominatorTree
  in Z80LateOptimization).  Largest BIOS-side win potential.
- **#137 + #136 follow-through** — fix the test-runner port-1 capture first
  (small Rust change), then bisect the O1 edge_* failures with proper per-line
  diagnostic output.
