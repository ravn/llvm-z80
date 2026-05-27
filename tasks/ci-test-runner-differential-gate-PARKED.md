# PARKED: wire the test-runner differential oracles into CI

**Status:** PARKED 2026-05-27 by user ("document thoroughly and park it — I will
not implement it now").  No CI YAML committed.  Until unparked, the gate is run
**locally** (see §6) — Claude runs it after Z80 backend changes.

**Goal:** make the z80-utils test-runner's runtime suite — and especially the two
differential oracles (`-diff-opt`, `-native-oracle`) — a blocking CI lane, so the
runtime-miscompile class (#202 / #204 / #136 and future siblings) is caught
automatically instead of by luck.

---

## 1. Why this is needed

`.github/workflows/z80-ci.yml` is **lit-only**: it builds clang/llc and runs
`ninja -C build check-llvm-codegen-z80` (static FileCheck over
`llvm/test/CodeGen/Z80`).  It does **not** run the z80-utils test-runner, which is
the only thing that *executes* generated code.

Consequence — the recently-added runtime regression tests are **not CI-guarded**:
- `test_175_double_ptr_swap` (guards #204)
- `test_176_loop_fill_o1` (guards #136)
- `test_90_*` / `test_91_*` edge fixtures (the #136 family)
- the `-diff-opt` / `-native-oracle` differential checks themselves

`loop-idiom-fill.ll` *is* in the lit suite, but it runs at `-O2`, whereas #136 is
an `-O1`-only miscompile — so even that lit test does not guard the bug it relates
to.  A relapse of #136/#204/#202 would sail through CI today.

## 2. What the job needs that the lit job doesn't

- **z88dk-ticks** — the Z80 instruction-set emulator the runner uses to execute
  each test and read the result register.  Built from source in the `z88dk`
  repo (`z88dk/src/ticks`, `make`; needs `flex` + `bison`).  *Not present on the
  GitHub runner; not vendored in llvm-z80.*  This is the main new dependency.
- **cargo / Rust** — to build `z80-utils/test-runner` (preinstalled on
  `ubuntu-latest`).
- **a host C compiler** — for `-native-oracle` (clang/gcc; preinstalled).

## 3. Sketch of the job (NOT committed — reference only)

Add a second job to `z80-ci.yml` (or a step after the lit job), reusing the same
clang/llc build:

```yaml
  test-runner-differential:
    name: z80-utils runtime + differential oracles
    runs-on: ubuntu-latest
    timeout-minutes: 120
    steps:
      - uses: actions/checkout@v4
      - uses: hendrikmuhs/ccache-action@v1.2
        with: { key: z80-ci-${{ runner.os }} }
      # 1. Build clang + llc (same cache as the lit job; or `needs:` it and
      #    upload/download build/bin as an artifact to avoid a second LLVM build).
      - run: |
          cmake -C clang/cmake/caches/Z80.cmake -G Ninja -S llvm -B build \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DLLVM_OPTIMIZED_TABLEGEN=ON
          ninja -C build clang llc llvm-objcopy llvm-nm ld.lld
      # 2. Build z88dk-ticks (the emulator).  z88dk is a separate repo; either
      #    add it as a CI checkout (actions/checkout with repository: z88dk/z88dk)
      #    or a thin submodule.  Only src/ticks + its deps are needed.
      - run: |
          sudo apt-get update && sudo apt-get install -y flex bison
          git clone --depth 1 https://github.com/z88dk/z88dk z88dk
          make -C z88dk/src/ticks
          echo "$PWD/z88dk/bin" >> "$GITHUB_PATH"   # z88dk-ticks lands here
      # 3. Run the differential gate (see §4 for the gate logic).
      - run: |
          cd z80-utils/test-runner
          BUILD_DIR=../../build \
            cargo run --release -- clang -full -diff-opt -native-oracle \
            | tee /tmp/run-default.txt
          BUILD_DIR=../../build \
            cargo run --release -- clang -static-stack -full -diff-opt -native-oracle \
            | tee /tmp/run-ss.txt
          # Gate: zero differential failures (see §4 — do NOT use exit code yet).
          if grep -qE '_(DIFFOPT|NATIVE)' /tmp/run-default.txt /tmp/run-ss.txt; then
            echo "::error::differential oracle regression"; exit 1; fi
```

## 4. Gate logic — IMPORTANT

The runner exits `FAILURE` (`!SuiteResult::all_ok()`) on **any** fail *or* fatal.
That is NOT usable as the gate yet, because a clean run still reports **~56
FATAL** even with the emulator present (a mix of pre-existing
known-failing/known-unemulatable cells — these need triage; tracked loosely under
the verify-cluster / #136 era work).  Until those are driven to zero, the gate
must be **oracle-specific**:

> **Pass iff zero `_DIFFOPT` and zero `_NATIVE` lines** appear in the default and
> +static-stack runs.

That is the property that is provably clean *today* (this session): both configs
report 0 DIFFOPT / 0 NATIVE.  Grep for those tags and fail on any hit (as in §3).

A stricter "exit 0" gate becomes possible only after the ~56 FATAL are triaged to
zero or to an allow-list.

## 5. Caveats / gotchas (all learned this session)

- **sm83 is not emulatable** by this z88dk-ticks build — a `-target sm83` run is
  ~431 FATAL (can't execute), DIFFOPT 0.  Do **not** add sm83 to the gate.
- **`-native-oracle` host-int-width caveat:** host `int` is 32-bit vs Z80's
  16-bit, so tests relying on 16-bit `int` wraparound can legitimately diverge;
  mark such tests `NATIVE-SKIP` (only test_94's overlapping-memcpy UB needed it so
  far).  The gate's NATIVE check is clean today.
- **test_36** (mutual recursion) is `SKIP-IF: +static-stack` (non-reentrant);
  keep that.
- **fast-math** must NOT be combined with `-diff-opt` (reassociation legitimately
  changes float results across opt levels — false positives).  omit-fp is safe.
- A second full LLVM build is expensive; prefer `needs:` the lit job + artifact
  the `build/bin` dir, or share the ccache.

## 6. Local invocation (the de-facto gate, run today)

```bash
cd llvm-z80/z80-utils/test-runner
PATH="$PWD/../../../z88dk/bin:$PATH" BUILD_DIR=../../build-macos \
  cargo run --release -- clang -full -diff-opt -native-oracle
PATH="$PWD/../../../z88dk/bin:$PATH" BUILD_DIR=../../build-macos \
  cargo run --release -- clang -static-stack -full -diff-opt -native-oracle
# clean == no _DIFFOPT and no _NATIVE lines (ignore the ~56 known FATAL)
```

## 7. Value when unparked

Catches the entire opt-dependent / consistently-wrong miscompile class
automatically — the class that produced #202, #204, #136 this session and was
previously found only by luck.  This is the durable payoff of the differential
oracles; the oracles exist and are green, so unparking is purely the CI plumbing
above.
