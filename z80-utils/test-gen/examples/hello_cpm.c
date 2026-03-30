/*
 * CP/M Hello World — compile with clang, run with z88dk-ticks
 *
 * Build: make -C examples hello_cpm
 * Run:   z88dk-ticks examples/HELLO.COM
 */
int putchar(int c);

static void print(const char *s) {
    while (*s) putchar(*s++);
}

int main(void) {
    print("Hello World!\r\n");
    return 0;
}
