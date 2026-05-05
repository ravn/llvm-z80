# External references — reading list for later

Curated external links worth reading when working on Z80 codegen
quality, SDCC comparison, or peer-target optimization patterns.

## SDCC code optimization

- **z88dk wiki: Writing Optimal Code** —
  <https://github.com/z88dk/z88dk/wiki/WritingOptimalCode>
  Tips for writing C that SDCC compiles well on Z80.  Useful for two
  parallel reasons: (a) the rcbios sources should follow these
  patterns where they don't conflict with clang/llvm-z80; (b) the
  patterns themselves are a checklist of optimisations the llvm-z80
  backend should match or beat.  Filed 2026-05-05 by user.
