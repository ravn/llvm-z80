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
- **EX DE,HL** — swap two 16-bit pointers in 1 byte (vs 6 bytes of LD copies).
- **CP (HL)** — compare A with memory pointed by HL directly (1 byte, no temp register needed).
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

## Code Review Notes

When modifying control flow in `llvm/` code, verify that performance profile data and debug information (especially for branches and calls) remain valid.
