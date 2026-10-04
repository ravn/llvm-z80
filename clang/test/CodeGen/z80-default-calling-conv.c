// RUN: %clang_cc1 -triple z80 -fdefault-calling-conv=sdcccall0 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC0
// RUN: %clang_cc1 -triple z80 -ffreestanding -fdefault-calling-conv=sdcccall0 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC0-FREESTANDING
// RUN: %clang_cc1 -triple z80 -fdefault-calling-conv=sdcccall1 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC1
// RUN: %clang_cc1 -triple z80 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC1
// RUN: %clang -target z80 -fdefault-calling-conv=sdcccall0 -emit-llvm -S -o - %s | FileCheck %s --check-prefix=CC0
// RUN: %clang -target z80 -fdefault-calling-conv=sdcccall1 -emit-llvm -S -o - %s | FileCheck %s --check-prefix=CC1

typedef unsigned short u16;
typedef unsigned long u32;
typedef u16 (*binary_fn)(u16, u16);
typedef u16 (*explicit_call1_fn)(u16, u16) __attribute__((sdcccall(1)));

u16 sum(u16 a, u16 b) { return a + b; }
u16 direct_call(u16 a) { return sum(a, 9); }
u16 indirect_call(binary_fn f) { return f(0x1234, 0x5678); }
u16 explicit_pointer_call(explicit_call1_fn f) { return f(4, 6); }

extern u16 from_other_translation_unit(u16, u16);
u16 cross_tu_call(u16 a) { return from_other_translation_unit(a, 7); }
extern u16 fixed_library_api(u16, u16) __attribute__((sdcccall(1)));
u16 library_boundary_call(u16 a) { return fixed_library_api(a, 5); }

u16 no_arguments(void) { return 0x1234; }
u32 wide_return(u16 a, u16 b) { return (u32)a * b; }
u16 variadic(u16 first, ...) { return first; }
u16 call_variadic(void) { return variadic(3, 5, 7); }

__attribute__((sdcccall(0))) u16 explicit_call0(u16 a, u16 b) {
  return a - b;
}
__attribute__((sdcccall(1))) u16 explicit_call1(u16 a, u16 b) {
  return a + b;
}
__attribute__((smallc)) u16 explicit_smallc(u16 a, u16 b) { return a + b; }
__attribute__((z88dk_fastcall)) u16 explicit_fastcall(u16 a) { return a; }
__attribute__((z88dk_callee)) u16 explicit_callee(u16 a, u16 b) {
  return a + b;
}
__attribute__((smallc)) __attribute__((z88dk_callee))
u16 explicit_smallc_callee(u16 a, u16 b) {
  return a + b;
}
__attribute__((z88dk_callee))
u16 default_base_callee(u16 a, u16 b, u16 c) {
  return a + b + c;
}

int main(void) { return sum(1, 2); }

// CC0: define{{.*}}z80_sdcccall0{{.*}}i16 @sum(
// CC0-LABEL: @direct_call(
// CC0: call z80_sdcccall0{{.*}}i16 @sum(
// CC0-LABEL: @indirect_call(
// CC0: call z80_sdcccall0{{.*}}i16 %{{.*}}(
// CC0-LABEL: @explicit_pointer_call(
// CC0: call{{.*}}i16 %{{.*}}(
// CC0-LABEL: @cross_tu_call(
// CC0: call z80_sdcccall0{{.*}}i16 @from_other_translation_unit(
// CC0: declare{{.*}}z80_sdcccall0{{.*}}i16 @from_other_translation_unit(
// CC0-LABEL: @library_boundary_call(
// CC0: call{{.*}}i16 @fixed_library_api(
// CC0: declare{{.*}}i16 @fixed_library_api(
// CC0: define{{.*}}z80_sdcccall0{{.*}}i16 @no_arguments(
// CC0: define{{.*}}z80_sdcccall0{{.*}}i32 @wide_return(
// CC0: define{{.*}}z80_sdcccall0{{.*}}i16 @variadic(
// CC0-LABEL: @call_variadic(
// CC0: call z80_sdcccall0{{.*}}@variadic(
// Both explicit attributes and their compositions override the default.
// CC0: define{{.*}}z80_sdcccall0{{.*}}i16 @explicit_call0(
// CC0: define{{.*}}i16 @explicit_call1(
// CC0: define{{.*}}cc129{{.*}}i16 @explicit_smallc(
// CC0: define{{.*}}cc130{{.*}}i16 @explicit_fastcall(
// CC0: define{{.*}}cc132{{.*}}i16 @explicit_callee(
// CC0: define{{.*}}cc133{{.*}}i16 @explicit_smallc_callee(
// z88dk_callee modifies the selected default base: CC0 is cc132.
// CC0: define{{.*}}cc132{{.*}}i16 @default_base_callee(
// The runtime entry point keeps the target ABI while its calls use CC0.
// CC0: define dso_local i16 @main(
// CC0: call z80_sdcccall0{{.*}}i16 @sum(
// CC0-FREESTANDING: define dso_local i16 @main(
// CC0-FREESTANDING: call z80_sdcccall0{{.*}}i16 @sum(

// CC1: define{{.*}}i16 @sum(
// CC1-LABEL: @direct_call(
// CC1: call{{.*}}i16 @sum(
// CC1-LABEL: @indirect_call(
// CC1: call{{.*}}i16 %{{.*}}(
// CC1-LABEL: @explicit_pointer_call(
// CC1: call{{.*}}i16 %{{.*}}(
// CC1-LABEL: @cross_tu_call(
// CC1: call{{.*}}i16 @from_other_translation_unit(
// CC1: declare{{.*}}i16 @from_other_translation_unit(
// CC1-LABEL: @library_boundary_call(
// CC1: call{{.*}}i16 @fixed_library_api(
// CC1: declare{{.*}}i16 @fixed_library_api(
// CC1: define{{.*}}i16 @no_arguments(
// CC1: define{{.*}}i32 @wide_return(
// CC1: define{{.*}}i16 @variadic(
// CC1-LABEL: @call_variadic(
// CC1: call{{.*}}@variadic(
// CC1: define{{.*}}z80_sdcccall0{{.*}}i16 @explicit_call0(
// CC1: define{{.*}}i16 @explicit_call1(
// CC1: define{{.*}}cc129{{.*}}i16 @explicit_smallc(
// CC1: define{{.*}}cc130{{.*}}i16 @explicit_fastcall(
// CC1: define{{.*}}cc131{{.*}}i16 @explicit_callee(
// CC1: define{{.*}}cc133{{.*}}i16 @explicit_smallc_callee(
// CC1: define{{.*}}cc131{{.*}}i16 @default_base_callee(
// CC1: define dso_local i16 @main(
// CC1: call{{.*}}i16 @sum(
