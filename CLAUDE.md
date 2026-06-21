# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LLVM-Z80 is an LLVM fork adding a backend for the Zilog Z80 CPU family and SM83 (Game Boy). It is experimental, AI-assisted in development, and not production-ready. The backend uses **GlobalISel** (not SelectionDAG) for instruction selection.

## Build Commands

```bash
# Configure and build (Release by default)
cmake -C clang/cmake/caches/Z80.cmake -G Ninja -S llvm -B build
ninja -C build

# Build a specific tool only
ninja -C build llc
ninja -C build clang
```

The cache file (`clang/cmake/caches/Z80.cmake`) disables all targets except Z80 (experimental) and enables only the clang project. Runtime libraries are built to `build/lib/z80/` and `build/lib/sm83/`.

## Testing

### LLVM Lit Tests
```bash
# Run Z80 codegen tests
build/bin/llvm-lit llvm/test/CodeGen/Z80/
# Run a single lit test
build/bin/llvm-lit llvm/test/CodeGen/Z80/add.ll
```

### z80-utils Test Suite (primary integration tests)
Run from `z80-utils/test-runner/`. Requires `z88dk-ticks` emulator on PATH.

```bash
cd z80-utils/test-runner
cargo run                                # Default suites (O1, O2, Os)
cargo run -- -full                       # All opt levels (O0-Oz)
cargo run -- -opt O1                     # Single opt level
cargo run -- clang                       # Clang C suite only
cargo run -- clang -target sm83          # SM83 target
cargo run -- clang -fast-math            # With -ffast-math
cargo run -- clang -static-stack         # With +static-stack (BSS locals) -- exposes ravn/llvm-z80#192 class
cargo run -- clang add                   # Filter by test name pattern
cargo run -- sdcc                        # SDCC cross-build compatibility
cargo run -- llc                         # LLC (LLVM IR) suite
cargo run -- utils                       # elf2rel/rel2elf converter tests
cargo run -- bench                       # Code size benchmark (Clang vs SDCC)
```

Test cases live in `z80-utils/test-runner/testcases/{clang,llc,sdcc,custom}/`. Tests use metadata directives: `/* expect: 0x00FF */` for expected return values, `/* SKIP-IF: sm83 */` to skip targets.

The `BUILD_DIR` env var overrides the default build directory (`../build`).

### What CI gates (lit + runtime suite)
`.github/workflows/z80-ci.yml` has two jobs:
- **`build-and-lit`** — builds clang/llc/lld and runs the lit suites (`CodeGen/Z80`,
  `MC/Z80`; cost-model `Analysis/CostModel/Z80` runs via the codegen target deps).
- **`runtime-tests`** — builds `z88dk-ticks` (from a `z88dk/z88dk` checkout) + the
  Z80 runtime, then runs `cargo run -- clang` (the runtime/value oracle).

**Discipline for compiler changes:**
- The CI-gated proof of any behavior change is a **lit test** that pins the generated
  instruction sequence (FileCheck) — add one even when the bug is most naturally shown
  at runtime. lit is the fast, deterministic, always-on gate.
- A **test-runner runtime fixture** (`testcases/clang/*.c` with `/* expect 0xNNNN */`)
  is the right *additional* proof when correctness is only observable at runtime
  (e.g. an over-run sentinel). These are auto-discovered (glob, no manifest) and now
  CI-gated by `runtime-tests` — but they are slower and emulator-dependent, so they
  complement, never replace, the lit test.
- Net rule: **every compiler change ships with a lit test; runtime correctness that a
  lit test can't express also ships with a runtime fixture.** (ravn/llvm-z80#205 is the
  worked example: lit pins the `seed + ldir`/`bc,K*(N-1)` lowering; the fixtures catch
  a one-pattern over-run via in-array sentinels.)

## Design Goal: Z80 Instruction-Driven Code Generation

The primary optimization goal is to **work backwards from Z80's unique instructions** to shape register allocation and code generation, rather than generating generic code and hoping late peepholes catch it.

The Z80 has a rich set of special-purpose instructions that are far more compact than equivalent generic sequences. The compiler should aggressively seek opportunities to use them:

