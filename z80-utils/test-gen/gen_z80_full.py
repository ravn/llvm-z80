#!/usr/bin/env python3
"""Generate Z80 edge-case test files for the llvm-z80 clang test suite.

Each file is a standalone C program returning 0 on success (all tests pass)
or non-zero on failure.  Compatible with z80-utils test runner via the
/* expect 0x0000 */ directive.

Usage: gen_z80_full.py <files> <tests_per_file> <seed>
"""
import random
import sys

# ----------------------------
# Helpers
# ----------------------------
def u8(): return random.randint(0, 255)
def s8(): return random.randint(-128, 127)
def u16(): return random.randint(0, 65535)
def s16(): return random.randint(-32768, 32767)
def u32(): return random.randint(0, 0xFFFFFFFF)

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
# Struct field access
# ----------------------------
def gen_struct(i):
    a, b, c, d = u8(), u8(), u16(), u8()
    field = random.choice(["a", "b", "c", "d"])
    expected = {"a": a, "b": b, "c": c & 0xFFFF, "d": d}[field]
    cast = "uint16_t" if field == "c" else "uint8_t"

    return f"""
    {{
        struct {{ uint8_t a; uint8_t b; uint16_t c; uint8_t d; }} s = {{{a},{b},{c},{d}}};
        if (s.{field} != ({cast}){expected}) failures++;
    }}
"""

# ----------------------------
# Global variables
# ----------------------------
# Globals are emitted as file-scope helpers, tested via function calls.
# We use a noinline function to force the value through memory.
def gen_global(i):
    val = u16()
    return f"""
    {{
        g16 = {val};
        if (read_g16() != {val}) failures++;
    }}
"""

# ----------------------------
# Loops (for / while / do-while)
# ----------------------------
def gen_loop_for(i):
    n = random.randint(1, 20)
    step = random.randint(1, 5)
    expected = sum(range(0, n, step)) & 0xFFFF

    return f"""
    {{
        uint16_t sum = 0;
        for (uint16_t j = 0; j < {n}; j += {step}) sum += j;
        if (sum != {expected}) failures++;
    }}
"""

def gen_loop_dowhile(i):
    n = random.randint(1, 30)
    expected = n & 0xFF

    return f"""
    {{
        uint8_t cnt = 0;
        uint8_t k = {n};
        do {{ cnt++; }} while (--k);
        if (cnt != {expected}) failures++;
    }}
"""

def gen_loop_while(i):
    start = u8()
    mask = 1 << random.randint(0, 7)
    # Count iterations until bit is set after incrementing
    v = start
    count = 0
    for _ in range(300):
        v = (v + 1) & 0xFF
        count += 1
        if v & mask:
            break

    return f"""
    {{
        uint8_t v = {start};
        uint16_t count = 0;
        while (!((++v) & {mask})) count++;
        count++;
        if (count != {count}) failures++;
    }}
"""

# ----------------------------
# Multi-dimensional array
# ----------------------------
def gen_multidim_array(i):
    rows, cols = random.randint(2, 4), random.randint(2, 4)
    arr = [[u8() for _ in range(cols)] for _ in range(rows)]
    r = random.randint(0, rows - 1)
    c = random.randint(0, cols - 1)
    expected = arr[r][c]
    init = ",".join("{" + ",".join(map(str, row)) + "}" for row in arr)

    return f"""
    {{
        uint8_t m[{rows}][{cols}] = {{{init}}};
        if (m[{r}][{c}] != {expected}) failures++;
    }}
"""

# ----------------------------
# Pointer arithmetic
# ----------------------------
def gen_pointer_arith(i):
    arr = [u8() for _ in range(8)]
    offset = random.randint(0, 7)
    expected = arr[offset]
    init = ",".join(map(str, arr))

    return f"""
    {{
        uint8_t buf[8] = {{{init}}};
        uint8_t *p = buf;
        p += {offset};
        if (*p != {expected}) failures++;
    }}
"""

# ----------------------------
# Bit manipulation (AND mask, bit test, set, clear)
# ----------------------------
def gen_bitmask(i):
    val = u8()
    bit = random.randint(0, 7)
    op = random.choice(["test", "set", "clear", "toggle"])
    mask = 1 << bit

    if op == "test":
        expected = 1 if (val & mask) else 0
        return f"""
    {{
        uint8_t v = {val};
        int r = (v & {mask}) ? 1 : 0;
        if (r != {expected}) failures++;
    }}
"""
    elif op == "set":
        expected = val | mask
        return f"""
    {{
        uint8_t v = {val};
        v |= {mask};
        if (v != {expected}) failures++;
    }}
"""
    elif op == "clear":
        expected = val & ~mask & 0xFF
        return f"""
    {{
        uint8_t v = {val};
        v &= ~(uint8_t){mask};
        if (v != {expected}) failures++;
    }}
"""
    else:  # toggle
        expected = (val ^ mask) & 0xFF
        return f"""
    {{
        uint8_t v = {val};
        v ^= {mask};
        if (v != {expected}) failures++;
    }}
"""

