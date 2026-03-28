/* Z80 edge-case test (auto-generated) */
/* expect 0x0000 */

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned long uint32_t;

__attribute__((noinline))
uint16_t call6(uint16_t a,uint16_t b,uint16_t c,
               uint16_t d,uint16_t e,uint16_t f) {
    return a+b+c+d+e+f;
}

__attribute__((noinline))
uint16_t add2(uint16_t a, uint16_t b) { return a + b; }

__attribute__((noinline))
uint16_t sub2(uint16_t a, uint16_t b) { return a - b; }

static volatile uint16_t g16;

__attribute__((noinline))
uint16_t read_g16(void) { return g16; }

int main(void) {
    int failures = 0;

    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 14: result = 134; break;
        case 4: result = 219; break;
        case 8: result = 204; break;
        case 18: result = 185; break;
        case 7: result = 112; break;
        case 11: result = 70; break;
        case 17: result = 252; break;
        case 13: result = 46; break;
        default: result = 24; break;
        }
        if (result != 219) failures++;
    }

    return failures;
}