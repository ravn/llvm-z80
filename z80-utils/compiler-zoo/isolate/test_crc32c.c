/* Isolate: CRC-32 — return the CRC value directly to inspect */
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

/* Return CRC low 16 bits so we can see it in DE */
int main(void) {
    uint8_t data[1];
    data[0] = 0x41;
    uint32_t r = crc32(data, 1);
    /* Expect: 0xD3D99E8B. Return low 16 bits = 0x9E8B */
    return (int)(r & 0xFFFF);
}
