/* Isolate: 16-bit signed division (-3300)/11=-300 rem 0 */
typedef short int16_t;
typedef unsigned short uint16_t;
int main(void) {
    volatile int16_t a = -3300, b = 11;
    int16_t q = a / b;
    int16_t r = a % b;
    uint16_t status = 0;
    if (q == -300 && r == 0) status |= 4;
    return status;
}
