/* Test 176: small array-fill loop must be correct at every opt level
 * (regression for ravn/llvm-z80#136).
 *
 * Z80LoopIdiomFill rewrites `for (j<N) buf[j]=v` to a seed + an overlapping
 * memcpy that the backend lowers as a forward LDIR.  At -O1 a generic
 * InstCombine used to inline that small overlapping memcpy into a wide
 * load+store (load-all-then-store), which reads not-yet-written bytes and
 * filled buf[2] with garbage (returned 0x6C vs 0xB1) -- only at -O1.  Fixed by
 * marking the memcpy volatile so it always lowers as LDIR.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
int main(void) {
    uint16_t j;
    uint8_t buf[3];
    for (j = 0; j < 3; j++) buf[j] = 177;
    return buf[2];   /* expect 0x00B1 */
}
