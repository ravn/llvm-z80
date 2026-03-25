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
cargo run -- clang add                   # Filter by test name pattern
cargo run -- sdcc                        # SDCC cross-build compatibility
cargo run -- llc                         # LLC (LLVM IR) suite
cargo run -- utils                       # elf2rel/rel2elf converter tests
cargo run -- bench                       # Code size benchmark (Clang vs SDCC)
```

Test cases live in `z80-utils/test-runner/testcases/{clang,llc,sdcc,custom}/`. Tests use metadata directives: `/* expect: 0x00FF */` for expected return values, `/* SKIP-IF: sm83 */` to skip targets.

The `BUILD_DIR` env var overrides the default build directory (`../build`).

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
Always test in MAME. The compiler has known bugs (branch relaxation crashes with simple do-while loops + `-disable-lsr`, pre-existing test failures in calling-conv.ll and large-frame.ll).

## Project Status

### Feature Flags
- `+static-stack` — allocate function locals in BSS instead of stack. Non-reentrant.
- `+shadow-regs` — enable EXX shadow register infrastructure (not yet functional for spill reduction).

### Docker Build Image
Use `llvm-z80-build` docker image (pre-installed cmake/ninja/clang/lld/python3) instead of installing packages every invocation. Build with:
```
docker build -t llvm-z80-build - <<'EOF'
FROM ubuntu:24.04
RUN apt-get update -qq && apt-get install -y -qq cmake ninja-build clang lld python3 && rm -rf /var/lib/apt/lists/*
EOF
```
Usage: `docker run --rm -v ~/git/llvm-z80:/src -w /src llvm-z80-build ninja -C build`

### Known Working Optimizations
- CP (HL) fusion in instruction selector
- LDIR/LDDR for memcpy/memset
- RLCA bit-7 test for signed comparisons (slt X,0 / sgt X,-1)
- 16-bit right shift by 5-7 via byte swap + ADD HL,HL
- Static stack (BSS-allocated locals, sequential per-function layout)
- Direct BSS addressing for all 16-bit spills: HL 3B, DE/BC/IX/IY 4B (vs 6B IX-indexed)
- EXX-based ISR save/restore with +shadow-regs
- Unused IX/IY setup removal
- BC last in 16-bit allocation order
- Conditional RET (branch-over-RET pattern)
- IX/IY allocatable as general 16-bit registers (see below)
- Undocumented LD for IX/IY→GR16 copies (4B, SP-safe — PUSH/POP is 3B but corrupts SP-relative addressing)
- Register allocation hints: DE/BC preferred for ADD/SUB HL,rr operands (avoids IX/IY)
- CostPerUse: IX=1, IY=2 (IY higher because it's never FP, so allocator picks it freely; extra cost discourages low-pressure use)
- Post-RA peephole: PUSH IX; POP HL; ADD HL,rr; PUSH HL; POP IX → ADD IX,rr

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
- **IX/IY copy preference**: `PUSH IX; POP DE` (3B, documented) preferred over `LD E,IXL; LD D,IXH` (4B, undocumented) for 16-bit IX/IY→GR16 register copies
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
- **hasFP=false for static-stack**: Would save ~150B by freeing IX. Runtime bug undiagnosed — SPILL_GR16 is expanded in both expandPostRAPseudo (Z80InstrInfo.cpp:887, has its own BSS path) and eliminateFrameIndex (Z80RegisterInfo.cpp:1116). The two paths may conflict. Debugging plan in `glowing-bouncing-dream.md`.
- **BSS overlay**: Call-graph-based BSS sharing disabled (sequential layout now). The overlay algorithm worked but is parked alongside hasFP=false since both interact.
- DJNZ: infrastructure complete but never fires (B always occupied by outer loops)
- EXX spill conversion: shadow bank is a CONTEXT SWITCH, not extra registers. Cannot be inserted at arbitrary points. See issue #7.
- Direct BSS for DE/BC spills: clobbers HL. Needs liveness check. See issue #8.
- Conditional RET with epilogue duplication: crashes with -ffunction-sections
- Machine outliner: disabled (CALL overhead > most instruction sizes on Z80)

### Code Size: Clang vs SDCC (RC700 PROM)
SDCC: 1872 bytes, Clang: 2401 bytes (529B / 28% larger). Root causes:
1. **IX frame overhead** (~80B): PUSH IX + LD IX,addr + POP IX per function (10 functions × 8B). hasFP=false with static-stack would save this but has a runtime bug (parked).
2. **IY prefix overhead** (~35B): 35 FD-prefixed instructions across 10 functions. IY is used to hold values across calls (only callee-saved register besides IX/FP). CostPerUse=2 doesn't help because the alternative (spilling) is even more expensive.
3. **Register pressure / spills** (~80B): Clang spills more conservatively than SDCC.
4. **Comparison sequences** (~50B): Clang generates longer compare/branch patterns.
5. **DJNZ not firing** (~30B): Infrastructure exists but B is always occupied.

Top 3 worst functions: fdc_read_data (+95B), check_sysfile (+59B), lookup_sectors (+54B). Optimization plan in `glowing-bouncing-dream.md`.

**PUSH/POP for IX/IY copies corrupts SP**: Using `PUSH IX; POP DE` (3B) instead of `LD E,IXL; LD D,IXH` (4B) saves 1 byte but modifies SP, breaking SP-relative addressing in surrounding code. Always use undocumented LD for IX/IY extraction in `copyPhysReg`.

### Bug Reports
File bugs in `ravn/llvm-z80` only, never upstream LLVM. Collect crash artifacts (preprocessed source, run script) into a zip.

## Code Review Notes

When modifying control flow in `llvm/` code, verify that performance profile data and debug information (especially for branches and calls) remain valid.
