typedef unsigned short u16;
typedef unsigned long u32;
typedef u16 (*weighted_fn)(u16, u16, u16);
typedef u16 (*library_callback_fn)(u16, u16, u16)
    __attribute__((sdcccall(1)));
typedef u16 (*callee_fn)(u16, u16, u16) __attribute__((z88dk_callee));

__attribute__((noinline)) u16 weighted(u16 a, u16 b, u16 c) {
  return 3 * a + 5 * b + 7 * c;
}

__attribute__((noinline)) u16 apply(weighted_fn fn, u16 a, u16 b, u16 c) {
  return fn(a, b, c);
}

__attribute__((noinline)) u32 multiply32(u32 a, u32 b) { return a * b; }

__attribute__((noinline, z88dk_callee))
u16 default_base_callee(u16 a, u16 b, u16 c) {
  return 19 * a + 23 * b + 29 * c;
}

extern u16 from_another_translation_unit(u16, u16, u16);
extern u16 library_abi_function(u16, u16, u16)
    __attribute__((sdcccall(1)));

int main(void) {
  volatile u16 stack_guard = 0x5a5a;
  weighted_fn cross_tu_callback = from_another_translation_unit;
  library_callback_fn library_callback = library_abi_function;
  callee_fn callee_callback = default_base_callee;

  if (weighted(11, 13, 17) != 217)
    return 0xe1;
  if (apply(weighted, 3, 5, 7) != 83)
    return 0xe2;
  if (from_another_translation_unit(2, 3, 4) != 129)
    return 0xe3;
  if (apply(cross_tu_callback, 2, 3, 4) != 129)
    return 0xe4;
  if (multiply32(12345, 17) != 209865)
    return 0xe6;
  if (library_abi_function(1, 2, 3) != 152)
    return 0xe7;
  if (library_callback(1, 2, 3) != 152)
    return 0xe8;
  if (default_base_callee(1, 2, 3) != 152)
    return 0xe9;
  if (callee_callback(1, 2, 3) != 152)
    return 0xeb;

  for (u16 i = 0; i != 4; ++i)
    if (from_another_translation_unit(2, 3, 4) != 129)
      return 0xe5;

  return stack_guard == 0x5a5a ? 0xd9 : 0xea;
}
