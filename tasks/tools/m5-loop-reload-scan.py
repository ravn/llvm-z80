#!/usr/bin/env python3
"""m5-loop-reload-scan.py -- M5 (#250) per-iteration base-reload oracle.

Turns the luck-based discovery of the M5 "scale-1 char-array loops miss
pointer strength reduction" pattern (ravn/llvm-z80#250) into a repeatable
detector.  Point it at clang/llc `-S` output and it enumerates every
`add hl,`/`add ix,`/`add iy,` that executes inside a loop, tagging the ones
that re-load a loop-invariant GLOBAL base every iteration -- the exact M5
waste (`ld hl,_flags; add hl,de` per iter instead of a running pointer).

Why it works
------------
LLVM annotates the START LINE of every basic block with its loop status:
  - a header block:  `.LBB0_2:   ; =>This Loop Header: Depth=1`
                     `.LBB0_4:   ; =>  This Inner Loop Header: Depth=2`
  - a body block:    `; %bb.2:   ;   in Loop: Header=BB6_1 Depth=1`
  - a NON-loop block (fall-through / exit) has NO such annotation.
So `in_loop` is read off the block-start line, NOT accumulated over the body.
(The accumulate-over-body version had a false positive: a non-loop `; %bb.N:`
exit block fell through under the preceding loop label and its `add hl,de`
was wrongly reported as in-loop -- cpnos transport_pio.c, 2026-07-12.)

Tags
----
  GLOBAL-BASE(sym)  a `ld hl,<sym>` (sym is a real global, not a __sfrend/
                    __sframe/.L spill slot) precedes the `add` in the SAME
                    block -> genuine per-iteration base reload = M5.
  PAIR-RECON        a `ld l,<r>` reg-pair reconstruct precedes the `add`
                    -> the #99 IY-exile / index-shuffle family, related but
                    distinct (pointer is formed, just mis-allocated).
  add-in-loop       a bare in-loop add (index/stride advance) -- usually fine.

A GLOBAL-BASE hit is a *candidate*; confirm by eye that the reload is really
per-iteration (inside the back-edge), then profile to see if the loop is hot
(a cold, call-bounded init loop pays the 21 T reload as noise -- e.g. the
autoload FDC loops).  Reversal/scan kernels are where it bites: red/green on
fannkuch measured -20 % T-states from removing the hot _perm reloads.

Usage
-----
  clang --target=z80 -Oz -mllvm -disable-lsr ... -S foo.c -o foo.s
  tasks/tools/m5-loop-reload-scan.py foo.s [bar.s ...]
"""
import re
import sys

label_re = re.compile(r'^(\.?L?[\w$.]+):')
bb_re    = re.compile(r'^; %bb\.\d+:')
add_re   = re.compile(r'^\s*add\s+(hl|ix|iy),\s*(\w+)', re.I)
ldhl_sym = re.compile(r'^\s*ld\s+hl,\s*([A-Za-z_.$][\w$.]*)', re.I)
ldl_re   = re.compile(r'^\s*ld\s+l,\s*[bcdeha]\s*$', re.I)
loop_hdr = re.compile(r'This (Inner )?Loop Header|in Loop:\s*Header=')


def is_spill(sym):
    return sym.startswith('__sfrend') or sym.startswith('__sframe') \
        or sym.startswith('.L')


def scan(path):
    lines = open(path).read().splitlines()
    blocks = []
    cur = None
    for i, ln in enumerate(lines):
        m = label_re.match(ln)
        is_bb = bb_re.match(ln)
        if (m and not ln.strip().startswith(';')) or is_bb:
            label = m.group(1) if m else ln.strip().split()[1].rstrip(':')
            cur = {'label': label, 'in_loop': bool(loop_hdr.search(ln)),
                   'lines': []}
            blocks.append(cur)
            continue
        if cur is not None:
            cur['lines'].append(i)

    hits = []
    for b in blocks:
        if not b['in_loop']:
            continue
        base_syms = []
        pair_recon = False
        for idx in b['lines']:
            ln = lines[idx]
            ms = ldhl_sym.match(ln)
            if ms and not is_spill(ms.group(1)):
                base_syms.append(ms.group(1))
            if ldl_re.match(ln):
                pair_recon = True
            am = add_re.match(ln)
            if am:
                kind = []
                if base_syms:
                    kind.append('GLOBAL-BASE(' +
                                ','.join(sorted(set(base_syms))) + ')')
                if pair_recon:
                    kind.append('PAIR-RECON')
                hits.append((b['label'], idx + 1, ln.strip(),
                             ';'.join(kind) or 'add-in-loop'))
    return hits


def main(argv):
    total = 0
    globals_ = 0
    for path in argv:
        hits = scan(path)
        if not hits:
            continue
        print(f'\n=== {path} ===')
        for label, lno, txt, kind in hits:
            print(f'  {label:14} L{lno:<5} {txt:28} [{kind}]')
            if kind.startswith('GLOBAL-BASE'):
                globals_ += 1
        total += len(hits)
    print(f'\nTOTAL add-in-loop hits: {total}  '
          f'(GLOBAL-BASE / M5 candidates: {globals_})')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
