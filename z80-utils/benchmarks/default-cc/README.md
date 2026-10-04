# Default calling-convention experiment

Run from the workspace root:

```sh
python3 llvm-z80/z80-utils/benchmarks/default-cc/run.py \
    --output scratch/tmp/default-cc-results.json
```

`--baseline-only` captures the unmodified compiler before implementation.
Requires the workspace z88dk, ntvcm, assertions clang and existing
`scratch/dcc-clang-bench/ticks_cpm.py`. Uses only Python's standard library.
All compiler/emulator working directories are created below `scratch/tmp`
and automatically removed. Results and the adjacent artifact directory
(sources, binaries, maps and assembly) are intentionally retained.

The corpus reuses z88dk's `test/suites` callbench, queenbench, sortbench and
sieve without changing workloads. The small replacement `test.h` runs tests
immediately and emits `PASS` only after their assertions finish. It avoids
introducing the test framework's separate callback ABI. No LTO or static
frames are enabled; recursive workloads remain reentrant. Normal compiler
inlining stays enabled, so check retained assembly before attributing any
change to a particular call.

Cells at both `-Os` and `-O2`:

| Cell | Default | Additional annotations |
|---|---|---|
| baseline | No option | None |
| A | sdcccall1 | None |
| B | sdcccall0 | None |
| C | sdcccall0 | Frozen named-function policy in `run.py` |

C uses callee cleanup on callbench's dispatch/apply/compose, queenbench's
safe, and sortbench's qsort_rec/ins_sort/checksum/is_sorted. Queenbench's
single-argument place uses fastcall. Pointer-target signatures are left
unmodified; sieve has no annotations and is a control. This is one explicit
policy, not a claim of optimal annotation coverage or automatic selection.

Correctness precedes scoring: the host compiler and two emulators must pass.
Python independently verifies the prime and queen counts and sorting
checksums using trial division, permutations and `sorted`, plus a masked
integer reference for the call checksum. No-op/A binaries and cycles must
match exactly. A hand-countable prefixed-instruction probe checks ticks
against 63 T-states. Timing covers the entire CP/M executable, including
startup and the fixed console/BDOS shim; it is not kernel-only timing.

Binary size includes everything serialized in the COM file; use retained
linker maps to distinguish code, initialized data, read-only data and BSS.
Do not label COM byte counts as code size. The four workloads are a bounded
initial corpus, not a universal calling-convention performance claim.
