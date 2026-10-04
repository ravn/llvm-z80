// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -O1 -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -O1 -emit-llvm -o - %s | FileCheck %s --check-prefix=IR

extern int compare(const void *, const void *);
// The classic z88dk header declares qsort smallc, not sdcccall(0).
extern void qsort(void *, unsigned int, unsigned int,
                  int (*)(const void *, const void *))
    __attribute__((smallc));
extern void default_sort(void *, unsigned int, unsigned int,
                         int (*)(const void *, const void *));

void classic_sort(void) {
  qsort((void *)0x1234, 7, 2, compare);
}

// __smallc pushes left-to-right: base is nearest the return address.
// CHECK-LABEL: _classic_sort:
// CHECK:      ld hl,4660
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,7
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,2
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,_compare
// CHECK-NEXT: push hl
// CHECK-NEXT: call _qsort
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: ret

void target_default_sort(void) {
  default_sort((void *)0x1234, 7, 2, compare);
}

// The default C convention in z88dk is __smallc: same left-to-right layout.
// CHECK-LABEL: _target_default_sort:
// CHECK:      ld hl,4660
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,7
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,2
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,_compare
// CHECK-NEXT: push hl
// CHECK-NEXT: call _default_sort
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: ret

int invoke_comparator(const void *left, const void *right) {
  return compare(left, right);
}

// In the IR, __smallc is CallingConv::C (no annotation) in the z88dk environment.
// IR-LABEL: define{{.*}} @classic_sort(
// IR: call void @qsort(
// IR-LABEL: define{{.*}} @target_default_sort(
// IR: call void @default_sort(
// IR-LABEL: define{{.*}} @invoke_comparator(
// IR: call i16 @compare(
