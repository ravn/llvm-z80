// REQUIRES: z80, elf
// RUN: %cc -O2 -S -emit-llvm %s -o - | %llc -O2 -o - | FileCheck %s
// RUN: %cc %s %S/packet-parser-opt.s -o %t
// RUN: %run --expect-result=0x002A %t

// CHECK-LABEL: _parse_packet_c:
// CHECK:       ld a,(_rx_status)
// CHECK:       cp (hl)
// CHECK:       inc (hl)

/* Differential test verifying that the C reference implementation and
 * the pure optimized assembler implementation (in packet-parser-opt.s)
 * produce identical results across normal, boundary (len=0), and error conditions. */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

volatile uint8_t rx_status = 0;
uint8_t packet_errors = 0;

/* 1. C reference implementation */
__attribute__((noinline))
uint8_t parse_packet_c(uint8_t *base, uint16_t offset, uint8_t len, uint8_t delim, uint8_t *flags) {
    if (rx_status & 0x01) {
        packet_errors++;
        return 0;
    }

    uint8_t *ptr = base + offset;
    uint8_t matches = 0;

    for (uint8_t i = len; i != 0; --i) {
        if (*ptr == delim) {
            matches++;
            *flags |= 0x80;
        } else {
            (*flags)++;
        }
        ptr++;
    }

    return matches;
}

/* 2. Pure assembler implementation defined in packet-parser-opt.s */
extern uint8_t parse_packet_asm(uint8_t *base, uint16_t offset, uint8_t len, uint8_t delim, uint8_t *flags);

int main(void) {
    uint8_t buffer[16] = {
        0x10, 0x20, 0x30, 0x40,
        'A', 'B', 'X', 'C', 'X', 'D', 'X', 0x55,
        0x66, 0x77, 0x88, 0x99
    };
    uint8_t flags_c = 0;
    uint8_t flags_asm = 0;

    /* Test 1: Normal execution - both must return count=3 and flags=0x84 */
    uint8_t count_c = parse_packet_c(buffer, 4, 7, 'X', &flags_c);
    uint8_t count_asm = parse_packet_asm(buffer, 4, 7, 'X', &flags_asm);
    if (count_c != 3 || count_asm != 3)
        return 1;
    if (flags_c != 0x84 || flags_asm != 0x84)
        return 2;

    /* Test 2: len = 0 (boundary case: 0 bytes processed) */
    flags_c = 0x10;
    flags_asm = 0x10;
    count_c = parse_packet_c(buffer, 4, 0, 'X', &flags_c);
    count_asm = parse_packet_asm(buffer, 4, 0, 'X', &flags_asm);
    if (count_c != 0 || count_asm != 0)
        return 3;
    if (flags_c != 0x10 || flags_asm != 0x10)
        return 4;

    /* Test 3: Error condition (rx_status = 1) */
    rx_status = 1;
    packet_errors = 0;
    count_c = parse_packet_c(buffer, 4, 7, 'X', &flags_c);
    uint8_t err_c = packet_errors;

    packet_errors = 0;
    count_asm = parse_packet_asm(buffer, 4, 7, 'X', &flags_asm);
    uint8_t err_asm = packet_errors;

    if (count_c != 0 || count_asm != 0)
        return 5;
    if (err_c != 1 || err_asm != 1)
        return 6;

    return 42; /* Success: both implementations verified identical */
}
