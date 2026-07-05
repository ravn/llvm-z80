/* heap.c — minimal malloc/calloc/free for Z80 CP/M test programs
 *
 * There is no OS memory-management syscall under CP/M, so this is a
 * classic K&R-style free-list allocator over one static byte arena,
 * with splitting: a free block bigger than the request is carved into
 * a used head and a smaller free tail, instead of being handed out
 * whole. Splitting is not optional here — dcc/tests/tm.c's allocation
 * sizes grow strictly within each outer iteration (`cb = 8 + i*10` for
 * i = 0..65), so a freed block is almost never the right size for the
 * *next* request within the same iteration; without splitting, those
 * freed-but-too-small blocks are permanently wasted and the arena fills
 * up well before the working set's true peak (measured: an unsplit
 * design exhausted a 32000 B arena at i=55 of the very first outer
 * iteration, i.e. before any reuse could even occur — see the tm.c
 * NULL-pointer-write bug this caused, fixed together with this file).
 *
 * malloc() uses BEST-fit, not first-fit, to choose which free block to
 * split. This matters, and isn't just a quality-of-implementation
 * detail: a first-fit search against tm.c's repeating-but-graduated
 * size pattern reliably carves slivers off the *wrong* (too-large)
 * free block whenever an exact-size match exists later in the list,
 * scattering same-sized blocks that would otherwise have been reused
 * whole. That fragmentation compounds every outer iteration (verified
 * with a host-side simulation of this exact allocation pattern: a
 * naive first-fit search exhausted a 100000 B arena partway through
 * the *third* outer iteration, growing without bound, whereas best-fit
 * reaches a stable steady state of ~45.5 KB after the first iteration
 * and never grows again for the remaining nine). Best-fit finds the
 * smallest free block that still satisfies the request, which for
 * this workload means it reliably finds the exact-size block freed by
 * the previous identical-pattern iteration instead of splitting a
 * larger one.
 *
 * Layout: each block is a `block_t` header immediately followed by its
 * payload. Live blocks are not linked anywhere (the header is all that's
 * needed to find the payload's start from a pointer); free blocks are
 * threaded onto `free_list` via `next`. malloc() best-fits the free
 * list, splitting off any leftover big enough to be a useful block of
 * its own, and only bump-allocates from the untouched tail of the arena
 * (`brk_offset`) when no free block is big enough. free() does not
 * coalesce adjacent blocks (best-fit + splitting already keeps
 * fragmentation low enough for this workload: dcc/tests/tm.c's
 * per-outer-iteration alloc pattern is identical every time, so
 * same-sized blocks freed at the end of iteration N are immediately
 * reusable, right-sized, by iteration N+1's identical request
 * sequence).
 */
#include <stdlib.h>
#include <string.h>

typedef struct block {
    unsigned int size;   /* payload size in bytes, excluding this header */
    struct block *next;  /* free-list link; meaningless while allocated */
} block_t;

/* dcc/tests/tm.c's peak *live* set (ap[0..65], sizes 8..658 B) is only
 * ~22 KB, but the strictly-increasing-then-all-freed pattern above means
 * the allocator transiently needs much more before settling into
 * steady-state reuse. With best-fit + splitting, the host-side
 * simulation (see file header) bump-allocates a peak of ~45.5 KB during
 * the first outer iteration and then never grows again. 48000 B gives
 * headroom over that measured peak, while still leaving room under
 * CP/M's 64 KB TPA for code + the other static data. */
#define ARENA_SIZE 48000u
static unsigned char arena[ARENA_SIZE];
static unsigned int brk_offset = 0;
static block_t *free_list = 0;

/* Below this, a split-off remainder block isn't worth creating (payload
 * would be smaller than a pointer) — just let the whole block go to the
 * caller instead of fragmenting into a header-sized-or-smaller sliver. */
#define MIN_SPLIT_PAYLOAD 4u

static block_t *block_from_payload(void *p) {
    return (block_t *)((unsigned char *)p - sizeof(block_t));
}

void *malloc(size_t size) {
    if (size == 0) size = 1;

    /* Best-fit: scan the whole free list and remember the smallest
     * block that still satisfies the request, rather than stopping at
     * the first one big enough (see file header for why first-fit
     * fragments this workload's repeating-but-graduated size pattern
     * into unbounded growth). */
    block_t *best = 0;
    block_t *best_prev = 0;
    block_t *prev = 0;
    block_t *b = free_list;
    while (b) {
        if (b->size >= size && (!best || b->size < best->size)) {
            best = b;
            best_prev = prev;
        }
        prev = b;
        b = b->next;
    }
    if (best) {
        unsigned int leftover = best->size - size;
        if (leftover >= sizeof(block_t) + MIN_SPLIT_PAYLOAD) {
            /* Carve the unused tail off as its own free block instead
             * of handing the whole (oversized) block to the caller —
             * without this, e.g. a freed 658 B block satisfying a
             * 13 B request would waste 645 B permanently, which is
             * exactly the fragmentation that exhausted the arena
             * before splitting was added (see file header). */
            block_t *tail =
                (block_t *)((unsigned char *)best + sizeof(block_t) + size);
            tail->size = leftover - sizeof(block_t);
            tail->next = best->next;
            if (best_prev) best_prev->next = tail;
            else free_list = tail;
            best->size = size;
        } else if (best_prev) {
            best_prev->next = best->next;
        } else {
            free_list = best->next;
        }
        return (unsigned char *)best + sizeof(block_t);
    }

    unsigned int need = sizeof(block_t) + size;
    if (brk_offset + need > ARENA_SIZE) return (void *)0; /* arena exhausted */
    b = (block_t *)(arena + brk_offset);
    b->size = size;
    brk_offset += need;
    return (unsigned char *)b + sizeof(block_t);
}

void free(void *ptr) {
    if (!ptr) return; /* freeing NULL is a no-op, same as libc */
    block_t *b = block_from_payload(ptr);
    b->next = free_list;
    free_list = b;
}

void *calloc(size_t nmemb, size_t size) {
    unsigned int total = (unsigned int)nmemb * (unsigned int)size;
    void *p = malloc(total);
    if (p) {
        /* Use memset's LDIR-based fill (z80_rt memset.asm), not a
         * hand-written byte loop: a manual `for` loop pays per-iteration
         * compare+branch+increment overhead on top of the store, while
         * LDIR is a single instruction that block-copies BC bytes with no
         * per-byte loop overhead. Measured on dcc/tests/tm.c's benchmark
         * (6600 calloc calls, sizes 8..658 B): calloc's zero-fill alone
         * was ~11% of all dynamically executed instructions before this
         * fix — a real, avoidable cost of this ad-hoc allocator, not a
         * property of the C source being compiled. */
        memset(p, 0, total);
    }
    return p;
}

