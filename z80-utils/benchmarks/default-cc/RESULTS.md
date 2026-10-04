# Initial default-CC results (2026-10-04)

Experimental branch: `experiment-default-cc-ab-20261004`, based on
`cdf7fbabe800`. Compiler changes are confined to the experiment branch. No production
default was changed. See README for the fixed corpus and annotation policy.

The real driver accepts `-fdefault-calling-conv=sdcccall0` and `sdcccall1`
for C on Z80. A uses 1, B uses 0, C uses 0 plus the documented annotations.
Explicit external conventions and compiler-generated runtime calls remain
fixed. Under default 0, standalone `z88dk_callee` selects
`Z80_SDCCCall0Callee` (cc132), not the register-base callee convention.

## Correctness and controls

All 32 cells completed and passed their assertions under ntvcm and ticks.
Native compilation plus independent Python algorithms verify expected
counts/checksums. The prefixed-instruction cycle probe yields the
hand-calculated 63 T-states. No-flag versus explicit-1 binaries and cycles
are identical at both optimization levels for all four benchmarks. The final
no-flag artifacts and cycles also match the pre-implementation baseline.

Latest regression gates: clang runtime 438 PASS / 6 SM83-only SKIP,
runner lit 155 PASS, SDCC cross-compiler runtime 174 PASS and active z88dk
integration 14/14 PASS. Agent-run focused frontend/backend lit: 5/5 PASS;
new runtime fixtures: 12/12 PASS. A broader Rust unit-test run had two
failures reported by the implementation agent:
`keeps_dirs_owned_by_a_live_process` and
`rel_link_error_is_fatal_even_when_ihx_exists`. They were not fixed here;
that broader suite is not green and no baseline comparison was captured
to independently establish their pre-existing status.

## Measurements

Bytes below are linker-map code + rodata + initialized data, not COM bytes.
Cycles cover complete execution including CRT and the fixed console shim.
Positive deltas mean more cycles (slower); all deltas are relative to A.

| Workload | Opt | A bytes | B bytes | C bytes | A cycles | B cycle delta | C cycle delta |
|---|---|---:|---:|---:|---:|---:|---:|
| callbench | Os | 6215 | 6377 | 6379 | 36,742,247 | +26.83% | +30.06% |
| callbench | O2 | 6183 | 6208 | 6210 | 27,427,016 | +8.63% | +13.50% |
| queenbench | Os | 5965 | 5981 | 5983 | 21,066,306 | +18.72% | +20.51% |
| queenbench | O2 | 5967 | 5983 | 5985 | 21,054,504 | +18.73% | +20.52% |
| sortbench | Os | 6933 | 6981 | 6988 | 22,066,097 | +0.96% | +1.09% |
| sortbench | O2 | 6950 | 7000 | 7005 | 22,200,668 | +0.96% | +1.09% |
| sieve | Os | 5950 | 5948 | 5948 | 6,079,413 | -0.85% | -0.85% |
| sieve | O2 | 5986 | 5984 | 5984 | 6,079,369 | -0.85% | -0.85% |

B/C BSS is 2 bytes larger for callbench and 4 bytes smaller for queenbench
than A; sortbench/sieve BSS is unchanged. Full COM sizes, individual section
sizes, source/compiler/binary hashes and cycles are in
`scratch/tmp/default-cc-final.json` at the workspace root. Its adjacent
artifact directory retains source, COM, assembly and linker maps.

## Interpretation and limitations

In these LLVM-Z80 workloads, register-default A reduces cycles substantially
for indirect calls and N-queens, barely for sorting, and loses slightly for
sieve. The documented fastcall/callee policy does not recover the difference
here; it is not an optimal-policy claim. Normal inlining remains enabled;
retained queenbench assembly confirms real calls to safe and recursive
place survive. Microbenchmark and recursive-call results cannot establish
a universal percentage for applications or another compiler.

Earlier preliminary C scores used an incorrect register-base callee
composition under default 0. They are superseded by this entire rerun.
Do not cite them as evidence for sdcccall(0) plus callee cleanup.

Commands have a 180-second timeout; ticks also has a 500-million-cycle
limit, which is rejected as a completed measurement. No benchmark hit
either limit. This run exercises C on Z80 only, not SM83, C++, LTO or
multi-compiler comparisons. Multi-TU and callback ABI correctness are
covered by runtime fixtures, not a separate multi-TU benchmark workload.