- **DJNZ** — `do { } while(--counter)` loops: decrement B and branch in 2 bytes (vs DEC+OR+JR = 4 bytes). Requires counter in register B.
- **LDIR/LDDR** — block copy (memcpy): (HL)→(DE), increment both, decrement BC, repeat. Replaces load/store/increment/branch loops.
- **CPIR/CPDR** — block search/compare (memchr, memcmp): compare A with (HL), increment HL, decrement BC, repeat.
- **Carry bit tricks** — RLCA/RLA to shift bit 7 into carry, BIT n to test single bits, SBC A,A to materialize carry as 0x00/0xFF. Avoids explicit comparisons.
- **Register exchange instructions** — Z80 has several atomic swap operations:
  - **EX DE,HL** — swap DE↔HL in 1 byte (vs 6 bytes of LD copies). Note: modifies BOTH registers. Only safe when the "source" side is dead, otherwise use two LDs.
  - **EXX** — swap BC↔BC', DE↔DE', HL↔HL' atomically (shadow register set). **CRITICAL: swaps ALL THREE pairs at once.** Cannot be inserted between instructions with live BC/DE/HL values — it destroys all of them. Safe only at function entry/exit or after CALL (where BC/DE/HL are dead per calling convention).
  - **EX AF,AF'** — swap AF↔AF' (accumulator + flags).
  - **EX (SP),HL/IX/IY** — exchange top-of-stack with register pair.
- **CP (HL)** — compare A with memory pointed by HL directly (1 byte, no temp register needed).
- **ADD HL,HL** — 16-bit left shift by 1 in 1 byte (11T). For multi-bit shifts, cheaper than SLA L; RL H (4 bytes). Enables byte-swap + left-shift for efficient 16-bit right shifts by 5-7.
- **INC/DEC rr** — 16-bit increment/decrement in 1 byte.

The approach: when implementing optimizations, start from "what does this Z80 instruction need?" and work backwards through register allocation and instruction selection to create those conditions, rather than starting from "what generic LLVM ops does this C code produce?" and trying to pattern-match forward.

## Architecture

### Z80 Backend (`llvm/lib/Target/Z80/`)
- **GlobalISel pipeline**: `Z80LegalizerInfo` → `Z80RegisterBankInfo` → `Z80InstructionSelector`
- **Call lowering**: `Z80CallLowering.cpp` (Z80), `SM83CallLowering.cpp` (SM83) — implements SDCC `__sdcccall(0)`/`__sdcccall(1)` calling conventions
- **TableGen definitions**: `Z80InstrCommon.td` (shared), `Z80InstrInfo.td` (Z80-specific), `SM83InstrInfo.td` (SM83-specific)
- **Optimization passes**: `Z80LateOptimization`, `Z80BranchCleanup`, `Z80PostRACompareMerge`, `Z80ShiftRotateChain`, `Z80LowerSelect`
- **Pseudo expansion**: `Z80ExpandPseudo.cpp` — expands pseudo instructions post-RA
- **MC layer** (`MCTargetDesc/`): integrated assembler, ELF object writer, machine code emitter

### Dual Toolchain
- **ELF path** (`--target=z80`): integrated assembler + `ld.lld` — produces ELF binaries
- **SDCC path** (`--target=z80-unknown-none-sdcc`): uses `sdasz80` + `sdldz80` — produces Intel HEX
- SM83 equivalents: `--target=sm83`, `--target=sm83-nintendo-none-sdcc`

### Compiler-RT Builtins (`compiler-rt/lib/builtins/{z80,sm83}/`)
Hand-written assembly (SDCC dialect) for runtime support: integer arithmetic, floating-point, memory/string operations. Built as both ELF `.a` and SDCC `.lib` archives.

### z80-utils (Rust workspace, `z80-utils/`)
Three crates: `test-runner` (integration test harness), `elf2rel` (ELF→SDCC .rel converter), `rel2elf` (reverse converter). The converters enable cross-linking Clang and SDCC compiled code.

## Key Files

