/* EXTRA-FLAGS: -mllvm -z80-unreserve-iy -Xclang -target-feature -Xclang +static-stack */
/* Test 168: crc inner 8-step reduction, no outer loop (ravn/llvm-z80#112 IY).
   Isolated IY-heavy inner reduction from crc32.  crc_one(0xFF) low16 == 0xEF8D
   (full = 0x2D02EF8D, verified with host cc). */
typedef unsigned char uint8_t;
typedef unsigned long uint32_t;

uint32_t crc_one(uint32_t crc) {
    for (uint8_t j = 0; j < 8; j++)
        crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320UL : 0);
    return crc;
}

int main() {
    uint32_t r = crc_one(0x000000FFUL);
    return (int)(r & 0xFFFFu); /* expect 0xEF8D */
}
