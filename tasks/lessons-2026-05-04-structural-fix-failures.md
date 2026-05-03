# Lessons from session 42's three failed structural-fix attempts

**Date:** 2026-05-04 (drawn from session 42, 2026-05-03)
**Audience:** future sessions touching Z80 backend cost models,
combiners, or peephole-retirement work.

## What this doc is

Session 42 attempted three structural fixes to advance the
"underlying datastructures should reflect Z80 properties, not fix
bad modelling with peephole optimizations" principle.  All three
were directionally plausible, all three regressed real workloads
or produced wrong code, all three were reverted.  This doc names
the common pattern so the next session does not retry the same
shape of mistake.

## The three attempts

### Attempt 1 — #89 Path 1: drop `isAsCheapAsAMove` from `LD_r16_nn`

**Premise:** the `LD rr,nn` pseudo's `isAsCheapAsAMove = true`
flag is the structural lever — claiming a 3-byte instruction is
"cheap as a 1-byte move" is the lie that biases regalloc and
blocks MachineLICM hoisting.

**Reality:** the flag is consumed by **two passes that need
opposite answers on Z80**.  MachineLICM (`MachineLICM.cpp:1191`)
wants `false` so it hoists the instruction out of loops.
RegisterCoalescer remat (`RegisterCoalescer.cpp:1316`) wants
`true` so it can fold the cheap def into copy users to shorten
live ranges and reduce spill pressure.  A single boolean cannot
encode this disjunction.

**Measured cost:** BIOS +15 B / cpnos-rom +20 B across 9
regression sites and 2 improvement sites.  See
`tasks/issue-89-investigation-2026-05-03.md` Path 1 section.

### Attempt 2 — #89 Path 2: loop-depth check in RegisterCoalescer

**Premise:** rematerializing into a strictly hotter (deeper-loop)
block always loses; loop depth is the missing gate.
RegisterCoalescer already has `MachineLoopInfo`
(`RegisterCoalescer.cpp:135`); a one-liner suffices.

**Reality:** loop depth is a proxy for *frequency*, but on Z80's
3-pair register file the *register pressure* term in the cost
dominates the *frequency* term.  Blocking the in-loop remat
forces the def to live across the loop, which increases pressure
inside the loop, which forces a *worse* spill (typically a
16-bit pointer to BSS).  RegisterCoalescer has loop info but not
register-pressure info.

**Measured cost:** BIOS +3 B / cpnos-rom +4 B across 7
regression sites and 2 improvement sites — 5x smaller blast
radius than Attempt 1, but still net negative.  Two variants
(`UseDepth > DefDepth` vs `DefDepth == 0 && UseDepth > 0`) gave
byte-identical results because every regression site already
matched the depth-0-to-N pattern.  See same investigation doc,
Path 2 section.

### Attempt 3 — #120 GISel combiner for `(shl 7; ashr 7)` from G_ICMP

**Premise:** the canonical `sext i1 → i8` shift idiom is an
identity on Z80 because the G_ICMP lowering physically leaves a
full mask in A (via `add a,$ff; sbc a,a`).  A GISel combiner
that elides the shift round-trip would let peephole #26 retire.

**Reality:** at the GISel layer **only IR contracts are in
scope**, not target-specific lowering invariants.  Z80's
`BooleanContents` is `ZeroOrOneBooleanContent`
(`Z80ISelLowering.cpp:49`) — the G_ICMP s8 result is
contractually `0x01`/`0x00` (low bit only, high bits zero) by IR
contract, regardless of what asm sequence the instruction
selector picks at lowering time.  The `(shl 7; ashr 7)` pair IS
a meaningful widen at the GISel layer; eliding it propagates
`0x01` instead of `0xFF`.

**Measured cost:** silently incorrect for any consumer that
needs the full mask — e.g. `kbstat = (kbtail != kbhead) ? 0xFF
: 0x00;` in `rcbios-in-c/bios.c:936` would store `0x01` instead
of `0xFF`.  Lit suite passed (90/90) and even helped the
synthetic regress less, masking the bug.  See
`tasks/issue-120-combiner-scoping-2026-05-03.md` "Session 42
attempted implementation" section.

