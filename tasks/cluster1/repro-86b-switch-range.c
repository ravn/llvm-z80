#include <stdint.h>
extern void f0(void), f1(void), f2(void), f3(void);
extern void def(void);

/* Range-check switch: 4+ cases triggers a range bound check.
 * If v >= 4, jump to default. */
void switch_4(uint8_t v) {
    switch (v) {
    case 0: f0(); break;
    case 1: f1(); break;
    case 2: f2(); break;
    case 3: f3(); break;
    default: def(); break;
    }
}

/* Sparse with high values -- forces 16-bit cmp if widened */
void switch_sparse(uint16_t v) {  /* uint16_t to force 16-bit upfront */
    switch ((uint8_t)v) {
    case 0x10: f0(); break;
    case 0x20: f1(); break;
    case 0x30: f2(); break;
    case 0x40: f3(); break;
    default: def(); break;
    }
}
