/* expect 0x0010 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-frame -mllvm -disable-lsr */
/*
 * ravn/llvm-z80#195: loop-carried i32 value with BSS-homed high half under
 * +static-frame.  The BSS-spill->PUSH/POP peephole dropped the back-edge
 * store because its safety scan missed the top-of-loop reload.  Result:
 * the high half was never decremented and popcount32(0xA5A5A5A5) hung.
 *
 * 0xA5A5A5A5 has 16 set bits (4 per byte × 4 bytes).
 */
typedef unsigned long uint32_t;
typedef unsigned char uint8_t;

__attribute__((noinline))
uint8_t popcount32(uint32_t n) {
    uint8_t count = 0;
    while (n) {
        count += (uint8_t)(n & 1);
        n >>= 1;
    }
    return count;
}

int main(void) {
    return (int)(uint8_t)popcount32(0xA5A5A5A5UL); /* expect 16 == 0x0010 */
}
