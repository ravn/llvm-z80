# tasks/tools

Build-and-iterate helpers for llvm-z80 backend work.  Versioned so they
travel with the compiler they target.

## `llvm-snap.sh` — snapshot built binaries for A/B comparison

Backend-pass changes (regalloc heuristics, `.td` flags, peepholes) need
pre/post comparison on the AES corpus or test-runner.  A full
`clang+llc` rebuild is ~120 s; snapshotting binaries is ~2 s.

After each meaningful commit:

```
tasks/tools/llvm-snap.sh save <name>     # snapshot current build-macos/
tasks/tools/llvm-snap.sh list            # list snapshots
tasks/tools/llvm-snap.sh use <name>      # print env-vars
tasks/tools/llvm-snap.sh rm <name>       # delete
```

To A/B against a snapshot:

```
eval $(tasks/tools/llvm-snap.sh use postS3prime)
cd /Users/ravn/z80/rc700-gensmedet/tasks/aes256-corpus
make clean && make CLANG="$CLANG" LLDLD="$LLDLD" \
                   LLVMOBJCOPY="$LLVMOBJCOPY" LLVMNM="$LLVMNM" clang.bin
```

Per-snapshot disk: ~200 MB.  Snapshots live in
`/Users/ravn/z80/llvm-z80/build-snapshots/` (gitignored).

## sccache integration

The build-macos/ cmake cache is wired to `~/.cargo/bin/sccache` via
`CMAKE_C_COMPILER_LAUNCHER`.  Speedup on the typical iteration:

| Action | Pre-sccache | Post-sccache |
|---|---:|---:|
| Touch one .cpp + `ninja llc` | ~120 s | ~18 s (6.5×) |
| Touch .td + `ninja clang llc` | ~120 s | ~42 s (2.9×) |

Cache lives in `~/Library/Caches/Mozilla.sccache/`.  Inspect with
`sccache --show-stats`.  Restart daemon with `sccache --stop-server &&
sccache --start-server`.

The link step dominates remaining time (~25 s for `clang-23` link);
sccache cannot accelerate linking.

If you reconfigure cmake from scratch (delete `build-macos/`), add
`-DCMAKE_C_COMPILER_LAUNCHER=$HOME/.cargo/bin/sccache
-DCMAKE_CXX_COMPILER_LAUNCHER=$HOME/.cargo/bin/sccache` to the cmake
command.
