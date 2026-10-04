// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -O1 -S -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple z80-unknown-none-z88dk -O1 -emit-llvm -o - %s | FileCheck %s --check-prefix=IR

// No calling-convention attribute: this exercises the target ABI itself.
unsigned int stack_worker(unsigned int, unsigned int, unsigned int)
    __attribute__((noinline));

unsigned int stack_caller(void) {
  return stack_worker(0x1234, 0x5678, 0x1357);
}

// Distinct coefficients make argument permutations change the result.
unsigned int stack_worker(unsigned int a, unsigned int b, unsigned int c) {
  return a + 3 * b + 5 * c;
}

// Push c, b, a; caller removes all six argument bytes, preserving HL.
// CHECK-LABEL: _stack_caller:
// CHECK:      ld hl,4951
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,22136
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,4660
// CHECK-NEXT: push hl
// CHECK-NEXT: call _stack_worker
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: ret

// Entry layout: return at SP, a at SP+2, b at SP+4, c at SP+6.
// CHECK-LABEL: _stack_worker:
// CHECK:      ld hl,2
// CHECK-NEXT: add hl,sp
// CHECK-NEXT: ld c,(hl)
// CHECK-NEXT: inc hl
// CHECK-NEXT: ld b,(hl)
// CHECK-NEXT: ld hl,4
// CHECK-NEXT: add hl,sp
// CHECK-NEXT: ld e,(hl)
// CHECK-NEXT: inc hl
// CHECK-NEXT: ld d,(hl)
// CHECK-NEXT: ld l,e
// CHECK-NEXT: ld h,d
// CHECK-NEXT: add hl,hl
// CHECK-NEXT: add hl,de
// CHECK-NEXT: add hl,bc
// CHECK-NEXT: ld c,l
// CHECK-NEXT: ld b,h
// CHECK-NEXT: ld hl,6
// CHECK-NEXT: add hl,sp
// CHECK-NEXT: ld e,(hl)
// CHECK-NEXT: inc hl
// CHECK-NEXT: ld d,(hl)
// CHECK-NEXT: ld l,e
// CHECK-NEXT: ld h,d
// CHECK-NEXT: add hl,hl
// CHECK-NEXT: add hl,hl
// CHECK-NEXT: add hl,de
// CHECK-NEXT: ex de,hl
// CHECK-NEXT: ld l,c
// CHECK-NEXT: ld h,b
// CHECK-NEXT: add hl,de
// CHECK-NEXT: ret

// IR-LABEL: define{{.*}}z80_sdcccall0{{.*}} i16 @stack_caller(
// IR: call z80_sdcccall0 i16 @stack_worker(i16 noundef 4660, i16 noundef 22136, i16 noundef 4951)
// IR-LABEL: define{{.*}}z80_sdcccall0{{.*}} i16 @stack_worker(

unsigned int var_worker(unsigned int, ...) __attribute__((noinline));

unsigned int var_caller(void) {
  return var_worker(0x1234, 0x5678, 0x1357);
}

unsigned int var_worker(unsigned int a, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, a);
  unsigned int b = __builtin_va_arg(ap, unsigned int);
  unsigned int c = __builtin_va_arg(ap, unsigned int);
  __builtin_va_end(ap);
  return a + 3 * b + 5 * c;
}

// Variadic calls use the same right-to-left order and caller cleanup.
// CHECK-LABEL: _var_caller:
// CHECK:      ld hl,4951
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,22136
// CHECK-NEXT: push hl
// CHECK-NEXT: ld hl,4660
// CHECK-NEXT: push hl
// CHECK-NEXT: call _var_worker
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: pop af
// CHECK-NEXT: ret

// Saved IX moves a to IX+4; va_start must address b at IX+6.
// CHECK-LABEL: _var_worker:
// CHECK:      push ix
// CHECK-NEXT: ld ix,0
// CHECK-NEXT: add ix,sp
// CHECK-NEXT: push af
// CHECK-NEXT: push af
// CHECK-NEXT: push ix
// CHECK-NEXT: pop hl
// CHECK-NEXT: ld bc,6
// CHECK-NEXT: add hl,bc
// CHECK-NEXT: ld b,h
// CHECK-NEXT: ld c,l
// CHECK-NEXT: ld (ix+-2),c
// CHECK-NEXT: ld (ix+-1),b
// CHECK-NEXT: inc bc
// CHECK-NEXT: inc bc
// CHECK-NEXT: ld (ix+-2),c
// CHECK-NEXT: ld (ix+-1),b
// CHECK-NEXT: ld e,(hl)
// CHECK-NEXT: inc hl
// CHECK-NEXT: ld d,(hl)
// The next cursor is IX+8 (c), then advances to IX+10.
// CHECK:      ld l,c
// CHECK-NEXT: ld h,b
// CHECK-NEXT: inc bc
// CHECK-NEXT: inc bc
// CHECK-NEXT: ld (ix+-2),c
// CHECK-NEXT: ld (ix+-1),b
// CHECK-NEXT: ld e,(hl)
// CHECK-NEXT: inc hl
// CHECK-NEXT: ld c,e
// CHECK-NEXT: ld b,(hl)
// The fixed argument is read separately, not from the varargs cursor.
// CHECK:      ld e,(ix+4)
// CHECK-NEXT: ld d,(ix+5)
// CHECK:      add hl,bc
// CHECK-NEXT: ld sp,ix
// CHECK-NEXT: pop ix
// CHECK-NEXT: ret

// IR-LABEL: define{{.*}}z80_sdcccall0{{.*}} i16 @var_caller(
// IR: call z80_sdcccall0 i16 (i16, ...) @var_worker(i16 noundef 4660, i16 noundef 22136, i16 noundef 4951)
// IR-LABEL: define{{.*}}z80_sdcccall0{{.*}} i16 @var_worker(
// IR: call void @llvm.va_start.p0(
// IR: va_arg ptr %ap, i16
// IR: va_arg ptr %ap, i16
