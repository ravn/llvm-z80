# Backend-research findings investigated but NOT filed as ravn/llvm-z80 issues

Reference doc spanning sessions 60c–60f.  Each entry: pattern, why
investigated, evidence checked, why not filed, and re-investigation
triggers.

The point of this doc: if circumstances change (codebase grows,
regalloc shape shifts, new witnesses appear), these are the
deferred candidates worth revisiting.  Prevents future investigators
from re-treading the same ground without context.

## A — Not compiler-attributable (source / project / ABI)

### A1.  `recv_byte_t` API: replace `uint16_t r` + 0xFFFF sentinel with `bool recv(uint8_t *out)`

  - **Considered**: rcvmsg_c analysis surfaced ~30 B of inherent
    overhead from the wide-return-with-sentinel idiom that no
    compiler can eliminate while honouring the C function signature.
  - **Why not filed (in ravn/llvm-z80)**: the fix is a source-level
    rewrite in `rc700-gensmedet/cpnos-rom/transport.h` and call
    sites — not a compiler change.  Per user's corpus-mining
    workflow, source-side fixes for compiler-attributable bloat
    are off-corpus.  Briefly filed in rc700-gensmedet earlier this
    session series, then withdrawn.
  - **Re-investigate when**: never (architectural cost, not a bug).
    Note in case a custom Z80 intrinsic for carry-return APIs is
    ever scoped — would change the analysis.

### A2.  Missing `z80_preserves_regs` on `xport_recv_byte`

  - **Considered**: cpnos.lis has 4 occurrences of `ex de, hl;
    ld de, (slot)` in snios_sndmsg_force / snios_rcvmsg_c that
    looked like dead EX instructions.  Analysis showed the EX is
    forced by the BSS spill of `hcs` across `recv_byte_t` calls.
  - **Why not filed**: the spill exists because clang assumes
    `xport_recv_byte` clobbers HL (default sdcccall(1) caller-
    saves).  Declaring it `z80_preserves_regs("h","l")` would
    eliminate the spill and the EX entirely.  Source-level
    annotation in `transport.h`, not a compiler issue.
  - **Re-investigate when**: source change would be cheap; pursue
    if cpnos source work resumes.

### A3.  RST-instruction placement for hot call targets

  - **Considered**: `_xport_send_byte` called 13 times; `_recv_byte_t`
    12 times.  Z80 `RST n` is 1 byte (vs 3-byte `CALL nn`).
    Potential ~50 B savings on cpnos.
  - **Why not filed**: routing routines to RST vector addresses
    (0x00, 0x08, ..., 0x38) is a **linker/source-level placement**
    decision, not compiler-attributable.  The Z80 backend can
    already emit RST instructions; what's missing is the project's
    linker script / source attributes to place hot routines at
    RST addresses.
  - **Re-investigate when**: cpnos density goal becomes urgent
    (currently 1904 B vs 2048 B ceiling — 144 B headroom).  RST
    optimization would also need verification that the RST
    addresses don't conflict with cpnos's existing PROM layout.

### A4.  `pop h; jmp common_exit` inter-procedural unwind trick

  - **Considered**: hand-asm `RCVMSG` discards a nested-call return
    address with `pop h` (1 byte) to escape directly to the outer
    frame's error handler.  ~12 B saved on rcvmsg_c.
  - **Why not filed**: no realistic compiler implementation path
    on portable C semantics — would require structured exception
    handling, setjmp/longjmp, or a Z80-specific intrinsic.  The
    setjmp/longjmp form on Z80 is net-negative (returns_twice
    semantics force locals to memory, worse than the existing
    enum-return propagation).
  - **Re-investigate when**: never on standard C surfaces.
    Possibly if a Z80-specific control-flow intrinsic is ever
    designed.

## B — No witness found / not currently a surface

### B1.  Memory-operand arithmetic (`ADD A,(HL)`, `OR A,(HL)`,
`AND A,(HL)`, `XOR A,(HL)`, `SUB (HL)`, `ADC A,(HL)`, `SBC A,(HL)`)

  - **Considered**: Z80 has 1-byte forms of arithmetic ops that
    take (HL) as the second operand.  Saves 1-2 B when applicable
    (`ld a, (hl); add a, r` → `ld r, a; add a, (hl)` etc.).
  - **Why not filed**: grep'd cpnos.lis for `ld a, (hl)` followed
    by `add/or/and/sub/xor a, <reg>` — no hits.  The patterns
    that do exist are all either store-after-load (saved to a
    register) or test-only (`or a` for zero check).  No
    arithmetic-with-(HL) opportunities currently survive.
    `CP (HL)` (the only one already in use) fires 5 times.
  - **Re-investigate when**: a new codebase added to the corpus
    has different patterns.  rcbios is a likely source — it has
    more byte-twiddling than cpnos's protocol parsers.

### B2.  Small-N `inc hl × N` vs `ld de, N; add hl, de`

  - **Considered**: for offset N ∈ {1, 2, 3}, `inc hl × N` is
    shorter than `ld de, N; add hl, de` (3 B vs 4 B for N=3, etc.).
  - **Why not filed**: cpnos.lis has 0 instances of `ld de, N;
    add hl, de` where N < 4.  All existing instances are N ∈ {5, 4}
    where the LD-DE form is already optimal or equal.  clang's
    cost model appears to handle this correctly already.
  - **Re-investigate when**: any new codebase that does small-
    offset pointer arithmetic shows the pattern.

