typedef unsigned short u16;

__attribute__((noinline)) u16 from_another_translation_unit(u16 a, u16 b,
                                                             u16 c) {
  return 11 * a + 13 * b + 17 * c;
}

__attribute__((noinline, sdcccall(1)))
u16 library_abi_function(u16 a, u16 b, u16 c) {
  return 19 * a + 23 * b + 29 * c;
}
