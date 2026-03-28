#!/usr/bin/env python3
"""Generate Z80 edge-case test files for the llvm-z80 clang test suite.

Each file is a standalone C program returning 0 on success (all tests pass)
or non-zero on failure.  Compatible with z80-utils test runner via the
/* expect: 0x0000 */ directive.

Usage: gen_z80_full.py <files> <tests_per_file> <seed>
"""
import random
import sys

# ----------------------------
# Helpers
# ----------------------------
def u8(): return random.randint(0, 255)
def u16(): return random.randint(0, 65535)

# ----------------------------
# HL aliasing tests
# ----------------------------
def gen_hl_alias(i):
    val = u16()
    return f"""
    {{
        uint16_t x = {val};
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }}
"""

# ----------------------------
# Carry propagation
# ----------------------------
def gen_carry_chain(i):
    a = u8()
    b = u8()
    expected = (a + b) & 0xFFFF

    return f"""
    {{
        uint16_t x = {a};
        x = x + {b};
        if (x != {expected}) failures++;
    }}
"""

def gen_carry_clobber(i):
    return f"""
    {{
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }}
"""

# ----------------------------
# Array / IX/IY style
# ----------------------------
def gen_array(i):
    arr = [u8() for _ in range(6)]
    idx = random.randint(0, 5)
    val = arr[idx]
    init = ",".join(map(str, arr))

    return f"""
    {{
        uint8_t a[6] = {{{init}}};
        if (a[{idx}] != {val}) failures++;
    }}
"""

# ----------------------------
# Register pressure
# ----------------------------
def gen_reg_pressure(i):
    vals = [u16() for _ in range(8)]
    expr = " + ".join(map(str, vals))
    expected = sum(vals) & 0xFFFF

    return f"""
    {{
        uint16_t r = {expr};
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Calling convention
# ----------------------------
def gen_call(i):
    args = [u8() for _ in range(6)]
    expected = sum(args) & 0xFFFF
    argstr = ",".join(map(str, args))

    return f"""
    {{
        uint16_t r = call6({argstr});
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Mixed width (8/16 bugs)
# ----------------------------
def gen_mixed(i):
    a = u8()
    b = u16()
    expected = (a + b) & 0xFFFF

    return f"""
    {{
        uint16_t r = (uint16_t)((uint8_t){a}) + (uint16_t){b};
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Shift edge cases
# ----------------------------
def gen_shift(i):
    val = u8()
    shift = random.randint(0,7)
    expected = (val << shift) & 0xFF

    return f"""
    {{
        uint8_t x = {val};
        x <<= {shift};
        if (x != {expected}) failures++;
    }}
"""

# ----------------------------
# Comparison (flags correctness)
# ----------------------------
def gen_cmp(i):
    a = random.randint(-128,127)
    b = random.randint(-128,127)
    expected = 1 if a < b else 0

    return f"""
    {{
        int8_t a = {a};
        int8_t b = {b};
        int r = (a < b);
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Expression fuzz (safe)
# ----------------------------
OPS = ["+", "-", "^", "&", "|"]

def gen_expr(depth=0):
    if depth > 2 or random.random() < 0.3:
        return str(u8())
    return f"({gen_expr(depth+1)} {random.choice(OPS)} {gen_expr(depth+1)})"

def gen_expr_test(i):
    expr = gen_expr()
    val = eval(expr) & 0xFFFF

    return f"""
    {{
        if (((uint16_t){expr}) != {val}) failures++;
    }}
"""

# ----------------------------
# File generator
# ----------------------------
def gen_file(idx, ntests, extra_flags=""):
    out = []

    out.append("/* Z80 edge-case test (auto-generated) */")
    out.append("/* expect 0x0000 */")
    if extra_flags:
        out.append(f"/* EXTRA-FLAGS: {extra_flags} */")
    out.append("")
    out.append("typedef unsigned char uint8_t;")
    out.append("typedef signed char int8_t;")
    out.append("typedef unsigned short uint16_t;")
    out.append("typedef unsigned long uint32_t;")
    out.append("")

    out.append("""__attribute__((noinline))
uint16_t call6(uint16_t a,uint16_t b,uint16_t c,
               uint16_t d,uint16_t e,uint16_t f) {
    return a+b+c+d+e+f;
}
""")

    out.append("int main(void) {")
    out.append("    int failures = 0;")

    generators = [
        gen_hl_alias,
        gen_carry_chain,
        gen_carry_clobber,
        gen_array,
        gen_reg_pressure,
        gen_call,
        gen_mixed,
        gen_shift,
        gen_cmp,
        gen_expr_test
    ]

    for i in range(ntests):
        g = random.choice(generators)
        out.append(g(i))

    out.append("    return failures;")
    out.append("}")

    return "\n".join(out)

# ----------------------------
# MAIN
# ----------------------------
def main():
    import argparse
    parser = argparse.ArgumentParser(description="Generate Z80 edge-case tests")
    parser.add_argument("files", type=int, help="Number of files to generate")
    parser.add_argument("tests", type=int, help="Tests per file")
    parser.add_argument("seed", type=int, help="Random seed")
    parser.add_argument("--outdir", default=".", help="Output directory")
    parser.add_argument("--prefix", default="z80_edge_", help="Filename prefix")
    parser.add_argument("--extra-flags", default="", help="EXTRA-FLAGS directive for test runner")
    args = parser.parse_args()

    random.seed(args.seed)

    import os
    os.makedirs(args.outdir, exist_ok=True)

    for i in range(args.files):
        path = os.path.join(args.outdir, f"{args.prefix}{i:04d}.c")
        with open(path, "w") as f:
            f.write(gen_file(i, args.tests, args.extra_flags))

    print(f"Generated {args.files} files with {args.tests} tests each "
          f"(seed={args.seed}, dir={args.outdir})")

if __name__ == "__main__":
    main()
