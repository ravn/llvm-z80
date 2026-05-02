#!/usr/bin/env python3
"""Per-function size baseline tracker for the Z80 backend.

Walks BIOS / cpnos-rom / PROM ELF outputs in the workspace, extracts
per-function sizes via `llvm-nm --print-size --size-sort`, and either
writes a baseline JSON or diffs the current build against the baseline.

Subcommands:
    record  - write current per-function sizes to tasks/size-baseline.json
    check   - diff current vs baseline; non-zero exit on regression
    show    - print current sizes only

Per-artifact baseline is keyed by ELF path relative to the workspace
root.  Per roadmap §12.1 (Phase 1).
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

WORKSPACE = Path(__file__).resolve().parent.parent.parent  # /Users/ravn/z80
DEFAULT_BASELINE = Path(__file__).resolve().parent / "size-baseline.json"
DEFAULT_NM = Path(__file__).resolve().parent.parent / "build-macos" / "bin" / "llvm-nm"

ARTIFACTS = [
    "rc700-gensmedet/rcbios-in-c/clang/bios.elf",
    "rc700-gensmedet/cpnos-rom/clang/payload.elf",
    "rc700-gensmedet/cpnos-rom/clang/relocator.elf",
]


def find_nm() -> Path:
    """Locate llvm-nm.  Prefer the local native build."""
    if DEFAULT_NM.exists():
        return DEFAULT_NM
    fallback = subprocess.run(
        ["which", "llvm-nm"], capture_output=True, text=True
    )
    if fallback.returncode == 0 and fallback.stdout.strip():
        return Path(fallback.stdout.strip())
    raise SystemExit(
        f"llvm-nm not found at {DEFAULT_NM} and not on PATH. "
        f"Build the native toolchain first (ninja -C build-macos)."
    )


def per_function_sizes(nm: Path, elf: Path) -> dict[str, int]:
    """Return {function_name: size_in_bytes} for code symbols in elf."""
    if not elf.exists():
        return {}
    out = subprocess.run(
        [str(nm), "--print-size", "--size-sort", str(elf)],
        capture_output=True,
        text=True,
        check=True,
    ).stdout
    sizes: dict[str, int] = {}
    for line in out.splitlines():
        # Format: "<addr> <size> <type> <name>"
        parts = line.split(maxsplit=3)
        if len(parts) != 4:
            continue
        _addr, size_hex, sym_type, name = parts
        # T = global text, t = local text (function code).
        if sym_type not in ("T", "t"):
            continue
        try:
            sizes[name] = int(size_hex, 16)
        except ValueError:
            continue
    return sizes


def collect(nm: Path) -> dict[str, dict[str, int]]:
    """Return {artifact_path: {function: size}}."""
    out: dict[str, dict[str, int]] = {}
    for rel in ARTIFACTS:
        elf = WORKSPACE / rel
        out[rel] = per_function_sizes(nm, elf)
    return out


def diff(current: dict[str, dict[str, int]],
         baseline: dict[str, dict[str, int]]) -> list[tuple[str, str, int, int]]:
    """Return list of (artifact, function, old_size, new_size) for changes.

    Includes additions (old=0), removals (new=0), and resizes.
    """
    rows: list[tuple[str, str, int, int]] = []
    artifacts = sorted(set(current.keys()) | set(baseline.keys()))
    for art in artifacts:
        cur = current.get(art, {})
        base = baseline.get(art, {})
        funcs = sorted(set(cur.keys()) | set(base.keys()))
        for fn in funcs:
            old = base.get(fn, 0)
            new = cur.get(fn, 0)
            if old != new:
                rows.append((art, fn, old, new))
    return rows


def total(d: dict[str, dict[str, int]]) -> dict[str, int]:
    return {a: sum(funcs.values()) for a, funcs in d.items()}


def cmd_record(args: argparse.Namespace) -> int:
    nm = find_nm()
    data = collect(nm)
    args.baseline.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")
    print(f"wrote baseline -> {args.baseline}")
    for art, sz in total(data).items():
        print(f"  {art}: {sz} B  ({len(data[art])} fns)")
    return 0


def cmd_check(args: argparse.Namespace) -> int:
    if not args.baseline.exists():
        print(f"no baseline at {args.baseline}; run `record` first", file=sys.stderr)
        return 2
    nm = find_nm()
    current = collect(nm)
    baseline = json.loads(args.baseline.read_text())
    rows = diff(current, baseline)
    if not rows:
        print("no per-function size changes vs baseline")
        return 0
    regressions = 0
    improvements = 0
    print(f"{'artifact':40} {'function':40} {'old':>8} {'new':>8} {'delta':>8}")
    for art, fn, old, new in rows:
        delta = new - old
        if delta > 0:
            regressions += 1
        elif delta < 0:
            improvements += 1
        marker = "+" if delta > 0 else ("-" if delta < 0 else " ")
        short_art = art.rsplit("/", 1)[-1] if len(art) > 40 else art
        print(f"{short_art:40} {fn:40} {old:>8} {new:>8} {marker}{abs(delta):>7}")
    cur_tot = total(current)
    base_tot = total(baseline)
    print()
    print(f"per-artifact totals:")
    for art in sorted(set(cur_tot) | set(base_tot)):
        cur = cur_tot.get(art, 0)
        base = base_tot.get(art, 0)
        print(f"  {art}: {base} -> {cur}  ({cur - base:+d} B)")
    print()
    print(f"summary: {regressions} regression(s), {improvements} improvement(s)")
    return 1 if (regressions and not args.allow_regress) else 0


def cmd_show(args: argparse.Namespace) -> int:
    nm = find_nm()
    data = collect(nm)
    for art, funcs in data.items():
        print(f"# {art}: {sum(funcs.values())} B  ({len(funcs)} fns)")
        for fn in sorted(funcs, key=lambda f: -funcs[f]):
            print(f"  {funcs[fn]:>6}  {fn}")
        print()
    return 0


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument("--baseline", type=Path, default=DEFAULT_BASELINE)
    sub = p.add_subparsers(dest="cmd", required=True)
    sub.add_parser("record", help="write current sizes as baseline")
    chk = sub.add_parser("check", help="diff current vs baseline")
    chk.add_argument("--allow-regress", action="store_true",
                     help="exit 0 even if regressions exist")
    sub.add_parser("show", help="print current sizes only")
    args = p.parse_args()
    return {"record": cmd_record, "check": cmd_check, "show": cmd_show}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
