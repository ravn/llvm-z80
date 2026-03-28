/* Isolate: CRC-32 of 'A' — check high and low 16 bits separately */
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

int main(void) {
    uint8_t data[1];
    data[0] = 0x41;
    uint32_t r = crc32(data, 1);
    uint16_t hi = (uint16_t)(r >> 16);
    uint16_t lo = (uint16_t)(r & 0xFFFF);
    uint16_t status = 0;
    if (hi == 0xD3D9) status |= 1;   /* bit 0: high word correct */
    if (lo == 0x9E8B) status |= 2;   /* bit 1: low word correct */
    if (r == 0xD3D99E8BUL) status |= 4;  /* bit 2: full 32-bit compare */
    return status;
}
