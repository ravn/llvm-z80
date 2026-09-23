/* expect 0xEF8D */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-frame -mllvm -disable-lsr */
/*
 * ravn/llvm-z80#192: i32 select((crc&1)==0, 0, CONST) in a loop miscompiled
 * under +static-frame at O1/Os.  The #173 BSS peephole folded the first
 * compare's result into PUSH/POP DE, clobbering D which is the zero input for
 * the second compare half.  Result: crc_one(0xFF) returned 0xB6662D3D instead
 * of 0x2D02EF8D.
 *
 * We return the low 16 bits of the result: correct = 0xEF8D, buggy = 0x2D3D.
 */
typedef unsigned long uint32_t;
typedef unsigned char uint8_t;

__attribute__((noinline))
uint32_t crc_one(uint32_t crc) {
    for (uint8_t i = 0; i < 8; i++)
        crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320UL : (crc >> 1);
    return crc;
}

int main(void) {
    uint32_t r = crc_one(0xFF);
    return (int)(r & 0xFFFFu); /* low 16 of 0x2D02EF8D == 0xEF8D */
}
