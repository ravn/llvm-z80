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

## Verification / hygiene
- Throwaway probe (env-gated un-reserve) fully reverted; `Z80RegisterInfo.cpp` diff empty.
- Baseline rebuilt and re-verified: AES `make clang.ram` **PASS** (11 516 046 tstates), lit 110+5.
- No source files left uncommitted.

## Files touched
- None (read-only audit + reverted throwaway probe). Evidence: this writeup.
