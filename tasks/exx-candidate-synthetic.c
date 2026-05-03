// Synthetic EXX-bracket candidate (ravn/llvm-z80#114).
//
// Goal: exhibit the textbook "outer-counter parked across inner
// no-CALL byte-twiddle loop" shape so we can quantify what an EXX
// bracket would save.
//
// The pattern (verbatim from _specc 0xde19-0xde3c, see strand-B
// notes 2026-05-03):
//
//   outer:
//     <outer-state held in BC>
//     LD (parked_slot),BC      ; 4 B  spill BC to BSS
//     <inner loop body uses BC freely>
//     LD BC,(parked_slot)      ; 4 B  reload BC from BSS
//     INC BC; CP n; JR NZ,outer
//
// EXX bracket replaces the 8 B/iter spill with 2 B fixed:
//
//   outer:
//     <outer-state held in BC>
//     EXX                      ; 1 B  swap to shadow bank
//     <inner loop body uses BC' freely>
//     EXX                      ; 1 B  swap back
//     INC BC; CP n; JR NZ,outer
//
// Build (BIOS-matching flags):
//   build-macos/bin/clang --target=z80 -Oz -ffreestanding -nostdlib \
//     -std=c23 -Xclang -target-feature -Xclang +static-stack \
//     -mllvm -disable-lsr -S \
//     tasks/exx-candidate-synthetic.c -o /tmp/exx.s
//
// Acceptance for #114 prototype:
//   - inner-loop preheader currently shows 4-byte `LD (nn),BC` and
//     outer back-edge shows 4-byte `LD BC,(nn)` — matched pair on
//     same nn (the `parked_slot` BSS sframe slot);
//   - after the prototype, that same MBB pair shows EXX/EXX
//     instead, and the `(nn)` BSS slot disappears.
//
// Important construction notes (why the source looks the way it does):
//
//   - All inputs are GLOBALS, not parameters.  +static-stack does
//     not kick in when the function has stack-passed parameters
//     (the function falls back to IX-frame mode and uses (ix+-N)
//     slots instead of named BSS slots, which doesn't match the
//     `_specc` shape we are modeling).
//
//   - Inner trip count is read from a `volatile`-qualified global
//     so the optimizer cannot unroll the inner loop away.  Without
//     volatile, clang at -Oz may bound-prove the count and unroll
//     small fixed-trip loops.
//
//   - Inner body uses three pointer-shaped quantities (dp, sp,
//      and the 80-byte stride literal) plus `acc` in A and `k`
//      in B (DJNZ counter) — pressuring the outer 16-bit counter
//      `i` out of all three pairs for the inner-loop duration.
//
//   - The outer back-edge increments `i` and compares against
//     `end_idx`, requiring `i` to be live across the inner loop —
//     exactly the parked-value requirement of EXX bracketing.

#include <stdint.h>

extern uint8_t        out_buf[];
extern const uint8_t  in_buf[];
extern uint16_t       end_idx;
extern volatile uint8_t inner_n;

void render(void) {
    for (uint16_t i = 0; i < end_idx; i++) {
        uint8_t       *dp = out_buf + i;
        const uint8_t *sp = in_buf  + i;
        uint8_t acc = (uint8_t)i;
        uint8_t k = inner_n;
        do {
            uint8_t v = *sp;
            sp += 80;
            acc ^= v;
            *dp = acc;
            dp += 80;
        } while (--k);
    }
}
