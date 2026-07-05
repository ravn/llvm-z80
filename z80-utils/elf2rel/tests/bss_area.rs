//! Regression test for the local (non-upstream) fix of the elf2rel .bss
//! area bug filed upstream as https://github.com/llvm-z80/llvm-z80/issues/30
//! (root cause writeup: ravn/llvm-z80#253). `.bss` sections used to be
//! routed into the SDCC `_DATA` area by `section_to_area()`, and because
//! they are `SHT_NOBITS`, their zero bytes were materialized directly into
//! that area's byte buffer -- inflating the emitted `.rel` file by the full
//! size of every uninitialized static, even though nothing is actually
//! initialized.
//!
//! Fix (local to this fork only, NOT upstreamed -- see main.rs's SDCC_AREAS
//! comment): `.bss`/`.bss.*` now routes to a dedicated `_BSS` area. For
//! `SHT_NOBITS` sections, only the area's logical size grows; no bytes are
//! appended to the area's byte buffer, so no `T` record is ever emitted for
//! it (matching SDCC's own `_BSS` convention: allocated/zeroed at load time,
//! not file-resident).
//!
//! Fixture: `tests/fixtures/bss_repro.o`, built from
//! `tests/fixtures/bss_repro.c` (a single 4096-byte uninitialized global,
//! `char buf[4096]`, plus one trivial function) via:
//!   clang --target=z80 -Os -ffreestanding -ffunction-sections \
//!     -fdata-sections -c bss_repro.c -o bss_repro.o
//! Confirmed via `llvm-objdump -h bss_repro.o` that `buf` lands in a real
//! `.bss._buf` section of type `BSS` (SHT_NOBITS), size 0x1000.
//!
//! Before the fix: `_DATA` area sized 0x1000 with 4096 literal zero bytes
//! written to the `.rel` file (668 B input -> 22+ KB output).
//! After the fix: `_BSS` area sized 0x1000, zero `T` records, `_DATA` absent
//! entirely (668 B input -> 231 B output, verified manually).

use std::process::Command;

#[test]
fn bss_only_static_does_not_inflate_rel_file() {
    let fixture = concat!(env!("CARGO_MANIFEST_DIR"), "/tests/fixtures/bss_repro.o");
    let out_path = std::env::temp_dir().join(format!(
        "elf2rel_bss_area_test_{}.rel",
        std::process::id()
    ));

    let status = Command::new(env!("CARGO_BIN_EXE_elf2rel"))
        .arg(fixture)
        .arg(&out_path)
        .status()
        .expect("failed to run elf2rel");
    assert!(status.success(), "elf2rel exited with failure");

    let rel_text = std::fs::read_to_string(&out_path).expect("failed to read .rel output");
    let out_size = rel_text.len();
    let _ = std::fs::remove_file(&out_path);

    // `buf` (4096 B, uninitialized) must not show up as _DATA content at
    // all: with nothing else in .data, the _DATA area should be fully
    // absent from the output (empty areas are filtered out, see
    // `active_areas` in main.rs).
    assert!(
        !rel_text.lines().any(|l| l.starts_with("A _DATA")),
        "did not expect a `_DATA` area header line -- buf is .bss, not \
         real data, and there is no other _DATA content in this fixture; \
         got:\n{rel_text}"
    );

    // `buf` must land in a dedicated _BSS area sized exactly 0x1000 (4096).
    let bss_area_line = rel_text
        .lines()
        .find(|l| l.starts_with("A _BSS"))
        .expect("expected an `A _BSS ...` area header line in the .rel output");
    assert!(
        bss_area_line.contains("size 1000 "),
        "expected `_BSS` area sized 0x1000 (4096, sizeof(buf)), got: {bss_area_line:?}"
    );

    // No T-records (data bytes) should ever be emitted for the _BSS area --
    // that's the whole point of the fix. A "T " line only ever follows an
    // "A _BSS" header in this fixture's output, and _BSS must have none.
    let bss_idx = rel_text.find("A _BSS").unwrap();
    let after_bss = &rel_text[bss_idx..];
    assert!(
        !after_bss.lines().skip(1).any(|l| l.starts_with('T')),
        "did not expect any T (data) records following the _BSS area header; \
         _BSS must not be file-resident:\n{after_bss}"
    );

    // Sanity bound on total output size: before the fix this was 22096 B
    // (4096 real zero bytes plus T-record overhead); after the fix it's
    // 231 B. Use a generous threshold well below the old bugged size so the
    // test doesn't need updating for minor unrelated format changes, but
    // still catches any regression back toward materializing .bss content.
    assert!(
        out_size < 1024,
        "expected a small .rel output with no materialized .bss content, \
         got {out_size} bytes (old buggy behavior produced 22096 bytes)"
    );
}
