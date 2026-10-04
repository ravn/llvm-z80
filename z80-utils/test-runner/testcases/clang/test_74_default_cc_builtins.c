/* EXTRA-FLAGS: -fdefault-calling-conv=sdcccall0 */
/* EXTRA-SOURCES: z80_default_cc_builtin_workers.c */
/* expect 0x00D9 */

extern int puts(const char *);
extern int printf(const char *, ...);
extern int vsnprintf(char *, unsigned, const char *, __builtin_va_list);

__attribute__((noinline)) static int forward(char *out, const char *fmt, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, fmt);
  int result = vsnprintf(out, 32, fmt, ap);
  __builtin_va_end(ap);
  return result;
}

int main(void) {
  char out[2];
  if (puts("AB") != 263)
    return 1;
  if (printf("%s", "AB") != 263)
    return 2;
  if (__builtin_printf("%s", "AB") != 263)
    return 3;
  if (forward(out, "%d", 7, 13) != 46)
    return 4;
  if (out[0] != 7 || out[1] != 13)
    return 5;
  return 0x00D9;
}
