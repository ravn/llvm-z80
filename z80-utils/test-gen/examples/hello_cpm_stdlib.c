#define Z88DK __attribute__((sdcccall(0)))
extern Z88DK int printf(const char *fmt, ...);

/* Name this 'cmain' — we'll rename to 'main' in the asm conversion */
Z88DK int cmain(void) {
    printf("Hello World!\n");
    return 0;
}
