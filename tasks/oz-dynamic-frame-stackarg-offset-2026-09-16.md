# -Oz miscompile: dynamic-SP-frame stack-argument read at wrong offset

**Date:** 2026-09-16
**Status:** pre-existing bug report (predates the #326-#331 density work — verified by
git-bisect A/B). **Production autoload is NOT affected** (it builds `+static-frame`,
which is correct). Bug report only — no fix proposed here
(`feedback_file_bugs_not_fixes`).

## Symptom

`z80-utils/test-runner/testcases/clang/test_33_string_ops.c` returns `0x0008`
instead of `0x000F` at **-Oz only** (O0/O1/O2/O3/Os all PASS). This is the sole
FAIL in the clang runtime suite (911 PASS / 1 FAIL / 66 FATAL-known / 258 SKIP,
2026-09-16, peephole-clean HEAD).

## Isolation

Splitting test_33 into one function per file (dynamic frame = suite default):
- `my_strlen` : correct at all opt levels.
- `my_strcmp` : correct at all opt levels.
- `my_strrev` : correct O0..Os, **returns garbage at -Oz**.

So the single root cause is `my_strrev` @ -Oz. In the full test_33 its out-of-bounds
write additionally clobbers neighbouring memory (`status` / adjacent arrays),
turning the isolated single-function failure into the observed 3-bit cascade.

```c
void my_strrev(uint8_t *s, uint8_t len) {   // len is a STACK argument
    for (uint8_t i = 0; i < len / 2; i++) {
        uint8_t t = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = t;
    }
}
```

## Root cause (real disassembly, dynamic frame)

`len` is passed on the stack. At entry the stack is `[ret(2)][len]`, so `len` is at
`SP+2` before any prologue push.

**-Os (correct):** one prologue `push hl` (borrow HL to read the arg), then
`ld hl,#4; add hl,sp; ld d,(hl)` reads `SP+4` = `len` (stack is now
`[hl][ret][len]`). Correct.

**-Oz (wrong):** the prologue emits **two** `push hl` (an extra 2-byte stack
allocation), but the stack-argument read still uses **offset 4**:

```
0: push hl          ; extra frame allocation (NOT accounted for below)
1: push hl          ; borrow HL
2: ld hl,#4
4: add hl,sp        ; hl = SP+4  -> points at the RETURN ADDRESS, not len
6: ld d,(hl)        ; d = ret-addr low byte  (should be len)
7: pop hl
```

With two pushes the stack is `[hl][hl][ret][len]`, so `len` is at `SP+6`. The read
uses `SP+4` and loads the return-address byte instead. Garbage `len` -> wrong
`len/2` loop count and wrong `len-1-i` index -> corrupt reversal + out-of-bounds
store.

**The bug:** an -Oz-only extra prologue stack allocation whose 2 bytes are not
added to the fixed-stack-object (incoming stack argument) offset. Same class as the
historical stack-argument-offset issues noted in CLAUDE.md ("SPILL_GR16 used direct
BSS for stack args (wrong address)"; hasFP=false / mixed-mode-BSS caveats).

## Why production is safe

autoload/rcbios/cpnos build with `+static-frame`. Recompiling `my_strrev` with the
exact autoload flags (`-Oz +static-frame +shadow-regs -disable-lsr
-z80-closed-world`) yields a **single** prologue `push hl` and reads `len` at the
correct `SP+4`; linked + run under z88dk-ticks it returns `0x000F`. The defect is
confined to the dynamic-SP-frame path (no `+static-frame`/`+static-stack`), which
production never uses.

## Detector

`test_33_string_ops.c` @ -Oz is the standing runtime detector (already in the
suite; this is a *known* pre-existing FAIL, not a regression from this session's
work). A minimal repro is the `my_strrev` function above compiled `--target=z80
-Oz` (no static-frame).

## Attribution (git A/B)

Rebuild + `cargo run -- clang test_33` at:
- HEAD (post #327-#330): FAIL @Oz
- `abf84ee5` (#326, pre comparison peepholes): FAIL @Oz
- `abf84ee5^` (pre #326): FAIL @Oz

=> pre-existing; the density peepholes #326-#331 are exonerated.
