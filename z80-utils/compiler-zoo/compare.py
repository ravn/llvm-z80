#!/usr/bin/env python3
"""Multi-compiler Z80 code comparison framework.

Compiles each benchmark program with multiple Z80 compilers, measures
code size and execution speed (T-states), and produces a comparison table.

Usage:
    python3 compare.py                          # compare all programs
    python3 compare.py --program bench_i8       # filter by name
    python3 compare.py --compiler clang         # single compiler
    python3 compare.py --asm                    # also dump assembly
"""

import argparse
import glob
import os
import re
import subprocess
import sys
import tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LLVM_Z80 = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
PROGRAMS_DIR = os.path.join(SCRIPT_DIR, "programs")
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "output")

# ---------------------------------------------------------------------------
# Compiler definitions
# ---------------------------------------------------------------------------

COMPILERS = {
    "clang": {
        "docker_image": "llvm-z80-test",
        "docker_volumes": lambda: [("-v", f"{LLVM_Z80}:/src")],
        "compile": [
            "/src/build/bin/clang", "--target=z80", "-Os",
            "-g",  # debug info for source-interleaved listing
            "-Xclang", "-target-feature", "-Xclang", "+static-stack",
            "-Xclang", "-target-feature", "-Xclang", "+shadow-regs",
            "-mllvm", "-disable-lsr",
            "{input}", "-o", "{output_elf}",
        ],
        "objcopy": [
            "/src/build/bin/llvm-objcopy", "-O", "binary",
            "{output_elf}", "{output_bin}",
        ],
        "size_cmd": ["/src/build/bin/llvm-size", "{output_elf}"],
        "size_parse": "llvm_size",
        "halt_cmd": ["/src/build/bin/llvm-nm", "{output_elf}"],
        "halt_parse": "llvm_nm",
        "asm_cmd": [
            "/src/build/bin/llvm-objdump", "-d", "-S", "--triple=z80",
            "{output_elf}",
        ],
        "asm_ext": ".clang.asm",
    },
    "zsdcc": {
        "docker_image": "z88dk:v2.4",
        "docker_volumes": lambda: [],
        "compile": [
            "zcc", "+z80", "-clib=sdcc_iy", "-SO3", "--opt-code-size",
            "-Cs--fomit-frame-pointer",
            "-Cs--allow-unsafe-read",
            "-Cs--sdcccall 1",
            "-Cs--max-allocs-per-node 1000000",
            "-Cs--fverbose-asm",
            "-Cs--disable-warning 296",
            "-pragma-define:CRT_ORG_CODE=0x0000",
            "-pragma-define:CLIB_MALLOC_HEAP_SIZE=32768",
            "-m", "--list", "-create-app",
            "{input}", "-o", "{output_base}",
        ],
        "size_parse": "zcc_bin",
        "halt_parse": "zcc_map",
        "asm_cmd": [
            "zcc", "+z80", "-clib=sdcc_iy", "-SO3", "--opt-code-size",
            "-Cs--fomit-frame-pointer",
            "-Cs--allow-unsafe-read",
            "-Cs--sdcccall 1",
            "-Cs--max-allocs-per-node 1000000",
            "-Cs--fverbose-asm",
            "-Cs--disable-warning 296",
            "-S",
            "{input}", "-o", "{output_asm}",
        ],
        "asm_ext": ".zsdcc.asm",
    },
}

# ---------------------------------------------------------------------------
# Docker helpers
# ---------------------------------------------------------------------------

def docker_run(image, volumes, workdir, cmd, capture=True):
    """Run a command in a Docker container."""
    args = ["docker", "run", "--rm"]
    for flag, vol in volumes:
        args.extend([flag, vol])
    args.extend(["-v", f"{workdir}:/work", "-w", "/work", image])
    args.extend(cmd)
    if capture:
        r = subprocess.run(args, capture_output=True, text=True, timeout=120)
        return r
    else:
        return subprocess.run(args, timeout=120)


# ---------------------------------------------------------------------------
# Size extraction
# ---------------------------------------------------------------------------

def parse_llvm_size(output):
    """Parse `llvm-size` output: text + data = total code size."""
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[0].isdigit():
            return int(parts[0]) + int(parts[1])  # text + data
    return None


