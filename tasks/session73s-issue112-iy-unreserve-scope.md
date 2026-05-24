# Session 73s — #112 (un-reserve IX/IY): scoping drill + live IY probe

**Date:** 2026-05-24
**Issue:** ravn/llvm-z80#112 / #38 (IX/IY reserved; un-reserving gated, per CLAUDE.md, on "Phase 3 regalloc cost-model work").
**Outcome:** The blocker is **relocated, not what the framing says**. The session-40 "387 FATAL opcode-0 encoder crashes" are **fixed** — un-reserving IY now passes the lit suite clean with **zero** new failures. But a **runtime miscompile remains**: IY-allocatable AES runs away (no termination) instead of computing. The **−33 B / −1.5%** size prize is real and sits behind that one remaining bug.

## Why IX/IY are reserved (documented in Z80RegisterInfo.cpp:256)

Session 40 found un-reserving IY produced ~387 `Unsupported instruction : <MCInst 0>` FATALs. Root cause was the *encoder*, not regalloc: pseudo expansions that decompose a 16-bit operand into 8-bit halves call sub-register-keyed opcode helpers (`getSRLOpcode`, `getRROpcode`, `getSUBOpcode`, ...) that **return 0** for `IXH/IXL/IYH/IYL`; that 0 reaches `BuildMI(..., get(0))` and the encoder rejects the bare opcode-0 MI.

## The encoder blocker is already fixed (GR16NoIR / GR16_BCDE discipline)

Audited all ~25 expansion sites that decompose operands via the 8 sub-register
helpers (`Z80InstrInfo.cpp` lines 1232–1760). **Every one already constrains its
decomposed operand to an IX/IY-excluding class:**
- the `ADD/SUB HL,rr` + SM83 byte-by-byte family use `GR16_BCDE` (BC/DE only);
- `LSHR16`/`ASHR16` use `GR16NoIR` (`.td` comment at 1600 explains: `getSRLOpcode(IYH)`/`getRROpcode(IYL)` return 0);
- `CMP16_*` / `XOR_CMP_Z16` use `GR16NoIR`.

So the allocator never places IX/IY where an 8-bit decomposition would look up
opcode 0. **Live confirmation:** with IY un-reserved, the Z80 lit suite is
**110 PASS + 5 XFAIL — identical to baseline, no FATALs.** The session-40 crash
class is gone (those guards were added after session 40).

## But there is a runtime miscompile (the real remaining #112 work)

Probe: `getenv("Z80_UNRESERVE_IY_EXPERIMENT")`-gated un-reserve of IY only (IX left
reserved), built clang+llc, ran the AES corpus runtime oracle (`make clang.ram`,
z88dk-ticks, PASS sentinels at ram[0xC010]=enc, [0xC021]=dec, [0xC022]=0xA5):

| build | AES09 .text | lit | AES runtime |
|---|---|---|---|
| IY reserved (baseline) | 2228 B | 110+5 | **PASS**, 11 516 046 tstates |
| IY allocatable | **2195 B (−33 / −1.5%)** | 110+5 | **FAIL** — runaway, 300 000 006 tstates, sentinels never written |

No undocumented half-register instructions leak (undocumented is off, so `IXH/IXL/IYH/IYL`
stay reserved; IY is used only as a full pair — `grep -ic 'ixh|ixl|iyh|iyl'` on the
IY-on AES asm = 0). So the miscompile is **not** an undocumented-instruction issue.
The runaway (no termination, sentinels zero) points to a control-flow / loop-counter or
pointer value held in IY being clobbered — consistent with the parked CLAUDE.md #14
("PUSH/POP for IY copies crashes when IY is allocatable — 'y' screen crash", a latent
regalloc/copy bug exposed by IY allocation and code-layout change).

## Reframing #112 and the next step

