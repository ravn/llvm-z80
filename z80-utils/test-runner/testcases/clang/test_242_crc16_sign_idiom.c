/* Test 242: CRC-16/CCITT via the natural `if (crc & 0x8000)` idiom.
   The middle-end canonicalises `crc & 0x8000` (sign-bit test) to a signed
   16-bit compare against -1 (`sgt crc, -1`).  The Z80 ISel must lower that as a
   one-instruction sign test (ADD A,A; JR C/NC), NOT a full 16-bit
   `LD HL,0xFFFF; SBC HL,rr` subtraction — otherwise the inner bit loop is
   ~20 instructions instead of ~5.  This fixture validates the *value* is still
   correct after that ISel optimisation (the lit test pins the instructions).

   crc16(buf[16], buf[i] = i*7+13) == 0xFA16 (host-cc verified). */
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;

static uint16_t crc16(const uint8_t *d, uint16_t n) {
    uint16_t crc = 0xFFFF;
    uint16_t i;
    uint8_t  j;
    for (i = 0; i < n; i++) {
        crc ^= (uint16_t)((uint16_t)d[i] << 8);
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (uint16_t)((crc << 1) ^ 0x1021);
            else
                crc = (uint16_t)(crc << 1);
        }
    }
    return crc;
}

int main(void) {
    uint8_t buf[16];
    uint16_t i;
    for (i = 0; i < 16; i++)
        buf[i] = (uint8_t)(i * 7 + 13);
    return (int)crc16(buf, 16); /* expect 0xFA16 */
}