def parse_clang_user_size(nm_output, size_output):
    """Get user code + runtime size (excluding CRT bootstrap).
    CRT is _start to _main. User code is _main onwards."""
    total = parse_llvm_size(size_output)
    if total is None:
        return None
    # Find _main address to subtract CRT size
    main_addr = None
    for line in nm_output.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[2] == "_main":
            main_addr = int(parts[0], 16)
            break
    if main_addr is not None:
        return total - main_addr  # subtract CRT bytes before _main
    return total


def parse_zcc_bin_size(output_base):
    """Get user code + runtime size from z88dk .map file.
    Sums __code_compiler_size + library sections (l_sdcc, math, l, string).
    Excludes CRT, init, error handling, malloc, compression overhead.
    Falls back to _CODE.bin + _DATA.bin if map not available."""
    map_file = output_base + ".map"
    if os.path.exists(map_file):
        with open(map_file) as f:
            content = f.read()
        # Sections that contain user code and its runtime dependencies
        user_sections = [
            "code_compiler",     # user code
            "code_l_sdcc",       # SDCC-specific runtime
            "code_l",            # general runtime
            "code_math",         # math library
            "code_string",       # string library
            "data_compiler",     # user initialized data
        ]
        total = 0
        for sec in user_sections:
            m = re.search(rf"__{sec}_size\s+=\s+\$([0-9a-fA-F]+)", content)
            if m:
                total += int(m.group(1), 16)
        if total > 0:
            return total
    # Fallback: raw binary sizes
    code_bin = output_base + "_CODE.bin"
    data_bin = output_base + "_DATA.bin"
    size = 0
    if os.path.exists(code_bin):
        size += os.path.getsize(code_bin)
    if os.path.exists(data_bin):
        size += os.path.getsize(data_bin)
    return size if size > 0 else None


# ---------------------------------------------------------------------------
# Halt address extraction
# ---------------------------------------------------------------------------

def parse_llvm_nm_halt(output):
    """Parse `llvm-nm` output for _halt symbol."""
    for line in output.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[2] == "_halt":
            return parts[0]
    return None


def parse_zcc_map_halt(map_file):
    """Parse z88dk .map file for exit symbol (_halt or __Exit)."""
    if not os.path.exists(map_file):
        return None
    with open(map_file) as f:
        content = f.read()
    # z88dk uses __Exit as the exit point (after main returns)
    # Format: "__Exit = $00B4 ; addr, public, ..."
    for sym in ("_halt", "__Exit"):
        m = re.search(rf"{re.escape(sym)}\s+=\s+\$([0-9a-fA-F]+)", content)
        if m:
            return m.group(1)
    return None


# ---------------------------------------------------------------------------
# T-states measurement
# ---------------------------------------------------------------------------

def measure_tstates(image, volumes, bin_path, halt_addr):
    """Run binary in z88dk-ticks and return (tstates, de_value)."""
    cmd = ["z88dk-ticks", "-mz80", "-trace", "-end", f"0x{halt_addr}", bin_path]
    r = docker_run(image, volumes, SCRIPT_DIR, cmd)
    if r.returncode != 0:
        return None, None
    output = r.stdout + r.stderr
    # Parse registers from trace — take the last occurrence.
    # Clang CRT: return value in HL (sdcccall convention), then HALT.
    # z88dk CRT: return value in HL, then copied to DE before __Exit.
    # We check both and prefer HL for clang, DE for z88dk.
    de = None
    hl = None
    for m in re.finditer(r"de=([0-9a-fA-F]{4})", output):
        de = m.group(1).lower()
    for m in re.finditer(r"hl=([0-9a-fA-F]{4})", output):
        hl = m.group(1).lower()
    # T-states: z88dk-ticks prints the count as the last line (just a number)
    tstates = None
    lines = output.strip().splitlines()
    if lines:
        last = lines[-1].strip()
        if last.isdigit():
            tstates = int(last)
    return tstates, de, hl


# ---------------------------------------------------------------------------
# Compile & measure for each compiler
# ---------------------------------------------------------------------------

