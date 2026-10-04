#!/usr/bin/env python3
"""Controlled LLVM-Z80 default-CC experiment; no production defaults change."""

import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
WORKSPACE = HERE.parents[3]
Z88DK = WORKSPACE / "z88dk"
CLANG = WORKSPACE / "llvm-z80/build-macos-asserts/bin/clang"

# Freeze the annotation policy before scores are available. Pointer-target
# signatures remain unchanged; only these named direct-call interfaces vary.
POLICY = {
    "callbench": {"dispatch": "z88dk_callee", "apply": "z88dk_callee",
                  "compose": "z88dk_callee"},
    "queenbench": {"safe": "z88dk_callee", "place": "z88dk_fastcall"},
    "sortbench": {"qsort_rec": "z88dk_callee", "ins_sort": "z88dk_callee",
                  "checksum": "z88dk_callee", "is_sorted": "z88dk_callee"},
    "sieve": {},
}


def checked(args, cwd, env=None):
    result = subprocess.run(args, cwd=cwd, env=env, capture_output=True,
                            text=True, timeout=180)
    if result.returncode:
        raise RuntimeError(f"{args}\n{result.stdout}\n{result.stderr}")
    return result


def ticks(com, work):
    # The existing CP/M shim is reused, not its success-shaped exit status:
    # require our explicit completion marker and a real cycle count.
    result = checked(["python3", str(WORKSPACE / "scratch/dcc-clang-bench/ticks_cpm.py"),
                      "--counter", "500000000", str(com)], work,
                     {**os.environ, "TMPDIR": str(work)})
    if result.stdout.strip() != "PASS":
        raise RuntimeError(f"Runtime did not complete: {result.stdout!r}\n{result.stderr}")
    match = re.search(r"^\[ticks\] (\d+) cycles$", result.stderr, re.MULTILINE)
    if not match:
        raise RuntimeError(f"Missing cycle count: {result.stderr}")
    cycles = int(match[1])
    if cycles >= 500000000:
        raise RuntimeError("Ticks reached the cycle limit instead of a bounded completion")
    return cycles


def cycle_control(work):
    # LD IX,0 (14), LD A,(IX+0) (19), LD BC,(nn) (20), JP 0 (10).
    # The stop address is tested before HALT executes: exactly 63 T-states.
    raw = bytearray(65536)
    instructions = bytes.fromhex("dd210002 dd7e00 ed4b0002 c30000")
    raw[0x100:0x100 + len(instructions)] = instructions
    image = work / "cycle-control.bin"
    image.write_bytes(raw)
    result = checked([str(Z88DK / "bin/z88dk-ticks"), "-mz80", "-l", "0",
                      "-pc", "0x100", "-end", "0", str(image)], work)
    if result.stdout.split()[-1:] != ["63"]:
        raise RuntimeError(f"Cycle control expected 63: {result.stdout}\n{result.stderr}")


def annotate(source, bench):
    for name, attribute in POLICY[bench].items():
        pattern = rf"^(static\s+(?:unsigned\s+)?(?:int|void)\s+)({name}\s*\()"
        source, count = re.subn(pattern, rf"\1__attribute__(({attribute})) \2",
                               source, flags=re.MULTILINE)
        if count != 1:
            raise RuntimeError(f"Policy function {bench}:{name} matched {count} times")
    return source


def independent_oracles():
    # Different algorithms: sorted() instead of the target's quicksort,
    # exhaustive permutations instead of its column-recursive search, and
    # trial division instead of its sieve.
    queens = sum(all(abs(p[i] - p[j]) != j - i
                     for i in range(8) for j in range(i + 1, 8))
                 for p in itertools.permutations(range(8)))
    primes = sum(all(n % d for d in range(2, int(n ** 0.5) + 1))
                 for n in range(2, 8000))
    sums = []
    for size, repeats, seed in ((100, 16, 0xBEEF), (64, 24, 0x1234)):
        total = 0
        for repeat in range(repeats):
            state = seed + repeat
            values = []
            for _ in range(size):
                state = (state * 181 + 17) & 65535
                values.append(state & 32767)
            total += sum((i + 1) * v for i, v in enumerate(sorted(values)))
        sums.append(total & 65535)
    operations = (
        lambda a, b: a + b, lambda a, b: a - b, lambda a, b: a ^ b,
        lambda a, b: a & b, lambda a, b: a | b, lambda a, b: (a << 1) ^ b,
        lambda a, b: (a >> 3) + b, lambda a, b: (a & 255) | (b << 8),
    )
    seed, checksum = 0x2F84, 0
    for _ in range(500 * 22):
        seed = (seed * 25173 + 13849) & 65535
        k = (seed >> 3) & 7
        checksum += operations[k](seed, seed >> 5) & 65535
        acc = seed
        for i in range(4):
            acc = operations[k](acc, i * 7) & 65535
        checksum += acc
        inner = operations[k](seed, seed >> 2) & 65535
        checksum += operations[(k + 1) & 7](inner, seed) & 65535
    actual = [queens, primes, *sums, checksum & 65535]
    expected = [92, 1007, 23215, 43825, 8252]
    if actual != expected:
        raise RuntimeError(f"Independent expected-value check failed: {actual} != {expected}")
    return dict(zip(("queens", "primes", "quicksort", "insertion", "calls"), actual))