### B3.  Direct addressing for byte loads (`ld hl, n; ld a, (hl)` →
`ld a, (n)`)

  - **Considered**: per CLAUDE.md #45 ("Direct addressing for
    constant address loads/stores"), this is documented as working.
    Verified empirically — no `ld hl, n; ld a, (hl)` patterns in
    cpnos where direct form would have applied.
  - **Why not filed**: already optimal in current cpnos.
  - **Re-investigate when**: regression suspected.

### B4.  `inc (HL)` / `dec (HL)` for in-memory increment/decrement

  - **Considered**: hand-asm idiom `(*p)++` could compile to
    `inc (hl)` if p is in HL.  Currently might be `ld a, (hl);
    inc a; ld (hl), a` (3 B vs 1 B).
  - **Why not filed**: cpnos.lis has 1 `inc (hl)` use (at edca-area
    keyboard ISR), where clang has already applied this.  No
    obvious missing-application sites found.
  - **Re-investigate when**: code with more memory-state mutation
    (counters, queue heads, etc.) is added to the corpus.

### B5.  Dead-load detection (consecutive `ld r, X; ld r, Y`)

  - **Considered**: post-RA dead-store elimination for redundant
    loads.
  - **Why not filed**: no witnesses in cpnos.  clang's existing
    DCE catches these reliably.
  - **Re-investigate when**: a target adds new lowering paths
    that bypass DCE.

### B6.  Switch / jump-table lowering bloat

  - **Considered**: dispatch via `JP (HL)` / `JP (IY)` for u8
    switches, vs chained `cp X; jr z` cascade.
  - **Why not filed**: cpnos.lis has 0 `JP (HL)` / `JP (IY)`
    dispatch and only 2 `cp X; jr z` chains (not enough to
    suggest a switch-lowering surface).  CLAUDE.md mentions
    issue #86 work on u8-switch-range; the optimization appears
    to be doing its job in current code.
  - **Re-investigate when**: a function with a real switch
    statement is in scope (rcbios has command-byte dispatch,
    likely a source).

### B7.  `dec a` for `A == 1` (in branched test, not just A==K)

  - **Partial overlap with #148**: #148 specifically covers `xor
    $1; jr {z,nz}` → `dec a; jr {z,nz}` when A is dead.
  - **Considered also**: `cp $1; jr z` (same 4 B as xor form,
    but preserves A).  Could be `dec a; jr z` only when A is dead.
    Equivalent fix; #148 covers it.
  - **Not separately filed** — folded into #148's generalisation
    section.

## C — Already firing / already optimal

### C1.  `xor a` to zero A (vs `ld a, 0`)

  - cpnos has many `xor a` instances; 0 `ld a, 0` instances.
    Already optimal.

### C2.  `cpl` for `~A` (vs `xor $ff`)

  - 2 `cpl` uses in cpnos.lis; 0 `xor $ff`.  Already firing.

### C3.  Tail-call optimization (CALL nn; RET → JP nn)

  - Per CLAUDE.md "Working Optimizations".  Empirically confirmed
    in multiple cpnos functions (e.g. `delete_line: jp $ef3d`
    instead of `call; ret`).

### C4.  LDIR/LDDR for memcpy and `__builtin_memcpy`

  - Working per CLAUDE.md; confirmed in `scroll_lines` (uses
    both LDIR and LDDR).

### C5.  Single-call-site inlining

  - Per CLAUDE.md.  `try_recv_frame` and `try_send_frame` are
    fully inlined into their callers; no separate symbols.

### C6.  Conditional RET pattern (`branch over RET` → `RET cc`)

  - Per CLAUDE.md.  6 conditional RETs in cpnos.lis (across all
    functions).  Likely a few more opportunities exist but no
    clear witness — could revisit if higher-priority work runs out.

### C7.  Multiplication by power-of-2 constant via `ADD HL, HL` chain

  - `scroll_lines` mul-by-80 uses `add hl, hl × 7` for 80x scaling.
    Hand-rolled-quality.

## D — Subset / superseded by other filed issues

### D1.  `xor $K; jr z` for arbitrary K vs `cp K; jr z`

  - Both 4 B, no cost difference.  The `dec a` / `inc a` short
    forms only apply for K ∈ {1, 0xFF} — covered by #148.

### D2.  i16 select-on-equality short-circuiting

  - Covered by #144 (i16 select-on-equality bloat).

### D3.  i8 zext for compare with constant <128

  - Covered by #142 (i8→i16 zext residual).

## E — Future-corpus candidates (not yet investigated)

These haven't been investigated; flagged as candidates for when
the corpus grows:

  - **rcbios-in-c codebase**: 5961 B clang.  Different code style
    (BIOS hot loops, FDC drivers, screen handling).  Likely surfaces
    different gaps than cpnos's protocol parsers.
  - **autoload-in-c codebase**: ~1750 B clang.  Cold-init heavy;
    likely surfaces issues around hardware-state setup.
  - **`_isr_crt`** (95 B) and **`_specc`** (96 B) in cpnos:
    inspected briefly, no obvious new patterns found, but a
    full analysis pass not yet done.
  - **`_netboot_mpm`** (206 B) in cpnos: cold-init / protocol
    handshake; not yet analysed.

## How this doc is maintained

  - Entries added when an investigation concludes without filing.
  - Entries promoted to filed issues if circumstances change (new
    witness, new corpus source, etc.) — leave a forwarding note
    pointing at the issue.
  - Entries demoted to "future-corpus candidates" if they require
    work outside the current corpus (new function analysis, etc.).

Cross-referenced by individual session docs (60c–60f) for
trace-back of when each finding was investigated.