def compile_clang(program, name, do_asm=False):
    """Compile with clang and return result dict."""
    cfg = COMPILERS["clang"]
    vols = cfg["docker_volumes"]()
    out_elf = os.path.join(OUTPUT_DIR, f"{name}.clang.elf")
    out_bin = os.path.join(OUTPUT_DIR, f"{name}.clang.bin")

    # Compile
    cmd = [s.format(input=f"/work/programs/{os.path.basename(program)}",
                    output_elf=f"/work/output/{name}.clang.elf")
           for s in cfg["compile"]]
    r = docker_run(cfg["docker_image"], vols, SCRIPT_DIR, cmd)
    if r.returncode != 0:
        return {"error": f"compile: {r.stderr.strip()[:200]}"}

    # Objcopy
    cmd = [s.format(output_elf=f"/work/output/{name}.clang.elf",
                    output_bin=f"/work/output/{name}.clang.bin")
           for s in cfg["objcopy"]]
    r = docker_run(cfg["docker_image"], vols, SCRIPT_DIR, cmd)
    if r.returncode != 0:
        return {"error": f"objcopy: {r.stderr.strip()[:200]}"}

    # Size + symbols (nm for both halt and size calculation)
    cmd = [s.format(output_elf=f"/work/output/{name}.clang.elf")
           for s in cfg["halt_cmd"]]
    r_nm = docker_run(cfg["docker_image"], vols, SCRIPT_DIR, cmd)
    nm_output = r_nm.stdout if r_nm.returncode == 0 else ""
    halt = parse_llvm_nm_halt(nm_output)

    cmd = [s.format(output_elf=f"/work/output/{name}.clang.elf")
           for s in cfg["size_cmd"]]
    r_sz = docker_run(cfg["docker_image"], vols, SCRIPT_DIR, cmd)
    size_output = r_sz.stdout if r_sz.returncode == 0 else ""
    size = parse_clang_user_size(nm_output, size_output)

    # T-states
    tstates, de, hl = None, None, None
    if halt:
        tstates, de, hl = measure_tstates(
            cfg["docker_image"], vols,
            f"/work/output/{name}.clang.bin", halt)

    # Assembly listing (source-interleaved)
    if do_asm:
        cmd = [s.format(output_elf=f"/work/output/{name}.clang.elf")
               for s in cfg["asm_cmd"]]
        r = docker_run(cfg["docker_image"], vols, SCRIPT_DIR, cmd)
        if r.returncode == 0:
            asm_path = os.path.join(OUTPUT_DIR, f"{name}{cfg['asm_ext']}")
            with open(asm_path, "w") as f:
                f.write(r.stdout)

    # sdcccall(1): return value in DE
    return {"size": size, "tstates": tstates, "de": de, "correct": de is not None}


def compile_zsdcc(program, name, do_asm=False):
    """Compile with z88dk zsdcc and return result dict."""
    cfg = COMPILERS["zsdcc"]
    vols = cfg["docker_volumes"]()
    # z88dk strips extension from -o and appends _CODE.bin etc.
    # Use underscore naming: output/{name}_zsdcc → {name}_zsdcc_CODE.bin
    zsdcc_base = f"{name}_zsdcc"
    out_base = os.path.join(OUTPUT_DIR, zsdcc_base)

    # Compile (full build with CRT)
    cmd = [s.format(input=f"/work/programs/{os.path.basename(program)}",
                    output_base=f"/work/output/{zsdcc_base}")
           for s in cfg["compile"]]
    r = docker_run(cfg["docker_image"], vols, SCRIPT_DIR, cmd)
    if r.returncode != 0:
        return {"error": f"compile: {r.stderr.strip()[:200]}"}

    # Size (from _CODE.bin + _DATA.bin)
    size = parse_zcc_bin_size(out_base)

    # Halt address (from .map)
    halt = parse_zcc_map_halt(out_base + ".map")

    # T-states
    tstates, de, hl = None, None, None
    bin_path = out_base + ".bin"
    if halt and os.path.exists(bin_path):
        tstates, de, hl = measure_tstates(
            cfg["docker_image"], vols,
            f"/work/output/{zsdcc_base}.bin", halt)

    # Assembly listing (source-annotated)
    if do_asm:
        cmd = [s.format(input=f"/work/programs/{os.path.basename(program)}",
                        output_asm=f"/work/output/{name}{cfg['asm_ext']}")
               for s in cfg["asm_cmd"]]
        r = docker_run(cfg["docker_image"], vols, SCRIPT_DIR, cmd)

    # z88dk CRT: copies return value from HL to DE before __Exit
    return {"size": size, "tstates": tstates, "de": de, "correct": de is not None}


COMPILER_FUNCS = {
    "clang": compile_clang,
    "zsdcc": compile_zsdcc,
}


# ---------------------------------------------------------------------------
# Expected value parsing
# ---------------------------------------------------------------------------

