/* Isolate: 16-bit unsigned division 50000/700=71 rem 300 */
typedef unsigned short uint16_t;
int main(void) {
    volatile uint16_t a = 50000u, b = 700;
    uint16_t q = a / b;
    uint16_t r = a % b;
    uint16_t status = 0;
    if (q == 71 && r == 300) status |= 2;
    return status;
}
