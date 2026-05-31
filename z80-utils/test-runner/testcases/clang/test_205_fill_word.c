/* expect 0x0001 */
/*
 * ravn/llvm-z80#205: a K=2-byte pattern-fill loop (Z80LoopIdiomFill -> the
 * llvm.z80.pattern.fill target intrinsic -> seed store + forward LDIR) must
 * fill correctly at EVERY opt level.  The old overlapping-memcpy form was UB
 * in IR (miscompiled at -O1, #136); a non-rotated loop's trip count also made
 * the fill overrun by one pattern.  Sentinels sit right past the fill region
 * WITHIN THE SAME array (adjacency guaranteed) to catch a one-pattern overrun.
 */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
volatile u16 pv = 0x1234;
static u16 buf[12];
int main(void) {
    u16 p = pv; int i;
    buf[10] = 0xAAAA; buf[11] = 0x5555;
    for (i = 0; i < 10; i++) buf[i] = p;
    for (i = 0; i < 10; i++) if (((volatile u16 *)buf)[i] != 0x1234) return 0;
    if (((volatile u16 *)buf)[10] != 0xAAAA || ((volatile u16 *)buf)[11] != 0x5555) return 0;
    return 1;
}
