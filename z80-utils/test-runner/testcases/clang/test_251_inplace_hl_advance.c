/* expect 0x0001 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-frame -mllvm -disable-lsr */
/*
 * ravn/llvm-z80 #99, #249, #251: In-place HL advance and 16-bit store lowering.
 *
 * Tests the three canonical shapes where an i16 pointer is walked (*p++)
 * concurrently with an i16 counter or loop termination condition:
 *
 * 1. countdown_i16_counter: constant 16-bit store (*p++ = 0) with 16-bit counter.
 * 2. fill_seq: word_fill loop (*p++ = n) with 16-bit counter as data value.
 * 3. f_walk: multi-BB while-loop with 16-bit advance and early-exit check.
 */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define NOINLINE __attribute__((noinline))

/* Shape 1: countdown_i16_counter (#99 / issue-97a) */
NOINLINE
static void countdown_i16_counter(uint16_t *p) {
    uint16_t i = 256;
    do {
        *p++ = 0;
    } while (--i);
}

/* Shape 2: fill_seq (#99 / word_fill benchmark) */
NOINLINE
static void fill_seq(uint16_t *p) {
    uint16_t n = 256;
    do {
        *p++ = n;
    } while (--n);
}

/* Shape 3: multi-BB walk (#249 / #251) */
NOINLINE
static void f_walk(uint16_t *p, uint16_t n) {
    while (n) {
        *p++ = n;
        n--;
    }
}

/* Shape 4: u8 countdown loop with 16-bit word advance (< 256 counter) */
NOINLINE
static void count_u8_walk(uint16_t *p, uint8_t n) {
    do {
        *p++ = 0x55AA;
    } while (--n);
}

static uint16_t buf[256];

int main(void) {
    uint16_t i;

    /* Test 1: fill with non-zero, then clear via countdown_i16_counter */
    for (i = 0; i < 256; ++i)
        buf[i] = 0xAA55;
    countdown_i16_counter(buf);
    for (i = 0; i < 256; ++i) {
        if (buf[i] != 0)
            return 0;
    }

    /* Test 2: fill_seq countdown 256 down to 1 */
    fill_seq(buf);
    for (i = 0; i < 256; ++i) {
        if (buf[i] != (uint16_t)(256 - i))
            return 0;
    }

    /* Test 3: f_walk with count 128 */
    for (i = 0; i < 256; ++i)
        buf[i] = 0;
    f_walk(buf, 128);
    for (i = 0; i < 128; ++i) {
        if (buf[i] != (uint16_t)(128 - i))
            return 0;
    }
    for (i = 128; i < 256; ++i) {
        if (buf[i] != 0)
            return 0;
    }

    /* Test 4: count_u8_walk with count 100 (< 256 u8 counter) */
    for (i = 0; i < 256; ++i)
        buf[i] = 0;
    count_u8_walk(buf, 100);
    for (i = 0; i < 100; ++i) {
        if (buf[i] != 0x55AA)
            return 0;
    }
    for (i = 100; i < 256; ++i) {
        if (buf[i] != 0)
            return 0;
    }

    return 1;
}
