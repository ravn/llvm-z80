/* Isolate: 16-bit multiply+divide 1234*5/5=1234 */
typedef unsigned short uint16_t;
int main(void) {
    volatile uint16_t a = 1234, b = 5;
    uint16_t prod = a * b;
    uint16_t back = prod / b;
    uint16_t status = 0;
    if (prod == 6170 && back == 1234) status |= 1;
    return status;
}
