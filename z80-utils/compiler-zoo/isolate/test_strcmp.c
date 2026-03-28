/* Isolate: strcmp */
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;

int8_t my_strcmp(const uint8_t *a, const uint8_t *b) {
    while (*a && *a == *b) { a++; b++; }
    if (*a == *b) return 0;
    return *a < *b ? -1 : 1;
}

int main(void) {
    uint8_t a[] = "abc";
    uint8_t b[] = "abc";
    uint8_t c[] = "abd";
    int8_t r1 = my_strcmp(a, b);
    int8_t r2 = my_strcmp(a, c);
    int8_t r3 = my_strcmp(c, a);
    uint16_t status = 0;
    if (r1 == 0 && r2 < 0 && r3 > 0) status |= 2;
    return status;
}
