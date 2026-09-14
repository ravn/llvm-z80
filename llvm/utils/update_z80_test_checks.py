#!/usr/bin/env python3
"""Update FileCheck patterns in Z80 lit tests.

Runs llc with each test's RUN-line flags, captures the assembly output,
and replaces CHECK blocks with the new output.  Handles sdasz80 and
intel/gnu asm formats.

Usage:
  python3 update_z80_test_checks.py [--llc-binary <path>] <test.ll>...
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path


def get_run_cmds(test_path: Path):
    """Extract RUN lines, substituting %s with the test path."""
    cmds = []
    for line in test_path.read_text().splitlines():
        m = re.match(r';\s*RUN:\s*(.*)', line)
        if not m:
            continue
        cmd = m.group(1).replace('%s', str(test_path))
        cmds.append(cmd)
    return cmds


def run_llc(llc_binary: str, run_cmd: str):
    """Run the llc portion of a RUN line (strip FileCheck), return asm output."""
    # Drop FileCheck and anything after it
    llc_cmd = re.split(r'\|\s*FileCheck', run_cmd)[0].strip()
    # Replace 'llc' at start with our binary
    llc_cmd = re.sub(r'^llc\b', llc_binary, llc_cmd)
    try:
        result = subprocess.run(
            llc_cmd, shell=True, capture_output=True, text=True, timeout=30
        )
        if result.returncode != 0:
            return None, result.stderr
        return result.stdout, None
    except subprocess.TimeoutExpired:
        return None, 'timeout'


def parse_functions(asm: str):
    """Parse assembly output into {func_name: [instruction_lines]}.

    Handles both sdasz80 (leading underscore, .area sections) and
    intel/gnu asm formats.
    """
    funcs = {}
    current = None
    for line in asm.splitlines():
        stripped = line.strip()
        # Skip directives and blank lines between functions
        if not stripped or stripped.startswith('.') or stripped.startswith(';'):
            if current is not None and stripped.startswith(';'):
                pass  # keep inline comments as part of function? skip them
            continue
        # Function label: _name: or name:
        m = re.match(r'^_?(\w+):\s*(?:;.*)?$', stripped)
        if m and not re.match(r'^\.', stripped):
            label = m.group(1)
            # Only treat as function start if it looks like a function name
            # (i.e., there's a .globl directive for it nearby — simplified:
            # treat any top-level label as a function)
            current = label
            funcs[current] = []
            continue
        if current is not None:
            # Skip kill/implicit markers and empty bb labels
            if re.match(r'^; (kill:|%bb\.|bb\.|-- Begin|-- End)', stripped):
                continue
            # Strip inline ; @function_name comments on label lines
            instr = re.sub(r'\s*;.*$', '', line.rstrip())
            if instr.strip():
                funcs[current].append(instr)
    return funcs


def update_test(test_path: Path, llc_binary: str):
    """Update CHECK patterns in a single test file."""
    text = test_path.read_text()
    run_cmds = get_run_cmds(test_path)
    if not run_cmds:
        print(f'  skip (no RUN lines): {test_path.name}')
        return False

    # Use the first RUN line that produces llc output
    asm = None
    for cmd in run_cmds:
        if 'llc' not in cmd:
            continue
        out, err = run_llc(llc_binary, cmd)
        if out is not None:
            asm = out
            break
        else:
            print(f'  llc failed for {test_path.name}: {err[:80] if err else "?"}',
                  file=sys.stderr)

    if asm is None:
        print(f'  skip (llc produced no output): {test_path.name}')
        return False

    funcs = parse_functions(asm)
    if not funcs:
        print(f'  skip (no functions in output): {test_path.name}')
        return False

    # Split file into non-CHECK lines and CHECK blocks.
    # Strategy: remove all existing ; CHECK* lines, then re-insert
    # them immediately before each IR function definition.
    lines = text.splitlines(keepends=True)

    # Remove all ; CHECK lines
    kept = []
    for line in lines:
        if re.match(r';\s*CHECK', line):
            continue
        kept.append(line)

    # Build map of function name -> IR define line number (in kept lines)
    func_line = {}
    for i, line in enumerate(kept):
        m = re.match(r'^define\b.*@(\w+)\s*\(', line)
        if m:
            func_line[m.group(1)] = i

    # Re-insert CHECK blocks before each define line
    # (adjust line numbers after each insertion)
    result = list(kept)
    inserted = 0
    for fname, lineno in sorted(func_line.items(), key=lambda x: x[1]):
        if fname not in funcs:
            continue
        instrs = funcs[fname]
        # Build CHECK block
        block = []
        block.append(f'; CHECK-LABEL: {fname}:\n')
        for instr in instrs:
            block.append(f'; CHECK:      {instr.rstrip()}\n')

        # Find where this define now is (adjusted for prior insertions)
        adj_lineno = lineno + inserted
        # Insert block just before the define line
        for j, bline in enumerate(block):
            result.insert(adj_lineno + j, bline)
        inserted += len(block)

    test_path.write_text(''.join(result))
    print(f'  updated: {test_path.name} ({len(funcs)} functions)')
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--llc-binary', default='llc',
                        help='Path to llc binary')
    parser.add_argument('tests', nargs='+', type=Path,
                        help='Test files to update')
    args = parser.parse_args()

    ok = fail = skip = 0
    for t in args.tests:
        if not t.exists():
            print(f'not found: {t}', file=sys.stderr)
            fail += 1
            continue
        try:
            if update_test(t, args.llc_binary):
                ok += 1
            else:
                skip += 1
        except Exception as e:
            print(f'  ERROR {t.name}: {e}', file=sys.stderr)
            fail += 1

    print(f'\nDone: {ok} updated, {skip} skipped, {fail} errors')


if __name__ == '__main__':
    main()
