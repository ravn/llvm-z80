// RUN: %clang_cc1 -triple z80 -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64 -emit-llvm -o - %s | FileCheck %s --check-prefix=X86
//
// ravn/llvm-z80 (z88dk#57): the z88dk classic clib declares standard C library
// functions with an explicit Z80 calling convention that is byte-for-byte the
// ABI of the linked worker -- e.g. fwrite/fread/fputc are __smallc (cc132:
// left-to-right stack args, caller cleanup); printf-family is sdcccall(0)
// (cc128).  These names are also recognized C library builtins, and in HOSTED
// mode (the default; -ffreestanding NOT passed) clang's redeclaration-merge
// logic used to STRIP an explicit CC off a builtin and reset it to the default C
// convention (Sema::MergeFunctionDecl, warned by -Wignored-attributes "... not
// supported on builtin function").  On Z80 that silently corrupted the ABI:
// fwrite() was then called passing/returning in the wrong registers/stack
// (HL vs DE + stack imbalance) -> garbage result + hang.  The fix HONORS an
// explicit Z80 CC on such redeclarations instead of resetting it.
//
// Everything here is at -O0 so no libcall simplification runs; the emitted call
// CC comes purely from the frontend merge, isolating the Sema behavior.

typedef struct _FILE FILE;

//======================================================================
// (a) EXACT bug pattern: fwrite (a builtin) declared __smallc keeps cc132.
//======================================================================
extern int fwrite(const void *, unsigned, unsigned, FILE *) __attribute__((z80_smallc));

// CHECK-LABEL: define {{.*}}i16 @call_fwrite(
// CHECK:         call cc132 i16 @fwrite(
// X86-LABEL:   define {{.*}}@call_fwrite(
// X86-NOT:       cc132
int call_fwrite(FILE *f, const void *p) {
  return fwrite(p, 1, 128, f);
}

//======================================================================
// (b) STRUCTURAL VARIATIONS: other builtins, other Z80 CCs.
//======================================================================

// fread, also __smallc -> cc132.
extern int fread(void *, unsigned, unsigned, FILE *) __attribute__((z80_smallc));
// CHECK-LABEL: define {{.*}}i16 @call_fread(
// CHECK:         call cc132 i16 @fread(
int call_fread(FILE *f, void *p) { return fread(p, 1, 64, f); }

// fputc, __smallc -> cc132.
extern int fputc(int, FILE *) __attribute__((z80_smallc));
// CHECK-LABEL: define {{.*}}i16 @call_fputc(
// CHECK:         call cc132 i16 @fputc(
int call_fputc(FILE *f) { return fputc('X', f); }

// memcpy (a builtin) declared __smallc -> cc132 (string/mem family).
extern void *memcpy(void *, const void *, unsigned) __attribute__((z80_smallc));
// CHECK-LABEL: define {{.*}}ptr @call_memcpy(
// CHECK:         call cc132 ptr @memcpy(
void *call_memcpy(void *d, const void *s) { return memcpy(d, s, 16); }

// A DIFFERENT Z80 CC on a builtin: putchar declared sdcccall(0) -> kept.
extern int putchar(int) __attribute__((sdcccall(0)));
// CHECK-LABEL: define {{.*}}i16 @call_putchar(
// CHECK:         call z80_sdcccall0 i16 @putchar(
int call_putchar(void) { return putchar('Z'); }

// The variadic printf-family: printf declared sdcccall(0) kept even though it
// is the most heavily builtin-recognized function.
extern int printf(const char *, ...) __attribute__((sdcccall(0)));
// CHECK-LABEL: define {{.*}}i16 @call_printf(
// CHECK:         call z80_sdcccall0 i16 (ptr, ...) @printf(
int call_printf(int x) { return printf("%d\n", x); }

//======================================================================
// (c) POSITIVE CONTROLS: paths the fix must NOT disturb.
//======================================================================

// A NON-builtin name declared __smallc still keeps cc132 (this path never went
// through the builtin-CC-strip branch; must remain correct).
extern int z88dk_worker(int, int) __attribute__((z80_smallc));
// CHECK-LABEL: define {{.*}}i16 @call_worker(
// CHECK:         call cc132 i16 @z88dk_worker(
int call_worker(int a, int b) { return z88dk_worker(a, b); }

// A builtin declared WITHOUT any explicit CC must stay default C (no spurious
// Z80 CC introduced by the fix): strlen here has no attribute -> plain call.
extern unsigned strlen(const char *);
// CHECK-LABEL: define {{.*}}i16 @call_strlen(
// CHECK:         call i16 @strlen(
// CHECK-NOT:     call cc132 i16 @strlen(
unsigned call_strlen(const char *s) { return strlen(s); }

//======================================================================
// (d) SAFETY / BOUNDARY: a builtin redeclared with the DEFAULT C convention
// explicitly must still be plain C (the guard keys on Z80 CCs only, so a
// default-CC redeclaration takes the normal, unchanged path).
//======================================================================
extern int fflush(FILE *);
// CHECK-LABEL: define {{.*}}i16 @call_fflush(
// CHECK:         call i16 @fflush(
// CHECK-NOT:     call cc1{{[0-9][0-9]}} i16 @fflush(
int call_fflush(FILE *f) { return fflush(f); }