# ----------------------------
# Switch statement
# ----------------------------
def gen_switch(i):
    ncases = random.randint(3, 8)
    values = random.sample(range(0, 20), ncases)
    results = [u8() for _ in values]
    default_val = u8()
    test_val = random.choice(values + [99])  # 99 = default
    if test_val == 99:
        expected = default_val
    else:
        expected = results[values.index(test_val)]

    cases = "\n".join(f"        case {v}: result = {r}; break;" for v, r in zip(values, results))

    return f"""
    {{
        uint8_t input = {test_val};
        uint8_t result;
        switch (input) {{
{cases}
        default: result = {default_val}; break;
        }}
        if (result != {expected}) failures++;
    }}
"""

# ----------------------------
# 32-bit arithmetic
# ----------------------------
def gen_arith32(i):
    a = u32()
    b = u32()
    op = random.choice(["+", "-", "&", "|", "^"])
    if op == "+":
        expected = (a + b) & 0xFFFFFFFF
    elif op == "-":
        expected = (a - b) & 0xFFFFFFFF
    elif op == "&":
        expected = a & b
    elif op == "|":
        expected = a | b
    else:
        expected = a ^ b

    return f"""
    {{
        uint32_t a = {a}UL;
        uint32_t b = {b}UL;
        uint32_t r = a {op} b;
        if (r != {expected}UL) failures++;
    }}
"""

# ----------------------------
# Memcpy / memset patterns (LDIR)
# ----------------------------
def gen_memcpy(i):
    n = random.randint(1, 16)
    src = [u8() for _ in range(n)]
    idx = random.randint(0, n - 1)
    expected = src[idx]
    init = ",".join(map(str, src))

    return f"""
    {{
        uint8_t src[{n}] = {{{init}}};
        uint8_t dst[{n}];
        for (uint8_t j = 0; j < {n}; j++) dst[j] = src[j];
        if (dst[{idx}] != {expected}) failures++;
    }}
"""

def gen_memset(i):
    n = random.randint(1, 16)
    val = u8()

    return f"""
    {{
        uint8_t buf[{n}];
        for (uint8_t j = 0; j < {n}; j++) buf[j] = {val};
        if (buf[{n - 1}] != {val}) failures++;
    }}
"""

# ----------------------------
# Signed division / modulo
# ----------------------------
def gen_divmod(i):
    a = random.randint(-128, 127)
    b = random.choice([x for x in range(-128, 128) if x != 0])
    op = random.choice(["div", "mod"])
    if op == "div":
        # C truncates toward zero
        import math
        expected = int(math.trunc(a / b)) & 0xFFFF
        return f"""
    {{
        int16_t r = (int16_t)((int8_t){a}) / (int16_t)((int8_t){b});
        if ((uint16_t)r != (uint16_t){expected}) failures++;
    }}
"""
    else:
        expected = (a - int(a / b) * b) if b != 0 else 0
        expected = expected & 0xFFFF
        return f"""
    {{
        int16_t r = (int16_t)((int8_t){a}) % (int16_t)((int8_t){b});
        if ((uint16_t)r != (uint16_t){expected}) failures++;
    }}
"""

# ----------------------------
# Nested function calls (caller-save pressure)
# ----------------------------
def gen_nested_calls(i):
    a, b, c = u8(), u8(), u8()
    # inner(a,b) + inner(b,c) + inner(a,c)
    inner_ab = (a + b) & 0xFFFF
    inner_bc = (b + c) & 0xFFFF
    inner_ac = (a + c) & 0xFFFF
    expected = (inner_ab + inner_bc + inner_ac) & 0xFFFF

    return f"""
    {{
        uint16_t r = add2({a},{b}) + add2({b},{c}) + add2({a},{c});
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Function pointer / indirect call
# ----------------------------
def gen_funcptr(i):
    a, b = u8(), u8()
    op = random.choice(["add", "sub"])
    if op == "add":
        expected = (a + b) & 0xFFFF
        return f"""
    {{
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn({a},{b}) != {expected}) failures++;
    }}
"""
    else:
        expected = (a - b) & 0xFFFF
        return f"""
    {{
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn({a},{b}) != {expected}) failures++;
    }}
