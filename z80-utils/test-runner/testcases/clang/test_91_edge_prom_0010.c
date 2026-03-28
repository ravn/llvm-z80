/* Z80 edge-case test (auto-generated) */
/* expect 0x0000 */
/* EXTRA-FLAGS: -Xclang -target-feature -Xclang +static-stack -Xclang -target-feature -Xclang +shadow-regs -mllvm -disable-lsr */

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned long uint32_t;

__attribute__((noinline))
uint16_t call6(uint16_t a,uint16_t b,uint16_t c,
               uint16_t d,uint16_t e,uint16_t f) {
    return a+b+c+d+e+f;
}

__attribute__((noinline))
uint16_t add2(uint16_t a, uint16_t b) { return a + b; }

__attribute__((noinline))
uint16_t sub2(uint16_t a, uint16_t b) { return a - b; }

static volatile uint16_t g16;

__attribute__((noinline))
uint16_t read_g16(void) { return g16; }

int main(void) {
    int failures = 0;

    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        volatile int16_t a = 25067;
        volatile int16_t b = 28602;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {84,84,185,237,6,150};
        if (a[4] != 6) failures++;
    }


    {
        uint16_t r = call6(141,59,140,217,166,70);
        if (r != 793) failures++;
    }


    {
        uint8_t v = 63;
        v |= 128;
        if (v != 191) failures++;
    }


    {
        uint8_t x = 89;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 13;
        if (buf[6] != 13) failures++;
    }


    {
        uint16_t r = add2(126,230) + add2(230,64) + add2(126,64);
        if (r != 840) failures++;
    }


    {
        uint16_t x = 38967;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[11] = {248,143,29,189,10,250,110,212,5,136,245};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[5] != 250) failures++;
    }


    {
        uint8_t m[2][2] = {{195,53},{92,246}};
        if (m[0][0] != 195) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 9: result = 243; break;
        case 2: result = 129; break;
        case 6: result = 33; break;
        case 0: result = 117; break;
        default: result = 13; break;
        }
        if (result != 117) failures++;
    }


    {
        uint8_t v = 56;
        v |= 128;
        if (v != 184) failures++;
    }


    {
        uint32_t a = 2499873176UL;
        uint32_t b = 502100790UL;
        uint32_t r = a - b;
        if (r != 1997772386UL) failures++;
    }


    {
        uint8_t buf[8] = {125,182,68,230,193,118,83,111};
        uint8_t *p = buf;
        p += 5;
        if (*p != 118) failures++;
    }


    {
        uint8_t m[3][4] = {{86,50,252,140},{151,15,235,189},{151,147,194,156}};
        if (m[1][2] != 235) failures++;
    }


    {
        uint16_t r = call6(176,3,170,39,158,39);
        if (r != 585) failures++;
    }


    {
        volatile uint8_t port = 87;
        uint8_t r = port;
        if (r != 87) failures++;
    }


    {
        volatile int16_t a = 10228;
        volatile int16_t b = -12774;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 87;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t buf[8] = {142,225,227,104,52,40,177,77};
        uint8_t *p = buf;
        p += 0;
        if (*p != 142) failures++;
    }


    {
        uint8_t a[6] = {91,126,184,181,100,46};
        if (a[1] != 126) failures++;
    }


    {
        uint16_t r = call6(27,236,27,77,196,9);
        if (r != 572) failures++;
    }


    {
        g16 = 1150;
        if (read_g16() != 1150) failures++;
    }


    {
        uint8_t x = 144;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t a[6] = {198,178,1,3,188,49};
        if (a[5] != 49) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t src[5] = {67,181,206,58,22};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[1] != 181) failures++;
    }


    {
        volatile uint8_t port = 143;
        uint8_t r = port;
        if (r != 143) failures++;
    }


    {
        uint8_t a[6] = {22,22,28,31,18,214};
        if (a[4] != 18) failures++;
    }


    {
        uint16_t r = 59471 + 64634 + 23865 + 24867 + 40416 + 54471 + 1081 + 15665;
        if (r != 22326) failures++;
    }


    {
        uint8_t a[6] = {113,247,44,78,9,157};
        if (a[0] != 113) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint8_t v = 214;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint32_t a = 3357134695UL;
        uint32_t b = 2138483403UL;
        uint32_t r = a & b;
        if (r != 1209043523UL) failures++;
    }


    {
        uint8_t x = 209;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t a[6] = {12,200,38,34,90,162};
        if (a[2] != 38) failures++;
    }


    {
        uint16_t x = 47861;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(141,219) + add2(219,194) + add2(141,194);
        if (r != 1108) failures++;
    }


    {
        uint16_t x = 24171;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 2278485405UL;
        uint32_t b = 1724511793UL;
        uint32_t r = a + b;
        if (r != 4002997198UL) failures++;
    }


    {
        uint16_t x = 75;
        x = x + 231;
        if (x != 306) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)78) / (int16_t)((int8_t)-30);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-106) % (int16_t)((int8_t)99);
        if ((uint16_t)r != (uint16_t)65529) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 27;
        if (buf[12] != 27) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint8_t v = 39;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 25) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {184,88,45282,16};
        if (s.a != (uint8_t)184) failures++;
    }


    {
        g16 = 62187;
        if (read_g16() != 62187) failures++;
    }


    {
        uint32_t a = 3705139185UL;
        uint32_t b = 3501544431UL;
        uint32_t r = a + b;
        if (r != 2911716320UL) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 19: result = 253; break;
        case 8: result = 174; break;
        case 2: result = 38; break;
        case 18: result = 170; break;
        default: result = 40; break;
        }
        if (result != 38) failures++;
    }


    {
        volatile uint8_t port = 173;
        uint8_t r = port;
        if (r != 173) failures++;
    }


    {
        uint16_t r = call6(236,228,209,51,84,99);
        if (r != 907) failures++;
    }


    {
        volatile uint8_t port = 190;
        uint8_t r = port;
        if (r != 190) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 108;
        if (buf[9] != 108) failures++;
    }


    {
        uint8_t m[2][3] = {{121,62,45},{14,15,121}};
        if (m[0][2] != 45) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint16_t r = 19561 + 38934 + 47060 + 30428 + 65442 + 47480 + 45638 + 36967;
        if (r != 3830) failures++;
    }


    {
        uint16_t r = call6(150,207,54,223,219,178);
        if (r != 1031) failures++;
    }


    {
        volatile int16_t a = -31616;
        volatile int16_t b = 27617;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint32_t a = 710392301UL;
        uint32_t b = 1399146005UL;
        uint32_t r = a & b;
        if (r != 38076421UL) failures++;
    }


    {
        int8_t a = -72;
        int8_t b = -17;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        if (((uint16_t)(((170 | 80) & 173) + ((78 | 197) - (155 + 3)))) != 217) failures++;
    }


    {
        int8_t a = 20;
        int8_t b = -117;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)111) != 111) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 15: result = 23; break;
        case 7: result = 147; break;
        case 6: result = 154; break;
        default: result = 148; break;
        }
        if (result != 154) failures++;
    }


    {
        uint8_t x = 235;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t r = 30829 + 54025 + 35590 + 59849 + 58534 + 57947 + 12316 + 37986;
        if (r != 19396) failures++;
    }


    {
        if (((uint16_t)219) != 219) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)87) / (int16_t)((int8_t)110);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {96,121,171,149,92,173,239,90};
        uint8_t *p = buf;
        p += 2;
        if (*p != 171) failures++;
    }


    {
        uint16_t r = add2(89,240) + add2(240,148) + add2(89,148);
        if (r != 954) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint32_t a = 2869692837UL;
        uint32_t b = 2831719318UL;
        uint32_t r = a & b;
        if (r != 2819099012UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        if (((uint16_t)56) != 56) failures++;
    }


    {
        uint8_t src[8] = {181,122,127,176,235,12,83,214};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[5] != 12) failures++;
    }


    {
        if (((uint16_t)128) != 128) failures++;
    }


    {
        uint32_t a = 481062834UL;
        uint32_t b = 391697498UL;
        uint32_t r = a - b;
        if (r != 89365336UL) failures++;
    }


    {
        uint16_t r = call6(86,48,193,106,175,195);
        if (r != 803) failures++;
    }


    {
        uint8_t x = 20;
        x <<= 0;
        if (x != 20) failures++;
    }


    {
        uint8_t v = 20;
        v &= ~(uint8_t)32;
        if (v != 20) failures++;
    }


    {
        uint8_t v = 104;
        v &= ~(uint8_t)128;
        if (v != 104) failures++;
    }


    {
        int8_t a = 22;
        int8_t b = -16;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(6,39) + add2(39,70) + add2(6,70);
        if (r != 230) failures++;
    }


    {
        uint8_t x = 204;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t m[3][4] = {{176,78,54,148},{56,54,217,197},{14,250,135,62}};
        if (m[0][0] != 176) failures++;
    }


    {
        uint16_t r = add2(52,27) + add2(27,203) + add2(52,203);
        if (r != 564) failures++;
    }


    {
        uint8_t v = 235;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 1) sum += j;
        if (sum != 153) failures++;
    }


    {
        g16 = 47734;
        if (read_g16() != 47734) failures++;
    }


    {
        volatile uint8_t port = 9;
        uint8_t r = port;
        if (r != 9) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(158,50) != 208) failures++;
    }


    {
        g16 = 5731;
        if (read_g16() != 5731) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(225,37,11,239,105,98);
        if (r != 715) failures++;
    }


    {
        uint8_t a[6] = {98,39,176,91,207,150};
        if (a[1] != 39) failures++;
    }


    {
        uint8_t a[6] = {39,98,28,43,48,181};
        if (a[3] != 43) failures++;
    }


    {
        uint8_t v = 151;
        v &= ~(uint8_t)128;
        if (v != 23) failures++;
    }


    {
        uint8_t v = 204;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {187,247,84,195,112,153};
        if (a[3] != 195) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-119) % (int16_t)((int8_t)126);
        if ((uint16_t)r != (uint16_t)65417) failures++;
    }


    {
        g16 = 14115;
        if (read_g16() != 14115) failures++;
    }


    {
        volatile uint8_t port = 109;
        uint8_t r = port;
        if (r != 109) failures++;
    }


    {
        uint8_t x = 99;
        x <<= 1;
        if (x != 198) failures++;
    }


    {
        volatile uint8_t port = 114;
        uint8_t r = port;
        if (r != 114) failures++;
    }


    {
        uint8_t src[13] = {62,54,210,166,21,131,232,143,159,142,179,35,231};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[7] != 143) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 38;
        if (buf[9] != 38) failures++;
    }


    {
        uint8_t m[3][2] = {{167,187},{226,148},{34,16}};
        if (m[1][0] != 226) failures++;
    }


    {
        uint8_t a[6] = {37,176,152,254,14,40};
        if (a[0] != 37) failures++;
    }


    {
        uint8_t v = 24;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[3][3] = {{224,67,60},{8,143,231},{92,231,94}};
        if (m[1][1] != 143) failures++;
    }


    {
        volatile int16_t a = 18605;
        volatile int16_t b = -8000;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 224;
        uint8_t r = port;
        if (r != 224) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)24) % (int16_t)((int8_t)87);
        if ((uint16_t)r != (uint16_t)24) failures++;
    }


    {
        uint16_t r = add2(117,199) + add2(199,28) + add2(117,28);
        if (r != 688) failures++;
    }


    {
        uint8_t m[3][2] = {{140,179},{29,129},{108,196}};
        if (m[2][0] != 108) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 3;
        if (buf[0] != 3) failures++;
    }


    {
        g16 = 19320;
        if (read_g16() != 19320) failures++;
    }


    {
        uint16_t r = call6(172,55,7,147,220,50);
        if (r != 651) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(19,137) != 156) failures++;
    }


    {
        uint8_t src[16] = {54,160,36,111,73,67,172,29,146,212,126,25,163,166,78,112};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[9] != 212) failures++;
    }


    {
        uint8_t src[13] = {198,85,243,47,111,51,230,144,102,186,88,240,209};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[7] != 144) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 5: result = 115; break;
        case 13: result = 75; break;
        case 14: result = 33; break;
        case 18: result = 130; break;
        default: result = 144; break;
        }
        if (result != 115) failures++;
    }


    {
        if (((uint16_t)(218 + ((95 - 133) - (147 - 167)))) != 200) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)16) + (uint16_t)59396;
        if (r != 59412) failures++;
    }


    {
        uint16_t r = add2(159,66) + add2(66,88) + add2(159,88);
        if (r != 626) failures++;
    }


    {
        uint8_t x = 88;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint32_t a = 930950422UL;
        uint32_t b = 957952400UL;
        uint32_t r = a ^ b;
        if (r != 241441926UL) failures++;
    }


    {
        if (((uint16_t)(((153 & 1) - 219) | ((41 - 155) ^ (56 + 102)))) != 65334) failures++;
    }


    {
        uint8_t v = 52;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 12) failures++;
    }


    {
        uint16_t r = add2(95,231) + add2(231,65) + add2(95,65);
        if (r != 782) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint8_t v = 43;
        int r = (v & 8) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 187;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint8_t x = 190;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t r = 62176 + 46244 + 24974 + 19928 + 46895 + 18442 + 62924 + 59472;
        if (r != 13375) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-9) % (int16_t)((int8_t)42);
        if ((uint16_t)r != (uint16_t)65527) failures++;
    }


    {
        uint16_t r = add2(203,254) + add2(254,197) + add2(203,197);
        if (r != 1308) failures++;
    }


    {
        volatile uint8_t port = 254;
        uint8_t r = port;
        if (r != 254) failures++;
    }


    {
        uint16_t r = 34418 + 61540 + 5371 + 28551 + 35964 + 42934 + 4320 + 63946;
        if (r != 14900) failures++;
    }


    {
        uint16_t r = call6(180,5,68,47,231,208);
        if (r != 739) failures++;
    }


    {
        uint16_t r = call6(36,111,5,133,241,142);
        if (r != 668) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 94;
        if (buf[0] != 94) failures++;
    }


    {
        uint16_t r = call6(201,105,150,225,228,0);
        if (r != 909) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 28435;
        if (read_g16() != 28435) failures++;
    }


    {
        uint8_t buf[8] = {221,190,175,12,131,250,211,44};
        uint8_t *p = buf;
        p += 4;
        if (*p != 131) failures++;
    }


    {
        uint16_t x = 173;
        x = x + 45;
        if (x != 218) failures++;
    }


    {
        uint16_t x = 176;
        x = x + 236;
        if (x != 412) failures++;
    }


    {
        uint8_t v = 177;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t v = 69;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        g16 = 53441;
        if (read_g16() != 53441) failures++;
    }


    {
        uint8_t buf[8] = {155,116,84,107,118,28,7,192};
        uint8_t *p = buf;
        p += 5;
        if (*p != 28) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)82) / (int16_t)((int8_t)-58);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint32_t a = 4070205936UL;
        uint32_t b = 3916648232UL;
        uint32_t r = a + b;
        if (r != 3691886872UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 1) sum += j;
        if (sum != 171) failures++;
    }


    {
        uint16_t r = call6(248,223,192,167,238,42);
        if (r != 1110) failures++;
    }


    {
        uint8_t src[15] = {46,121,250,53,253,7,164,16,156,3,197,23,139,154,96};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[6] != 164) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 18: result = 183; break;
        case 16: result = 179; break;
        case 3: result = 177; break;
        case 7: result = 189; break;
        case 4: result = 218; break;
        case 15: result = 185; break;
        case 10: result = 97; break;
        case 5: result = 103; break;
        default: result = 173; break;
        }
        if (result != 177) failures++;
    }


    {
        uint16_t x = 10416;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 3551488430UL;
        uint32_t b = 3321234010UL;
        uint32_t r = a | b;
        if (r != 3623840766UL) failures++;
    }


    {
        uint8_t a[6] = {47,39,179,255,175,121};
        if (a[5] != 121) failures++;
    }


    {
        uint8_t src[4] = {112,171,48,211};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 211) failures++;
    }


    {
        uint16_t r = 20395 + 50374 + 17305 + 60654 + 27731 + 49207 + 22415 + 5641;
        if (r != 57114) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(71,158) != 65449) failures++;
    }


    {
        uint16_t x = 30;
        x = x + 166;
        if (x != 196) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(201,217) != 418) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 77;
        if (buf[4] != 77) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 15: result = 30; break;
        case 4: result = 228; break;
        case 14: result = 148; break;
        default: result = 26; break;
        }
        if (result != 30) failures++;
    }


    {
        if (((uint16_t)113) != 113) failures++;
    }


    {
        uint8_t m[3][3] = {{96,107,122},{29,243,88},{164,125,103}};
        if (m[0][0] != 96) failures++;
    }


    {
        int8_t a = 42;
        int8_t b = 1;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 118;
        int8_t b = -89;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 56239;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 171;
        if (buf[7] != 171) failures++;
    }


    {
        uint8_t x = 122;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(119,158) != 65497) failures++;
    }


    {
        int8_t a = 53;
        int8_t b = -20;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)41) + (uint16_t)11653;
        if (r != 11694) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t src[8] = {89,50,61,175,216,71,131,166};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 50) failures++;
    }


    {
        int8_t a = 53;
        int8_t b = -71;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {250,12,45,189,26,40};
        if (a[0] != 250) failures++;
    }


    {
        uint16_t r = call6(209,219,149,255,181,246);
        if (r != 1259) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {119,173,711,174};
        if (s.c != (uint16_t)711) failures++;
    }


    {
        uint16_t r = 58842 + 19359 + 63644 + 36983 + 22570 + 16779 + 11170 + 264;
        if (r != 33003) failures++;
    }


    {
        uint16_t x = 10;
        x = x + 99;
        if (x != 109) failures++;
    }


    {
        uint8_t src[14] = {211,75,84,78,159,75,131,129,88,92,245,80,29,153};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[6] != 131) failures++;
    }


    {
        uint16_t x = 87;
        x = x + 222;
        if (x != 309) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {197,131,186,218,226,29,120,183};
        uint8_t *p = buf;
        p += 5;
        if (*p != 29) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint8_t x = 252;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)105) + (uint16_t)51624;
        if (r != 51729) failures++;
    }


    {
        int8_t a = 30;
        int8_t b = 44;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 3;
        uint8_t r = port;
        if (r != 3) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 35;
        x = x + 105;
        if (x != 140) failures++;
    }


    {
        uint8_t src[14] = {165,233,133,7,177,103,86,255,234,144,31,84,61,99};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[4] != 177) failures++;
    }


    {
        uint16_t r = call6(20,80,221,174,127,48);
        if (r != 670) failures++;
    }


    {
        uint32_t a = 108026041UL;
        uint32_t b = 4057403675UL;
        uint32_t r = a & b;
        if (r != 5247001UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(245,40) != 285) failures++;
    }


    {
        uint8_t v = 20;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {53,64,125,90,181,107,82,29};
        uint8_t *p = buf;
        p += 6;
        if (*p != 82) failures++;
    }


    {
        uint8_t x = 191;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint8_t x = 116;
        x <<= 3;
        if (x != 160) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        volatile int16_t a = -13560;
        volatile int16_t b = 12385;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(192,119) != 311) failures++;
    }


    {
        uint16_t r = add2(73,160) + add2(160,131) + add2(73,131);
        if (r != 728) failures++;
    }


    {
        uint16_t r = call6(88,84,62,86,46,153);
        if (r != 519) failures++;
    }


    {
        uint8_t src[1] = {147};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 147) failures++;
    }


    {
        uint16_t r = call6(80,253,137,218,165,156);
        if (r != 1009) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 157;
        if (buf[10] != 157) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {2,125,56477,166};
        if (s.c != (uint16_t)56477) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 1) sum += j;
        if (sum != 36) failures++;
    }


    {
        int8_t a = 43;
        int8_t b = 100;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {179,112,114,105,151,137};
        if (a[2] != 114) failures++;
    }


    {
        uint8_t v = 173;
        v ^= 1;
        if (v != 172) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint8_t a[6] = {54,25,87,166,14,91};
        if (a[5] != 91) failures++;
    }


    {
        uint16_t r = add2(131,165) + add2(165,103) + add2(131,103);
        if (r != 798) failures++;
    }


    {
        uint8_t v = 215;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t x = 45649;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(105,0) != 105) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {7,3,413,252};
        if (s.b != (uint8_t)3) failures++;
    }


    {
        uint32_t a = 1217759923UL;
        uint32_t b = 1751225417UL;
        uint32_t r = a & b;
        if (r != 1208057857UL) failures++;
    }


    {
        uint16_t r = add2(125,240) + add2(240,199) + add2(125,199);
        if (r != 1128) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint32_t a = 1492525254UL;
        uint32_t b = 1414449371UL;
        uint32_t r = a - b;
        if (r != 78075883UL) failures++;
    }


    {
        uint8_t v = 179;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)96) + (uint16_t)55950;
        if (r != 56046) failures++;
    }


    {
        volatile uint8_t port = 11;
        uint8_t r = port;
        if (r != 11) failures++;
    }


    {
        int8_t a = 53;
        int8_t b = 48;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        if (((uint16_t)(((95 - 11) + 13) & ((74 | 234) | 45))) != 97) failures++;
    }


    {
        uint8_t a[6] = {144,108,246,234,2,35};
        if (a[4] != 2) failures++;
    }


    {
        uint8_t buf[8] = {215,4,247,172,249,40,105,2};
        uint8_t *p = buf;
        p += 5;
        if (*p != 40) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint16_t x = 113;
        x = x + 79;
        if (x != 192) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 7: result = 230; break;
        case 17: result = 71; break;
        case 18: result = 218; break;
        default: result = 63; break;
        }
        if (result != 218) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)129) + (uint16_t)59236;
        if (r != 59365) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 11: result = 78; break;
        case 18: result = 93; break;
        case 14: result = 49; break;
        case 17: result = 235; break;
        case 8: result = 248; break;
        default: result = 80; break;
        }
        if (result != 248) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 52;
        if (buf[1] != 52) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)66) % (int16_t)((int8_t)99);
        if ((uint16_t)r != (uint16_t)66) failures++;
    }


    {
        uint16_t x = 36886;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)((101 | (168 ^ 133)) ^ ((45 & 123) & (242 & 179)))) != 77) failures++;
    }


    {
        uint32_t a = 1615617386UL;
        uint32_t b = 3016821091UL;
        uint32_t r = a - b;
        if (r != 2893763591UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[12] = {137,31,118,124,5,195,108,51,151,240,161,77};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[10] != 161) failures++;
    }


    {
        volatile int16_t a = 12753;
        volatile int16_t b = 1317;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 423285696UL;
        uint32_t b = 1666171698UL;
        uint32_t r = a & b;
        if (r != 17482496UL) failures++;
    }


    {
        uint8_t m[4][2] = {{148,28},{72,146},{117,106},{184,12}};
        if (m[3][0] != 184) failures++;
    }


    {
        uint8_t v = 66;
        v ^= 16;
        if (v != 82) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 12: result = 96; break;
        case 16: result = 90; break;
        case 17: result = 227; break;
        case 18: result = 87; break;
        case 2: result = 231; break;
        case 3: result = 151; break;
        default: result = 42; break;
        }
        if (result != 227) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {213,232,51621,187};
        if (s.d != (uint8_t)187) failures++;
    }


    {
        if (((uint16_t)(((161 - 185) & (50 | 209)) ^ 227)) != 3) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint16_t r = call6(158,73,57,127,37,247);
        if (r != 699) failures++;
    }


    {
        volatile uint8_t port = 10;
        uint8_t r = port;
        if (r != 10) failures++;
    }


    {
        uint8_t x = 214;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint32_t a = 1782610042UL;
        uint32_t b = 3390254775UL;
        uint32_t r = a ^ b;
        if (r != 2689816269UL) failures++;
    }


    {
        uint16_t r = call6(33,190,150,172,204,172);
        if (r != 921) failures++;
    }


    {
        uint8_t v = 155;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 37) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)60) + (uint16_t)59566;
        if (r != 59626) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)40) + (uint16_t)1956;
        if (r != 1996) failures++;
    }


    {
        uint8_t v = 46;
        v |= 1;
        if (v != 47) failures++;
    }


    {
        uint16_t x = 31083;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)182) + (uint16_t)49288;
        if (r != 49470) failures++;
    }


    {
        uint8_t a[6] = {43,33,171,26,88,176};
        if (a[0] != 43) failures++;
    }


    {
        uint8_t v = 4;
        v ^= 16;
        if (v != 20) failures++;
    }


    {
        g16 = 28713;
        if (read_g16() != 28713) failures++;
    }


    {
        uint16_t r = add2(136,76) + add2(76,215) + add2(136,215);
        if (r != 854) failures++;
    }


    {
        uint8_t a[6] = {238,30,157,49,251,247};
        if (a[5] != 247) failures++;
    }


    {
        uint8_t x = 44;
        x <<= 3;
        if (x != 96) failures++;
    }


    {
        uint16_t x = 67;
        x = x + 17;
        if (x != 84) failures++;
    }


    {
        uint8_t x = 153;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t v = 139;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 21) failures++;
    }


    {
        uint16_t x = 237;
        x = x + 123;
        if (x != 360) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 8: result = 231; break;
        case 14: result = 119; break;
        case 12: result = 202; break;
        case 15: result = 37; break;
        default: result = 245; break;
        }
        if (result != 37) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)28) + (uint16_t)65297;
        if (r != 65325) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 165;
        if (buf[9] != 165) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 13: result = 111; break;
        case 2: result = 119; break;
        case 9: result = 239; break;
        case 4: result = 254; break;
        case 8: result = 228; break;
        default: result = 148; break;
        }
        if (result != 254) failures++;
    }


    {
        uint8_t src[2] = {160,19};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 160) failures++;
    }


    {
        uint32_t a = 1184828316UL;
        uint32_t b = 1996266067UL;
        uint32_t r = a + b;
        if (r != 3181094383UL) failures++;
    }


    {
        uint8_t buf[8] = {86,28,7,108,216,82,132,160};
        uint8_t *p = buf;
        p += 6;
        if (*p != 132) failures++;
    }


    {
        uint16_t r = add2(169,219) + add2(219,194) + add2(169,194);
        if (r != 1164) failures++;
    }


    {
        uint8_t src[10] = {65,189,223,205,133,68,102,57,228,252};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[2] != 223) failures++;
    }


    {
        uint16_t x = 113;
        x = x + 147;
        if (x != 260) failures++;
    }


    {
        volatile uint8_t port = 146;
        uint8_t r = port;
        if (r != 146) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t a[6] = {84,89,9,213,120,234};
        if (a[2] != 9) failures++;
    }


    {
        uint16_t x = 173;
        x = x + 137;
        if (x != 310) failures++;
    }


    {
        uint8_t m[3][2] = {{210,106},{198,208},{103,227}};
        if (m[0][1] != 106) failures++;
    }


    {
        uint16_t r = call6(151,37,144,35,154,195);
        if (r != 716) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 195;
        if (buf[13] != 195) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        g16 = 15867;
        if (read_g16() != 15867) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 17: result = 57; break;
        case 16: result = 182; break;
        case 6: result = 17; break;
        case 8: result = 95; break;
        case 19: result = 201; break;
        case 12: result = 82; break;
        default: result = 220; break;
        }
        if (result != 201) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-117) % (int16_t)((int8_t)64);
        if ((uint16_t)r != (uint16_t)65483) failures++;
    }


    {
        uint16_t r = call6(132,167,213,181,204,36);
        if (r != 933) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint16_t r = add2(139,15) + add2(15,15) + add2(139,15);
        if (r != 338) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)173) + (uint16_t)30059;
        if (r != 30232) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {163,38,34600,220};
        if (s.c != (uint16_t)34600) failures++;
    }


    {
        uint8_t src[8] = {45,70,59,42,254,170,185,32};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[4] != 254) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 163;
        if (buf[5] != 163) failures++;
    }


    {
        volatile int16_t a = 11720;
        volatile int16_t b = -27247;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(36,232) != 268) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)249) + (uint16_t)41210;
        if (r != 41459) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(54,2) != 56) failures++;
    }


    {
        g16 = 56363;
        if (read_g16() != 56363) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)219) + (uint16_t)29924;
        if (r != 30143) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)106) != 106) failures++;
    }


    {
        uint8_t buf[8] = {237,149,13,36,86,101,183,244};
        uint8_t *p = buf;
        p += 4;
        if (*p != 86) failures++;
    }


    {
        volatile int16_t a = 28493;
        volatile int16_t b = 3558;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        g16 = 11802;
        if (read_g16() != 11802) failures++;
    }


    {
        uint8_t src[2] = {255,10};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 10) failures++;
    }


    {
        uint16_t r = 32049 + 32248 + 56348 + 31582 + 21755 + 20376 + 1514 + 48795;
        if (r != 48059) failures++;
    }


    {
        uint8_t a[6] = {128,44,142,90,174,151};
        if (a[4] != 174) failures++;
    }


    {
        uint16_t x = 52883;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {193,15,63125,149};
        if (s.a != (uint8_t)193) failures++;
    }


    {
        uint8_t buf[8] = {16,213,81,231,167,99,89,198};
        uint8_t *p = buf;
        p += 7;
        if (*p != 198) failures++;
    }


    {
        uint16_t r = add2(219,215) + add2(215,226) + add2(219,226);
        if (r != 1320) failures++;
    }


    {
        uint8_t m[4][4] = {{146,75,226,133},{143,44,115,95},{34,145,201,202},{57,32,248,119}};
        if (m[2][2] != 201) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(112,54) != 58) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {122,151,53597,121};
        if (s.a != (uint8_t)122) failures++;
    }


    {
        uint16_t x = 109;
        x = x + 196;
        if (x != 305) failures++;
    }


    {
        uint8_t a[6] = {100,128,46,36,4,254};
        if (a[1] != 128) failures++;
    }


    {
        uint16_t r = add2(86,194) + add2(194,242) + add2(86,242);
        if (r != 1044) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)88) % (int16_t)((int8_t)53);
        if ((uint16_t)r != (uint16_t)35) failures++;
    }


    {
        uint16_t r = 12199 + 32779 + 20527 + 27824 + 1645 + 25732 + 13891 + 30322;
        if (r != 33847) failures++;
    }


    {
        uint8_t v = 57;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 7) failures++;
    }


    {
        g16 = 17879;
        if (read_g16() != 17879) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 9565;
        if (read_g16() != 9565) failures++;
    }


    {
        uint8_t a[6] = {50,38,248,56,183,253};
        if (a[4] != 183) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 4: result = 63; break;
        case 18: result = 149; break;
        case 10: result = 179; break;
        default: result = 162; break;
        }
        if (result != 179) failures++;
    }


    {
        uint8_t v = 219;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 38;
        x = x + 148;
        if (x != 186) failures++;
    }


    {
        volatile uint8_t port = 63;
        uint8_t r = port;
        if (r != 63) failures++;
    }


    {
        int8_t a = -46;
        int8_t b = -20;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 222;
        uint8_t r = port;
        if (r != 222) failures++;
    }


    {
        uint8_t buf[8] = {42,42,64,46,92,119,10,242};
        uint8_t *p = buf;
        p += 6;
        if (*p != 10) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(61,216) != 277) failures++;
    }


    {
        uint32_t a = 2638327378UL;
        uint32_t b = 189309472UL;
        uint32_t r = a ^ b;
        if (r != 2517175410UL) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 168;
        if (buf[15] != 168) failures++;
    }


    {
        volatile uint8_t port = 137;
        uint8_t r = port;
        if (r != 137) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)72) / (int16_t)((int8_t)60);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 5: result = 27; break;
        case 9: result = 133; break;
        case 11: result = 40; break;
        case 18: result = 48; break;
        default: result = 30; break;
        }
        if (result != 30) failures++;
    }


    {
        volatile int16_t a = -2252;
        volatile int16_t b = 24088;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 7: result = 187; break;
        case 15: result = 136; break;
        case 17: result = 215; break;
        case 3: result = 83; break;
        default: result = 28; break;
        }
        if (result != 136) failures++;
    }


    {
        uint8_t v = 31;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint16_t r = call6(75,80,41,19,66,155);
        if (r != 436) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(211,175) != 386) failures++;
    }


    {
        uint16_t x = 47929;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(26,228) + add2(228,140) + add2(26,140);
        if (r != 788) failures++;
    }


    {
        volatile int16_t a = 1218;
        volatile int16_t b = 24174;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 36426 + 18263 + 22654 + 17208 + 26750 + 62635 + 23761 + 3233;
        if (r != 14322) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint16_t r = 3754 + 5994 + 63608 + 58496 + 53592 + 822 + 21123 + 52201;
        if (r != 62982) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 9: result = 60; break;
        case 11: result = 223; break;
        case 4: result = 51; break;
        default: result = 159; break;
        }
        if (result != 159) failures++;
    }


    {
        uint8_t src[13] = {194,121,214,153,211,113,142,190,160,206,243,171,156};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[8] != 160) failures++;
    }


    {
        uint16_t r = 33345 + 18705 + 60535 + 65042 + 6775 + 34322 + 12618 + 31485;
        if (r != 683) failures++;
    }


    {
        volatile uint8_t port = 92;
        uint8_t r = port;
        if (r != 92) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t buf[8] = {241,151,43,173,6,15,226,155};
        uint8_t *p = buf;
        p += 5;
        if (*p != 15) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 16: result = 231; break;
        case 9: result = 3; break;
        case 1: result = 159; break;
        case 12: result = 46; break;
        case 6: result = 172; break;
        case 11: result = 161; break;
        case 10: result = 46; break;
        default: result = 154; break;
        }
        if (result != 172) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)126) / (int16_t)((int8_t)-19);
        if ((uint16_t)r != (uint16_t)65530) failures++;
    }


    {
        uint8_t x = 199;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        uint8_t src[1] = {105};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 105) failures++;
    }


    {
        uint16_t x = 51803;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[2][3] = {{72,182,32},{172,154,26}};
        if (m[0][1] != 182) failures++;
    }


    {
        uint8_t m[4][2] = {{111,37},{87,178},{199,124},{165,245}};
        if (m[2][0] != 199) failures++;
    }


    {
        uint8_t buf[8] = {226,39,143,185,74,24,87,106};
        uint8_t *p = buf;
        p += 0;
        if (*p != 226) failures++;
    }


    {
        uint8_t a[6] = {17,244,184,158,233,253};
        if (a[3] != 158) failures++;
    }


    {
        if (((uint16_t)(99 | (148 - (103 ^ 168)))) != 65511) failures++;
    }


    {
        uint8_t a[6] = {146,243,253,53,215,41};
        if (a[3] != 53) failures++;
    }


    {
        uint16_t x = 3675;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 144;
        uint8_t r = port;
        if (r != 144) failures++;
    }


    {
        volatile uint8_t port = 239;
        uint8_t r = port;
        if (r != 239) failures++;
    }


    {
        uint8_t m[2][4] = {{198,142,89,189},{148,205,178,120}};
        if (m[0][3] != 189) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)112) + (uint16_t)1317;
        if (r != 1429) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 10: result = 86; break;
        case 12: result = 120; break;
        case 18: result = 36; break;
        case 3: result = 244; break;
        case 14: result = 214; break;
        case 2: result = 240; break;
        case 7: result = 171; break;
        case 0: result = 204; break;
        default: result = 59; break;
        }
        if (result != 214) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(221,48) != 269) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(184,221,46,154,17,137);
        if (r != 759) failures++;
    }


    {
        uint32_t a = 710999158UL;
        uint32_t b = 1951637936UL;
        uint32_t r = a ^ b;
        if (r != 1580424646UL) failures++;
    }


    {
        uint8_t a[6] = {162,193,111,31,121,61};
        if (a[4] != 121) failures++;
    }


    {
        uint16_t r = call6(69,27,214,133,70,217);
        if (r != 730) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(16,201) != 217) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 107;
        if (buf[13] != 107) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint16_t r = call6(34,85,143,229,253,145);
        if (r != 889) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 81;
        if (buf[8] != 81) failures++;
    }


    {
        uint8_t v = 42;
        v &= ~(uint8_t)4;
        if (v != 42) failures++;
    }


    {
        uint8_t v = 46;
        v &= ~(uint8_t)16;
        if (v != 46) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {3,233,36809,171};
        if (s.d != (uint8_t)171) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 3: result = 45; break;
        case 18: result = 27; break;
        case 5: result = 209; break;
        case 14: result = 249; break;
        default: result = 196; break;
        }
        if (result != 249) failures++;
    }


    {
        uint16_t x = 56;
        x = x + 125;
        if (x != 181) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(10,120) != 130) failures++;
    }


    {
        uint16_t r = 29201 + 22839 + 18476 + 33456 + 48727 + 2635 + 28322 + 63817;
        if (r != 50865) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {223,41,88,178,217,89,45,160};
        uint8_t *p = buf;
        p += 1;
        if (*p != 41) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-78) / (int16_t)((int8_t)77);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        if (((uint16_t)(163 - ((98 | 21) - (206 & 4)))) != 48) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 1) sum += j;
        if (sum != 36) failures++;
    }


    {
        uint16_t x = 30027;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 226;
        v &= ~(uint8_t)128;
        if (v != 98) failures++;
    }


    {
        uint16_t r = 7612 + 29059 + 41660 + 14163 + 1829 + 41399 + 32851 + 53539;
        if (r != 25504) failures++;
    }


    {
        g16 = 15786;
        if (read_g16() != 15786) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        if (((uint16_t)(((65 - 86) | (149 & 240)) ^ ((192 - 202) ^ 151))) != 154) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 15: result = 50; break;
        case 2: result = 52; break;
        case 1: result = 253; break;
        case 12: result = 30; break;
        case 14: result = 70; break;
        case 6: result = 95; break;
        default: result = 23; break;
        }
        if (result != 52) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(173,231) != 65478) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {226,106,18335,248};
        if (s.b != (uint8_t)106) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        int8_t a = 69;
        int8_t b = 72;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 2598110638UL;
        uint32_t b = 2372987922UL;
        uint32_t r = a | b;
        if (r != 2684153278UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {106,5,51518,100};
        if (s.d != (uint8_t)100) failures++;
    }


    {
        uint16_t x = 172;
        x = x + 222;
        if (x != 394) failures++;
    }


    {
        g16 = 38975;
        if (read_g16() != 38975) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {247,121,52034,5};
        if (s.d != (uint8_t)5) failures++;
    }


    {
        volatile int16_t a = 16087;
        volatile int16_t b = -32738;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 133;
        if (buf[11] != 133) failures++;
    }


    {
        uint8_t x = 242;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t v = 80;
        int r = (v & 4) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t x = 155;
        x = x + 95;
        if (x != 250) failures++;
    }


    {
        g16 = 35192;
        if (read_g16() != 35192) failures++;
    }


    {
        uint8_t m[3][4] = {{5,15,177,219},{49,100,189,235},{51,100,40,88}};
        if (m[0][1] != 15) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 1) sum += j;
        if (sum != 15) failures++;
    }


    {
        g16 = 56348;
        if (read_g16() != 56348) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)6) / (int16_t)((int8_t)114);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 67;
        uint8_t r = port;
        if (r != 67) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-91) / (int16_t)((int8_t)99);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t m[3][2] = {{17,140},{104,90},{120,26}};
        if (m[2][1] != 26) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(40,152) != 65424) failures++;
    }


    {
        volatile uint8_t port = 141;
        uint8_t r = port;
        if (r != 141) failures++;
    }


    {
        volatile uint8_t port = 166;
        uint8_t r = port;
        if (r != 166) failures++;
    }


    {
        g16 = 91;
        if (read_g16() != 91) failures++;
    }


    {
        uint8_t v = 154;
        v ^= 1;
        if (v != 155) failures++;
    }


    {
        uint16_t r = 33763 + 43825 + 62934 + 20803 + 14037 + 53670 + 43591 + 49981;
        if (r != 60460) failures++;
    }


    {
        uint16_t r = 55022 + 54708 + 63625 + 10454 + 4064 + 33771 + 51260 + 62371;
        if (r != 7595) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 218;
        if (buf[11] != 218) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 93;
        if (buf[13] != 93) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)234) + (uint16_t)32894;
        if (r != 33128) failures++;
    }


    {
        g16 = 58933;
        if (read_g16() != 58933) failures++;
    }


    {
        uint16_t x = 62431;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 133;
        v ^= 8;
        if (v != 141) failures++;
    }


    {
        if (((uint16_t)(((114 - 212) + 15) ^ 15)) != 65442) failures++;
    }


    {
        uint16_t r = 17267 + 40355 + 61941 + 25711 + 54813 + 54662 + 37028 + 62409;
        if (r != 26506) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)64) % (int16_t)((int8_t)48);
        if ((uint16_t)r != (uint16_t)16) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 15: result = 17; break;
        case 4: result = 135; break;
        case 0: result = 97; break;
        default: result = 93; break;
        }
        if (result != 17) failures++;
    }


    {
        uint8_t x = 181;
        x <<= 4;
        if (x != 80) failures++;
    }


    {
        uint8_t buf[8] = {45,161,234,198,209,51,109,185};
        uint8_t *p = buf;
        p += 3;
        if (*p != 198) failures++;
    }


    {
        volatile uint8_t port = 87;
        uint8_t r = port;
        if (r != 87) failures++;
    }


    {
        uint8_t src[11] = {65,222,20,37,198,36,126,76,239,200,169};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[2] != 20) failures++;
    }


    {
        if (((uint16_t)(((125 | 39) ^ 61) | ((126 - 145) ^ (243 & 253)))) != 65374) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t r = add2(189,176) + add2(176,211) + add2(189,211);
        if (r != 1152) failures++;
    }


    {
        if (((uint16_t)(((25 - 178) + 201) & 251)) != 48) failures++;
    }


    {
        uint8_t a[6] = {237,251,106,229,43,132};
        if (a[5] != 132) failures++;
    }


    {
        uint32_t a = 2713144113UL;
        uint32_t b = 1988878191UL;
        uint32_t r = a - b;
        if (r != 724265922UL) failures++;
    }


    {
        uint8_t m[4][2] = {{5,3},{158,200},{218,42},{53,206}};
        if (m[3][1] != 206) failures++;
    }


    {
        uint8_t a[6] = {170,91,45,104,148,26};
        if (a[2] != 45) failures++;
    }


    {
        uint16_t r = 55397 + 57071 + 62914 + 4169 + 24637 + 24524 + 44042 + 38443;
        if (r != 49053) failures++;
    }


    {
        uint16_t x = 92;
        x = x + 197;
        if (x != 289) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 52;
        if (buf[10] != 52) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 19: result = 190; break;
        case 11: result = 148; break;
        case 14: result = 15; break;
        case 8: result = 34; break;
        case 1: result = 185; break;
        case 18: result = 34; break;
        default: result = 168; break;
        }
        if (result != 185) failures++;
    }


    {
        g16 = 52963;
        if (read_g16() != 52963) failures++;
    }


    {
        uint16_t r = call6(44,149,73,13,39,154);
        if (r != 472) failures++;
    }


    {
        if (((uint16_t)(210 ^ ((135 ^ 113) + (165 | 28)))) != 353) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 27;
        if (buf[7] != 27) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(82,199) != 65419) failures++;
    }


    {
        uint32_t a = 3716638158UL;
        uint32_t b = 3832846358UL;
        uint32_t r = a + b;
        if (r != 3254517220UL) failures++;
    }


    {
        uint32_t a = 2478495512UL;
        uint32_t b = 769961504UL;
        uint32_t r = a | b;
        if (r != 3221159736UL) failures++;
    }


    {
        uint16_t x = 117;
        x = x + 23;
        if (x != 140) failures++;
    }


    {
        uint8_t src[11] = {88,155,69,125,74,47,149,45,8,192,10};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[5] != 47) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint16_t x = 196;
        x = x + 39;
        if (x != 235) failures++;
    }


    {
        uint8_t v = 22;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 46795;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 116;
        int8_t b = 64;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 126;
        v &= ~(uint8_t)64;
        if (v != 62) failures++;
    }


    {
        if (((uint16_t)(((32 | 107) & (93 & 190)) - 149)) != 65395) failures++;
    }


    {
        uint8_t src[11] = {57,73,152,30,69,146,230,82,203,225,220};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[4] != 69) failures++;
    }


    {
        uint16_t x = 23522;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 21520 + 21444 + 56747 + 40916 + 12685 + 63970 + 15008 + 16254;
        if (r != 51936) failures++;
    }


    {
        volatile int16_t a = -24700;
        volatile int16_t b = -10384;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 4117679976UL;
        uint32_t b = 210788997UL;
        uint32_t r = a | b;
        if (r != 4261343213UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 2) sum += j;
        if (sum != 42) failures++;
    }


    {
        uint8_t v = 196;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 1049360230UL;
        uint32_t b = 4215215117UL;
        uint32_t r = a + b;
        if (r != 969608051UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)123) + (uint16_t)20704;
        if (r != 20827) failures++;
    }


    {
        uint8_t x = 164;
        x <<= 2;
        if (x != 144) failures++;
    }


    {
        uint16_t r = 33880 + 5971 + 5507 + 36343 + 29311 + 34278 + 26568 + 17549;
        if (r != 58335) failures++;
    }


    {
        uint8_t v = 90;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {165,72,69,106,57,174,20,40};
        uint8_t *p = buf;
        p += 5;
        if (*p != 174) failures++;
    }


    {
        uint16_t r = call6(160,6,40,207,129,201);
        if (r != 743) failures++;
    }


    {
        int8_t a = 47;
        int8_t b = 32;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 45973 + 60038 + 42437 + 16613 + 17747 + 24980 + 1294 + 38996;
        if (r != 51470) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {206,177,12343,38};
        if (s.d != (uint8_t)38) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-110) / (int16_t)((int8_t)-3);
        if ((uint16_t)r != (uint16_t)36) failures++;
    }


    {
        g16 = 31520;
        if (read_g16() != 31520) failures++;
    }


    {
        int8_t a = -91;
        int8_t b = 4;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {1,49,13550,175};
        if (s.d != (uint8_t)175) failures++;
    }


    {
        uint8_t a[6] = {200,205,121,214,134,186};
        if (a[1] != 205) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint8_t m[2][4] = {{106,6,96,214},{248,11,3,172}};
        if (m[0][2] != 96) failures++;
    }


    {
        uint16_t x = 83;
        x = x + 160;
        if (x != 243) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(124,132) != 65528) failures++;
    }


    {
        uint8_t v = 238;
        v |= 2;
        if (v != 238) failures++;
    }


    {
        uint16_t r = call6(93,118,38,143,13,205);
        if (r != 610) failures++;
    }


    {
        uint16_t r = call6(36,236,4,21,43,81);
        if (r != 421) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 39;
        if (buf[1] != 39) failures++;
    }


    {
        uint8_t v = 218;
        v |= 2;
        if (v != 218) failures++;
    }


    {
        uint8_t v = 254;
        v ^= 2;
        if (v != 252) failures++;
    }


    {
        uint16_t x = 10232;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {17,87,16328,52};
        if (s.a != (uint8_t)17) failures++;
    }


    {
        uint8_t x = 84;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        volatile uint8_t port = 83;
        uint8_t r = port;
        if (r != 83) failures++;
    }


    {
        uint8_t v = 165;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 27) failures++;
    }


    {
        uint16_t r = add2(125,194) + add2(194,67) + add2(125,67);
        if (r != 772) failures++;
    }


    {
        uint16_t r = call6(167,25,113,195,13,102);
        if (r != 615) failures++;
    }


    {
        uint16_t x = 129;
        x = x + 3;
        if (x != 132) failures++;
    }


    {
        uint8_t v = 127;
        v ^= 1;
        if (v != 126) failures++;
    }


    {
        uint8_t buf[8] = {150,233,156,70,170,97,105,40};
        uint8_t *p = buf;
        p += 5;
        if (*p != 97) failures++;
    }


    {
        uint16_t x = 65;
        x = x + 74;
        if (x != 139) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(104,24) != 128) failures++;
    }


    {
        uint8_t buf[8] = {50,41,118,183,161,123,141,101};
        uint8_t *p = buf;
        p += 7;
        if (*p != 101) failures++;
    }


    {
        uint8_t buf[8] = {104,79,115,203,243,205,158,97};
        uint8_t *p = buf;
        p += 0;
        if (*p != 104) failures++;
    }


    {
        uint16_t r = call6(124,47,195,183,7,198);
        if (r != 754) failures++;
    }


    {
        uint8_t v = 129;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = call6(93,191,121,171,71,111);
        if (r != 758) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 19: result = 2; break;
        case 9: result = 28; break;
        case 17: result = 255; break;
        case 11: result = 29; break;
        case 12: result = 122; break;
        case 2: result = 84; break;
        default: result = 114; break;
        }
        if (result != 2) failures++;
    }


    {
        uint32_t a = 1875820771UL;
        uint32_t b = 1146355295UL;
        uint32_t r = a & b;
        if (r != 1145225283UL) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 15: result = 229; break;
        case 3: result = 252; break;
        case 8: result = 26; break;
        default: result = 12; break;
        }
        if (result != 12) failures++;
    }


    {
        uint16_t r = call6(149,170,172,140,80,178);
        if (r != 889) failures++;
    }


    {
        uint16_t x = 199;
        x = x + 155;
        if (x != 354) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 1: result = 165; break;
        case 7: result = 66; break;
        case 2: result = 124; break;
        case 8: result = 103; break;
        case 0: result = 238; break;
        case 14: result = 19; break;
        default: result = 91; break;
        }
        if (result != 238) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {162,233,41026,202};
        if (s.c != (uint16_t)41026) failures++;
    }


    {
        uint8_t src[9] = {235,19,133,71,201,61,104,0,99};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 133) failures++;
    }


    {
        g16 = 58307;
        if (read_g16() != 58307) failures++;
    }


    {
        uint8_t v = 192;
        v |= 16;
        if (v != 208) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 11: result = 116; break;
        case 4: result = 100; break;
        case 10: result = 26; break;
        default: result = 109; break;
        }
        if (result != 100) failures++;
    }


    {
        uint16_t r = 7834 + 12840 + 18028 + 32008 + 17237 + 40303 + 63841 + 41948;
        if (r != 37431) failures++;
    }


    {
        uint8_t buf[8] = {124,130,196,143,149,194,181,148};
        uint8_t *p = buf;
        p += 0;
        if (*p != 124) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 12: result = 184; break;
        case 13: result = 116; break;
        case 10: result = 144; break;
        case 19: result = 170; break;
        case 6: result = 63; break;
        case 8: result = 174; break;
        case 7: result = 117; break;
        case 2: result = 150; break;
        default: result = 53; break;
        }
        if (result != 150) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 215;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        volatile int16_t a = -30137;
        volatile int16_t b = 30151;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {126,6,145,214,39,196};
        if (a[2] != 145) failures++;
    }


    {
        uint8_t src[10] = {86,84,174,67,198,167,187,200,56,112};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[2] != 174) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)117) / (int16_t)((int8_t)29);
        if ((uint16_t)r != (uint16_t)4) failures++;
    }


    {
        uint16_t r = add2(179,209) + add2(209,120) + add2(179,120);
        if (r != 1016) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {153,6,58502,64};
        if (s.a != (uint8_t)153) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 8: result = 46; break;
        case 16: result = 111; break;
        case 18: result = 69; break;
        case 13: result = 96; break;
        case 2: result = 206; break;
        case 12: result = 87; break;
        case 15: result = 14; break;
        case 3: result = 41; break;
        default: result = 67; break;
        }
        if (result != 111) failures++;
    }


    {
        uint8_t src[8] = {181,91,57,217,38,204,94,251};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[5] != 204) failures++;
    }


    {
        uint8_t buf[8] = {158,214,54,201,136,241,74,144};
        uint8_t *p = buf;
        p += 1;
        if (*p != 214) failures++;
    }


    {
        uint16_t r = add2(27,110) + add2(110,209) + add2(27,209);
        if (r != 692) failures++;
    }


    {
        volatile int16_t a = -16055;
        volatile int16_t b = -21692;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint32_t a = 2178793418UL;
        uint32_t b = 2753846332UL;
        uint32_t r = a ^ b;
        if (r != 637133814UL) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 140;
        if (buf[3] != 140) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {74,161,43862,57};
        if (s.b != (uint8_t)161) failures++;
    }


    {
        uint8_t buf[8] = {209,248,165,247,86,193,190,243};
        uint8_t *p = buf;
        p += 6;
        if (*p != 190) failures++;
    }


    {
        int8_t a = 113;
        int8_t b = 6;
        int r = (a < b);
        if (r != 0) failures++;
    }

    return failures;
}