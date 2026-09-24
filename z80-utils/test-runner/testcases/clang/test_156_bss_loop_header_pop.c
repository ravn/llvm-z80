/* expect 0x0002 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-frame */
/*
 * ravn/llvm-z80#156: the cross-MBB BSS-spill peephole rewrote the entry-block
 * store of the loop comparison target to PUSH, and the loop-header load to POP.
 * Each back-edge iteration POPped without a matching PUSH, corrupting SP by 2
 * bytes per iteration.  gf_log_repro(5) runs exactly 2 iterations:
 *   step 1: val=1 -> val=1^2=3 (not 5 yet)
 *   step 2: val=3 -> val=3^6=5 (exit, count=2)
 * With the bug the second visit to the loop header POPs garbage as the
 * comparison target -> comparison against garbage -> wrong return value.
 * Runtime oracle: correct=2, buggy=wrong value or crash.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

__attribute__((noinline))
uint8_t gf_log_repro(uint16_t x) {
    uint8_t count = 0;
    uint16_t val = 1;
    while ((val & 0xFF) != (x & 0xFF)) {
        uint8_t v = (uint8_t)val;
        uint8_t t = (uint8_t)((v << 1) ^ ((val & 0x80) ? 0x1Bu : 0u));
        val = (uint16_t)(v ^ t);   /* GF(256) step: val = lo_byte ^ shift */
        if (++count == 0) break;   /* overflow guard */
    }
    return count;
}

int main(void) {
    return (int)gf_log_repro(5);
}
