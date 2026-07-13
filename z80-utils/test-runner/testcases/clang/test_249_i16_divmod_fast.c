/* Test 249: i16 signed/unsigned div+mod correctness (ravn/llvm-z80 #244).
 *
 * At -O3 the backend routes these to the fully-unrolled __*hi3_fast runtime
 * variants; every other opt level uses the small default routines.  Running
 * this fixture under -full (which includes O3) proves the unrolled routines
 * return bit-identical results to the small ones across the algorithm's two
 * internal paths: the 8-bit-divisor fast path (divisor < 256) and the general
 * 16-bit path (divisor >= 256), plus every sign combination.
 */
typedef unsigned short uint16_t;
typedef short int16_t;

int main() {
    uint16_t status = 0;

    /* Bit 0: unsigned, 8-bit-divisor fast path: 60000/200==300, 60000%200==0 */
    {
        volatile uint16_t a = 60000, b = 200;
        if (a / b == 300 && a % b == 0) status |= (1 << 0);
    }

    /* Bit 1: unsigned, 8-bit path with remainder: 1000/7==142, 1000%7==6 */
    {
        volatile uint16_t a = 1000, b = 7;
        if (a / b == 142 && a % b == 6) status |= (1 << 1);
    }

    /* Bit 2: unsigned, 16-bit-divisor path (divisor >= 256): 50000/1000==50 */
    {
        volatile uint16_t a = 50000, b = 1000;
        if (a / b == 50 && a % b == 0) status |= (1 << 2);
    }

    /* Bit 3: unsigned, 16-bit path with remainder: 65535/511==128, rem 127 */
    {
        volatile uint16_t a = 65535, b = 511;
        if (a / b == 128 && a % b == 127) status |= (1 << 3);
    }

    /* Bit 4: signed neg/pos: -1000/7==-142, -1000%7==-6 (C truncates toward 0) */
    {
        volatile int16_t a = -1000, b = 7;
        if (a / b == -142 && a % b == -6) status |= (1 << 4);
    }

    /* Bit 5: signed pos/neg: 1000/-7==-142, 1000%-7==6 */
    {
        volatile int16_t a = 1000, b = -7;
        if (a / b == -142 && a % b == 6) status |= (1 << 5);
    }

    /* Bit 6: signed neg/neg: -1000/-7==142, -1000%-7==-6 */
    {
        volatile int16_t a = -1000, b = -7;
        if (a / b == 142 && a % b == -6) status |= (1 << 6);
    }

    /* Bit 7: divide by 1 and self: 12345/1==12345, 12345/12345==1 */
    {
        volatile uint16_t a = 12345;
        if (a / 1 == 12345 && a / a == 1) status |= (1 << 7);
    }

    /* Bit 8: signed 16-bit divisor: -30000/1000==-30, -30000%1000==0 */
    {
        volatile int16_t a = -30000, b = 1000;
        if (a / b == -30 && a % b == 0) status |= (1 << 8);
    }

    return status; /* expect 0x01FF = 511 */
}