- `llvm/lib/Target/Z80/Z80InstructionSelector.cpp` — largest file, GlobalISel instruction selection patterns
- `llvm/lib/Target/Z80/Z80LateOptimization.cpp` — peephole optimizations, frequently modified
- `llvm/lib/Target/Z80/Z80ExpandPseudo.cpp` — pseudo-instruction expansion
- `llvm/lib/Target/Z80/Z80CallLowering.cpp` — calling convention implementation
- `clang/cmake/caches/Z80.cmake` — build configuration cache

## Lessons Learned (from bugs and failed approaches)

### Z80 Exchange Instructions Destroy Multiple Registers
- **EX DE,HL** modifies BOTH DE and HL. Cannot be used as a one-way copy — use two LDs instead when the source must survive.
- **EXX** swaps ALL THREE pairs (BC, DE, HL) simultaneously. Cannot be inserted between instructions with live values in any of these registers. Safe only at function entry/exit or after CALL.
- Both have caused runtime bugs when inserted by peephole passes that didn't account for the global effect.

### PUSH/POP as Register Borrowing
PUSH rr (1B, 11T) + POP rr (1B, 10T) = 2 bytes to temporarily free a register pair. Cheaper than IX-indexed save/restore (3 bytes per access). Useful for borrowing a register for a specific instruction (DJNZ needs B, LDIR needs HL/DE/BC).

### IX-Indexed Access is Expensive
Each `LD r,(IX+d)` or `LD (IX+d),r` is 3 bytes, 19 T-states. The register allocator's spill decisions should reflect this cost. On architectures with cheap stack access, spilling is nearly free — on Z80, each spill/reload costs 3 bytes.

### Loop Strength Reduction (LSR) is Harmful
LSR widens 8-bit loop counters to 16-bit and creates extra induction variables, causing massive spills on Z80's 3-pair register file. Disabled via `-mllvm -disable-lsr`.

### Debugging: DISKETTE ERROR means check delay timing
The FDC needs ~260ms after power-on. If delay timing is wrong (wrong DELAY_T constant for the compiler), the FDC Specify command fails and all disk operations fail with DISKETTE ERROR.

### The compiler is experimental — never assume generated code is correct
Always test in MAME. The compiler is experimental and has known bugs. All 40 lit tests pass.

## Project Status

