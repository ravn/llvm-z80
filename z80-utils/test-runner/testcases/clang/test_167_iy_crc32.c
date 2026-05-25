/* EXTRA-FLAGS: -mllvm -z80-unreserve-iy -Xclang -target-feature -Xclang +static-stack */
/* Test 167: crc32 i32 loop-carried-value shift loop (ravn/llvm-z80#112 IY).
   Isolated from test_40.  crc32 of single byte 'A' (0x41) == 0xD3D99E8B.
   The loop carries a u32 (crc) shifted right each iteration; with IY
   unreserved the high half lands in IY and may miscompile. */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

uint32_t crc32(const uint8_t *data, uint16_t len) {
    uint32_t crc = 0xFFFFFFFFUL;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320UL : 0);
    }
    return ~crc;
}

int main() {
    uint8_t data[1];
    data[0] = 0x41;
    uint32_t r = crc32(data, 1);
    /* return low 16 bits of the crc; full value is 0xD3D99E8B */
    return (int)(r & 0xFFFFu); /* expect 0x9E8B */
}
