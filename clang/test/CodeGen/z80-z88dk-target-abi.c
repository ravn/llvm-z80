// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -emit-llvm -o - %s | FileCheck %s --check-prefix=Z88DK
// RUN: %clang_cc1 -triple z80 -emit-llvm -o - %s | FileCheck %s --check-prefix=BARE

typedef int (*callback)(int);
extern int puts(const char *);
extern int printf(const char *, ...);
extern int explicit_register(int) __attribute__((sdcccall(1)));
extern int explicit_stack(int) __attribute__((sdcccall(0)));

int ordinary(int value) { return value; }
int indirect(callback f, int value) { return f(value); }
int variadic(int value, ...) { return value; }
int library(const char *s) { return puts(s); }
int library_alias(const char *s) { return __builtin_printf("%s", s); }
int explicit_calls(int value) {
  return explicit_register(value) + explicit_stack(value);
}
int main(void) { return ordinary(7); }

// Z88DK: define{{.*}}z80_sdcccall0 i16 @ordinary(
// Z88DK-LABEL: define{{.*}} @indirect(
// Z88DK: call z80_sdcccall0 i16 %
// Z88DK: define{{.*}}z80_sdcccall0 i16 @variadic(
// Z88DK-LABEL: define{{.*}} @library(
// Z88DK: call z80_sdcccall0 i16 @puts(
// Z88DK-LABEL: define{{.*}} @library_alias(
// Z88DK: call z80_sdcccall0 i16 {{.*}}@printf(
// Z88DK-LABEL: define{{.*}} @explicit_calls(
// Z88DK: call i16 @explicit_register(
// Z88DK: call z80_sdcccall0 i16 @explicit_stack(
// main retains clang's existing entry-point convention.
// Z88DK: define dso_local i16 @main(
// Z88DK: call z80_sdcccall0 i16 @ordinary(

// BARE: define dso_local i16 @ordinary(
// BARE-LABEL: define{{.*}} @indirect(
// BARE: call i16 %
// BARE: define dso_local i16 @variadic(
// BARE-LABEL: define{{.*}} @library(
// BARE: call i16 @puts(
// BARE-LABEL: define{{.*}} @library_alias(
// BARE: call i16 {{.*}}@printf(
// BARE: define dso_local i16 @main(
