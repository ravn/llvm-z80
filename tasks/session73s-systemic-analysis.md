# Systemic analysis — what the session-73s bug cluster reveals (2026-05-25)

A step-back after the session-73s correctness campaign (6 fixes; see
`session73s-correctness-summary.md`).  The bugs were not independent — they rhyme.
This note records the systemic root and the leverage points, so future work targets
the cause rather than the symptoms.

## Single systemic root

**The Z80 backend offloads dataflow-sensitive work into a post-RA peephole layer
that lacks the framework's analysis invariants, and it runs with the correctness
safety-nets switched off, while its feedback loop optimizes for size on a config it
does not test.**  Silent miscompiles at feature interactions are the predictable
emergent result.

## Evidence (the patterns)

1. **Peephole layer carries load that belongs upstream, re-deriving analysis by
   hand and incompletely.**  #14, #192, #193, #195×2, #194 are the same shape: a
   post-RA peephole (`BSS-spill->PUSH/POP`, `COPY16_PUSHPOP` transfer, `#173`)
   mutating instructions, each re-implementing a slice of liveness/aliasing/iterator
   reasoning ad-hoc and getting an edge wrong (indirect reads, loop-carried reloads,
   cross-block, cross-class, dangling iterators, stale live-ins).  These peepholes
   exist because framework infra is missing (no spill cost model, poor IX/IY
   allocation, no working remat #178).  #180: "16 of 38 peepholes are stand-ins for
   missing upstream infrastructure."  The peephole that used the shared sound
   primitive `isRegDeadAfter` was never the buggy one — that is the tell.

2. **Safety-nets off -> violations are silent.**  `-verify-machineinstrs` never run
   (instantly found #194; would have caught #193).  Assertions off in shipping
   builds -> `llvm_unreachable` is a no-op (the `LEA_IX_FI` silent no-op).
   Forcing `+static-stack` silently bypasses AutoStaticStack's recursion guard.

3. **Test matrix does not mirror production.**  `+static-stack` is THE production
   config (all PROM/BIOS/cpnos) yet was never compiled by the suite until the
   `-static-stack` mode — which found 4 bugs in minutes (#192, #195×3).

4. **Bugs cluster at feature x general-shape interactions**, never in straight-line
   code: `+static-stack x loop x i32`, `IX/IY-as-GPR x loop x i32`, `+static-stack
   x recursion`.  The Z80-specific compensations (BSS locals, push/pop spilling,
   IX/IY juggling) are each locally reasonable but their cross-product with the IR
   shape space is uncovered.

5. **(Workflow, meta)** The reliable diagnostic was always differential/instrumented
   (`-print-after-all`/`-stop-after` MIR bisection, lldb, byte-diff, the test-runner
   oracle) — never forward hand-reading of asm (which misled repeatedly).  And the
   pessimism bias held again: "multi-week regalloc" framings (#112) collapsed to
   5-line peephole guards (#14); complexity kept being mis-attributed to the
   framework when it lived in the compensation layer (cf.
   `feedback_dig_deeper_before_parking`).

## Why it is self-reinforcing

The success metric is **code density**; the feedback loop is **size + the
default-config test suite**.  That loop rewards adding peepholes and never looks at
the three places bugs accumulate: the production config, the verifier's invariants,
and peephole dataflow safety.  The density goal drives peephole proliferation while
the correctness scaffolding lags — structurally.  This is a young, density-optimized,
AI-built backend whose **optimization machinery has outrun its verification
machinery**.

## Leverage points (cause, not symptom)

1. **Wire in the tripwires** — `-verify-machineinstrs` + assertions CI lanes
   (**#197**).  Cheap; converts silent miscompiles into caught failures.  (Gate the
   verify lane: backend is not verify-clean yet.)
2. **Make the test matrix mirror production** — `+static-stack`/`-Os` CI lanes
   (#197 + **#196** for mode fidelity).  Highest ROI seen this session.
3. **Drain the peephole layer to where the framework guarantees the analysis** —
   the **#180/#181/#178/#27** agenda is not just upstream-readiness, it is
   bug-class elimination: each dataflow-sensitive peephole removed is a whole
   family of #14/#192/#195 bugs that cannot recur.
4. **For peepholes that must stay, route them through one audited safety helper** —
   `isRegDeadAfter` is the model; a shared "can I eliminate/move this slot/reg?"
   guard handling indirect / loop-carried / cross-block / cross-class would have
   prevented the entire family at once.  See `feedback_peephole_safety_guards`
   (memory rule) + `session73s-late-opt-liveness-audit.md`.

## Recommendation
Items 1-2 (#196, #197) are small test/CI infra and change the trajectory more than
any single codegen fix — do them before the next density push.  Item 3 (#180/#181)
is the durable structural fix and is already on the upstream-readiness path.
