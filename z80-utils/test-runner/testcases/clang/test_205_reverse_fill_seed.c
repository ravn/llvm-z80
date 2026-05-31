/* expect 0x0001 */
/* EXTRA-FLAGS: -mllvm -z80-reverse-fill-seed */
/*
 * ravn/llvm-z80#205 follow-up: the experimental -z80-reverse-fill-seed peephole
 * rewrites a constant-value K=2 LDIR fill seed into a reversed (HL) byte store
 * that lands HL on the fill base.  This fixture forces that path (constant
 * 0xCAFE word fill) WITH the flag and verifies the fill is byte-correct and
 * does not over-run -- the reversed seed must write the same two bytes to the
 * same addresses as the absolute `ld (nn),hl` it replaces.  Sentinels sit just
 * past the fill region within the same array (adjacency guaranteed).
 */
typedef unsigned short u16;
static u16 buf[12];
int main(void) {
    int i;
    buf[10] = 0x1234;
    buf[11] = 0x5678;
    for (i = 0; i < 10; i++) buf[i] = 0xCAFE;
    for (i = 0; i < 10; i++)
        if (((volatile u16 *)buf)[i] != 0xCAFE) return 0;
    if (((volatile u16 *)buf)[10] != 0x1234 || ((volatile u16 *)buf)[11] != 0x5678)
        return 0;   /* over-run clobbered a sentinel */
    return 1;
}
