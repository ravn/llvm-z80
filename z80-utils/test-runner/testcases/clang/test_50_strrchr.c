/* Test 50: strrchr codegen (M6 narrowing) — verify the narrowed 8-bit compare
 * still finds the LAST occurrence correctly across positive/negative chars. */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

/* Local strrchr so the test exercises clang's own codegen, not the library. */
static char *my_strrchr(const char *s, int c) {
    char *last = 0;
    while (*s) {
        if (*s == (char)c)
            last = (char *)s;
        s++;
    }
    return last;
}

int main() {
    uint16_t status = 0;

    static const char str[] = "abracadabra";

    /* Bit 0: last 'a' is index 10 */
    if (my_strrchr(str, 'a') == &str[10]) status |= (1 << 0);
    /* Bit 1: last 'b' is index 8 */
    if (my_strrchr(str, 'b') == &str[8]) status |= (1 << 1);
    /* Bit 2: last 'r' is index 9 */
    if (my_strrchr(str, 'r') == &str[9]) status |= (1 << 2);
    /* Bit 3: 'z' not present -> NULL */
    if (my_strrchr(str, 'z') == 0) status |= (1 << 3);
    /* Bit 4: first-and-only char */
    if (my_strrchr(str, 'c') == &str[4]) status |= (1 << 4);

    /* Bit 5: high-bit char (0x80..0xFF) — the (char)c cast is signed, so the
     * compare must treat the byte value correctly.  Byte 0xC3 present once. */
    static const char hs[] = { 'x', (char)0xC3, 'y', (char)0xC3, 'z', 0 };
    if (my_strrchr(hs, 0xC3) == &hs[3]) status |= (1 << 5);
    /* Bit 6: searching for 0 is not exercised (loop stops at NUL); instead
     * confirm a char that only differs in the high byte of the int arg still
     * matches on the low byte: 0x141 truncates to (char)0x41 = 'A'. */
    static const char as[] = { 'A', 'b', 'A', 0 };
    if (my_strrchr(as, 0x141) == &as[2]) status |= (1 << 6);

    return status; /* expect 0x007F */
}
