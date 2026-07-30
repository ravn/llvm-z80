/* misc.c — atoi() and exit() for Z80 CP/M test programs
 *
 * Not part of z80_rt.lib — compile alongside the program, same as
 * printf.c / heap.c (see z80-utils/test-gen/examples).
 */

int atoi(const char *nptr) {
    int sign = 1;
    int result = 0;
    while (*nptr == ' ' || *nptr == '\t') nptr++; /* skip leading whitespace, like libc */
    if (*nptr == '-') { sign = -1; nptr++; }
    else if (*nptr == '+') nptr++;
    while (*nptr >= '0' && *nptr <= '9') {
        result = result * 10 + (*nptr - '0');
        nptr++;
    }
    return sign * result;
}

/* CP/M has no process exit status; a warm boot (JP 0x0000) is the same
 * "return to the OS" sequence cpm_crt0_sdcc.asm uses after main() returns
 * normally, so exit(status) just does that early instead. `status` is
 * dropped, matching the CRT's own exit path. */
void exit(int status) {
    (void)status;
    __asm__ volatile("jp 0x0000");
    __builtin_unreachable();
}
