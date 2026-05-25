# Session 73s — codegen-correctness campaign summary (2026-05-24/25)

(Continues `session73s-summary.md`, which covered the earlier #180/C2 peephole-deletion
audit.  This file covers the correctness campaign that followed.)

## Arc
Began on the #178/#112 deep-regalloc thread; pivoted into a sustained
**codegen-correctness** campaign that shipped **6 compiler fixes**, built a
bug-finding test mode, audited the worst-offending pass, and mapped the
production-config blind spot.

## Analysis — the recurring root cause
Five of the six bugs are the **same family**: a **post-RA peephole in
`Z80LateOptimization.cpp` mutates instructions (erase / move / convert) without a
complete safety check** — missing a liveness guard, a slot-aliasing guard, or
iterator safety.  The pass otherwise guards correctly (systematic `isRegDeadAfter`,
sound dataflow), so these were the ad-hoc exceptions.  The **`COPY16_PUSHPOP`
IX/IY-transfer** and **BSS-spill→PUSH/POP** peepholes accounted for four of them.

## Fixes shipped (all on `session-73s`, pushed)
| # | Bug | Fix |
|---|-----|-----|
| **#14** | loop-carried IY copy dropped (IX/IY transfer peephole, no liveness guard) | `dfa073a` computeRegisterLiveness guard |
| **#191** | objdump/readelf can't auto-detect Z80 ELFs (no `EM_Z80` in `getArch`) | `afb6662` map EM_Z80→z80 |
| **#192** | `+static-stack -Os` i32 `select+shift+xor` loop miscompiles (#173 peephole relocated `LD r,A` into a region reading `r`) | `2565620` read-in-interval guard |
| **#193** | `Z80LateOptimization` **segfault** on 16-bit DE BSS spill (dangling `--MII`) | `25fa914` anchor to inserted PUSH |
| **#195a** | `+static-stack` i32 loop hang — BSS-spill→PUSH/POP dropped a **loop-carried** slot's store-back (top-of-loop reload missed) | `c2bbe80` before-store same-block read check |
| **#195b** | `+static-stack` 2D-array sum off-by-one — BSS-spill→PUSH/POP dropped a store to an **address-taken** slot (indirect read missed) | `02e56e8` skip address-taken frame slots |

Each landed with the full value oracle: AES corpus byte-identical (or measured),
cpnos PROM1 size + **polypascal MAME boot**, Z80 lit, test-runner.  Net cpnos:
2028→2029 B (+1 B, within the ~19 B margin under the 2 KB hard cap).

## New capability
**`cargo run -- clang -static-stack`** test mode (`2d6ccd4`) — first time the suite
exercises the production `+static-stack` config.  Found 4 of the 6 bugs above.

## Audit
`session73s-late-opt-liveness-audit.md`: all ~50 peepholes classified.  The pass
systematically uses the sound shared primitive `isRegDeadAfter`; the fixed bugs
were ad-hoc exceptions; siblings (cross-class/cross-MBB BSS-spill, in/cross-block
#60, BSS load-forwarding, LDIR-aftermath) audited clean.  The verifier cross-check
surfaced #194.

## Open items (filed, characterized)
- **#189** IY-unreserve gate: split-i32-in-IY regalloc miscompile (cost-model work).
- **#190** IY-unreserve: dynamic alloca FATAL (frame-pointer class).
- **#194** cross-block #60 leaves stale live-ins (verify-machineinstrs); benign at
  runtime; `fullyRecomputeLiveIns` fix costs +2 B cpnos and doesn't achieve
  verify-clean (PEI etc. also stale) — **deferred** pending a surgical live-in
  update or a coordinated verify-clean effort.
- **#195** production `+static-stack` triage: 4 fixed; **test_36** = recursion
  artifact (AutoStaticStack excludes in production); **test_48** = alloca-incompatible;
  **test_01/04/15/28/54** = O0-only (production uses -Os).
- **#196** `-static-stack` test mode over-forces vs production (flags recursion tests
  as false bugs) — harness fidelity.

## Recommended next (priority)
1. **#189 IY-unreserve** — strategic prize (4th register pair; density goal).  Needs
   the i32-split regalloc cost model; repros ready (`test_166/167/168`).
2. **#196** harness fidelity + a `-verify-machineinstrs` CI lane (would have caught
   #193, surfaces #194) — the lane is gated on the broader verify-clean effort.
3. **#194** surgical byte-neutral live-in update in cross-block #60.
4. **O0-only `+static-stack` bugs** (#195) — only if O0 builds matter.

## Lesson (memory-rule candidate)
New peepholes that erase/move/convert instructions must use the shared
`isRegDeadAfter` + "anchor resumption to an un-erased instruction" patterns, and
must account for **indirect (pointer) reads of a slot** and **loop-carried reloads**
— not just direct, after-the-store, same-register-class accesses.

## Side observation (not filed; unconfirmed)
cpnos PROM1 size fluxed 2027↔2028 across rebuilds during rapid iteration.  `llc` is
run-to-run deterministic (5/5 identical on AES), and a comment-only `.cpp` change
should give an identical release `.o`, so this was most likely build-state artifacts
from the fast iterate/revert cycle, **not** confirmed nondeterminism.  Re-check (and
file) only if it recurs on clean builds.