CLAUDE.md / #38 frame this as "Phase 3 regalloc cost-model work." That is now stale:
1. The **encoder opcode-0 crash is FIXED** (operand-class discipline).
2. What remains is a **single runtime IY-allocation miscompile** (likely a COPY/spill
   pattern, cf. #14), worth a focused isolation drill — NOT an open-ended cost-model
   project.
3. The payoff is concrete: **−33 B / −1.5% on AES09** from one extra allocatable pair,
   which also relieves the 3-pair pressure behind #27 / #110 / #115 and the `ADD16_acc`
   HL-pinning regression (#178).

**Next-session plan:** reduce the IY-allocatable AES miscompile to the smallest failing
function (the corpus is one computation; bisect by function via `-ffunction-sections` +
selective IY-reservation, or build small loop/pointer probes that force IY allocation).
Then `-print-after-all` around regalloc + `Z80ExpandPseudo` to find the clobbered IY
value. With that fixed, un-reserve IY (then evaluate IX separately), and validate the
**full** oracle (AES 13/13, test-runner 990, cpnos/autoload/BIOS sizes + MAME boots —
the #14 crash was a MAME runtime bug, so production boots are mandatory before trusting
the size win).

## Isolation: the IY miscompile is systemic (test-runner A/B)

Ran the full clang integration suite (z80-utils/test-runner, 990 cases via
z88dk-ticks) IY-off vs IY-on and diffed:

| run | Pass | Fail | Fatal | Skip |
|---|---|---|---|---|
| IY reserved (baseline) | 690 | 37 | 56 | 207 |
| IY allocatable | 622 | 85 | 76 | 207 |

**70 tests regress PASS -> FAIL/FATAL purely from IY allocation.** Excluding the
known fuzz noise (`test_90_edge_*` / `test_91_edge_prom_*`, #136), the regressing
**named functional** tests are clean minimal repros:

| repro (base name) | FAIL opt-levels | FATAL |
|---|---|---|
| `test_28_pointer_arith` | 3 | — |
| `test_26_array_basics` | 4 | — |
| `test_27_array_2d` | 3 | — |
| `test_35_memory_ops` | 3 | — |
| `test_36_stack_pressure` | 5 | — |
| `test_04_i32_bitwise` | 5 | — |
| `test_41_integer_boundary` | 5 | — |
| `test_40_hash_crc` | 2 | — |
| `test_31_struct_ops` | 1 | — |
| `test_58_fixed_point` | 1 | — |
| `test_33_string_ops` | 5 | 1 |
| `test_38_sort_search` | 3 | 1 |
| `test_48_dynamic_alloca` | — | (fatal) |

The breadth (pointer arith, arrays, struct/string ops, stack pressure, i32 bitwise —
anything that puts a 4th live 16-bit value in play) shows this is **one systemic
IY-allocation codegen bug**, not a per-test edge case. Both wrong-answer (FAIL) and
crash/trap (FATAL) modes occur. Most likely location: IY copy/spill/reload lowering
(`copyPhysReg` -> `COPY16_PUSHPOP`, or `Z80ExpandPseudo` SPILL/RELOAD for IY), exercised
whenever IY holds a live value across a clobber — consistent with the parked #14
('y'-screen-crash) framing.

**Fix-session entry point:** `test_28_pointer_arith` (smallest, FAIL not FATAL). Reduce
to the failing opt level, build IY-on, diff IY-on vs IY-off MIR with `-print-after-all`
around regalloc + `Z80ExpandPseudo`, find the clobbered IY value. The fix is shared
across all 70 regressions (systemic), so one root-cause likely clears the whole set.
Then re-run this A/B to confirm zero IY-caused regressions before un-reserving for real,
followed by the full production oracle (AES 13/13, cpnos/autoload/BIOS + MAME boots).

## Root characterization: IY read before written (def dropped)

Drilled the smallest repro `test_28_pointer_arith` (IY-on returns `status=0x0B`,
bit 2 = 0x04 missing -> the "array of 3 pointers summed in a loop" block is the one
that miscompiles; O0/O1/Oz fail, O2/O3/Os pass). Reduced to a 10-line function
(`t28b2.c`: `int16_t *ptrs[3]={&a,&b,&c}; for i in 0..3: sum += *ptrs[i]`) and diffed
IY-off vs IY-on asm at `-Oz`. The IY-on output has IY references in this program order:

```
push iy        ; setup: stores a pointer value into ptrs[]  <-- READS IY
ld   iy,3      ; <-- FIRST WRITE to IY (the loop counter)
.LBB0_1:
push iy        ; loop condition test (reads counter)
dec  iy
```

**`push iy` reads IY before any `ld iy,...` writes it — IY is read uninitialized.**
The allocator placed a pointer-address value in IY but **its defining instruction was
dropped**; only the copy-OUT (`push iy; pop hl`, the COPY16_PUSHPOP lowering of
`%hl = COPY %iy`) survived. Reading stale/caller IY -> wrong pointer stored to `ptrs[]`
-> `*ptrs[i]` reads garbage -> `sum != 600` -> bit 2 clears. This is exactly the parked
**#14** class ("PUSH/POP for IY copies + code-layout change trigger a latent regalloc
bug"): the COPY-out-of-IY survives while the COPY-into-IY **def** is lost.

**Hypothesis for the fix session:** a coalescing or `COPY16_PUSHPOP` expansion path
elides/mis-orders the def when the source of a `%iy = COPY %x` (or the def of an
IY-allocated vreg) is itself dead/rematerialized — leaving a use with no reaching def.
Trace with `-print-after-all` on `t28b2.c -Oz` IY-on, watching the IY vreg's def from
`finalize-isel` through coalescing, two-address, regalloc, and `Z80ExpandPseudo`; find
the pass that drops it. Because the failure is systemic (70 tests), one fix should clear
the set. Re-run the test-runner A/B to confirm zero IY-caused regressions, then the full
production oracle (AES 13/13, cpnos/autoload/BIOS + MAME boots) before un-reserving.

## FIX LANDED: LEA_IX_FI IY case (dominant cause, 63/70 cleared)

Root cause of the dominant miscompile: `LEA_IX_FI`'s `eliminateFrameIndex`
(Z80RegisterInfo.cpp) handled dst = HL/DE/BC/IX but had **no IY case** — it fell
to `llvm_unreachable`, which in a **Release build (assertions off, our default)** is
`__builtin_unreachable()`, a no-op. The pseudo was erased emitting **nothing**, so IY
was never defined; the next `push iy` (spill) read garbage. (In test_28: the
`LEA_IX_FI %stack.0` that should load `&a` into IY produced no instruction → wrong
pointer stored in `ptrs[]` → `*ptrs[i]` garbage → sum≠600.)

Fix (commit `27be55e2569d`): added IY cases to all three `LEA_IX_FI` branches
(SP-relative, IX-based offset==0, IX-based large-offset), mirroring the IX path
(`PUSH HL`/`PUSH IX`; `POP IY`). Added a hidden bring-up flag `-z80-unreserve-iy`
(default OFF; replaces the throwaway env probe) and a lit guard `lea-fi-iy-112.ll`.

Result (test-runner clang suite):

| run | Pass | Fail | Fatal |
|---|---|---|---|
| baseline (IY reserved) | 690 | 37 | 56 |
| IY-on, before fix | 622 | 85 | 76 |
| **IY-on, after LEA_IX_FI fix** | **684** | **42** | **57** |

`test_28_pointer_arith` now PASSes all 6 opt levels. Production unchanged
(flag off, IY reserved): AES09 .text 2228 B byte-identical, AES runtime PASS, lit 111+5.

## Residual (6 tests): i32 decomposition emits undocumented IYH/IYL

The remaining ~6 regressions (`test_04_i32_bitwise` O1/O2/O3/Os, `test_40_hash_crc`
O2/O3) are a **separate, narrower bug**: with IY allocatable, a 16-bit chunk of a 32-bit
value gets allocated to **IY**, then `G_UNMERGE_VALUES` extracts its 8-bit halves — which
are physically **IYH/IYL**, undocumented. The 8-bit `G_XOR`/`G_AND`/`G_OR` path
(`Z80InstructionSelector.cpp:3454+`) then emits `xor iyh` / `xor iyl` / `ld a,iyh`
(confirmed in `test_04` asm, `-O2`, IY-on). Undocumented is OFF, so these must not be
emitted (`feedback_no_undocumented_default`). There is no 32-bit reg class that includes
IY (only `Fakei32`); the path is i32 -> 16-bit chunks -> `G_UNMERGE` -> 8-bit ops, and the
chunk landed in IY.

**The residual is PERVASIVE, not a single site (session 73s drill).** First attempt:
constrain the `G_UNMERGE_VALUES` selection source (`Z80InstructionSelector.cpp:5146`) to
`GR16NoIR` when `!hasUndocumented`. Built it — **test_04 still had 26 undocumented refs**
(`xor iyh`×12, `xor iyl`×12, `ld a,iyh/iyl`). Reverted (earned nothing). Reason: the 8-bit
ALU paths (`G_AND`/`G_OR`/`G_XOR` at lines 3314/3473) **fold a `G_UNMERGE` operand by
extracting the sub-register of its source directly**, bypassing the `G_UNMERGE_VALUES`
selection. And there are **40+ `sub_lo`/`sub_hi` extraction sites** in the selector, any of
which yields `IYH/IYL` when its GR16 source is allocated to IY.

This is the crux: **IY's byte halves are undocumented**, and byte-decomposition of 16-bit
values is pervasive across the backend — which is *why* IY was reserved. So "un-reserve IY"
splits into two regimes:

1. **Pure 16-bit values never byte-accessed** (pointers, addresses, `ADD IY,rr`, 16-bit
   load/store) — safe with IY today. The `LEA_IX_FI` fix above enables exactly this class,
   and that is where most of the −33 B AES pressure relief originates.
2. **Byte-decomposed values** (i32 chunks, 16-bit logic done byte-wise, byte extraction) —
   **cannot** use IY without `+undocumented`, because the access *is* `IYH/IYL`.

Realistic completion paths for #112 (a strategic choice, not a quick patch):
- **(A) Enable `+undocumented` in production.** The user's real Z80 and MAME both execute
  `IYH/IYL` correctly; the only barrier is the rule `feedback_no_undocumented_default` (don't
  emit undocumented *without* the flag). Flipping it makes IY fully allocatable and legal —
  the cheapest path to the win; needs user sign-off on the undocumented-ops policy.
- **(B) Keep `!undocumented` and prevent IY for byte-accessed values.** Needs a regalloc-level
  constraint (a vreg with any sub-register use must not get IX/IY) or guarding all 40+
  extraction sites with `GR16NoIR` — large and fragile.

**Recommendation:** the `LEA_IX_FI` fix is shipped; take #112 to the user as the (A)-vs-(B)
decision. The −33 B AES win + 3-pair-pressure relief is real either way; (A) is far cheaper.
In all cases, run the full production oracle (AES 13/13, cpnos/autoload/BIOS sizes + MAME
boots — #14 was a MAME runtime crash, so boots are mandatory) before un-reserving for real.

## (B) attempted and REVERTED: class-narrowing is insufficient AND the residual is deeper

User chose path (B) (keep `!undocumented`, constrain IY off byte-accessed values).
Built `Z80ConstrainByteAccess`, a pre-RA pass (addPreRegAlloc, all opt levels) that:
(1) narrows any vreg with a sub-register (byte) operand to `GR16NoIR`; (2) enforces
each instruction's *declared* operand register class when it excludes IX/IY (the
`CMP16`/`XOR_CMP` pseudos declare `GR16NoIR` but BuildMI doesn't auto-constrain, so
it was unenforced); (3) narrows `REG_SEQUENCE`/`INSERT_SUBREG` destinations.  Gated to
no-op unless IX/IY un-reserved and `!undocumented` (production byte-identical).

Result (full test-runner, IY-on):
- It **did** remove the undocumented half-register ops: `test_04`/`test_40` `-O2`
  asm went from 26 `iyh`/`iyl` refs to **0**.
- But the tests **still FAIL** (`test_04` O1/O2/O3/Os, `test_40` O2/O3 — same as before),
  and it **newly broke** `test_30_linked_list` (O1/O2/O3/Os).  Net **10 regressions vs
  baseline 690/37/56** (680/47/56) — *worse* than the LEA-only path's 6.

**This corrects the earlier diagnosis.** The undocumented `IYH/IYL` were a *co-symptom*,
not the cause.  With byte access made documented (0 undoc refs), `test_04`/`test_40` still
compute the wrong value -> the real residual is a **wrong-value regalloc miscompile when IY
holds byte-decomposed (i32-chunk) values**, and the narrowing additionally *triggers* the
same failure class on a pointer test (`test_30`).  This is the **same RegisterCoalescer
out-of-class behaviour as #178** (assigns / mishandles registers against a narrowed class).
Class-narrowing at the vreg level does not make the coalescer/regalloc honour the
constraint reliably.

Reverted the pass entirely (net-harmful, gated-off infra that doesn't fix its target).
The `LEA_IX_FI` fix and `-z80-unreserve-iy` flag stay (committed, correct, 63/70).

**ticks DOES execute undocumented IXH/IYH (correction).** I checked the emulator:
`src/ticks/cpu.h:40  #define canixh() (c_cpu & (CPU_Z80|CPU_Z80N|CPU_R800|CPU_EZ80))`,
and ticks.c substitutes H/L -> IYH/IYL for the FD-prefixed half-register ops gated on
`canixh()`.  So on a Z80 target ticks runs `xor iyh` etc. **correctly**.  This *reverses*
an earlier worry and sharpens the conclusion:

- The IY-on test failures are **real wrong-value miscompiles**, not an emulation gap (ticks
  executes the ops faithfully, yet the computed value is wrong).
- Therefore **(A) `+undocumented` would NOT fix the residual** either: it emits
  correctly-executing `xor iyh`, but the underlying value is genuinely wrong.  (A) only
  removes the *policy* objection to the half-reg ops; it does not fix the miscompile.
- (B) class-narrowing removed the undoc ops (0 refs) but the value is *still* wrong, and it
  broke a pointer test (test_30).  So **narrow 16-bit classes under IX/IY-allocatable
  pressure trigger regalloc miscompiles** — the same regalloc-correctness family as #178's
  out-of-class assignment.

**Revised recommendation:** the IY-allocation residual is a genuine **regalloc/coalescer
correctness bug** (manifests as out-of-class assignment in #178 and as wrong-value
miscompiles under narrowed classes here).  Neither (A) nor (B)-narrowing fixes it.  The real
fix is a focused **register-allocator investigation that needs an assertions-enabled LLVM
build** (`-debug-only=regalloc`, `-verify-machineinstrs`) — the current Release build has
assertions OFF, so the coalescer/greedy internals can't be traced.  That, plus disk headroom
(was 91% used), are prerequisites for the next attempt.  The shipped `LEA_IX_FI` fix stays
valuable for the eventual un-reserve (pure-pointer IY values, which do NOT hit this bug).

## Verification / hygiene
- Throwaway probe (env-gated un-reserve) fully reverted; `Z80RegisterInfo.cpp` diff empty.
- Baseline rebuilt and re-verified: AES `make clang.ram` **PASS** (11 516 046 tstates), lit 110+5.
- No source files left uncommitted.

## Files touched
- None (read-only audit + reverted throwaway probe). Evidence: this writeup.
