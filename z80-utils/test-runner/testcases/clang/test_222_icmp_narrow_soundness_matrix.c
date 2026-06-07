/* expect 0x0000 */
/*
 * Thorough runtime matrix for the #160/#165 icmp-narrowing soundness bug:
 * the gate proved only the NON-graph operand narrow; an icmp consumes the
 * full wide value, so unproven graph-side values (t = x+1 with x near/above
 * 255) miscompile when the compare is narrowed to i8.
 *
 * Matrix: {const-other (#160), var-other (#165)} x {<, <=, >, >=, ==, !=}
 *         x 7 input values spanning in-range / boundary / out-of-range,
 *         plus the (sound) and-mask outside-user path as a control.
 *
 * Self-checking: expected results are computed at FULL width by reference
 * functions that contain no trunc-rooted graph (no (u8) casts of the
 * compared value), so they are immune to the narrowing transform.  No
 * hand-computed expected constants anywhere.
 *
 * Returns the number of failing subcases: 0x0000 when the compiler is sound.
 * Companion minimal repros: test_220 (var path), test_221 (const path).
 */
typedef unsigned char u8;

/* ---- vulnerable shapes: t = x+1 (unproven), trunc root + escaping icmp ---- */
#define VULN_CONST(NAME, OP, BOUND)                       \
    static u8 NAME(unsigned x) __attribute__((noinline)); \
    static u8 NAME(unsigned x) {                          \
        unsigned t = x + 1;                               \
        u8 r = (u8)t;                                     \
        return (t OP (BOUND)) ? r : 99;                   \
    }

#define VULN_VAR(NAME, OP)                                            \
    static u8 NAME(unsigned x, unsigned y) __attribute__((noinline)); \
    static u8 NAME(unsigned x, unsigned y) {                          \
        unsigned t = x + 1;                                           \
        u8 r = (u8)t;                                                 \
        return (t OP (y & 0xffu)) ? r : 99;                           \
    }

VULN_CONST(vc_lt, <, 10u)
VULN_CONST(vc_le, <=, 10u)
VULN_CONST(vc_gt, >, 10u)
VULN_CONST(vc_ge, >=, 10u)
VULN_CONST(vc_eq, ==, 5u)
VULN_CONST(vc_ne, !=, 5u)

VULN_VAR(vv_lt, <)
VULN_VAR(vv_le, <=)
VULN_VAR(vv_gt, >)
VULN_VAR(vv_ge, >=)
VULN_VAR(vv_eq, ==)
VULN_VAR(vv_ne, !=)

/* ---- wide-width reference predicates: no (u8) cast of t, hence no
 *      trunc-rooted graph, hence immune to the transform ---- */
#define REF(NAME, OP)                                                 \
    static u8 NAME(unsigned t, unsigned b) __attribute__((noinline)); \
    static u8 NAME(unsigned t, unsigned b) { return (t OP b) ? 1 : 0; }

REF(r_lt, <)
REF(r_le, <=)
REF(r_gt, >)
REF(r_ge, >=)
REF(r_eq, ==)
REF(r_ne, !=)

/* ---- and-mask control (sound; must stay correct before AND after fix) ---- */
volatile unsigned g_mask;
static u8 vmask(unsigned x) __attribute__((noinline));
static u8 vmask(unsigned x) {
    unsigned t = x + 1;
    u8 r = (u8)t;
    g_mask = t & 15u;
    return r;
}

/* input values: in-range, near-boundary, boundary, just-over, wrap-larger */
volatile unsigned xs[7] = {4u, 9u, 254u, 255u, 256u, 260u, 510u};
volatile unsigned ybound = 10u; /* var-path bound; & 0xff is fns' shape */

typedef u8 (*vc_fn)(unsigned);
typedef u8 (*vv_fn)(unsigned, unsigned);
typedef u8 (*ref_fn)(unsigned, unsigned);

static const vc_fn VC[6] = {vc_lt, vc_le, vc_gt, vc_ge, vc_eq, vc_ne};
static const vv_fn VV[6] = {vv_lt, vv_le, vv_gt, vv_ge, vv_eq, vv_ne};
static const ref_fn RF[6] = {r_lt, r_le, r_gt, r_ge, r_eq, r_ne};
/* const-path bounds must mirror the literals in VULN_CONST above */
static const unsigned CB[6] = {10u, 10u, 10u, 10u, 5u, 5u};

int main(void) {
    unsigned fails = 0;
    for (unsigned i = 0; i < 7; i++) {
        unsigned x = xs[i];
        unsigned t = x + 1;            /* wide truth value of t        */
        unsigned r8 = t & 0xffu;       /* wide image of the (u8) trunc */
        for (unsigned p = 0; p < 6; p++) {
            /* expected results from the immune wide-width reference */
            unsigned expc = RF[p](t, CB[p]) ? r8 : 99u;
            unsigned expv = RF[p](t, ybound & 0xffu) ? r8 : 99u;
            if ((unsigned)VC[p](x) != expc)
                fails++;
            if ((unsigned)VV[p](x, ybound) != expv)
                fails++;
        }
        /* and-mask control: both the return and the escaped mask value */
        unsigned rm = (unsigned)vmask(x);
        if (rm != r8)
            fails++;
        if (g_mask != (t & 15u))
            fails++;
    }
    return (int)fails;
}
