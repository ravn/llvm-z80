#ifndef DEFAULT_CC_BENCH_TEST_H
#define DEFAULT_CC_BENCH_TEST_H

#include <stdio.h>
#include <stdlib.h>

/* Run callbacks immediately: this harness has no hidden callback ABI. */
#define suite_setup(name) ((void)(name))
#define suite_add_test(fn) ((fn)())
#define suite_run() (puts("PASS"), 0)
#define Assert(condition, message) do { \
    if (!(condition)) { puts("FAIL: " message); exit(1); } \
} while (0)
#define assertEqual(actual, expected) Assert((actual) == (expected), "value")

#endif
