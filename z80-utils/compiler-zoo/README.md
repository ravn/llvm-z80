# compiler-zoo — multi-compiler Z80 code comparison

Two complementary comparison drivers:

## `compare.py` — bare-metal microbenchmarks (clang vs zsdcc)

Compiles bare-metal `bench_*.c` programs (CRT_ORG 0x0000, compute → `HALT`,
result in HL) with the Docker toolchains and measures code size + T-states with
a register-value correctness check.  The benchmark corpus itself was removed in
`5449952826fe`; this is the framework shell.

```
make all          # python3 compare.py
make full asm
```

## `cpm_zoo.py` — CP/M oracle (dcc vs clang vs zsdcc)

Compiles the **dcc C test corpus** (github.com/davidly/dcc, `tests/*.c`) for
CP/M 2.2 with three compilers and reports, per test:

| column  | meaning |
|---------|---------|
| size    | final `.COM` bytes (code + each toolchain's linked runtime) |
| raw     | the test TU's own code+rodata, **no runtime** (clang: `.text`/`.rodata` of the `.o`; zsdcc: `code_*`/`rodata_*` via `z88dk-z80nm`; dcc: `__BSSB` from the M80 listing) |
| tstates | execution time via `z88dk-ticks` (CP/M page-zero BDOS stub) |
| verdict | output cross-check by **consensus** — AGREE / SOLO / DIFF / BUILD_FAIL |

dcc is a faithful **size and speed** oracle, but **not** a correctness oracle:
it lacks `%ld`/`%lu` (prints literal `lu`), so a compiler is `AGREE` only when
its console output matches at least one *other* compiler.  This correctly flags
dcc as the outlier on long-printing tests rather than treating it as truth.

```
make cpm                       # curated subset
make cpm-all                   # full curated corpus (~170 tests, slow)
python3 cpm_zoo.py sieve e     # specific tests
python3 cpm_zoo.py --csv --all # CSV
python3 cpm_zoo.py --compilers dcc,clang
```

clang's CP/M runtime lives in `../cpm/` (crt0 + minimal libc); see issue #35.
Builds use `-fno-builtin` so clang doesn't elide the tests' malloc/free/printf.

### Headline (curated subset)

On **raw codegen** (runtime excluded) the three are within a small factor and
trade wins — zsdcc tightest on integer/long math, clang on structural code, dcc
the loosest.  dcc's small `.COM` files come from per-program RTL stripping, not
tighter codegen.  zsdcc fails to build several tests (no `FILE*`/full libc);
clang builds the whole curated subset.

### Paths (env-overridable; defaults match the macbook/sonnyboy layout)

`DCC_DIR`, `LLVM_Z80`, `Z88DK`, `VCPM_JAR`, `CLANG_BUILD`, `MAX_TSTATES`,
`VCPM_TIMEOUT`.
