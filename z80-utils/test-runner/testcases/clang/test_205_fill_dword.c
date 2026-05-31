/* expect 0x0001 */
/*
 * ravn/llvm-z80#205: a K=4-byte pattern-fill loop (Z80LoopIdiomFill -> the
 * llvm.z80.pattern.fill target intrinsic -> seed store + forward LDIR) must
 * fill correctly at EVERY opt level.  The old overlapping-memcpy form was UB
 * in IR (miscompiled at -O1, #136); a non-rotated loop's trip count also made
 * the fill overrun by one pattern.  Sentinels sit right past the fill region
 * WITHIN THE SAME array (adjacency guaranteed) to catch a one-pattern overrun.
 */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
volatile u32 pv = 0xDEADBEEFUL;
static u32 buf[10];
int main(void) {
    u32 p = pv; int i;
    buf[8] = 0x12345678UL; buf[9] = 0x9ABCDEF0UL;
    for (i = 0; i < 8; i++) buf[i] = p;
    for (i = 0; i < 8; i++) if (((volatile u32 *)buf)[i] != 0xDEADBEEFUL) return 0;
    if (((volatile u32 *)buf)[8] != 0x12345678UL || ((volatile u32 *)buf)[9] != 0x9ABCDEF0UL) return 0;
    return 1;
}
