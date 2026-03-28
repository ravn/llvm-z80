/* Isolate: CRC-32 of 'A' = 0xD3D99E8B */
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
    uint16_t status = 0;
    if (r == 0xD3D99E8BUL) status |= 2;
    return status;
}
