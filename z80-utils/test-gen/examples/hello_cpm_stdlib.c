#include <stdio.h>

int cmain(void) {
    int i;
    for (i = 1; i <= 5; i++) {
        printf("%d * 7 = %d\n", i, i * 7);
    }
    return 0;
}
