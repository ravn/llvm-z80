// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -fdefault-calling-conv=sdcccall0 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC0
// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -fdefault-calling-conv=sdcccall0 -O2 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC0
// RUN: %clang_cc1 -triple z80 -fdefault-calling-conv=sdcccall1 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC1
// RUN: %clang_cc1 -triple z80 -emit-llvm -o - %s | FileCheck %s --check-prefix=CC1

typedef unsigned int size_t;
extern int printf(const char *, ...);
extern int puts(const char *);
extern int vsnprintf(char *, size_t, const char *, __builtin_va_list);
extern int explicit_library(int) __attribute__((sdcccall(1)));
extern int putchar(int) __attribute__((smallc));

int console(void) {
  return printf("ALL PASS\n");
}

int direct(const char *s) {
  return puts(s);
}

int forward(char *out, const char *fmt, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, fmt);
  int result = vsnprintf(out, 32, fmt, ap);
  __builtin_va_end(ap);
  return result;
}

int fixed(int value) {
  return explicit_library(value) + putchar(value);
}

int builtin_alias(const char *value) {
  return __builtin_printf("%s", value);
}

// CC0-LABEL: define{{.*}} @console(
// CC0: call z80_sdcccall0 i16 {{.*}}@printf(
// CC0-LABEL: define{{.*}} @direct(
// CC0: call z80_sdcccall0 i16 @puts(
// CC0-LABEL: define{{.*}} @forward(
// CC0: call z80_sdcccall0 i16 @vsnprintf(
// CC0-LABEL: define{{.*}} @fixed(
// CC0: call i16 @explicit_library(
// CC0: call cc129 i16 @putchar(
// CC0-LABEL: define{{.*}} @builtin_alias(
// CC0: call z80_sdcccall0 i16 {{.*}}@printf(

// CC1-LABEL: define{{.*}} @console(
// CC1: call i16 {{.*}}@printf(
// CC1-LABEL: define{{.*}} @direct(
// CC1: call i16 @puts(
// CC1-LABEL: define{{.*}} @forward(
// CC1: call i16 @vsnprintf(
// CC1-LABEL: define{{.*}} @fixed(
// CC1: call i16 @explicit_library(
// CC1: call cc129 i16 @putchar(
// CC1-LABEL: define{{.*}} @builtin_alias(
// CC1: call i16 {{.*}}@printf(