### Feature Flags
- `+static-stack` — allocate function locals in BSS instead of stack. Non-reentrant.
- `+shadow-regs` — enable EXX shadow register infrastructure (not yet functional for spill reduction).
- `-mllvm -z80-experimental-tti-costs` (default OFF) — enable the accurate-but-currently-inert shift-width and wider-integer cast costs (ravn/llvm-z80#177 Steps 2/3). Proven codegen-neutral on all measured workloads (AES, cpnos, BIOS, compiler-comparison-corpus, CRC/MAC microbench); kept opt-in until a workload exercises them. The `isLegalAddImmediate` cost (Step 1, a real density win) and Mul=Expensive are NOT gated.

### TargetTransformInfo (cost model, ravn/llvm-z80#177)
`Z80TargetTransformInfo.{h,cpp}` (session 75 moved bodies into the `.cpp`). Live hooks: `getNumberOfRegisters=3`, `getRegisterBitWidth=8`, `getPredictableBranchThreshold=0`, `isLSRCostLess` (register-count-first), `areInlineCompatible`, `prefersVectorizedAddressing=false`, `getArithmeticInstrCost` (Mul=Expensive; shifts width-scaled under the experimental flag), `getCastInstrCost` (i8↔i16 always; wider/monotonic under the flag), **`isLegalAddImmediate`=|Imm|≤3** (Z80 has no `ADD rr,nn`; the one density win — AES −8…−124 B + faster on LSR-active configs, production byte-identical). Several hooks are intentionally NOT implemented because empirical attempts to add them produced zero codegen change on GISel-Z80 (the 2026-06-21 sweep): `getCmpSelInstrCost` (#227), `getCallInstrCost` (#228), `getIntImmCost` (#229) — each fires its IR-level consumer correctly but downstream Z80-specific machinery (Z80LowerSelect / regalloc `isReMaterializable` / `areInlineCompatible` short-circuit) already produces the right shape; `isLegalICmpImmediate` (#230 on TLI, not TTI) — only consumer is LSR's decline-only path on GISel. `getMemoryOpCost`/`getCFInstrCost`/`getGEPCost` not implemented because the defaults approximate Z80 reality well enough and the consumers are mostly vectorizer (no SIMD on Z80). The i16/i32 Add-width charge stays HELD (#184: flows through IndVarSimplify at TCK_RecipThroughput, can't be CostKind-isolated). NOTE: `opt -mtriple=z80` works again (session 75 fixed a Z80AutoStaticStack pass-name/cl::opt collision that crashed it) — cost-model lit tests live in `llvm/test/Analysis/CostModel/Z80/`. See `tasks/session-2026-06-21-z80-tti-modelling-investigation.md` for the empirical sweep details.

### Docker Build Image
Use `llvm-z80-build` docker image (pre-installed cmake/ninja/clang/lld/python3) instead of installing packages every invocation. Build with:
```
docker build -t llvm-z80-build - <<'EOF'
FROM ubuntu:24.04
RUN apt-get update -qq && apt-get install -y -qq cmake ninja-build clang lld python3 && rm -rf /var/lib/apt/lists/*
EOF
```
Usage: `docker run --rm -v ~/git/llvm-z80:/src -w /src llvm-z80-build ninja -C build`

### Docker Test Image
Extends build image with Rust/cargo and z88dk-ticks for integration tests. Build with:
```
docker build -t llvm-z80-test -f - ~/z80/z88dk <<'EOF'
FROM llvm-z80-build AS ticks-builder
RUN apt-get update -qq && apt-get install -y -qq flex bison && rm -rf /var/lib/apt/lists/*
COPY src/ticks /ticks/src/ticks
COPY src/common /ticks/src/common
COPY src/Make.common /ticks/src/Make.common
COPY ext/uthash /ticks/ext/uthash
RUN make -C /ticks/src/ticks clean && make -C /ticks/src/ticks

FROM llvm-z80-build
RUN apt-get update -qq && apt-get install -y -qq curl && rm -rf /var/lib/apt/lists/*
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
COPY --from=ticks-builder /ticks/src/ticks/z88dk-ticks /usr/local/bin/z88dk-ticks
ENV PATH="/root/.cargo/bin:${PATH}"
EOF
```
Run integration tests:
```
docker run --rm -v ~/z80/llvm-z80:/src \
  -w /src/z80-utils/test-runner \
  -e PATH="/root/.cargo/bin:/usr/local/bin:/src/build/bin:$PATH" \
  llvm-z80-test cargo run -- clang
```

### Known Working Optimizations
- CP (HL) fusion in instruction selector
- LDIR/LDDR for memcpy/memset
- RLCA bit-7 test for signed comparisons (slt X,0 / sgt X,-1)
- SGT X,0 / SLE X,0 branchless: (non-negative mask) AND (non-zero test) — 12B fused, 14B materialized (was ~34B)
- 16-bit right shift by 5-7 via byte swap + ADD HL,HL
- Static stack (BSS-allocated locals, sequential per-function layout)
- Direct BSS addressing for all 16-bit spills: HL 3B, DE/BC/IX/IY 4B (vs 6B IX-indexed)
- EXX-based ISR save/restore with +shadow-regs
- Unused IX/IY setup removal
- BC last in 16-bit allocation order
- Conditional RET (branch-over-RET pattern)
- IX/IY allocatable as general 16-bit registers (see below)
- COPY16_PUSHPOP pseudo for IX/IY→GR16 copies (3B documented PUSH/POP, SP-safe via single-MI expansion in Z80ExpandPseudo after optimization passes)
- Register allocation hints: DE/BC preferred for ADD/SUB HL,rr operands (avoids IX/IY)
- CostPerUse: IX=1, IY=2 (IY higher because it's never FP, so allocator picks it freely; extra cost discourages low-pressure use)
- Post-RA peephole: PUSH IX; POP HL; ADD HL,rr; PUSH HL; POP IX → ADD IX,rr
- RegMask on CALL instructions: register allocator properly tracks call-clobbered registers
- DJNZ for loop counters: GR8 order (D,E,H,L,C,B,A) keeps B available; hint + peephole chain fires for `do { } while(--n)` loops. Limitation: nested loops get DJNZ on outer loop only (hint lacks depth info); loops with CALL can't use DJNZ (B is caller-saved)
- IX constant propagation: when IX holds only a constant (LD IX,nn + DEC/INC + PUSH IX; POP rr), replaces extractions with direct LD rr,adjusted_value and removes IX chain. Saves ~10B per occurrence (boot_main: 39→27 bytes)
- Tail call optimization: CALL nn; RET → JP nn when no stack args pushed. Uses TAILJMP pseudo (isReturn, not isBranch) to avoid confusing branch passes. Saves 1B per tail call site
- LD rr,nn; INC/DEC rr → LD rr,nn±1: fold adjacent 16-bit increment/decrement into the preceding immediate load. Handles both immediate and symbol operands. Saves 1B per instance (16B on PROM)
- Single-call-site inlining: areInlineCompatible allows inlining internal functions with hasOneUse() (one caller). Eliminates CALL+RET overhead without code duplication. Multi-call-site functions blocked to prevent spill pressure
- BSS spill→PUSH/POP peephole (Z80LateOptimization): converts BSS store+load pairs across CALLs to cheaper PUSH/POP (2B vs 6-8B). Three safety guards (issue #41): (1) cross-block check — skip if any other MBB references the same BSS address, (2) cross-register stack depth — track ALL PUSH/POP for LIFO correctness, not just same-register pair, (3) MCSymbol offset comparison — `MO_MCSymbol::isIdenticalTo` only compares symbol pointer, not offset; explicit `getOffset()` check needed to distinguish e.g. `__sfrend-10` from `__sfrend-16`
- Direct addressing for constant address loads/stores (issue #45): `G_STORE(val, inttoptr(const))` → `LD (nn),A` (8-bit, 3B) or `LD (nn),HL` (16-bit, 3B) instead of indirect via register pair. Same for loads. Saves 1-3B per access. BIOS wboot_c: `*(volatile word *)0x0001 = val` now emits `LD ($1),HL` instead of `LD HL,$1; LD (HL),E; INC HL; LD (HL),D`
- Ptrtoint(GV+const) fold (issue #46): `G_PTRTOINT(G_PTR_ADD(G_GLOBAL_VALUE, G_CONSTANT))` → single `LD rr, GV+const` with link-time relocation. Saves ~5B per linker symbol arithmetic expression. Required linker fix (#47) to allow 16-bit wrapping for R_Z80_ADDR16
- BSS load forwarding (Z80LateOptimization): tracks values at absolute BSS addresses within each basic block, eliminates redundant loads when value is already in a register

### Investigated: Direct BSS addressing instead of IX-indexed
With static stack, locals have fixed BSS addresses. Most IX accesses are 16-bit pairs:
- `LD HL,(addr)` = 3B vs `LD L,(IX+d); LD H,(IX+d+1)` = 6B — **half the size**
- `LD HL,(addr); EX DE,HL` = 4B vs `LD E,(IX+d); LD D,(IX+d+1)` = 6B
- Plus no IX setup (8B saved per function)
- Estimated 60-80 bytes saving across the PROM's high-spill functions
- Requires changing eliminateFrameIndex to emit direct addressing instead of IX-relative
- Caution: `LD HL,(addr)` destroys HL; `EX DE,HL` destroys both (see EXX warning)

### IX/IY as Allocatable Registers
IX and IY are in GR16 (last, least preferred — CostPerUse=1 for DD/FD prefix overhead). This gives the register allocator 5 pairs (DE, HL, BC, IX, IY) instead of 3.
- With `+static-stack` and no stack arguments: hasFP=false would free IX for allocation, but has a runtime bug (parked — see Known Non-Working)
- IY is always allocatable on Z80 (never used as frame pointer)
- Functions with stack arguments (fixed objects) still use IX as frame pointer
- All ~440 undocumented Z80 instructions defined (gated by `+undocumented`): IXH/IXL/IYH/IYL 8-bit ops, SLL, DDCB/FDCB register-copy variants, IN (C), OUT (C),0
- All pseudo expansions handle IX/IY sub-registers (IXH/IXL/IYH/IYL)
- MAME fully supports all undocumented Z80 instructions — safe for testing
- Verified: CP/M boots in MAME with IX/IY-allocatable clang-built PROM
- **IX/IY copy preference**: `COPY16_PUSHPOP` pseudo (3B, documented PUSH/POP) for IX/IY copies without `+undocumented`. With `+undocumented`, uses `LD E,IXL; LD D,IXH` (4B, undocumented LD via 8-bit path)
- **Asymmetry**: `ADD IX,rr` exists (rr=BC/DE/IX/SP) but `ADD HL,IX` does not. Values used as ADD HL,rr operands are hinted to DE/BC to avoid costly IX/IY extraction
- **EX (SP),IX/IY** (2B): swaps IX/IY with top-of-stack. `PUSH HL; EX (SP),IX; POP DE` moves HL→IX and IX→DE simultaneously (4B). Useful for transferring a running sum into IX for `ADD IX,rr` accumulation
- **ADD IX/IY peephole**: post-RA pattern `PUSH IX; POP HL; ADD HL,rr; PUSH HL; POP IX` → `ADD IX,rr` (saves 6-8B). Also handles `ADD IX,IX` (left shift)
- **ADD16_tied pseudo**: defined for future use — generalizes 16-bit add with explicit accumulator register. Expands to ADD HL/IX/IY,rr based on allocated register. Not yet used by ISel (register allocator constraint issues with tied operands on IR16)

### Z80_AllReg Calling Convention
`__attribute__((z80_allreg))` / cc 129: pass all arguments in registers, no stack.
- i8: A, L, E, C, IXL, IYL (IXL/IYL require `+undocumented`)
- i16: HL, DE, BC, IX, IY
- i32: HLDE, BCIY
- Nothing callee-saved (caller saves everything)
- Ideal for static-stack bare-metal code with full register control

### Known Non-Working / Deferred
- **hasFP=false for static-stack** (issue #12): Only saves 8B (not 80B as estimated — IX constant propagation and unused IX removal already handle most overhead). Some functions get BIGGER with hasFP=false (fdc_read_data +26B from IX PUSH/POP copies). Runtime bug (PROM hangs after banner) still present — ISR/timing related, not stack imbalance. Low priority given minimal savings.
- **BSS overlay**: Call-graph-based BSS sharing disabled (sequential layout now). The overlay algorithm worked but is parked alongside hasFP=false since both interact.
- **Mixed-mode BSS**: Direct BSS for locals + IX-indexed for stack args in same function. Not needed for current PROM (all functions use globals, no stack args). Parked — will matter when source switches back to register parameters.
- DJNZ nested loop depth: DJNZ fires but on outer loop of nested pairs (hint can't distinguish depth). Inner-loop DJNZ would need pre-RA loop depth analysis
- EXX spill conversion: shadow bank is a CONTEXT SWITCH, not extra registers. Cannot be inserted at arbitrary points. See issue #7.
- Direct BSS for DE/BC spills: resolved — now uses ED-prefix LD (addr),DE/BC (4B).
- Conditional RET with epilogue duplication: crashes with -ffunction-sections
- Machine outliner: disabled (CALL overhead > most instruction sizes on Z80)
- **Rematerializable constants held in IX** (issue #15): Allocator puts rematerializable constants (LD rr,imm / LD rr,sym) in IX to keep them alive across calls/LDIR, costing 13B (PUSH IX + LD IX + copy-out + POP IX) vs 3B for rematerialization (LD BC,imm at use site). boot_main wastes 10B this way. Fix needs allocator to prefer remat over callee-saved for cheap constants.
- **Duplicate LD rr,imm peephole** (issue #16): When two registers are loaded with the same immediate and one is still live, the second load (3B) could be a register copy (1-2B). Example: `LD HL,$68e4; LD DE,$68e4` → `LD HL,$68e4; LD D,H; LD E,L`. Post-RA peephole in Z80LateOptimization.
- **HL hint for 16-bit loop counters** (issue #17): Parked — the 16-bit INC+NZ peephole already converts `LD HL,1; ADD HL,rr; SBC A,A; ...` to `INC rr; LD A,hi; OR lo; JR NZ` (5B). Zero instances remain in PROM. The HL hint would be a structural improvement (cleaner ISel) but has identical output.
- **Undocumented instructions without +undocumented** (issue #13): FIXED. IY reserved without +undocumented; copyPhysReg falls through to PUSH/POP for IX/IY copies. PROM has zero undocumented instructions.
- **PUSH/POP for IY copies crashes when IY is allocatable** (issue #14): Using PUSH/POP instead of undocumented LD for IY copies changes code layout enough to trigger a latent regalloc bug ('y' screen crash). Workaround: reserve IY without +undocumented.
- **i1 bit test materialization** (was 13B, now 3-5B): `(val & 0x80) != 0` returning `i1` was generating a 13-byte XOR/CP sequence because the Legalizer transforms it to `icmp slt val, 0` and ISel's SLT materialization used generic XOR 0x80 + CP. Fixed: ISel now detects SLT/SGE against 0 and SGT/SLE against -1 in 8-bit materialization, uses RLCA; AND 1 (3B) or RLCA; AND 1; XOR 1 (5B).

### Code Size: Clang vs SDCC (RC700 PROM)
SDCC: 1912 bytes, Clang: 1842 bytes (70B / 3.7% smaller). BIOS: Clang 5712B. Historical root causes (mostly fixed):
1. **IX frame overhead** (~80B): PUSH IX + LD IX,addr + POP IX per function (10 functions × 8B). hasFP=false with static-stack would save this but has a runtime bug (parked).
2. **BSS correctness fix** (+60B): Fixed bug where SPILL/RELOAD_GR16 in expandPostRAPseudo used direct BSS addressing for stack arguments (wrong address). Now correctly uses IX-indexed (6B vs 3-4B per access). Recovery requires mixed-mode BSS (direct for locals, IX for stack args).
3. **IY prefix overhead** (~35B): FD-prefixed instructions. CostPerUse=2 doesn't help because spilling is worse.
4. **Register pressure / spills** (~80B): Clang spills more conservatively than SDCC.
5. **Comparison sequences** (~50B): Clang generates longer compare/branch patterns.
6. **GR8 reorder** (+8B): B-last order for DJNZ enablement. DJNZ doesn't fire in PROM (no i8 loop counters, all loops have CALLs or are 16-bit).

Boot section optimized: `start()` at address 0x0000 (no JP indirection), unified C entry point shared by both SDCC and Clang using standard memcpy/memset (Clang inlines as LDIR). Code and data in `.pagezero.text` / `.pagezero.data` sections, banner between start() and NMI handler at 0x0066. start() is 29 bytes of plain C. `__z80__` macro (defined by cross-compiler) guards Z80-specific code paths; CLion sees all code via `__z80__` in CMakeLists.

Top 3 worst functions: fdc_read_data (+95B), check_sysfile (+59B), lookup_sectors (+54B). Optimization plan in `glowing-bouncing-dream.md`.

**PUSH/POP for IX/IY copies** (issue #32): FIXED. `copyPhysReg` now emits `COPY16_PUSHPOP` pseudo (single MI) instead of raw `PUSH IX; POP DE` for IX/IY copies without `+undocumented`. The pseudo expands to adjacent PUSH/POP in `Z80ExpandPseudo` (after all optimization passes), preventing passes from inserting between them. Documented instructions only, no +undocumented required. LateOptimization peepholes (IX constant propagation, ADD IX, IX transfer) updated to match the pseudo.

### Bug Reports
Default: file bugs in `ravn/llvm-z80` only, never official upstream LLVM. Collect crash artifacts (preprocessed source, run script) into a zip.

**Engagement-mode (session 77, user-confirmed):** when the user explicitly directs a *curated* upstream submission "z80 upstream only", file issues at the fork-of-record **`llvm-z80/llvm-z80`** (@zlfn) — one issue per underlying bug, with a linked failing test case and the fix framed as a proposal, **drafted and approved before filing**, plus ONE tests-only PR (never a PR per bug, never a fix PR). Never official `llvm/llvm-project` directly; reference (don't duplicate) bugs already open there. See memories `feedback_no_upstream_issues`, `feedback_no_pull_requests`, `feedback_upstream_routing_two_targets`.

## Code Review Notes

When modifying control flow in `llvm/` code, verify that performance profile data and debug information (especially for branches and calls) remain valid.
