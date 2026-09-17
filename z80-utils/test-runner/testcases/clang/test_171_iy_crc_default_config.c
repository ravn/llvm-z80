/* EXTRA-FLAGS: -Xclang -target-feature -Xclang -static-frame */
/* Test 171: ravn/llvm-z80#189 default-config (no +static-frame) correctness
   witness.  Same crc_one as test_168 but WITHOUT +static-frame, so spill slots
   are SP-relative (ld hl,N; add hl,sp).  Before the Z80NarrowSubRegGR16 pass the
   loop-carried i32 high half landed in IY and was byte-shuttled (push iy; pop
   rr), perturbing SP under the SP-relative slot access -> crc_one(0xFF) returned
   0x0044 instead of 0xEF8D.  The pass keeps byte-decomposed GR16 values out of
   IX/IY, so the corrupting shuttle cannot form. */
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
