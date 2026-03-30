#!/bin/sh
# Convert clang Z80 assembly (.s) to z88dk-compatible .asm
# Usage: clang2z88dk.sh input.s output.asm

input="$1"
output="$2"

if [ -z "$input" ] || [ -z "$output" ]; then
    echo "Usage: $0 input.s output.asm" >&2
    exit 1
fi

# Process assembly: strip ELF directives, fix labels, add EXTERN
python3 -c "
import re, sys

lines = open('$input').readlines()
out = []
externs = set()
defined = set()

for line in lines:
    s = line.rstrip()

    # Skip ELF directives
    if re.match(r'\s*\.(file|type|size|ident|addrsig)', s): continue
    if re.match(r'\.Lfunc_end', s): continue
    if re.match(r'\s*\.section.*note', s): continue
    if s.strip().startswith('; %bb'): continue
    if not s.strip(): continue

    # Convert .globl → PUBLIC (strip trailing comments)
    m_globl = re.match(r'\s*\.globl\s+(\S+)', s)
    if m_globl:
        out.append('\tPUBLIC ' + m_globl.group(1))
        continue

    # Strip standalone comments about function boundaries
    if '-- Begin function' in s or '-- End function' in s: continue

    # Convert .text → SECTION CODE
    if re.match(r'\s*\.text', s):
        out.append('\tSECTION CODE'); continue

    # Convert .section .rodata → SECTION RODATA
    if re.match(r'\s*\.section\s+.*rodata', s):
        out.append('\tSECTION RODATA'); continue

    # Convert .asciz → defm + defb 0
    m = re.match(r'\s*\.asciz\s+\"(.*)\"', s)
    if m:
        out.append('\tdefm \"' + m.group(1) + '\"')
        out.append('\tdefb 0')
        continue

    # Replace dots in local labels: L_.str.1 → L_str_1
    s = re.sub(r'L_\.(\w+)\.(\w+)', r'L_\1_\2', s)
    s = re.sub(r'L_\.(\w+)', r'L_\1', s)

    # Track defined labels and called symbols
    m2 = re.match(r'^(\w+):', s)
    if m2: defined.add(m2.group(1))
    m3 = re.search(r'call\s+(_\w+)', s)
    if m3: externs.add(m3.group(1))

    out.append(s)

# Add EXTERN declarations for undefined symbols
header = []
for sym in sorted(externs - defined):
    header.append('\tEXTERN ' + sym)

with open('$output', 'w') as f:
    for h in header: f.write(h + '\n')
    for o in out: f.write(o + '\n')
"
