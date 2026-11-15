// RUN: %cc %s -o %t
// RUN: %run --expect-result=0x002A %t

/* Test SET (HL) / RES (HL) memory bit-manipulation fold on pointer targets. */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

__attribute__((noinline)) static void set_b7(uint8_t *p) { *p |= 0x80; }
__attribute__((noinline)) static void set_b0(uint8_t *p) { *p |= 0x01; }
__attribute__((noinline)) static void res_b7(uint8_t *p) { *p &= ~0x80; }
__attribute__((noinline)) static void res_b0(uint8_t *p) { *p &= ~0x01; }
__attribute__((noinline)) static void set_b3(uint8_t *p) { *p |= 0x08; }
__attribute__((noinline)) static void res_b3(uint8_t *p) { *p &= ~0x08; }

static uint8_t global_flag;

int main(void) {
    uint8_t cell = 0x00;

    set_b7(&cell);
    if (cell != 0x80) return 1;

    set_b0(&cell);
    if (cell != 0x81) return 2;

    set_b3(&cell);
    if (cell != 0x89) return 3;

    res_b7(&cell);
    if (cell != 0x09) return 4;

    res_b0(&cell);
    if (cell != 0x08) return 5;

    res_b3(&cell);
    if (cell != 0x00) return 6;

    /* Global variable test */
    global_flag = 0x55;
    set_b7(&global_flag);
    if (global_flag != 0xD5) return 7;

    res_b0(&global_flag);
    if (global_flag != 0xD4) return 8;

    return 42; /* 0x002A */
}
