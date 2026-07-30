/* minimal repro: clang-z80 miscompiles a switch case that is (a) a fall-through
   target and (b) immediately does goto to a label positioned mid-switch.
   The direct case entry lands at default at -O1+ (works at -O0 and on host). */
typedef struct { unsigned char conv; char adj; } spec_t;
int parse(const char *fmt, spec_t *s){
    const char *cur = fmt;
    int tmp = 0;
    s->conv = 0; s->adj = 32;
    ++cur;
    switch(*cur++){
        case 'i': case 'd': tmp = 1; goto finish;
        case 'o': tmp = 2; goto finish;
        case 'u': tmp = 3; goto finish;
        case 'X': s->adj = 0;
        case 'x': tmp = 6; goto finish;
        finish: s->conv = (unsigned char)tmp; break;
        case 'F': s->adj = 0;
        case 'f': s->conv = 20; break;
        default: return 0;
    }
    return (int)(cur - fmt);
}