def parse_expected(source):
    """Extract expected return value from /* expect 0xXXXX */ comment."""
    m = re.search(r"/\*\s*expect\s+(0x[0-9a-fA-F]+)\s*\*/", source)
    if m:
        return m.group(1).lower().lstrip("0x").zfill(4)
    # Default: check return status in last line
    return "000f"  # common default


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Z80 multi-compiler comparison")
    parser.add_argument("--program", help="Filter programs by name substring")
    parser.add_argument("--compiler", help="Comma-separated compiler names (default: all)")
    parser.add_argument("--asm", action="store_true", help="Generate assembly listings")
    args = parser.parse_args()

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Discover programs
    programs = sorted(glob.glob(os.path.join(PROGRAMS_DIR, "bench_*.c")))
    if args.program:
        programs = [p for p in programs if args.program in os.path.basename(p)]

    if not programs:
        print("No programs found.", file=sys.stderr)
        return 1

    # Select compilers
    compilers = list(COMPILER_FUNCS.keys())
    if args.compiler:
        compilers = [c.strip() for c in args.compiler.split(",")]

    print(f"Z80 Compiler Comparison: {' vs '.join(compilers)}")
    print(f"Programs: {len(programs)}")
    print()

    # Run comparisons
    results = []
    for prog in programs:
        name = os.path.splitext(os.path.basename(prog))[0]
        with open(prog) as f:
            expected = parse_expected(f.read())

        row = {"name": name, "expected": expected}
        sys.stderr.write(f"  {name}...")
        sys.stderr.flush()

        for comp in compilers:
            fn = COMPILER_FUNCS.get(comp)
            if fn:
                try:
                    r = fn(prog, name, do_asm=args.asm)
                except Exception as e:
                    r = {"error": str(e)[:200]}
                # Check correctness
                if r.get("de") and r["de"] != expected:
                    r["correct"] = False
                    r["mismatch"] = f"got 0x{r['de']}, expected 0x{expected}"
                row[comp] = r
            else:
                row[comp] = {"error": f"unknown compiler: {comp}"}

        results.append(row)
        sys.stderr.write(" done\n")
        sys.stderr.flush()

    # Print table
    print()
    print_table(results, compilers)
    return 0


def print_table(results, compilers):
    """Print comparison table."""
    # Header
    size_hdrs = [f"{c:>8s}" for c in compilers]
    tstate_hdrs = [f"{c:>10s}" for c in compilers]

    print(f"{'Program':<25s} {''.join(size_hdrs)}  |{''.join(tstate_hdrs)}  | Winner")
    print("-" * 25 + " " + "-" * (8 * len(compilers)) + "--+" +
          "-" * (10 * len(compilers)) + "--+--------")

    totals = {c: 0 for c in compilers}
    wins = {c: 0 for c in compilers}

    for row in results:
        name = row["name"]
        sizes = []
        tstates_list = []

        for c in compilers:
            r = row.get(c, {})
            if "error" in r:
                sizes.append("  ERROR")
                tstates_list.append("     ERROR")
                sys.stderr.write(f"  {name}/{c}: {r['error']}\n")
            elif r.get("size") is not None:
                s = r["size"]
                correct = r.get("correct", True)
                mark = "" if correct else "!"
                sizes.append(f"{s:>6d}B{mark}")
                totals[c] += s
                ts = r.get("tstates")
                if ts:
                    tstates_list.append(f"{ts:>8d}T ")
                else:
                    tstates_list.append("        -  ")
            else:
                sizes.append("      -")
                tstates_list.append("        -  ")

        # Determine size winner
        valid = {c: row[c]["size"] for c in compilers
                 if c in row and "error" not in row[c] and row[c].get("size")}
        if len(valid) >= 2:
            best = min(valid, key=valid.get)
            worst_size = max(valid.values())
            delta = worst_size - valid[best]
            winner = f"{best} -{delta}B"
            wins[best] = wins.get(best, 0) + 1
        else:
            winner = ""

        print(f"{name:<25s} {''.join(sizes)}  |{''.join(tstates_list)}  | {winner}")

    # Totals
    print("-" * 25 + " " + "-" * (8 * len(compilers)) + "--+" +
          "-" * (10 * len(compilers)) + "--+--------")
    total_strs = [f"{totals[c]:>6d}B " for c in compilers]
    print(f"{'TOTAL':<25s} {''.join(total_strs)}  |{'':>{10 * len(compilers) + 2}s}| wins: " +
          ", ".join(f"{c}={wins.get(c, 0)}" for c in compilers))


if __name__ == "__main__":
    sys.exit(main() or 0)
