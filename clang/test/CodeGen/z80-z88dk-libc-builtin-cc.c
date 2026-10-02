// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -emit-llvm -O0 -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -emit-llvm -O2 -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple z80 -emit-llvm -O0 -verify=elf -o - %s | FileCheck %s --check-prefix=ELF
// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -emit-llvm -O2 -o - %s | FileCheck %s --check-prefix=FOLD
// Library builtins must retain classic z88dk header calling conventions.
// Runtime repro: fopen/fwrite read stack arguments, snprintf returns in HL.
typedef unsigned int size_t;
typedef struct FILE FILE;

FILE *fopen(const char *, const char *) __attribute__((smallc)); // elf-warning {{smallc calling convention is not supported on builtin function}}
size_t fwrite(const void *, size_t, size_t, FILE *) __attribute__((smallc)); // elf-warning {{smallc calling convention is not supported on builtin function}}
char *fgets(char *, int, FILE *) __attribute__((smallc));
int snprintf(char *, size_t, const char *, ...) __attribute__((sdcccall(0))); // elf-warning {{sdcccall(0) calling convention is not supported on builtin function}}
size_t strlen(const char *) __attribute__((smallc)); // elf-warning {{smallc calling convention is not supported on builtin function}}

// An ordinary redeclaration inherits the header's convention.
FILE *fopen(const char *, const char *);

// CHECK-LABEL: define{{.*}} @test(
// CHECK: call{{.*}} cc129 ptr @fopen(
// CHECK: call{{.*}} cc129 i16 @fwrite(
// CHECK: call{{.*}} cc129 ptr @fgets(
// CHECK: call{{.*}} z80_sdcccall0 i16 {{.*}}@snprintf(
// ELF-LABEL: define{{.*}} @test(
// ELF: call ptr @fopen(
// ELF: call i16 @fwrite(
// ELF: call cc129 ptr @fgets(
// ELF: call i16 {{.*}}@snprintf(
void test(const char *name, const char *mode, void *data, size_t len,
          char *buf, const char *fmt, int value) {
    FILE *f = fopen(name, mode);
    fwrite(data, 1, len, f);
    fgets(buf, len, f);
    snprintf(buf, len, fmt, value);
}

// The integration must not disable builtin constant-folding.
// FOLD-LABEL: define{{.*}} @folded(
// FOLD-NEXT: entry:
// FOLD-NEXT: ret i16 4
size_t folded(void) {
    return strlen("abcd");
}