def section_sizes(map_text):
    sizes = dict.fromkeys(("code", "rodata", "data", "bss"), 0)
    sections = {}
    for line in map_text.splitlines():
        match = re.match(r"^__(code|rodata|data|bss)_(\w+)_size\s*=\s*\$([0-9A-Fa-f]+)\s*;", line)
        if match:
            group, name, value = match.groups()
            size = int(value, 16)
            sections[f"{group}_{name}"] = size
            sizes[group] += size
    if not sections:
        raise RuntimeError("No code/data sections found in linker map")
    return sizes, sections


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline-only", action="store_true")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    oracles = independent_oracles()
    compiler_version = checked([str(CLANG), "--version"], HERE).stdout
    compiler_hash = hashlib.sha256(CLANG.read_bytes()).hexdigest()
    z88dk_revision = checked(["git", "-C", str(Z88DK), "rev-parse", "HEAD"], HERE).stdout.strip()
    env = {**os.environ, "ZCCCFG": str(Z88DK / "lib/config"),
           "LLVMZ80EXE": str(CLANG), "PATH": str(Z88DK / "bin") + ":" + os.environ["PATH"]}
    rows = []
    artifacts = args.output.with_suffix("")
    artifacts.mkdir(parents=True, exist_ok=True)
    tmp = WORKSPACE / "scratch/tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="default-cc.", dir=tmp) as directory:
        work = Path(directory)
        env["TMPDIR"] = str(work)
        cycle_control(work)
        for bench in POLICY:
            original = (Z88DK / f"test/suites/{bench}/{bench}.c").read_text()
            source = work / f"{bench}.c"
            source.write_text(original)
            host = work / "host"
            checked(["/usr/bin/cc", "-O2", "-I", str(HERE), str(source),
                     "-o", str(host)], work)
            if checked([str(host)], work).stdout.strip() != "PASS":
                raise RuntimeError(f"Independent native reference failed: {bench}")
            for opt in ("Os", "O2"):
                variants = ("baseline",) if args.baseline_only else ("baseline", "A", "B", "C")
                for variant in variants:
                    cell = work / f"{bench}-{opt}-{variant}"
                    cell.mkdir()
                    source = cell / f"{bench}.c"
                    source.write_text(annotate(original, bench) if variant == "C" else original)
                    env["TMPDIR"] = str(cell)
                    flags = [] if variant == "baseline" else [
                        "-Cg-fdefault-calling-conv=" +
                        ("sdcccall1" if variant == "A" else "sdcccall0")]
                    # No LTO/static frames: recursive benchmarks need reentrancy.
                    checked([str(Z88DK / "bin/zcc"), "+cpm", "-compiler=llvmz80",
                             f"-Cg-{opt}", *flags, "-I" + str(HERE), "-m",
                             "-no-cleanup", "-create-app", "-o", str(cell / "rt"),
                             str(source)], cell, env)
                    com = cell / "RT.COM"
                    cycles = ticks(com, cell)
                    native = checked([str(WORKSPACE / "ntvcm/ntvcm"), str(com)], cell)
                    if native.stdout.replace("\r", "").strip() != "PASS":
                        raise RuntimeError(f"ntvcm correctness failed: {native.stdout}")
                    payload = com.read_bytes()
                    evidence = artifacts / cell.name
                    evidence.mkdir(exist_ok=True)
                    shutil.copy2(source, evidence / source.name)
                    shutil.copy2(com, evidence / com.name)
                    for file in cell.iterdir():
                        if file.suffix.lower() in (".map", ".asm"):
                            shutil.copy2(file, evidence / file.name)
                    maps = list(cell.glob("*.map"))
                    if len(maps) != 1:
                        raise RuntimeError(f"Expected one linker map, found {maps}")
                    sizes, sections = section_sizes(maps[0].read_text())
                    rows.append({"benchmark": bench, "opt": opt, "variant": variant,
                                 "bytes": len(payload), "cycles": cycles,
                                 "section_bytes": sizes, "sections": sections,
                                 "code_rodata_data": sizes["code"] + sizes["rodata"] + sizes["data"],
                                 "sha256": hashlib.sha256(payload).hexdigest(),
                                 "source_sha256": hashlib.sha256(original.encode()).hexdigest()})
                    print(json.dumps({key: value for key, value in rows[-1].items()
                                      if key != "sections"}), flush=True)
        if not args.baseline_only:
            for bench in POLICY:
                for opt in ("Os", "O2"):
                    control = [r for r in rows if r["benchmark"] == bench and r["opt"] == opt]
                    if control[0]["sha256"] != control[1]["sha256"]:
                        raise RuntimeError(f"No-op control differs: {bench} {opt}")
                    if control[0]["cycles"] != control[1]["cycles"]:
                        raise RuntimeError(f"No-op cycles differ: {bench} {opt}")
    if hashlib.sha256(CLANG.read_bytes()).hexdigest() != compiler_hash:
        raise RuntimeError("Compiler changed during measurement")
    args.output.write_text(json.dumps({
        "compiler": compiler_version, "compiler_sha256": compiler_hash,
        "z88dk_revision": z88dk_revision,
        "annotation_policy": POLICY, "cycle_control": 63, "rows": rows,
        "independent_oracles": oracles,
    }, indent=2) + "\n")


if __name__ == "__main__":
    main()
