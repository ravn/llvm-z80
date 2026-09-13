# Static-frame regression analysis (llvmz80-23.1.0-r1 / PR #40) — 2026-09-11

## RESOLVED 2026-09-14 (commit `147d6f43`, merged `5fcf1c5b`)

Both of the R3-class hypotheses below (attribute drift, inverted safety gate)
were investigated with real pass output and **ruled out**. The actual root
cause: `Z80NonReentrant.cpp`'s context-reachability walk (the sole gate for
the `"nonreentrant"` attribute, which `usesStaticFrame()` requires) routes
inline-asm calls through the same "opaque call reaches every
externally-callable function" escalation as genuine indirect calls through
function pointers, because LLVM's generic `CallGraph` builder cannot tell
them apart (`Call->getCalledFunction()` returns null for both). The
ubiquitous Z80 ISR epilogue `__asm__ volatile("ei")` was, on its own, enough
to poison every non-`static` function in the module — including totally
unrelated zero-call leaves like `compare_6bytes` — via the pass's artificial
`CallsExternalNode -> ExternalCallingNode` edge combined with stock LLVM
already fanning `ExternalCallingNode` out to every externally-linked
function. Fixed by excluding inline-asm call records (`CallBase::isInlineAsm()`)
from both `markReentrantReachable` and `visitContext`'s traversal. See the
commit message on `147d6f43` for full before/after evidence (autoload: 0→27
functions get `"nonreentrant"`, `add hl,sp` 128→104) and the `#316` GitHub
comment thread. rcbios's separate `.bss` overflow is **not** fixed by this —
confirmed byte-identical before/after — and needs its own investigation
(likely the BIOS jump-vector table's genuinely address-taken shims, a real
reachability concern, not the inline-asm bug).


Root-cause analysis of the rcbios/autoload size regression tracked as
ravn/llvm-z80#314. **The "broad ~14% regression" is one bug, not many:** the
static-frame transformation (auto-inject `+static-frame` on non-reentrant
functions → place their frame in static BSS memory instead of an SP-relative
stack frame) no longer fires after the 23.1.0 merge.

## Symptoms

| Target | Before (baseline `d092732`, 2026-07-12) | After 23.1.0 | Δ |
|---|---|---|---|
| autoload code (excl `_sem702_font`) | 2046 B | 3223 B | **+1177 B / +57.5 %** |
| autoload compressed PROM | 1643 B (fit 2 KB cap) | 2618 B (over 2 KB; only fits temp 4 KB) | +975 B |
| rcbios | linked & booted | `.bss will not fit in region BIOS: overflowed by 1306 bytes` (link FAILS) | — |

rcbios and autoload share one root: the BIOS/PROM region places `.text` then
`.bss` contiguously; a +1.2 KB `.text` bloat pushes rcbios `.bss` past the
interrupt stack at 0xF600 → link overflow. autoload's growth shows directly in
the compressed PROM.

## Mechanism (proven)

Functions with register pressure now build a real SP-relative stack frame and
address every local through `ld hl,off; add hl,sp; ld (hl),r` instead of
staying register-resident or spilling to a fixed BSS address.

- autoload `add hl,sp` occurrences: **115** (after) vs **0** (before).
- autoload `*.frame` BSS symbols: **0** (after) vs **5** (before).

Worked example — `_compare_6bytes`, 20 B → 96 B (×4.8):

- **Before:** register-resident compare loop
  `ld c,l; ld b,h; ld l,e; ld h,d; ld e,6; .L: ld a,(bc); cp (hl); jr nz; inc hl; inc bc; dec e; jr nz; xor a; ret`
- **After:** `push af; push af; dec sp` frame setup, then the pointers and the
  counter are stored to and reloaded from `add hl,sp` slots every iteration.

## The transformation is inert, not merely gated off

- Both passes ARE in the pipeline (`-debug-pass=Structure` shows "Z80
  non-reentrant function analysis" and "Z80 static frame allocation").
- Their enabling flags default ON:
  - `z80-enable-auto-static-frame` — `cl::init(true)` (Z80AutoStaticFrame.cpp:53)
  - `z80-static-frames` / `useStaticFrames()` — `BOU_UNSET → true` (Z80TargetMachine.cpp:187)
- Yet `-Oz +static-frame` DEFAULT output is **byte-identical** to explicit
  `-mllvm -z80-enable-auto-static-frame=false` (both 23 `add hl,sp` on a
  spill-forcing probe). So the auto-inject pass runs but injects nothing
  effective — or the injected attribute is no longer honored downstream.

## Where the fix likely lives (NOT yet patched — analysis only)

The pass runs but its per-function effect is null. Two R3-class hypotheses:
1. The AutoStaticFrame safety/non-reentrancy gate now rejects every candidate
   (predicate inverted or an API it queries changed meaning in 23.1.0).
2. Attribute-name drift: the string AutoStaticFrame writes (`+static-frame`?)
   no longer matches what `Z80StaticFrameAlloc` / `Z80FrameLowering` reads —
   the same rename class as R3 (`-z80-unreserve-iy`) and the stale flag comment
   here (references `-z80-auto-static-frame` while the real flag is
   `-z80-enable-auto-static-frame`).

Next investigator: dump MIR after `Z80AutoStaticFrame` and after
`Z80StaticFrameAlloc` on the `spill.c` probe; check whether the function
carries the static-frame attribute post-inject and whether StaticFrameAlloc
reads that exact key.

## Reproduce

```
cat > /tmp/spill.c <<'EOF'
void sink(int);
void f(int*p){ int a=p[0],b=p[1],c=p[2],d=p[3],e=p[4],g=p[5],h=p[6];
  sink(a);sink(b);sink(c);sink(d);sink(e);sink(g);sink(h);
  sink(a+b);sink(c+d);sink(e+g);sink(h+a); }
EOF
build-macos/bin/clang --target=z80 -Oz -Xclang -target-feature -Xclang +static-frame -S -o - /tmp/spill.c | grep -c 'add	hl,sp'
# after: 23   (regressed)   before-fix expectation: 0
```

Autoload per-function delta table: parse committed `.lis` at `d092732` vs a
fresh build (perl hex-span parser in the session log).
