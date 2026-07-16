# Gap analysis: llvm-z80 -> z88dk officially-supported external backend

Date: 2026-07-16.
Branch: ravn/z88dk `rc700-gensmedet-1` (48 commits ahead of upstream/master).

## Model

llvm-z80 is **NOT merged into z88dk**. The model is the same as ez80-clang
(CE-Programming): z88dk ships the bridges/headers/copt rules; the user installs
llvm-z80 separately and points zcc at it via `LLVMZ80EXE` env var or PATH.

z88dk-side deliverable: the `rc700-gensmedet-1` branch cleaned up and submitted
as a PR to z88dk/z88dk.

llvm-z80-side bugs are documented limitations, not z88dk blockers — unless they
make the integration completely unusable.

---

## What is done and working (ravn/z88dk rc700-gensmedet-1)

- `zcc +cpm -compiler=llvmz80` pipeline: zcc.c integration, LLVMZ80EXE env var,
  copt bridge (llvmz80_rules.1 + bridge_postproc.sh + splitquad.pl + fixlabels.pl)
- `-D__CLANG -D__LLVMZ80 --target=z80` preprocessor defines
- `-O<n>` passthrough: zcc -O2 -> clang -O2 (commit ea5699ca)
- BSS kept out of .COM image (commit 62641e9d)
- .quad split for 64-bit global initializers (splitquad.pl, fixes #27)
- .rodata.cstN merged-constant sections emitted (fixes #30)
- **float.h**: IEEE-754 builtins under `__DBL_MANT_DIG__` (fixes #28)
- **stdarg.h** classic + _DEVELOPMENT: `__builtin_va_*` under `__LLVMZ80` (fixes #29)
- ABI bridges (verified under ntvcm): mem*, str[cpy/cmp/cat/chr/ncpy],
  strlen + 7 fastcall-redirect single-arg string fns, atoi/atol, malloc/free/calloc,
  fflush, argc, integer runtime (mulsi3/udivsi3/udivmodsi4 + 16-bit fast aliases + divmodsi4)
- Regression tests: runtime_mem/str/stdlib/intdiv/fastcall_abi_16/nontrivial_demo (ntvcm)
- Non-trivial demo: recursion + structs + sieve + sprintf("%s%d%ld%lu") + 32-bit mul/div/mod

---

## Gap 1 (BLOCKER): Hardcoded LLVMZ80EXE default

```c
// zcc.c line 354
static char *c_llvmz80_exe = "/Users/ravn/z80/llvm-z80/build-macos/bin/clang";
```

This is a macbook-local path. For upstream submission the default must be:
- `clang` (rely on PATH, user ensures the right clang is first), OR
- `llvmz80-clang` (canonical install name), OR
- Empty with a clear error: "LLVMZ80EXE not set — install llvm-z80 and set LLVMZ80EXE"

**Fix: 5 min change in zcc.c.**

---

## Gap 2 (BLOCKER): Incomplete ABI bridges — z88dk#26

These classic functions are still on old stack-ABI bridge (will hang or return garbage):

**string (remaining):** strstr, strspn, strcspn, strpbrk, strncmp, strncat,
strtok/strtok_r, strrchr, strcasecmp/stricmp, strnlen, strsep, strchrnul, strlcat

**stdlib:** strtol, strtod, qsort, bsearch, rand/srand, abs/labs, atexit

**ctype:** isdigit, isalpha, toupper, tolower and siblings

**stdio/fcntl (root: #20/#22/#23):** fopen/fclose/fread/fwrite/fprintf/fscanf all
broken — the classic `cpm_clib.lib` returns `int` in HL; clang reads DE.
Until fixed, file I/O is unavailable under `-compiler=llvmz80`.

Each remaining function needs: classify as `__ZPROTO` two-arg or fastcall
single-arg, rewrite bridge in-place, add ntvcm regression test.

**Estimated effort:** stdio/fcntl layer is the hardest; string/stdlib is
mechanical. Total: 1-2 days of systematic bridge work.

Note: the DE-vs-HL return register is the root of the stdio gap. The clean fix is
a backend change in llvm-z80 to return 16-bit values in HL (aligns with every
other Z80 toolchain); that would collapse all `ex de,hl` bridges to free aliases.
However, that change is llvm-z80-side and can be deferred — the bridges can be
written to handle DE in the meantime.

---

## Gap 3 (BLOCKER): Float library not in z88dk

`llvmz80-softfloat` (Berkeley SoftFloat 3 + nanoprintf) is a standalone project.
For z88dk to "officially support" float under `-compiler=llvmz80`, either:

**Option A:** Ship the compiled softfloat `.lib` in z88dk under `lib/llvmz80/`
(analogous to how z88dk ships `genmath_ez80_z80.lib`, `math32_ez80_z80.lib` etc.
for ez80-clang). zcc auto-links it when float ops are present.

**Option B:** Document it as a separately-installed package (user downloads
`llvmz80-softfloat` and adds `-Llib/llvmz80 -lsoftfloat` to zcc invocation).

Option A is better for "officially supported". Requires building the `.lib` from
`llvmz80-softfloat/src/` targeting z80 and committing the artefact.

Without this, `printf("%f")` and any runtime float are documented as unavailable
unless the user installs softfloat separately. This is an acceptable v1 limitation
if clearly documented.

---

## Gap 4 (IMPORTANT): #267 — jr relaxation, z80asm rejects far branches

At -O1/-O2/-O3 the backend can emit `jr cc` with a target >±127 bytes away;
z80asm (z88dk's assembler) rejects the file. Workaround: build affected files
at `-Oz` or `-O0`. This needs either:
- A backend fix in llvm-z80 (textual asm emitter always relaxes to `jp cc`)
- A post-process sed pass in `bridge_postproc.sh` (rewrite out-of-range jr -> jp)
- Documentation: "use -Oz for safety; -O2/-O3 may fail to assemble"

The sed-pass option is doable entirely in z88dk and would be invisible to users.
It's worth doing regardless of the backend fix timeline.

---

## Gap 5 (CLEANUP): 48 commits contain noise, need squashing

The 48-commit branch includes: macOS build markers, a zsdcc unrelated fix
(`const-expr uint16_t>>8`), merge commits, and iterative "fix the previous fix"
sequences. A submission PR to z88dk/z88dk needs these reorganised into ~6-8
logical commits:

1. zcc: add `-compiler=llvmz80` pipeline (zcc.c + config)
2. llvmz80 copt bridge: rules + postproc + splitquad + fixlabels
3. ABI bridges: mem/str/stdlib/integer-rt
4. Header fixes: float.h, stdarg.h, math.h / INFINITY/NAN
5. Float library (lib/llvmz80/ .lib if Option A above)
6. Tests: test/clang/ regression suite
7. Documentation: z88dk README / llvmz80 user guide note

**Estimated effort:** 2-4 hours of git rebase -i + testing.

---

## Gap 6 (DOCUMENTATION): Known limitations to document in z88dk

These go into a z88dk `doc/` note or the PR description:

| Limitation | Notes |
|------------|-------|
| File I/O (fopen/fwrite/…) unimplemented | DE-vs-HL; bridge work needed |
| `printf("%f")` requires separate softfloat install | or Gap 3 fixed |
| `%e`/`%g` unsupported | permanent nanoprintf design choice |
| `-clib=new` not wired | classic clib only |
| Far `jr cc` at -O2/-O3 may fail z80asm | use -Oz; see #267 |
| TPA: ~49 KB float closure | budget accordingly on 64 KB CP/M |
| 16-bit return in DE (not HL) | visible in inline asm; bridge fns handle it |

---

## Summary: what needs to happen, in order

1. **Fix LLVMZ80EXE default** (zcc.c) — 5 min, required before PR
2. **Complete string/stdlib bridges** (#26) + ntvcm tests — 1 day
3. **Decide on float library** (ship .lib in z88dk or document as separate install)
4. **Add jr-relaxation post-process in bridge_postproc.sh** — half day
5. **Fix stdio/fcntl** (#20/#22/#23) — 1-2 days; OR accept as known limitation
6. **Squash 48 commits** into clean PR-ready sequence — 2-4 hours
7. **Write user doc note** — 1 hour

Items 1 + 2 + 4 + 6 are the minimum for an honest submission.
Items 3 + 5 can be deferred as documented limitations in v1.
