// RUN: %clang_cc1 -triple z80  -emit-llvm -o /dev/null -verify=z80  %s
// RUN: %clang_cc1 -triple sm83 -emit-llvm -o /dev/null -verify=sm83 %s
//
// ravn/llvm-z80#208: __builtin_z80_im0/1/2 (IM n), __builtin_z80_set_i (LD I,A)
// and __builtin_z80_in/out (IN/OUT) are Z80-only -- SM83 (Game Boy) has no
// interrupt modes, no I register and no port instructions.
// They must be rejected on sm83 with a clean frontend diagnostic (gated on the
// "z80" target feature), while di/ei/halt/nop stay available on both triples.
//
// z80-no-diagnostics

void common(void) {
  __builtin_z80_di();
  __builtin_z80_ei();
  __builtin_z80_halt();
  __builtin_z80_nop();
}

void z80_only(void) {
  __builtin_z80_im0();    // sm83-error {{'__builtin_z80_im0' needs target feature z80}}
  __builtin_z80_im1();    // sm83-error {{'__builtin_z80_im1' needs target feature z80}}
  __builtin_z80_im2();    // sm83-error {{'__builtin_z80_im2' needs target feature z80}}
  (void)__builtin_z80_in(0); // sm83-error {{'__builtin_z80_in' needs target feature z80}}
  __builtin_z80_out(0, 0);   // sm83-error {{'__builtin_z80_out' needs target feature z80}}
  __builtin_z80_set_i(0); // sm83-error {{'__builtin_z80_set_i' needs target feature z80}}
}
