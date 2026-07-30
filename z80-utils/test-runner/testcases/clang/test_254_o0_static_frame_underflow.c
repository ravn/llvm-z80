/* ravn/llvm-z80 #254: -O0 static-stack frame allocated too small.
 *
 * At -O0 hasFP is true, so main uses the static-stack + frame-pointer path
 * (IX == __sfrend_main; the caller's IX is saved on the REAL stack via PUSH,
 * so it is excluded from the BSS frame: BSSSize = StackSize - CalleeSavedFrame
 * Size).  The frame-index -> BSS-address lowering skipped only the saved-IX
 * slot (+2) but NOT that callee-saved region, so the deepest local was emitted
 * at __sfrend_main-8 while __sframe_main sat at __sfrend_main-6.  The spill at
 * __sfrend_main-7/-8 underflowed BELOW __sframe_main into the adjacent global
 * `bufb`, silently corrupting it (returned 0x0278 instead of 0x0203).
 *
 * A forward-overlap memmove via __memmove_rt spills a pointer to a frame slot
 * at -O0 (the fold to inline LDIR/LDDR only happens at -O1+), which is what
 * drives the deep spill slot.  This fixture deliberately runs at every opt
 * level including O0 (no skip directive) — O0 is the level under test.
 * -O1..-Oz fold the memmove and never touch the frame, so they already
 * returned 0x0203.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

uint8_t bufb[10];
volatile uint8_t two = 2, six = 6;

int main() {
    for (uint8_t i = 0; i < 10; i++) bufb[i] = i;
    uint8_t *d = bufb;
    uint8_t *s = bufb + two;                 /* forward overlap: d < s */
    __builtin_memmove(d, s, six);
    /* bufb becomes {2,3,4,5,6,7, 6,7, 8,9}; high byte = bufb[0]=2, low = 3 */
    return ((uint16_t)bufb[0] << 8) | bufb[1]; /* expect 0x0203 */
}
