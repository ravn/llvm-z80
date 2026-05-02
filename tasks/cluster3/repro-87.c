#include <stdint.h>
extern uint8_t dst[];
extern const uint8_t src[8];

void copy8_const_src(void) { __builtin_memcpy(dst, src, 8); }
void copy7(void)           { __builtin_memcpy(dst, src, 7); }
void copy13(void)          { __builtin_memcpy(dst, src, 13); }
void copy16(void)          { __builtin_memcpy(dst, src, 16); }
