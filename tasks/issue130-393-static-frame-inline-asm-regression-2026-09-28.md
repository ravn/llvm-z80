# autoload-in-c +751 B regression: root cause confirmed — Z80NonReentrant vs. inline-asm ISR epilogues

Tracking: `ravn/rc700-gensmedet#130` (the +751 B `.text` regression since the 2026-07-01
historical minimum) and `ravn/rc700-gensmedet#131`. Backend bug filed as `ravn/llvm-z80#393`.

## User's hypothesis (confirmed)

"Local variables are now allocated on the stack via IX, whereas we could previously
force them into fixed memory addresses, but that mechanism is gone now."

Confirmed with hard evidence, not assumption:
- Current HEAD `autoload-in-c/clang/prom.clang.lis`: **89** `(ix`-relative-addressing
  occurrences.
- Baseline (`d0927323217ff8eb2365c17f889344ff0b569950`, 2026-07-01 minimum)
  `prom.clang.lis`: **0** occurrences.
- IX usage is concentrated in exactly the functions previously identified (in the
  earlier per-function size diff) as the top growth contributors: `_draw_qr` (22),
  `_compare_6bytes` (12), `_check_sysfile` (8), FDC functions (2-8 each).

## Root cause chain (each link independently verified)

1. **The mechanism did change, not just misconfigure.** Baseline used a target
   feature named `+static-stack` (`autoload-in-c` Makefile @ baseline, line 267);
   current Makefile uses `+static-frame` (line 322). `git log -S"static-stack"` in
   llvm-z80 shows commit `539d57ea3373` "Rename static-stack feature -> static-frame;
   rename AutoStaticStack pass to AutoStaticFrame" — but this rename happened
   *inside* a much bigger change: the **2026-09-05/06 upstream-merge reconstruction**
   (`9554fbcb7f24` "Z80: reconstruct backend after upstream merge",
   `de5aaeb411d8` "Fix merge-upstream-2026-09-05: restore fork features and apply
   upstream fixes"). Before that, `AutoStaticStack` was logic embedded directly in
   `Z80FrameLowering.cpp`/`Z80RegisterInfo.cpp` (per `de5aaeb411d8`'s commit message)
   with **no call-graph/interrupt-context safety analysis at all** — it just
   unconditionally allocated static frames for non-recursive functions.
2. **The new pass (`Z80NonReentrant.cpp`, added 2026-09-06,
   commit `59d8fad47f3c` "Allocate non-reentrant function frames in static memory")
   is a real correctness improvement in principle**: it added a call-graph
   reachability analysis so a function shared between two independent execution
   contexts (e.g. mainline code and an ISR) — which could genuinely corrupt data if
   both re-entered the same static frame — is excluded from static-frame placement.
3. **But its inline-asm handling is over-conservative.** `visitContext`'s walk
   follows the artificial edge
   `CG.getCallsExternalNode()->addCalledFunction(nullptr, CG.getExternalCallingNode())`.
   LLVM's stock `CallGraph` construction already routes inline-asm / indirect call
   sites to `CallsExternalNode`. Per LLVM's `CallGraph` convention,
   `ExternalCallingNode`'s callee set is "every externally-linked function in the
   module" — so a single inline-asm instruction anywhere in an ISR's reachable body
   poisons that ISR's entire reachable-set computation to "the whole externally-linked
   program", regardless of any real call relationship.
4. **`autoload-in-c`'s ISRs all call `ei()`** (`clang/intrinsic.h`:
   `static inline void intrinsic_ei(void) { __asm__ volatile("ei"); }`) as a
   completely standard, necessary interrupt epilogue (re-enable interrupts before
   `reti`). This is present at baseline too (`clang/intrinsic.h` is byte-identical
   at baseline and HEAD) — it's not a source regression, it's the backend's
   *handling* of this pre-existing pattern that regressed.
5. **Confirmed with `-mllvm -debug-only=z80-nonreentrant`** on the real `rom.c`:
   all three interrupt contexts (`nothing_int`, `refresh_crt_dma_50hz_interrupt`,
   `floppy_completed_operation_interrupt`) independently print the identical,
   near-total function list — including `main_relocated`, `draw_qr`,
   `check_sysfile` — as "Reachable from multiple contexts", none of which are
   plausibly ISR-reachable by real call edges.
6. **Confirmed with a minimal, isolated 3-way repro** (not just inference from the
   complex production file):

   | ISR variant | `(ix` count in an unrelated helper function |
   |---|---|
   | no ISR at all in the module | 0 (static frame used) |
   | ISR present, empty body | 0 (static frame used — `interrupt` attribute alone isn't the trigger) |
   | ISR calls `ei()` via inline asm | 4 (falls back to IX-relative stack frame) |

   The helper function has zero call relationship with the ISR in any of the three
   variants — the only variable is whether the ISR contains an inline-asm
   instruction.

## Status

- Filed upstream: `ravn/llvm-z80#393` (includes the minimal repro above, verbatim).
- Not yet fixed — no specific fix has been designed or proposed. Candidate
  directions (from the issue, not committed to): recognize trivial clang-intrinsic
  asm blocks (no memory clobbers / no operands, e.g. `ei`/`di`/`halt`/`nop`/`im 2`)
  as not contributing call-graph edges, or track real per-block call targets
  instead of collapsing indirect/asm calls to "reaches everything".
- Not yet confirmed against LLVM core's `CallGraph.cpp` inline-asm-handling source
  directly — the finding is based on `Z80NonReentrant.cpp`'s wiring plus observed
  debug-trace + repro behavior, not a line-by-line read of upstream LLVM's
  `CallGraph` construction code. Flagged as unconfirmed in the filed issue.

## Side finding (unresolved, not yet reported)

Commit `e907981` ("autoload-in-c: replace removed -z80-closed-world with
-ffreestanding") claims `-ffreestanding` "sets the 'Freestanding' module flag
Z80NonReentrant reads" — but `grep -r "Freestanding" llvm/lib/Target/Z80/` returns
zero hits in the current backend source. Either the claim was wrong when written,
or the mechanism was since refactored away by the 2026-09-05/06 reconstruction.
Not chased further this session (that commit's own measurement already found "no
effect", so it's not on the critical path for this regression) — worth a follow-up
look if anyone revisits `-ffreestanding` handling in this backend.
