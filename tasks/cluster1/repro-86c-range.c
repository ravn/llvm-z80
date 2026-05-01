#include <stdint.h>
extern void def(void);
typedef void (*fn)(void);
extern fn jt[16];

void switch_many(uint8_t v) {
    switch (v) {
    case 0: case 1: case 2: case 3:
    case 4: case 5: case 6: case 7:
    case 8: case 9: case 10: case 11:
        jt[v]();  break;
    default:
        def(); break;
    }
}
