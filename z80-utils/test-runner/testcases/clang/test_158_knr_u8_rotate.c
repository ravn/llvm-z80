/* expect 0x0003 */
/*
 * ravn/llvm-z80#158: a K&R-style u8 parameter is int-promoted to i16 at the C
 * ABI boundary (int = 16 bits on Z80).  TruncInstCombine now narrows the
 * rotate expression back through the Argument to native 8-bit ops.  This is the
 * runtime/value oracle for that path: rotl_u8(0x81) must rotate-left-by-1 to
 * 0x03 ((0x81 << 1) | (0x81 >> 7) = 0x02 | 0x01).  A miscompile of the narrowed
 * form (e.g. arithmetic vs logical shift, or a botched entry trunc) changes the
 * result.  `pv` is volatile so the rotate is not constant-folded away.
 */
typedef unsigned char u8;
volatile u8 pv = 0x81;

static u8 rotl_u8(x) u8 x;
{
    return (x << 1) | (x >> 7);
}

int main(void) {
    return rotl_u8(pv);
}
