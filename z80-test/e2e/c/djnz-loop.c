// REQUIRES: z80, elf
// RUN: %cc -O2 -S -emit-llvm %s -o - | %llc -O2 -o - | FileCheck %s
// RUN: %cc %s -o %t
// RUN: %run --expect-result=0x000F %t

// CHECK-LABEL: _sum_bytes:
// CHECK:       djnz

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

__attribute__((noinline))
uint8_t sum_bytes(const uint8_t *p, uint8_t n) {
    uint8_t sum = 0;
    do {
        sum += *p++;
    } while (--n);
    return sum;
}

__attribute__((noinline))
void countdown_store(uint8_t *p, uint8_t n, uint8_t val) {
    do {
        *p++ = val;
    } while (--n);
}

static const uint8_t data[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };
static uint8_t buf[8];

int main(void) {
    uint16_t status = 0;

    // Test 1: sum_bytes on 4 elements -> 10+20+30+40 = 100
    if (sum_bytes(data, 4) == 100)
        status |= 1;

    // Test 2: sum_bytes on 1 element -> 10
    if (sum_bytes(data, 1) == 10)
        status |= 2;

    // Test 3: sum_bytes on all 8 elements -> 360 & 0xFF = 104
    if (sum_bytes(data, 8) == 104)
        status |= 4;

    // Test 4: countdown_store fill 5 elements
    countdown_store(buf, 5, 0x5A);
    if (buf[0] == 0x5A && buf[4] == 0x5A && buf[5] == 0)
        status |= 8;

    return status;
}
