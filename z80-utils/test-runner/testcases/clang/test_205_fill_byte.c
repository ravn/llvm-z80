/* expect 0x0001 */
/*
 * ravn/llvm-z80#205: a K=1-byte pattern-fill loop (Z80LoopIdiomFill -> the
 * llvm.z80.pattern.fill target intrinsic -> seed store + forward LDIR) must
 * fill correctly at EVERY opt level.  The old overlapping-memcpy form was UB
 * in IR (miscompiled at -O1, #136); a non-rotated loop's trip count also made
 * the fill overrun by one pattern.  Sentinels sit right past the fill region
 * WITHIN THE SAME array (adjacency guaranteed) to catch a one-pattern overrun.
 */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
volatile u8 pv = 0xAB;
static u8 buf[22];
int main(void) {
    u8 p = pv; int i;
    buf[20] = 0x11; buf[21] = 0x22;
    for (i = 0; i < 20; i++) buf[i] = p;
    for (i = 0; i < 20; i++) if (((volatile u8 *)buf)[i] != 0xAB) return 0;
    if (((volatile u8 *)buf)[20] != 0x11 || ((volatile u8 *)buf)[21] != 0x22) return 0;
    return 1;
}
