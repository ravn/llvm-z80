int puts(const char *) __attribute__((sdcccall(0)));
int printf(const char *, ...) __attribute__((sdcccall(0)));
int vsnprintf(char *, unsigned, const char *, __builtin_va_list)
    __attribute__((sdcccall(0)));

int puts(const char *s) {
  return (unsigned char)s[0] + 3 * (unsigned char)s[1];
}

int printf(const char *fmt, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, fmt);
  const char *s = __builtin_va_arg(ap, const char *);
  __builtin_va_end(ap);
  return puts(s);
}

int vsnprintf(char *out, unsigned size, const char *fmt, __builtin_va_list ap) {
  if (size != 32 || fmt[0] != '%')
    return -1;
  int first = __builtin_va_arg(ap, int);
  int second = __builtin_va_arg(ap, int);
  out[0] = first;
  out[1] = second;
  return first + 3 * second;
}