## Common pattern: layer-mismatch

Every attempt pushed the fix to a higher abstraction layer than
where the necessary context lives.

| Attempt | Layer chosen      | Context the layer doesn't have |
|---------|-------------------|--------------------------------|
| #89 P1  | TableGen flag     | "Which pass is asking?"        |
| #89 P2  | Generic CodeGen   | Register pressure              |
| #120    | GISel combiner    | Post-ISel physical-register invariants |

The peephole layer succeeds for these patterns precisely because
it operates on **post-ISel physical-register state with full
visibility into the chosen lowering**.  That is irreplaceable
context for a constrained target like Z80.

## Sharpening the "structural fixes over peepholes" principle

The principle is sound but needs a discriminator.  A peephole is
the **wrong** layer if it's:

  - Re-deriving information available earlier in the pipeline
    (loop info, reg pressure, IR types).
  - Catching cases that should never have been emitted because a
    cleaner upstream pass would have prevented them.

A peephole is the **RIGHT** layer if it's:

  - Exploiting a target-specific physical-register invariant
    created by the chosen lowering (e.g., "after `SBC A,A` on
    Z80, A holds a full mask").
  - Pattern-rewriting on actual asm sequences where the cost
    model is fully resolved and target-specific.

The session 37 audit's "Migrate" / "Delete" classification did
not distinguish these two cases.  Several entries currently
classified Migrate or Delete are probably in the second category
and cannot be migrated without a target-specific intermediate
analysis pass.  See the cross-reference update in
`tasks/late-opt-audit-2026-05-02.md` for the reclassification of
#26.

## HARD RULE (user, 2026-05-04)

**Stop committing the first version of a structural fix the
moment lit + size are clean.**

For combiner / ISel / lowering / regalloc changes that could
affect emitted instructions, lit + matching baseline byte counts
are a *size oracle*, not a *value oracle*.  Run the value oracle
(`cargo run -- clang` from `z80-utils/test-runner/`, plus `make
mame` for BIOS-touching changes) BEFORE the commit.

A combiner+peephole composition that produces byte-identical
baseline output is a **red flag** for "the peephole is covering
for a broken combiner", not a green light.  Verify the combiner
independently (disable the peephole and re-run the value oracle)
before treating the size match as correctness evidence.

This rule is also recorded as a memory under
`feedback_no_commit_first_version.md` (the user's auto-memory).

## Process changes

The four practices below would have caught all three session-42
failures earlier or prevented them entirely.  Apply them on every
regalloc-area or combiner-area change going forward.  Process
rule 3 is the one elevated to HARD RULE above.

### 1. Pre-write MIR dump on a real-workload function

**Rule:** before writing a structural fix, dump the actual MIR on
at least one real-workload function the fix is supposed to
improve.  Verify the IR shape matches your premise.

**Procedure** (concrete recipe — adapt the function name and
target):

```bash
# Identify a real-workload function the fix should improve.
# (Use the per-function size baseline diff, or a bug repro.)

# For a rcbios function (e.g. _bios_conin):
cd /Users/ravn/z80/rc700-gensmedet/rcbios-in-c
/Users/ravn/z80/llvm-z80/build-macos/bin/clang \
    --target=z80 -Oz -g -nostdlib -ffreestanding -std=c23 \
    -ffunction-sections -fdata-sections \
    -Xclang -target-feature -Xclang +static-stack \
    -mllvm -disable-lsr -Iclang -I. \
    -DMSIZE=56 -DCBIOS=0xDA00 -DBIOSAD=0xDA00 \
    -DKBLANG_DANISH=1 -DKBDPORT_A=1 \
    -mllvm -print-after=PASS_NAME \
    -mllvm -filter-print-funcs=FUNCTION_NAME \
    -c bios.c -o /tmp/bios-mir.o 2>/tmp/bios-mir.log

# Common PASS_NAME values:
#   z80-postlegalizer-combiner   (our combiner pass)
#   register-coalescer
#   greedy                       (regalloc)
#   z80-late-opt
# Use -print-before-all / -print-after-all to find the right name
# the first time.

# For a cpnos-rom function: substitute the cpnos CFLAGS block.
# For an arbitrary lit-test reproducer: use llc directly.
grep -E "G_SEXT|G_ICMP|G_ASHR|G_SHL|G_AND|...the-opcodes-you-care-about"  \
    /tmp/bios-mir.log | head -20
```

**What this catches:** in session 42 #120, a pre-write dump of
`bios_conin` would have shown `%79:_(s8) = G_ICMP intpred(ne)`
followed by `G_SHL %79, 7` — and a 5-second sanity check of "is
G_ICMP's s8 result a full mask under Z80's BooleanContents?"
would have answered "no, ZeroOrOne, IR contract is low-bit-only"
before any code was written.

### 2. Layer-context check before writing the fix

**Rule:** before writing a structural fix, explicitly answer the
single question "does this layer have the context I need?"  If
the answer involves "well, the value is *physically* X even
though the contract says Y", you're at the wrong layer.

**Three sub-questions tied to common target layers:**

  - **TableGen flag changes:** enumerate every LLVM pass that
    queries this flag.  Do they all want the same answer?  If
    not, the flag is the wrong knob.  (Use `grep -rn 'isAsCheapAsAMove\|isReMaterializable\|isMoveImm' llvm/lib/CodeGen/`
    or the flag in question.)
  - **Generic CodeGen heuristics:** is the necessary signal
    available without target hooks?  If you find yourself wanting
    `MachineRegisterInfo` plus `MachineLoopInfo` plus
    `RegisterPressure` plus a target-specific cost table, you're
    likely outgrowing the generic layer.
  - **GISel combiners:** is the rewrite sound under IR contracts
    *alone*, or does it depend on the current lowering's physical
    invariants?  Check `setBooleanContents`, `LegalizerInfo`'s
    legalization actions for the relevant ops, and the IR-level
    type the combiner sees.

### 3. Distinguish size oracles from value oracles — and use both

**Rule:** any change that touches register allocation, register
coalescer, instruction selection, or the GISel combiner MUST be
measured against rcbios + cpnos-rom before believing the fix
works.  Lit tests are weak coverage.  But measurement comes in
TWO independent flavours and **size oracles do not detect value
miscompiles**.

**Size oracle (catches size regressions, NOT value miscompiles):**
just builds the binary and reads the resulting byte count.

```bash
# Baseline + after-change byte counts:
cd /Users/ravn/z80/rc700-gensmedet/rcbios-in-c && rm -f builddate.h \
  && make COMPILER=clang | grep BIOS

cd /Users/ravn/z80/rc700-gensmedet/cpnos-rom \
  && rm -rf clang/*.o clang/*.elf clang/cpnos_buildinfo.h \
            clang/transport_stamp \
  && make all | grep "non-padding"

# Per-function diff:
cd /Users/ravn/z80/llvm-z80 && python3 tasks/size-baseline.py check
```

**Value oracle (REQUIRED for combiner / ISel / lowering changes
that could change values, not just costs):**

```bash
# Run the C test-runner suite -- exercises real value semantics
# via `expect: 0xFF` style assertions:
cd /Users/ravn/z80/llvm-z80/z80-utils/test-runner
cargo run -- clang                 # default Os suite
# cargo run -- clang -opt O1       # add other opt levels if relevant
# cargo run -- clang <pattern>     # filter to specific tests

# Boot rcbios in MAME (catches anything the C suite missed in the
# BIOS path; per feedback_screenshot_to_verify):
cd /Users/ravn/z80/rc700-gensmedet/rcbios-in-c && make mame
```

**Why this distinction matters:**

  - In session 42 attempts 1 and 2 (#89 paths) the failure mode
    was size regression.  The size oracle alone caught both.
  - In session 42 attempt 3 (#120 combiner) the failure mode was
    a silent VALUE miscompile that left bytes UNCHANGED with the
    peephole still active (because the peephole deleted the
    wrong code the combiner produced).  **The size oracle missed
    it.**  The bug was caught only by post-hoc MIR inspection
    while investigating the residual gap when the peephole was
    disabled.  Had `kbstat` been compared against `== 0xFF`
    anywhere, this would have been a runtime regression in MAME.

**Failure mode rule of thumb:**

  - Cost-model changes (regalloc heuristics, scheduling, LICM
    biases) → size oracle is sufficient.
  - Anything that changes which instructions are emitted for a
    given IR (ISel patterns, GISel combiners, legalizer changes,
    BooleanContents, calling conventions) → **value oracle is
    required** in addition.  Always run the test-runner suite
    AND boot in MAME.

**Honesty note:** the test-runner suite and MAME boot have always
been part of the project workflow.  Session 42 attempt 3 used
rcbios + cpnos-rom only as a size oracle and committed a value-
miscompiling combiner because the bytes happened to match.  Don't
repeat that — the value oracle is one extra command.

### 4. For peephole-retirement: profile fire sites first

**Rule:** before writing a "delete peephole #N" migration, profile
which IR shapes feed each fire site.  Categorize by "redundant-
with-better-upstream-work" (migratable) vs "exploits-post-ISel-
invariant" (not migratable, peephole is the right home).

**Procedure:**

```bash
# Build the compiler in Debug mode (LLVM_ENABLE_ASSERTIONS=ON,
# CMAKE_BUILD_TYPE=Debug or RelWithDebInfo).  Release builds
# strip LLVM_DEBUG, so -debug-only is silent.

# Compile rcbios with the peephole's debug-channel enabled:
/path/to/debug/clang ... -mllvm -debug-only=z80-late-opt \
    -c bios.c -o /tmp/bios.o 2>/tmp/peephole-fires.log

grep "#NN: " /tmp/peephole-fires.log     # adjust to peephole's tag

# For each fire site, identify the source line and dump the
# pre-peephole MIR with `-print-before=z80-late-opt
# -filter-print-funcs=NAME`.  Categorize:
#   - "Could a GISel combiner have prevented this IR shape?" → migratable.
#   - "Does the rewrite depend on a physical-register invariant
#     post-ISel?" → not migratable; peephole IS the right home.
```

**What this catches:** session 42 #120 first tried the canonical
`G_SEXT (G_ICMP)` shape (which the lit test exercises), then the
post-legalization `G_ASHR (G_SHL X, 7), 7` shape (which more
real workloads exercise).  Profiling fire sites first would have
shown a third shape — `(x != y) ? 0xFF : 0x00` mask consumers —
that exposed the BooleanContents soundness issue immediately.

## What changes in next-session planning

  - **Park #120.**  Three migration paths remain open
    (post-ISel combiner, split G_ICMP lowering, change
    BooleanContents target-wide), all multi-session.  Don't
    revisit until the regalloc cluster is done; revisit with
    fuller context.
  - **Re-classify late-opt audit entries** under the new
    discriminator.  Done as a separate update to
    `tasks/late-opt-audit-2026-05-02.md` (this commit).
  - **Apply the four process rules above on every regalloc-area
    or combiner-area change going forward.**  Especially the
    pre-write MIR dump and the rcbios+cpnos-rom measurement.

## See also

  - `tasks/issue-89-investigation-2026-05-03.md` — Path 1 + Path 2
    full empirical results.
  - `tasks/issue-120-combiner-scoping-2026-05-03.md` — combiner
    attempt full empirical results + soundness analysis.
  - `tasks/late-opt-audit-2026-05-02.md` — original peephole
    classification, updated with the discriminator note.
  - `Z80LateOptimization.cpp:2670` (peephole #26) — source comment
    records the negative result for future readers.
