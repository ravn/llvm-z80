/* Test 166: i32 loop-carried-value shift loop (ravn/llvm-z80#112 IY residual).
   Isolated from test_04 bit 6.  popcount(0xA5A5A5A5) == 16.
   With -z80-unreserve-iy the i32 loop-carried value lands in IY and the
   loop miscompiles.  Reliable repro via the test-runner oracle. */
typedef unsigned char uint8_t;
typedef unsigned long uint32_t;

int main() {
    volatile uint32_t val = 0xA5A5A5A5UL;
    uint8_t count = 0;
    uint32_t v = val;
    while (v) { count += (v & 1); v >>= 1; }
    return count; /* expect 0x0010 */
}
