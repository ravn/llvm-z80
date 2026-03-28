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
        if (((uint16_t)(33 ^ ((70 | 231) - (247 - 180)))) != 133) failures++;
    }


    {
        uint8_t a[6] = {95,53,227,92,210,136};
        if (a[0] != 95) failures++;
    }


    {
        volatile int16_t a = 23682;
        volatile int16_t b = 13997;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {130,87,13,100,1,232,29,158};
        uint8_t *p = buf;
        p += 6;
        if (*p != 29) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 72;
        if (buf[6] != 72) failures++;
    }


    {
        uint32_t a = 1457477928UL;
        uint32_t b = 2945472498UL;
        uint32_t r = a + b;
        if (r != 107983130UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 1) sum += j;
        if (sum != 36) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {102,126,64471,1};
        if (s.b != (uint8_t)126) failures++;
    }


    {
        g16 = 6435;
        if (read_g16() != 6435) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {108,120,32386,162};
        if (s.d != (uint8_t)162) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {220,255,1784,214};
        if (s.a != (uint8_t)220) failures++;
    }


    {
        uint32_t a = 3031437918UL;
        uint32_t b = 257999603UL;
        uint32_t r = a & b;
        if (r != 69206610UL) failures++;
    }


    {
        uint16_t r = call6(245,16,209,225,44,247);
        if (r != 986) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 37;
        if (buf[2] != 37) failures++;
    }


    {
        uint8_t m[2][3] = {{10,229,87},{154,128,223}};
        if (m[0][0] != 10) failures++;
    }


    {
        uint16_t r = add2(97,191) + add2(191,22) + add2(97,22);
        if (r != 620) failures++;
    }


    {
        if (((uint16_t)156) != 156) failures++;
    }


    {
        uint8_t x = 169;
        x <<= 4;
        if (x != 144) failures++;
    }


    {
        volatile int16_t a = -24904;
        volatile int16_t b = 23057;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 10: result = 18; break;
        case 5: result = 136; break;
        case 11: result = 36; break;
        case 2: result = 139; break;
        case 8: result = 2; break;
        case 0: result = 87; break;
        default: result = 223; break;
        }
        if (result != 136) failures++;
    }


    {
        uint8_t m[3][2] = {{161,236},{71,198},{96,70}};
        if (m[0][0] != 161) failures++;
    }


    {
        volatile int16_t a = -26078;
        volatile int16_t b = -24053;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 55642;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[9] = {24,188,51,183,95,126,202,34,118};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 51) failures++;
    }


    {
        uint8_t a[6] = {62,39,98,245,104,49};
        if (a[0] != 62) failures++;
    }


    {
        uint16_t r = 38246 + 18905 + 21019 + 56445 + 23989 + 41711 + 39618 + 48129;
        if (r != 25918) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 10: result = 98; break;
        case 11: result = 156; break;
        case 5: result = 207; break;
        case 1: result = 185; break;
        case 16: result = 188; break;
        case 7: result = 124; break;
        case 15: result = 113; break;
        default: result = 63; break;
        }
        if (result != 113) failures++;
    }


    {
        uint16_t x = 46567;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 96;
        if (buf[15] != 96) failures++;
    }


    {
        volatile int16_t a = 29360;
        volatile int16_t b = 26342;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(181,149,37,212,139,83);
        if (r != 801) failures++;
    }


    {
        uint32_t a = 4187857616UL;
        uint32_t b = 3854677643UL;
        uint32_t r = a ^ b;
        if (r != 475796571UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 36231;
        if (read_g16() != 36231) failures++;
    }


    {
        uint8_t m[2][3] = {{165,129,18},{55,47,88}};
        if (m[0][0] != 165) failures++;
    }


    {
        uint8_t m[4][3] = {{208,146,2},{39,85,91},{9,123,225},{81,115,96}};
        if (m[3][2] != 96) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 126;
        if (buf[6] != 126) failures++;
    }


    {
        uint32_t a = 3309066225UL;
        uint32_t b = 3437414866UL;
        uint32_t r = a - b;
        if (r != 4166618655UL) failures++;
    }


    {
        uint16_t x = 43850;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 1) sum += j;
        if (sum != 21) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        if (((uint16_t)((33 + 42) | ((181 - 16) + 23))) != 255) failures++;
    }


    {
        uint32_t a = 1243318530UL;
        uint32_t b = 509939034UL;
        uint32_t r = a & b;
        if (r != 167840002UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)23) % (int16_t)((int8_t)96);
        if ((uint16_t)r != (uint16_t)23) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {244,216,1879,52};
        if (s.a != (uint8_t)244) failures++;
    }


    {
        volatile int16_t a = 5950;
        volatile int16_t b = -23324;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[5] = {12,178,82,224,120};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 224) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)113) + (uint16_t)39821;
        if (r != 39934) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)54) != 54) failures++;
    }


    {
        if (((uint16_t)(((161 & 77) & (214 ^ 205)) + ((69 | 209) - 244))) != 65506) failures++;
    }


    {
        g16 = 5946;
        if (read_g16() != 5946) failures++;
    }


    {
        g16 = 60931;
        if (read_g16() != 60931) failures++;
    }


    {
        uint32_t a = 871408518UL;
        uint32_t b = 2205324331UL;
        uint32_t r = a | b;
        if (r != 3019028399UL) failures++;
    }


    {
        uint8_t x = 71;
        x <<= 0;
        if (x != 71) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 17: result = 53; break;
        case 13: result = 206; break;
        case 16: result = 209; break;
        case 5: result = 246; break;
        case 12: result = 180; break;
        case 11: result = 27; break;
        default: result = 129; break;
        }
        if (result != 180) failures++;
    }


    {
        int8_t a = -33;
        int8_t b = -71;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 28025;
        volatile int16_t b = -6646;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{82,69,100},{224,12,60},{49,231,32},{183,241,163}};
        if (m[0][1] != 69) failures++;
    }


    {
        uint16_t r = 28385 + 24182 + 36764 + 31328 + 32513 + 60932 + 50820 + 20099;
        if (r != 22879) failures++;
    }


    {
        volatile uint8_t port = 123;
        uint8_t r = port;
        if (r != 123) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)193) + (uint16_t)63404;
        if (r != 63597) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)66) + (uint16_t)7977;
        if (r != 8043) failures++;
    }


    {
        uint8_t buf[8] = {88,113,236,234,120,114,255,57};
        uint8_t *p = buf;
        p += 0;
        if (*p != 88) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)92) + (uint16_t)16441;
        if (r != 16533) failures++;
    }


    {
        g16 = 16491;
        if (read_g16() != 16491) failures++;
    }


    {
        uint8_t m[3][4] = {{178,107,225,139},{238,193,234,127},{5,42,49,238}};
        if (m[1][0] != 238) failures++;
    }


    {
        int8_t a = -35;
        int8_t b = 120;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 6: result = 10; break;
        case 1: result = 50; break;
        case 18: result = 118; break;
        case 4: result = 20; break;
        case 13: result = 25; break;
        case 17: result = 252; break;
        case 16: result = 192; break;
        default: result = 12; break;
        }
        if (result != 118) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {214,227,18168,110};
        if (s.a != (uint8_t)214) failures++;
    }


    {
        uint16_t r = 40159 + 41867 + 7569 + 39446 + 59564 + 41745 + 49774 + 510;
        if (r != 18490) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {194,154,337,51};
        if (s.b != (uint8_t)154) failures++;
    }


    {
        uint16_t r = add2(164,162) + add2(162,71) + add2(164,71);
        if (r != 794) failures++;
    }


    {
        uint8_t src[2] = {78,153};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 153) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-34) % (int16_t)((int8_t)-121);
        if ((uint16_t)r != (uint16_t)65502) failures++;
    }


    {
        uint8_t v = 206;
        v |= 128;
        if (v != 206) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(171,103) != 274) failures++;
    }


    {
        uint8_t src[12] = {128,94,239,162,137,94,238,122,96,64,13,213};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[2] != 239) failures++;
    }


    {
        uint8_t a[6] = {41,187,238,132,157,69};
        if (a[5] != 69) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 13;
        if (buf[1] != 13) failures++;
    }


    {
        uint32_t a = 4076556001UL;
        uint32_t b = 4227285061UL;
        uint32_t r = a | b;
        if (r != 4227813093UL) failures++;
    }


    {
        uint16_t x = 132;
        x = x + 189;
        if (x != 321) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)68) + (uint16_t)24510;
        if (r != 24578) failures++;
    }


    {
        int8_t a = 96;
        int8_t b = 53;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)16) + (uint16_t)50621;
        if (r != 50637) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)9) != 9) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-31) % (int16_t)((int8_t)-110);
        if ((uint16_t)r != (uint16_t)65505) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 1) sum += j;
        if (sum != 105) failures++;
    }


    {
        volatile uint8_t port = 17;
        uint8_t r = port;
        if (r != 17) failures++;
    }


    {
        uint8_t x = 184;
        x <<= 3;
        if (x != 192) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)90) + (uint16_t)45075;
        if (r != 45165) failures++;
    }


    {
        uint8_t m[2][3] = {{144,201,152},{194,184,6}};
        if (m[1][0] != 194) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 145;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -23294;
        volatile int16_t b = -14922;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(((98 + 176) + (178 - 204)) ^ (120 | (164 | 122)))) != 6) failures++;
    }


    {
        uint16_t x = 25714;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 17: result = 120; break;
        case 18: result = 81; break;
        case 5: result = 99; break;
        case 6: result = 239; break;
        case 4: result = 166; break;
        case 11: result = 92; break;
        case 9: result = 158; break;
        case 10: result = 222; break;
        default: result = 190; break;
        }
        if (result != 99) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 4: result = 18; break;
        case 8: result = 84; break;
        case 5: result = 17; break;
        default: result = 139; break;
        }
        if (result != 139) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 241;
        if (buf[12] != 241) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {80,132,22961,207};
        if (s.c != (uint16_t)22961) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        volatile int16_t a = -3896;
        volatile int16_t b = 21464;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 31;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)143) + (uint16_t)7217;
        if (r != 7360) failures++;
    }


    {
        uint8_t src[8] = {66,191,218,224,250,218,41,106};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 191) failures++;
    }


    {
        uint8_t v = 247;
        v &= ~(uint8_t)1;
        if (v != 246) failures++;
    }


    {
        uint16_t r = 790 + 35123 + 56465 + 5573 + 5609 + 53675 + 46384 + 11887;
        if (r != 18898) failures++;
    }


    {
        uint8_t x = 69;
        x <<= 3;
        if (x != 40) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-72) / (int16_t)((int8_t)-7);
        if ((uint16_t)r != (uint16_t)10) failures++;
    }


    {
        int8_t a = -7;
        int8_t b = 7;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {231,17,30,75,66,219};
        if (a[2] != 30) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(5,204) != 65337) failures++;
    }


    {
        uint16_t x = 43765;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -73;
        int8_t b = 64;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 2209646290UL;
        uint32_t b = 3256659285UL;
        uint32_t r = a | b;
        if (r != 3283922903UL) failures++;
    }


    {
        uint8_t src[6] = {216,11,87,16,239,138};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[4] != 239) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)82) / (int16_t)((int8_t)-111);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)104) + (uint16_t)359;
        if (r != 463) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 237;
        if (buf[0] != 237) failures++;
    }


    {
        uint8_t x = 23;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t r = call6(135,106,163,26,125,126);
        if (r != 681) failures++;
    }


    {
        uint8_t x = 33;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t a[6] = {29,187,64,239,236,116};
        if (a[4] != 236) failures++;
    }


    {
        uint8_t m[4][4] = {{44,252,90,105},{203,245,69,10},{33,237,237,53},{31,147,56,168}};
        if (m[0][3] != 105) failures++;
    }


    {
        uint8_t x = 31;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(188,203) != 391) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(120,155) != 65501) failures++;
    }


    {
        uint8_t buf[8] = {96,210,82,4,12,146,80,126};
        uint8_t *p = buf;
        p += 4;
        if (*p != 12) failures++;
    }


    {
        uint16_t r = call6(17,216,98,98,245,132);
        if (r != 806) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {29,137,52866,138};
        if (s.c != (uint16_t)52866) failures++;
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
        for (uint16_t j = 0; j < 5; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint8_t src[11] = {220,163,243,74,89,52,47,250,14,203,106};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[4] != 89) failures++;
    }


    {
        uint8_t a[6] = {195,110,106,8,63,208};
        if (a[1] != 110) failures++;
    }


    {
        volatile int16_t a = -5507;
        volatile int16_t b = 19355;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 2;
        x <<= 3;
        if (x != 16) failures++;
    }


    {
        uint16_t r = call6(187,193,250,58,42,92);
        if (r != 822) failures++;
    }


    {
        uint16_t r = 35160 + 23903 + 1886 + 35545 + 42711 + 34229 + 29154 + 61610;
        if (r != 2054) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint16_t r = 6117 + 8720 + 14693 + 55296 + 28222 + 39647 + 11573 + 64618;
        if (r != 32278) failures++;
    }


    {
        if (((uint16_t)((221 - (44 - 217)) + ((58 ^ 36) + (139 & 130)))) != 554) failures++;
    }


    {
        uint16_t x = 8711;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 4062354622UL;
        uint32_t b = 150324726UL;
        uint32_t r = a ^ b;
        if (r != 4208417096UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(123,67,148,41,166,244);
        if (r != 789) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)101) % (int16_t)((int8_t)-91);
        if ((uint16_t)r != (uint16_t)10) failures++;
    }


    {
        g16 = 42490;
        if (read_g16() != 42490) failures++;
    }


    {
        uint8_t a[6] = {65,58,109,227,91,212};
        if (a[4] != 91) failures++;
    }


    {
        uint16_t x = 26;
        x = x + 229;
        if (x != 255) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(4,141) != 65399) failures++;
    }


    {
        uint8_t m[2][4] = {{115,192,238,39},{92,165,138,210}};
        if (m[0][3] != 39) failures++;
    }


    {
        volatile int16_t a = -6383;
        volatile int16_t b = -11642;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)9) + (uint16_t)40858;
        if (r != 40867) failures++;
    }


    {
        uint8_t v = 93;
        v &= ~(uint8_t)8;
        if (v != 85) failures++;
    }


    {
        uint8_t buf[8] = {218,18,170,9,145,215,19,154};
        uint8_t *p = buf;
        p += 1;
        if (*p != 18) failures++;
    }


    {
        uint8_t m[3][3] = {{239,170,10},{71,126,23},{228,0,184}};
        if (m[1][1] != 126) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {70,68,35700,224};
        if (s.c != (uint16_t)35700) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 138;
        if (buf[14] != 138) failures++;
    }


    {
        uint32_t a = 324734268UL;
        uint32_t b = 1861956946UL;
        uint32_t r = a + b;
        if (r != 2186691214UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t m[3][4] = {{209,190,193,168},{175,60,232,83},{64,31,81,162}};
        if (m[1][3] != 83) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(224,161) != 385) failures++;
    }


    {
        uint8_t buf[8] = {214,65,226,64,123,48,161,32};
        uint8_t *p = buf;
        p += 2;
        if (*p != 226) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 18: result = 103; break;
        case 0: result = 255; break;
        case 8: result = 142; break;
        default: result = 187; break;
        }
        if (result != 187) failures++;
    }


    {
        volatile int16_t a = -31909;
        volatile int16_t b = -4516;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-89) / (int16_t)((int8_t)-62);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(24,154) != 178) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 13: result = 19; break;
        case 14: result = 99; break;
        case 17: result = 2; break;
        case 12: result = 26; break;
        case 3: result = 46; break;
        default: result = 112; break;
        }
        if (result != 46) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)167) + (uint16_t)35781;
        if (r != 35948) failures++;
    }


    {
        uint16_t r = call6(84,128,4,197,206,218);
        if (r != 837) failures++;
    }


    {
        uint16_t r = call6(143,121,131,44,145,5);
        if (r != 589) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 190;
        x = x + 127;
        if (x != 317) failures++;
    }


    {
        uint16_t x = 50441;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 2: result = 130; break;
        case 9: result = 175; break;
        case 15: result = 246; break;
        case 6: result = 204; break;
        case 3: result = 186; break;
        case 19: result = 39; break;
        case 8: result = 21; break;
        case 14: result = 50; break;
        default: result = 192; break;
        }
        if (result != 175) failures++;
    }


    {
        uint8_t a[6] = {236,102,43,33,77,41};
        if (a[0] != 236) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 15: result = 111; break;
        case 16: result = 205; break;
        case 11: result = 96; break;
        case 1: result = 56; break;
        case 7: result = 121; break;
        case 14: result = 32; break;
        case 17: result = 22; break;
        default: result = 100; break;
        }
        if (result != 32) failures++;
    }


    {
        volatile int16_t a = 5569;
        volatile int16_t b = 23553;
        int r = (a > b);
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {114,10,15439,91};
        if (s.c != (uint16_t)15439) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(194,144) != 338) failures++;
    }


    {
        uint8_t v = 53;
        v |= 1;
        if (v != 53) failures++;
    }


    {
        g16 = 55611;
        if (read_g16() != 55611) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {125,26,896,197};
        if (s.c != (uint16_t)896) failures++;
    }


    {
        uint8_t x = 198;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint8_t buf[8] = {235,165,197,86,234,60,239,35};
        uint8_t *p = buf;
        p += 7;
        if (*p != 35) failures++;
    }


    {
        uint8_t a[6] = {234,68,212,166,220,58};
        if (a[1] != 68) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        int8_t a = 103;
        int8_t b = -75;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(22,107) + add2(107,78) + add2(22,78);
        if (r != 414) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t src[3] = {110,244,189};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 244) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(155,49) != 106) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)65) + (uint16_t)30409;
        if (r != 30474) failures++;
    }


    {
        uint16_t x = 38260;
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
        uint16_t x = 68;
        x = x + 3;
        if (x != 71) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)246) + (uint16_t)17490;
        if (r != 17736) failures++;
    }


    {
        uint8_t v = 147;
        v &= ~(uint8_t)1;
        if (v != 146) failures++;
    }


    {
        uint8_t x = 87;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t r = add2(75,144) + add2(144,182) + add2(75,182);
        if (r != 802) failures++;
    }


    {
        uint8_t buf[8] = {83,71,61,30,110,49,100,150};
        uint8_t *p = buf;
        p += 7;
        if (*p != 150) failures++;
    }


    {
        uint16_t r = 21566 + 33258 + 7355 + 54134 + 4394 + 24414 + 26638 + 23422;
        if (r != 64109) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {116,165,6404,165};
        if (s.b != (uint8_t)165) failures++;
    }


    {
        volatile int16_t a = -776;
        volatile int16_t b = 9279;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {63,118,13748,119};
        if (s.c != (uint16_t)13748) failures++;
    }


    {
        uint32_t a = 159964245UL;
        uint32_t b = 171930146UL;
        uint32_t r = a ^ b;
        if (r != 62369399UL) failures++;
    }


    {
        uint16_t r = call6(122,235,139,97,179,59);
        if (r != 831) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)247) + (uint16_t)31529;
        if (r != 31776) failures++;
    }


    {
        volatile int16_t a = 26300;
        volatile int16_t b = 2180;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint16_t x = 10;
        x = x + 53;
        if (x != 63) failures++;
    }


    {
        uint16_t r = 5357 + 48498 + 61989 + 2930 + 19949 + 10997 + 35757 + 47704;
        if (r != 36573) failures++;
    }


    {
        uint8_t buf[8] = {131,228,130,213,49,213,1,239};
        uint8_t *p = buf;
        p += 5;
        if (*p != 213) failures++;
    }


    {
        uint8_t buf[8] = {90,61,178,230,82,22,86,10};
        uint8_t *p = buf;
        p += 6;
        if (*p != 86) failures++;
    }


    {
        uint16_t x = 58717;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 29;
        v ^= 64;
        if (v != 93) failures++;
    }


    {
        uint8_t v = 11;
        v &= ~(uint8_t)32;
        if (v != 11) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 193;
        if (buf[14] != 193) failures++;
    }


    {
        uint8_t v = 161;
        v |= 1;
        if (v != 161) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {111,107,46739,86};
        if (s.a != (uint8_t)111) failures++;
    }


    {
        int8_t a = 12;
        int8_t b = 69;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(50,64) != 65522) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 56;
        if (buf[10] != 56) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 70;
        if (buf[1] != 70) failures++;
    }


    {
        uint8_t v = 210;
        v &= ~(uint8_t)8;
        if (v != 210) failures++;
    }


    {
        uint16_t r = call6(50,154,98,235,191,172);
        if (r != 900) failures++;
    }


    {
        uint8_t x = 184;
        x <<= 1;
        if (x != 112) failures++;
    }


    {
        uint16_t r = call6(8,53,94,35,53,55);
        if (r != 298) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint16_t x = 19085;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint8_t buf[8] = {215,61,1,31,128,199,238,236};
        uint8_t *p = buf;
        p += 2;
        if (*p != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(62,100) != 162) failures++;
    }


    {
        volatile uint8_t port = 12;
        uint8_t r = port;
        if (r != 12) failures++;
    }


    {
        uint8_t a[6] = {182,236,21,96,56,11};
        if (a[4] != 56) failures++;
    }


    {
        uint32_t a = 2476638616UL;
        uint32_t b = 2750292911UL;
        uint32_t r = a & b;
        if (r != 2207121800UL) failures++;
    }


    {
        if (((uint16_t)124) != 124) failures++;
    }


    {
        uint8_t a[6] = {37,34,7,36,161,37};
        if (a[2] != 7) failures++;
    }


    {
        uint8_t v = 150;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 34;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        volatile uint8_t port = 121;
        uint8_t r = port;
        if (r != 121) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 50925;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {67,24,203,175,84,4,92,42};
        uint8_t *p = buf;
        p += 4;
        if (*p != 84) failures++;
    }


    {
        volatile int16_t a = 16223;
        volatile int16_t b = 958;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -24059;
        volatile int16_t b = -2479;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 71;
        uint8_t r = port;
        if (r != 71) failures++;
    }


    {
        int8_t a = 111;
        int8_t b = 63;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(234,237) != 471) failures++;
    }


    {
        g16 = 27014;
        if (read_g16() != 27014) failures++;
    }


    {
        uint16_t r = add2(91,139) + add2(139,151) + add2(91,151);
        if (r != 762) failures++;
    }


    {
        uint8_t buf[8] = {178,14,148,82,252,22,124,181};
        uint8_t *p = buf;
        p += 3;
        if (*p != 82) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 214;
        if (buf[3] != 214) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 4066571399UL;
        uint32_t b = 1352630308UL;
        uint32_t r = a ^ b;
        if (r != 2734519459UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(39,151) != 190) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 137;
        if (buf[1] != 137) failures++;
    }


    {
        g16 = 46567;
        if (read_g16() != 46567) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint16_t r = call6(201,73,61,191,95,227);
        if (r != 848) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 1) sum += j;
        if (sum != 136) failures++;
    }


    {
        uint8_t v = 152;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t x = 8;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint32_t a = 3389921731UL;
        uint32_t b = 3077389962UL;
        uint32_t r = a - b;
        if (r != 312531769UL) failures++;
    }


    {
        uint8_t a[6] = {113,226,57,96,230,101};
        if (a[4] != 230) failures++;
    }


    {
        uint16_t r = add2(87,33) + add2(33,166) + add2(87,166);
        if (r != 572) failures++;
    }


    {
        if (((uint16_t)(148 + (137 | (219 & 91)))) != 367) failures++;
    }


    {
        uint8_t v = 128;
        v ^= 16;
        if (v != 144) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 16: result = 154; break;
        case 1: result = 152; break;
        case 2: result = 2; break;
        case 9: result = 49; break;
        case 0: result = 210; break;
        case 10: result = 30; break;
        case 18: result = 0; break;
        case 12: result = 108; break;
        default: result = 37; break;
        }
        if (result != 37) failures++;
    }


    {
        uint16_t r = add2(96,68) + add2(68,131) + add2(96,131);
        if (r != 590) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        uint8_t m[3][4] = {{207,69,181,231},{155,37,59,165},{158,173,87,112}};
        if (m[2][2] != 87) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)17) + (uint16_t)15821;
        if (r != 15838) failures++;
    }


    {
        uint8_t v = 113;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[3][4] = {{215,163,50,122},{134,215,222,120},{73,162,110,9}};
        if (m[1][1] != 215) failures++;
    }


    {
        uint8_t v = 10;
        v &= ~(uint8_t)128;
        if (v != 10) failures++;
    }


    {
        uint32_t a = 539286576UL;
        uint32_t b = 3803741280UL;
        uint32_t r = a | b;
        if (r != 3804036208UL) failures++;
    }


    {
        uint8_t src[14] = {149,165,197,32,134,160,106,247,150,175,73,236,32,255};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[6] != 106) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 201;
        if (buf[6] != 201) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(227,168) != 59) failures++;
    }


    {
        uint8_t v = 28;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)14) + (uint16_t)39156;
        if (r != 39170) failures++;
    }


    {
        g16 = 64520;
        if (read_g16() != 64520) failures++;
    }


    {
        uint8_t buf[8] = {36,5,47,202,159,19,235,136};
        uint8_t *p = buf;
        p += 3;
        if (*p != 202) failures++;
    }


    {
        volatile int16_t a = -30485;
        volatile int16_t b = 17048;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -13466;
        volatile int16_t b = -5719;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 186;
        v ^= 2;
        if (v != 184) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {229,61,28542,219};
        if (s.b != (uint8_t)61) failures++;
    }


    {
        uint8_t a[6] = {128,114,204,192,118,141};
        if (a[1] != 114) failures++;
    }


    {
        uint16_t r = 1143 + 65109 + 10644 + 41300 + 46826 + 7745 + 34412 + 36964;
        if (r != 47535) failures++;
    }


    {
        uint32_t a = 4234238203UL;
        uint32_t b = 2541746336UL;
        uint32_t r = a ^ b;
        if (r != 1797169243UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {131,114,21563,218};
        if (s.b != (uint8_t)114) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 7: result = 99; break;
        case 10: result = 203; break;
        case 13: result = 33; break;
        case 3: result = 77; break;
        case 1: result = 72; break;
        default: result = 211; break;
        }
        if (result != 211) failures++;
    }


    {
        uint8_t x = 190;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint32_t a = 4000045093UL;
        uint32_t b = 1180359582UL;
        uint32_t r = a - b;
        if (r != 2819685511UL) failures++;
    }


    {
        uint16_t r = call6(233,165,202,49,142,161);
        if (r != 952) failures++;
    }


    {
        uint32_t a = 3496738111UL;
        uint32_t b = 2446159168UL;
        uint32_t r = a ^ b;
        if (r != 1101435007UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {232,107,57395,0};
        if (s.c != (uint16_t)57395) failures++;
    }


    {
        g16 = 24093;
        if (read_g16() != 24093) failures++;
    }


    {
        uint16_t r = call6(7,51,41,173,22,215);
        if (r != 509) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(114,57) != 171) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-58) % (int16_t)((int8_t)-110);
        if ((uint16_t)r != (uint16_t)65478) failures++;
    }


    {
        uint16_t x = 28666;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)44) / (int16_t)((int8_t)60);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t m[2][3] = {{20,77,177},{155,252,198}};
        if (m[1][2] != 198) failures++;
    }


    {
        uint8_t buf[8] = {25,218,253,136,211,35,14,56};
        uint8_t *p = buf;
        p += 1;
        if (*p != 218) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)83) % (int16_t)((int8_t)67);
        if ((uint16_t)r != (uint16_t)16) failures++;
    }


    {
        uint16_t x = 172;
        x = x + 245;
        if (x != 417) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {144,244,3703,155};
        if (s.d != (uint8_t)155) failures++;
    }


    {
        uint8_t v = 74;
        v ^= 8;
        if (v != 66) failures++;
    }


    {
        uint8_t v = 94;
        v ^= 2;
        if (v != 92) failures++;
    }


    {
        g16 = 44174;
        if (read_g16() != 44174) failures++;
    }


    {
        int8_t a = -35;
        int8_t b = -113;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 246; break;
        case 17: result = 239; break;
        case 11: result = 118; break;
        case 9: result = 234; break;
        case 7: result = 206; break;
        default: result = 149; break;
        }
        if (result != 149) failures++;
    }


    {
        uint8_t x = 164;
        x <<= 3;
        if (x != 32) failures++;
    }


    {
        uint8_t v = 179;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 17965;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-33) / (int16_t)((int8_t)89);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = add2(19,131) + add2(131,15) + add2(19,15);
        if (r != 330) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 552698319UL;
        uint32_t b = 3053054114UL;
        uint32_t r = a + b;
        if (r != 3605752433UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(140,85) != 55) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {222,180,4587,99};
        if (s.c != (uint16_t)4587) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-84) / (int16_t)((int8_t)-112);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(226,46) != 180) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-105) % (int16_t)((int8_t)-6);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        uint8_t v = 54;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t x = 179;
        x <<= 0;
        if (x != 179) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {58,213,40432,119};
        if (s.b != (uint8_t)213) failures++;
    }


    {
        uint8_t src[4] = {175,197,158,10};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[1] != 197) failures++;
    }


    {
        uint8_t m[4][3] = {{28,160,132},{141,181,78},{78,164,232},{95,144,138}};
        if (m[0][2] != 132) failures++;
    }


    {
        uint8_t a[6] = {146,129,78,158,203,119};
        if (a[3] != 158) failures++;
    }


    {
        uint16_t x = 24826;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-39) / (int16_t)((int8_t)109);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 119;
        if (buf[1] != 119) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 1) sum += j;
        if (sum != 21) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 43;
        if (buf[15] != 43) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)234) + (uint16_t)64281;
        if (r != 64515) failures++;
    }


    {
        uint8_t buf[8] = {147,6,194,94,172,229,59,47};
        uint8_t *p = buf;
        p += 6;
        if (*p != 59) failures++;
    }


    {
        uint8_t v = 242;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(102,249) != 65389) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(32,243) != 65325) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)79) + (uint16_t)50265;
        if (r != 50344) failures++;
    }


    {
        uint8_t src[7] = {229,251,175,34,36,209,47};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[6] != 47) failures++;
    }


    {
        uint16_t x = 16;
        x = x + 240;
        if (x != 256) failures++;
    }


    {
        uint8_t v = 74;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 22) failures++;
    }


    {
        g16 = 14286;
        if (read_g16() != 14286) failures++;
    }


    {
        uint16_t r = 58918 + 10586 + 61102 + 25295 + 39460 + 54306 + 50227 + 23004;
        if (r != 60754) failures++;
    }


    {
        if (((uint16_t)(237 - (19 & (0 - 185)))) != 234) failures++;
    }


    {
        uint8_t buf[8] = {20,211,3,87,242,195,9,89};
        uint8_t *p = buf;
        p += 1;
        if (*p != 211) failures++;
    }


    {
        volatile int16_t a = 27932;
        volatile int16_t b = 20598;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 184;
        if (buf[7] != 184) failures++;
    }


    {
        uint8_t a[6] = {187,152,194,47,255,128};
        if (a[3] != 47) failures++;
    }


    {
        uint16_t r = 44302 + 24806 + 48562 + 17444 + 22718 + 48402 + 32923 + 36464;
        if (r != 13477) failures++;
    }


    {
        uint8_t a[6] = {79,60,163,233,96,75};
        if (a[0] != 79) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)10) + (uint16_t)56383;
        if (r != 56393) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {127,219,26057,252};
        if (s.c != (uint16_t)26057) failures++;
    }


    {
        uint8_t x = 140;
        x <<= 0;
        if (x != 140) failures++;
    }


    {
        uint8_t m[2][3] = {{16,26,239},{210,38,117}};
        if (m[1][1] != 38) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {65,119,46858,38};
        if (s.b != (uint8_t)119) failures++;
    }


    {
        volatile uint8_t port = 227;
        uint8_t r = port;
        if (r != 227) failures++;
    }


    {
        uint8_t m[2][3] = {{141,200,43},{104,76,94}};
        if (m[1][0] != 104) failures++;
    }


    {
        g16 = 1346;
        if (read_g16() != 1346) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {23,145,54116,31};
        if (s.d != (uint8_t)31) failures++;
    }


    {
        uint8_t x = 170;
        x <<= 4;
        if (x != 160) failures++;
    }


    {
        uint8_t m[4][4] = {{161,60,131,94},{233,252,159,8},{236,76,30,117},{130,216,92,36}};
        if (m[2][3] != 117) failures++;
    }


    {
        uint16_t x = 62245;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-85) / (int16_t)((int8_t)82);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t a[6] = {251,194,45,1,146,203};
        if (a[2] != 45) failures++;
    }


    {
        uint8_t src[9] = {14,49,217,24,40,109,120,136,64};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 217) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {88,29,16076,97};
        if (s.d != (uint8_t)97) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        int8_t a = 33;
        int8_t b = 45;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 649;
        if (read_g16() != 649) failures++;
    }


    {
        volatile int16_t a = 15093;
        volatile int16_t b = 388;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {175,163,14,166,221,30,212,45};
        uint8_t *p = buf;
        p += 4;
        if (*p != 221) failures++;
    }


    {
        volatile int16_t a = -17243;
        volatile int16_t b = -1862;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(((3 ^ 198) - 8) ^ 118)) != 203) failures++;
    }


    {
        uint8_t v = 3;
        v &= ~(uint8_t)128;
        if (v != 3) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {81,126,40768,44};
        if (s.a != (uint8_t)81) failures++;
    }


    {
        uint8_t buf[8] = {65,5,147,66,204,232,24,46};
        uint8_t *p = buf;
        p += 5;
        if (*p != 232) failures++;
    }


    {
        uint16_t x = 49999;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {35,247,207,58,116,113,204,243};
        uint8_t *p = buf;
        p += 5;
        if (*p != 113) failures++;
    }


    {
        uint8_t v = 115;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-91) % (int16_t)((int8_t)-33);
        if ((uint16_t)r != (uint16_t)65511) failures++;
    }


    {
        uint8_t x = 78;
        x <<= 5;
        if (x != 192) failures++;
    }


    {
        uint8_t buf[8] = {224,247,202,177,176,57,164,99};
        uint8_t *p = buf;
        p += 5;
        if (*p != 57) failures++;
    }


    {
        uint16_t r = add2(150,96) + add2(96,223) + add2(150,223);
        if (r != 938) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {251,42,38513,252};
        if (s.d != (uint8_t)252) failures++;
    }


    {
        int8_t a = 73;
        int8_t b = -53;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[16] = {73,152,57,154,213,13,139,138,24,254,241,131,178,210,64,215};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[15] != 215) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {127,27,16045,180};
        if (s.b != (uint8_t)27) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t src[8] = {118,170,198,49,111,138,240,10};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[4] != 111) failures++;
    }


    {
        g16 = 14017;
        if (read_g16() != 14017) failures++;
    }


    {
        uint8_t buf[8] = {43,172,108,102,167,213,140,211};
        uint8_t *p = buf;
        p += 3;
        if (*p != 102) failures++;
    }


    {
        volatile uint8_t port = 62;
        uint8_t r = port;
        if (r != 62) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        g16 = 19420;
        if (read_g16() != 19420) failures++;
    }


    {
        uint16_t r = call6(150,24,249,23,191,242);
        if (r != 879) failures++;
    }


    {
        uint8_t buf[8] = {206,240,123,17,62,153,202,216};
        uint8_t *p = buf;
        p += 5;
        if (*p != 153) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)90) + (uint16_t)55869;
        if (r != 55959) failures++;
    }


    {
        uint16_t r = call6(109,115,49,34,185,18);
        if (r != 510) failures++;
    }


    {
        volatile int16_t a = 23030;
        volatile int16_t b = -4973;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {133,86,185,207,91,144,193,62};
        uint8_t *p = buf;
        p += 2;
        if (*p != 185) failures++;
    }


    {
        if (((uint16_t)(222 ^ ((98 - 44) ^ 85))) != 189) failures++;
    }


    {
        uint8_t src[13] = {151,45,220,96,139,244,228,219,0,41,188,200,146};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[5] != 244) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)1) % (int16_t)((int8_t)45);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        volatile int16_t a = -23038;
        volatile int16_t b = 3634;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)179) + (uint16_t)44640;
        if (r != 44819) failures++;
    }


    {
        uint8_t v = 255;
        v &= ~(uint8_t)128;
        if (v != 127) failures++;
    }


    {
        if (((uint16_t)(199 | 150)) != 215) failures++;
    }


    {
        uint16_t r = 18990 + 7512 + 37939 + 39791 + 33571 + 36422 + 30532 + 17001;
        if (r != 25150) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(107,134) != 65509) failures++;
    }


    {
        uint16_t r = call6(38,193,181,210,142,0);
        if (r != 764) failures++;
    }


    {
        uint32_t a = 2474434056UL;
        uint32_t b = 3659380866UL;
        uint32_t r = a ^ b;
        if (r != 1231120010UL) failures++;
    }


    {
        uint16_t r = call6(112,246,130,41,126,141);
        if (r != 796) failures++;
    }


    {
        uint16_t r = add2(96,146) + add2(146,18) + add2(96,18);
        if (r != 520) failures++;
    }


    {
        uint8_t src[7] = {55,117,74,52,145,168,42};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[5] != 168) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 175;
        if (buf[0] != 175) failures++;
    }


    {
        volatile int16_t a = 8341;
        volatile int16_t b = 9458;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 35;
        uint8_t r = port;
        if (r != 35) failures++;
    }


    {
        uint8_t v = 90;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(96,206) + add2(206,94) + add2(96,94);
        if (r != 792) failures++;
    }


    {
        volatile int16_t a = -4891;
        volatile int16_t b = 130;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint8_t a[6] = {123,207,160,140,15,75};
        if (a[0] != 123) failures++;
    }


    {
        volatile uint8_t port = 127;
        uint8_t r = port;
        if (r != 127) failures++;
    }


    {
        uint16_t x = 62501;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 3546;
        volatile int16_t b = -28538;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 58;
        uint8_t r = port;
        if (r != 58) failures++;
    }


    {
        uint16_t x = 30961;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[4][2] = {{198,168},{79,84},{0,17},{44,91}};
        if (m[2][1] != 17) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 1: result = 54; break;
        case 16: result = 31; break;
        case 10: result = 97; break;
        case 0: result = 211; break;
        case 6: result = 20; break;
        default: result = 177; break;
        }
        if (result != 54) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)178) + (uint16_t)63637;
        if (r != 63815) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {137,227,58715,102};
        if (s.b != (uint8_t)227) failures++;
    }


    {
        g16 = 9635;
        if (read_g16() != 9635) failures++;
    }


    {
        uint8_t x = 97;
        x <<= 0;
        if (x != 97) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 39805;
        if (read_g16() != 39805) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {97,7,49161,237};
        if (s.a != (uint8_t)97) failures++;
    }


    {
        uint8_t m[2][3] = {{26,226,66},{27,135,135}};
        if (m[1][0] != 27) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 164;
        uint8_t r = port;
        if (r != 164) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)91) + (uint16_t)35547;
        if (r != 35638) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t r = 42984 + 24470 + 16917 + 55063 + 58852 + 50348 + 40172 + 47329;
        if (r != 8455) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 9: result = 22; break;
        case 15: result = 119; break;
        case 7: result = 30; break;
        case 4: result = 62; break;
        case 1: result = 227; break;
        case 19: result = 179; break;
        default: result = 129; break;
        }
        if (result != 129) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 7: result = 46; break;
        case 5: result = 229; break;
        case 12: result = 152; break;
        case 15: result = 164; break;
        case 10: result = 122; break;
        case 18: result = 112; break;
        case 16: result = 22; break;
        case 3: result = 8; break;
        default: result = 57; break;
        }
        if (result != 112) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 0: result = 3; break;
        case 5: result = 4; break;
        case 3: result = 8; break;
        case 9: result = 47; break;
        case 19: result = 108; break;
        case 13: result = 2; break;
        default: result = 184; break;
        }
        if (result != 3) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint32_t a = 388613986UL;
        uint32_t b = 1669050229UL;
        uint32_t r = a & b;
        if (r != 53053280UL) failures++;
    }


    {
        uint32_t a = 2682977813UL;
        uint32_t b = 3775262731UL;
        uint32_t r = a & b;
        if (r != 2164322305UL) failures++;
    }


    {
        uint16_t x = 32420;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {134,51,56338,183};
        if (s.b != (uint8_t)51) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)198) + (uint16_t)55508;
        if (r != 55706) failures++;
    }


    {
        if (((uint16_t)185) != 185) failures++;
    }


    {
        volatile uint8_t port = 25;
        uint8_t r = port;
        if (r != 25) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)17) + (uint16_t)34516;
        if (r != 34533) failures++;
    }


    {
        uint8_t v = 156;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 33137;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)126) % (int16_t)((int8_t)-116);
        if ((uint16_t)r != (uint16_t)10) failures++;
    }


    {
        int8_t a = -99;
        int8_t b = 121;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[11] = {185,182,139,16,73,242,158,198,26,74,219};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[9] != 74) failures++;
    }


    {
        uint8_t src[13] = {32,163,221,74,68,122,254,52,154,99,33,140,100};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[1] != 163) failures++;
    }


    {
        uint16_t r = call6(132,196,191,192,47,150);
        if (r != 908) failures++;
    }


    {
        uint16_t r = 18489 + 18679 + 15736 + 46263 + 3123 + 56563 + 11049 + 34266;
        if (r != 7560) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 80;
        if (buf[3] != 80) failures++;
    }


    {
        uint16_t r = 16843 + 29773 + 32441 + 31810 + 60945 + 8972 + 647 + 42818;
        if (r != 27641) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 1) sum += j;
        if (sum != 171) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {25,35,14407,197};
        if (s.c != (uint16_t)14407) failures++;
    }


    {
        uint16_t r = add2(70,233) + add2(233,105) + add2(70,105);
        if (r != 816) failures++;
    }


    {
        uint16_t r = 46109 + 23626 + 25381 + 61530 + 59227 + 59391 + 48316 + 34290;
        if (r != 30190) failures++;
    }


    {
        uint16_t r = 15847 + 53631 + 51800 + 33954 + 45199 + 43949 + 25167 + 31396;
        if (r != 38799) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 3: result = 196; break;
        case 6: result = 5; break;
        case 16: result = 164; break;
        case 19: result = 62; break;
        default: result = 187; break;
        }
        if (result != 187) failures++;
    }


    {
        g16 = 25333;
        if (read_g16() != 25333) failures++;
    }


    {
        uint16_t x = 1989;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 23885;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 100;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(36,72) + add2(72,115) + add2(36,115);
        if (r != 446) failures++;
    }


    {
        uint32_t a = 37385676UL;
        uint32_t b = 3485143937UL;
        uint32_t r = a & b;
        if (r != 37360000UL) failures++;
    }


    {
        uint8_t v = 250;
        v ^= 128;
        if (v != 122) failures++;
    }


    {
        int8_t a = -77;
        int8_t b = -37;
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
        uint16_t r = call6(58,96,121,163,138,142);
        if (r != 718) failures++;
    }


    {
        volatile uint8_t port = 85;
        uint8_t r = port;
        if (r != 85) failures++;
    }


    {
        if (((uint16_t)(110 & (108 & 228))) != 100) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)15) / (int16_t)((int8_t)57);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 76;
        uint8_t r = port;
        if (r != 76) failures++;
    }


    {
        uint8_t x = 163;
        x <<= 1;
        if (x != 70) failures++;
    }


    {
        uint8_t src[4] = {3,207,204,73};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 3) failures++;
    }


    {
        uint8_t src[7] = {14,189,111,19,246,83,159};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[4] != 246) failures++;
    }


    {
        uint8_t v = 130;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2615917731UL;
        uint32_t b = 3451398610UL;
        uint32_t r = a + b;
        if (r != 1772349045UL) failures++;
    }


    {
        uint16_t x = 19;
        x = x + 120;
        if (x != 139) failures++;
    }


    {
        uint16_t r = 35303 + 65320 + 16586 + 10482 + 4233 + 39645 + 37257 + 2600;
        if (r != 14818) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(41,215) != 256) failures++;
    }


    {
        uint16_t x = 62715;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 64760;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {141,114,158,84,63,191,172,210};
        uint8_t *p = buf;
        p += 3;
        if (*p != 84) failures++;
    }


    {
        volatile uint8_t port = 102;
        uint8_t r = port;
        if (r != 102) failures++;
    }


    {
        volatile uint8_t port = 26;
        uint8_t r = port;
        if (r != 26) failures++;
    }


    {
        uint32_t a = 260662029UL;
        uint32_t b = 2335740715UL;
        uint32_t r = a & b;
        if (r != 185074441UL) failures++;
    }


    {
        uint16_t r = 27725 + 43015 + 13112 + 52493 + 3887 + 58828 + 13220 + 32228;
        if (r != 47900) failures++;
    }


    {
        uint32_t a = 566374263UL;
        uint32_t b = 2249580273UL;
        uint32_t r = a - b;
        if (r != 2611761286UL) failures++;
    }


    {
        uint16_t r = 65292 + 45563 + 65386 + 32837 + 40102 + 22275 + 42860 + 62747;
        if (r != 49382) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)30) % (int16_t)((int8_t)102);
        if ((uint16_t)r != (uint16_t)30) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 2490260813UL;
        uint32_t b = 2066485545UL;
        uint32_t r = a + b;
        if (r != 261779062UL) failures++;
    }


    {
        int8_t a = -2;
        int8_t b = -53;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 224;
        x <<= 4;
        if (x != 0) failures++;
    }


    {
        volatile uint8_t port = 137;
        uint8_t r = port;
        if (r != 137) failures++;
    }


    {
        g16 = 23263;
        if (read_g16() != 23263) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)126) + (uint16_t)5115;
        if (r != 5241) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 89;
        if (buf[5] != 89) failures++;
    }


    {
        uint16_t r = call6(64,180,213,245,177,166);
        if (r != 1045) failures++;
    }


    {
        uint32_t a = 777906401UL;
        uint32_t b = 2333492512UL;
        uint32_t r = a | b;
        if (r != 2942299617UL) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 210;
        if (buf[6] != 210) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 16: result = 147; break;
        case 5: result = 124; break;
        case 0: result = 4; break;
        default: result = 107; break;
        }
        if (result != 107) failures++;
    }


    {
        uint32_t a = 429803514UL;
        uint32_t b = 1785653485UL;
        uint32_t r = a & b;
        if (r != 135151848UL) failures++;
    }


    {
        volatile uint8_t port = 86;
        uint8_t r = port;
        if (r != 86) failures++;
    }


    {
        uint8_t v = 164;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[3][3] = {{84,239,65},{39,255,128},{233,215,51}};
        if (m[2][1] != 215) failures++;
    }


    {
        uint8_t v = 119;
        v &= ~(uint8_t)64;
        if (v != 55) failures++;
    }


    {
        uint16_t r = 39974 + 53827 + 59773 + 21893 + 55207 + 42694 + 51868 + 194;
        if (r != 63286) failures++;
    }


    {
        if (((uint16_t)205) != 205) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)113) % (int16_t)((int8_t)-119);
        if ((uint16_t)r != (uint16_t)113) failures++;
    }


    {
        uint16_t x = 44785;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint8_t x = 10;
        x <<= 3;
        if (x != 80) failures++;
    }


    {
        uint8_t a[6] = {48,171,220,45,42,159};
        if (a[2] != 220) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 3: result = 224; break;
        case 6: result = 35; break;
        case 2: result = 107; break;
        case 1: result = 223; break;
        case 10: result = 229; break;
        case 15: result = 63; break;
        default: result = 73; break;
        }
        if (result != 73) failures++;
    }


    {
        uint8_t src[2] = {75,144};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 75) failures++;
    }


    {
        volatile uint8_t port = 53;
        uint8_t r = port;
        if (r != 53) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 127;
        if (buf[5] != 127) failures++;
    }


    {
        uint16_t r = call6(232,202,114,122,1,139);
        if (r != 810) failures++;
    }


    {
        g16 = 14372;
        if (read_g16() != 14372) failures++;
    }


    {
        uint8_t v = 53;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = 13886;
        volatile int16_t b = 30020;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint16_t r = call6(206,180,189,153,163,122);
        if (r != 1013) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)118) % (int16_t)((int8_t)63);
        if ((uint16_t)r != (uint16_t)55) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-20) % (int16_t)((int8_t)-72);
        if ((uint16_t)r != (uint16_t)65516) failures++;
    }


    {
        uint16_t x = 63671;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {5,170,224,74,97,202,140,234};
        uint8_t *p = buf;
        p += 1;
        if (*p != 170) failures++;
    }


    {
        uint32_t a = 3809038612UL;
        uint32_t b = 562916340UL;
        uint32_t r = a | b;
        if (r != 3817700340UL) failures++;
    }


    {
        uint8_t v = 47;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)213) + (uint16_t)28215;
        if (r != 28428) failures++;
    }


    {
        uint8_t v = 25;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 39) failures++;
    }


    {
        uint16_t r = add2(245,216) + add2(216,173) + add2(245,173);
        if (r != 1268) failures++;
    }


    {
        if (((uint16_t)((150 - (26 ^ 47)) & 40)) != 32) failures++;
    }


    {
        volatile int16_t a = 13603;
        volatile int16_t b = 21045;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {16,38,46295,108};
        if (s.d != (uint8_t)108) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint8_t buf[8] = {144,229,112,118,112,150,200,45};
        uint8_t *p = buf;
        p += 5;
        if (*p != 150) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-120) % (int16_t)((int8_t)-65);
        if ((uint16_t)r != (uint16_t)65481) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-10) % (int16_t)((int8_t)-88);
        if ((uint16_t)r != (uint16_t)65526) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 120;
        if (buf[4] != 120) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 14: result = 38; break;
        case 9: result = 189; break;
        case 18: result = 72; break;
        case 7: result = 190; break;
        case 10: result = 73; break;
        default: result = 182; break;
        }
        if (result != 182) failures++;
    }


    {
        uint8_t m[3][4] = {{224,152,73,242},{120,44,161,76},{71,209,148,92}};
        if (m[0][3] != 242) failures++;
    }


    {
        uint16_t x = 230;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 28;
        if (buf[13] != 28) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint8_t a[6] = {218,88,41,162,230,207};
        if (a[1] != 88) failures++;
    }


    {
        uint16_t x = 6315;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 148;
        if (buf[14] != 148) failures++;
    }


    {
        uint8_t v = 115;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t src[2] = {185,10};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 185) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 5: result = 175; break;
        case 19: result = 2; break;
        case 0: result = 27; break;
        case 11: result = 69; break;
        default: result = 158; break;
        }
        if (result != 175) failures++;
    }


    {
        uint8_t src[12] = {41,60,25,193,111,123,248,119,205,114,214,172};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[10] != 214) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(109,241) != 350) failures++;
    }


    {
        uint16_t x = 237;
        x = x + 234;
        if (x != 471) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,187,27117,197};
        if (s.a != (uint8_t)205) failures++;
    }


    {
        int8_t a = -123;
        int8_t b = -100;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 96; break;
        case 18: result = 30; break;
        case 10: result = 190; break;
        default: result = 59; break;
        }
        if (result != 59) failures++;
    }

    return failures;
}