"""

# ----------------------------
# 16-bit comparison (all predicates)
# ----------------------------
def gen_cmp16(i):
    a = s16()
    b = s16()
    pred = random.choice(["<", ">", "<=", ">=", "==", "!="])
    expected = 1 if eval(f"{a} {pred} {b}") else 0

    return f"""
    {{
        volatile int16_t a = {a};
        volatile int16_t b = {b};
        int r = (a {pred} b);
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Volatile I/O simulation
# ----------------------------
def gen_volatile(i):
    val = u8()
    return f"""
    {{
        volatile uint8_t port = {val};
        uint8_t r = port;
        if (r != {val}) failures++;
    }}
"""

# ----------------------------
# Inline call (inlined add/sub)
# ----------------------------
def gen_inline_call(i):
    a, b = u16(), u16()
    op = random.choice(["add", "sub"])
    if op == "add":
        expected = (a + b) & 0xFFFF
        return f"""
    {{
        uint16_t r = iadd2({a}, {b});
        if (r != {expected}) failures++;
    }}
"""
    else:
        expected = (a - b) & 0xFFFF
        return f"""
    {{
        uint16_t r = isub2({a}, {b});
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Inline nested (multiple inlined calls in one expression)
# ----------------------------
def gen_inline_nested(i):
    a, b, c = u8(), u8(), u8()
    # iadd2(iadd2(a,b), isub2(b,c))
    inner1 = (a + b) & 0xFFFF
    inner2 = (b - c) & 0xFFFF
    expected = (inner1 + inner2) & 0xFFFF
    return f"""
    {{
        uint16_t r = iadd2(iadd2({a},{b}), isub2({b},{c}));
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Mixed inline/noinline (stress register alloc at call boundaries)
# ----------------------------
def gen_mixed_inline(i):
    a, b, c, d = u8(), u8(), u8(), u8()
    # noinline add2(a,b) + inline iadd2(c,d)
    expected = ((a + b) + (c + d)) & 0xFFFF
    return f"""
    {{
        uint16_t r = add2({a},{b}) + iadd2({c},{d});
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# Inline max/abs (conditional inlined)
# ----------------------------
def gen_inline_cond(i):
    op = random.choice(["max", "abs"])
    if op == "max":
        a, b = u8(), u8()
        expected = max(a, b)
        return f"""
    {{
        uint8_t r = imax8({a}, {b});
        if (r != {expected}) failures++;
    }}
"""
    else:
        v = random.randint(-32768, 32767)
        expected = abs(v) & 0xFFFF
        return f"""
    {{
        uint16_t r = iabs16({v});
        if (r != {expected}) failures++;
    }}
"""

# ----------------------------
# File generator
# ----------------------------
def gen_file(idx, ntests, extra_flags="", skip_if=""):
    out = []

    out.append("/* Z80 edge-case test (auto-generated) */")
    out.append("/* expect 0x0000 */")
    if extra_flags:
        out.append(f"/* EXTRA-FLAGS: {extra_flags} */")
    if skip_if:
        out.append(f"/* SKIP-IF: {skip_if} */")
    out.append("")
    out.append("typedef unsigned char uint8_t;")
    out.append("typedef signed char int8_t;")
    out.append("typedef unsigned short uint16_t;")
    out.append("typedef signed short int16_t;")
    out.append("typedef unsigned long uint32_t;")
    out.append("")

    # Helper functions (noinline to exercise calling convention)
    # Use portable noinline: __attribute__ for clang/gcc, nothing for SDCC
    out.append("""#ifdef __SDCC
#define NOINLINE
#else
#define NOINLINE __attribute__((noinline))
#endif

NOINLINE
uint16_t call6(uint16_t a,uint16_t b,uint16_t c,
               uint16_t d,uint16_t e,uint16_t f) {
    return a+b+c+d+e+f;
}

NOINLINE
uint16_t add2(uint16_t a, uint16_t b) { return a + b; }

NOINLINE
uint16_t sub2(uint16_t a, uint16_t b) { return a - b; }

static volatile uint16_t g16;

NOINLINE
uint16_t read_g16(void) { return g16; }

/* Inline-eligible helpers — same operations without noinline.
   The compiler may inline these, testing register allocation across
   inlined code vs the noinline variants above. */
static uint16_t iadd2(uint16_t a, uint16_t b) { return a + b; }
static uint16_t isub2(uint16_t a, uint16_t b) { return a - b; }
static uint8_t imax8(uint8_t a, uint8_t b) { return a > b ? a : b; }
static uint16_t iabs16(int16_t x) { return x < 0 ? -x : x; }
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
        gen_expr_test,
        gen_struct,
        gen_global,
        gen_loop_for,
        gen_loop_dowhile,
        gen_loop_while,
        gen_multidim_array,
        gen_pointer_arith,
        gen_bitmask,
        gen_switch,
        gen_arith32,
        gen_memcpy,
        gen_memset,
        gen_divmod,
        gen_nested_calls,
        gen_funcptr,
        gen_cmp16,
        gen_volatile,
        gen_inline_call,
        gen_inline_nested,
        gen_mixed_inline,
        gen_inline_cond,
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
    parser.add_argument("--skip-if", default="", help="SKIP-IF directive (e.g. 'O0')")
    parser.add_argument("--categories", action="store_true",
                        help="Generate one file per test category (_cat_*.c)")
    args = parser.parse_args()

    random.seed(args.seed)

    import os
    os.makedirs(args.outdir, exist_ok=True)

    if args.categories:
        # Generate one file per category with a single test instance
        cat_generators = {
            "hl_alias": gen_hl_alias,
            "carry_chain": gen_carry_chain,
            "carry_clobber": gen_carry_clobber,
            "array": gen_array,
            "reg_pressure": gen_reg_pressure,
            "call6": gen_call,
            "mixed": gen_mixed,
            "shift": gen_shift,
            "cmp": gen_cmp,
            "expr": gen_expr_test,
            "struct": gen_struct,
            "global": gen_global,
            "loop_for": gen_loop_for,
            "loop_dowhile": gen_loop_dowhile,
            "loop_while": gen_loop_while,
            "multidim": gen_multidim_array,
            "ptr_arith": gen_pointer_arith,
            "bitmask": gen_bitmask,
            "switch": gen_switch,
            "arith32": gen_arith32,
            "memcpy": gen_memcpy,
            "memset": gen_memset,
            "divmod": gen_divmod,
            "nested": gen_nested_calls,
            "funcptr": gen_funcptr,
            "cmp16": gen_cmp16,
            "volatile": gen_volatile,
            "inline_call": gen_inline_call,
            "inline_nested": gen_inline_nested,
            "mixed_inline": gen_mixed_inline,
            "inline_cond": gen_inline_cond,
        }
        for name, gen in cat_generators.items():
            # Generate a file with just one test of this category
            content = gen_file(0, 1, args.extra_flags, args.skip_if)
            # Replace the random test with the specific category
            # Rebuild from scratch with only this generator
            out = []
            out.append("/* Z80 edge-case test (auto-generated) */")
            out.append("/* expect 0x0000 */")
            if args.extra_flags:
                out.append(f"/* EXTRA-FLAGS: {args.extra_flags} */")
            out.append("")
            out.append("typedef unsigned char uint8_t;")
            out.append("typedef signed char int8_t;")
            out.append("typedef unsigned short uint16_t;")
            out.append("typedef signed short int16_t;")
            out.append("typedef unsigned long uint32_t;")
            out.append("")
            out.append("""#ifdef __SDCC
#define NOINLINE
#else
#define NOINLINE __attribute__((noinline))
#endif

NOINLINE
uint16_t call6(uint16_t a,uint16_t b,uint16_t c,
               uint16_t d,uint16_t e,uint16_t f) {
    return a+b+c+d+e+f;
}

NOINLINE
uint16_t add2(uint16_t a, uint16_t b) { return a + b; }

NOINLINE
uint16_t sub2(uint16_t a, uint16_t b) { return a - b; }

static volatile uint16_t g16;

NOINLINE
uint16_t read_g16(void) { return g16; }

static uint16_t iadd2(uint16_t a, uint16_t b) { return a + b; }
static uint16_t isub2(uint16_t a, uint16_t b) { return a - b; }
static uint8_t imax8(uint8_t a, uint8_t b) { return a > b ? a : b; }
static uint16_t iabs16(int16_t x) { return x < 0 ? -x : x; }
""")
            out.append("int main(void) {")
            out.append("    int failures = 0;")
            out.append(gen(0))
            out.append("    return failures;")
            out.append("}")
            path = os.path.join(args.outdir, f"_cat_{name}.c")
            with open(path, "w") as f:
                f.write("\n".join(out))
        print(f"Generated {len(cat_generators)} category files in {args.outdir}")
    else:
        for i in range(args.files):
            path = os.path.join(args.outdir, f"{args.prefix}{i:04d}.c")
            with open(path, "w") as f:
                f.write(gen_file(i, args.tests, args.extra_flags, args.skip_if))

        print(f"Generated {args.files} files with {args.tests} tests each "
              f"(seed={args.seed}, dir={args.outdir})")

if __name__ == "__main__":
    main()
