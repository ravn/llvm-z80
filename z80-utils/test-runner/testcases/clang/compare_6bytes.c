/* expect 0x0001 */
/* Verifies that a two-pointer unit-stride loop (compare_6bytes pattern)
   produces correct results.  Regression guard for Z80IndexIV unit-stride
   skipping: if the pass wrongly rewrites the loop, register pressure forces
   spills and the function may read wrong memory. */

typedef unsigned char byte;

static byte compare_6bytes(const byte *a, const byte *b) {
    for (byte i = 0; i < 6; i++)
        if (a[i] != b[i]) return 1;
    return 0;
}

int main(void) {
    const byte x[6] = {1, 2, 3, 4, 5, 6};
    const byte y[6] = {1, 2, 3, 4, 5, 6};
    const byte z[6] = {1, 2, 3, 4, 5, 7};

    /* equal arrays → 0, different → 1; return combined result */
    byte eq   = compare_6bytes(x, y);  /* expect 0 */
    byte diff = compare_6bytes(x, z);  /* expect 1 */
    return (eq == 0 && diff == 1) ? 1 : 0;
}
