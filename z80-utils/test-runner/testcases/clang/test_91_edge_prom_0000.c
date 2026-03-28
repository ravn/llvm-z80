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
        uint16_t x = 137;
        x = x + 44;
        if (x != 181) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(208,136) != 344) failures++;
    }


    {
        volatile uint8_t port = 19;
        uint8_t r = port;
        if (r != 19) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {174,26,20927,69};
        if (s.c != (uint16_t)20927) failures++;
    }


    {
        uint8_t v = 170;
        v |= 8;
        if (v != 170) failures++;
    }


    {
        uint16_t x = 57183;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(44,193) != 237) failures++;
    }


    {
        uint16_t x = 41347;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(229,52) + add2(52,22) + add2(229,22);
        if (r != 606) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 64;
        if (buf[4] != 64) failures++;
    }


    {
        volatile int16_t a = -29985;
        volatile int16_t b = 5479;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 8: result = 247; break;
        case 15: result = 105; break;
        case 1: result = 161; break;
        case 9: result = 6; break;
        case 10: result = 203; break;
        case 19: result = 222; break;
        default: result = 248; break;
        }
        if (result != 203) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 191;
        if (buf[13] != 191) failures++;
    }


    {
        uint8_t buf[8] = {16,92,42,249,134,87,171,172};
        uint8_t *p = buf;
        p += 6;
        if (*p != 171) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(233,198) + add2(198,172) + add2(233,172);
        if (r != 1206) failures++;
    }


    {
        uint16_t x = 24780;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 11: result = 117; break;
        case 0: result = 207; break;
        case 19: result = 30; break;
        default: result = 147; break;
        }
        if (result != 147) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(119,247) != 366) failures++;
    }


    {
        uint16_t r = 58261 + 49622 + 60462 + 5784 + 1330 + 17659 + 59813 + 25320;
        if (r != 16107) failures++;
    }


    {
        uint16_t x = 33601;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 13375;
        volatile int16_t b = 2874;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[14] = {0,55,32,181,45,42,221,4,193,37,206,14,165,134};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[7] != 4) failures++;
    }


    {
        uint16_t x = 171;
        x = x + 212;
        if (x != 383) failures++;
    }


    {
        g16 = 3118;
        if (read_g16() != 3118) failures++;
    }


    {
        uint8_t src[6] = {214,32,73,190,120,40};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 40) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 156;
        if (x != 354) failures++;
    }


    {
        uint16_t r = 38167 + 51020 + 48298 + 16576 + 44266 + 15671 + 21863 + 50626;
        if (r != 24343) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t r = add2(69,232) + add2(232,10) + add2(69,10);
        if (r != 622) failures++;
    }


    {
        g16 = 58946;
        if (read_g16() != 58946) failures++;
    }


    {
        uint16_t r = add2(230,155) + add2(155,219) + add2(230,219);
        if (r != 1208) failures++;
    }


    {
        volatile int16_t a = -29065;
        volatile int16_t b = -11983;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {70,175,13733,188};
        if (s.b != (uint8_t)175) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 2: result = 164; break;
        case 8: result = 135; break;
        case 16: result = 203; break;
        case 7: result = 22; break;
        case 6: result = 41; break;
        default: result = 100; break;
        }
        if (result != 41) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)8) + (uint16_t)25449;
        if (r != 25457) failures++;
    }


    {
        int8_t a = -10;
        int8_t b = -85;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 1361625770UL;
        uint32_t b = 2309326613UL;
        uint32_t r = a ^ b;
        if (r != 3633137087UL) failures++;
    }


    {
        uint16_t r = 63433 + 64525 + 52213 + 3699 + 33666 + 63716 + 44632 + 51880;
        if (r != 50084) failures++;
    }


    {
        uint8_t x = 64;
        x <<= 0;
        if (x != 64) failures++;
    }


    {
        volatile int16_t a = 32043;
        volatile int16_t b = 12241;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 1: result = 185; break;
        case 11: result = 215; break;
        case 14: result = 161; break;
        default: result = 183; break;
        }
        if (result != 215) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(229,175) != 404) failures++;
    }


    {
        volatile uint8_t port = 92;
        uint8_t r = port;
        if (r != 92) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        uint8_t v = 192;
        int r = (v & 32) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)135) != 135) failures++;
    }


    {
        uint8_t buf[8] = {164,28,194,26,198,152,55,238};
        uint8_t *p = buf;
        p += 5;
        if (*p != 152) failures++;
    }


    {
        uint8_t x = 228;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        volatile uint8_t port = 224;
        uint8_t r = port;
        if (r != 224) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 11: result = 93; break;
        case 7: result = 63; break;
        case 14: result = 10; break;
        case 16: result = 219; break;
        case 8: result = 254; break;
        case 5: result = 67; break;
        case 0: result = 244; break;
        default: result = 21; break;
        }
        if (result != 21) failures++;
    }


    {
        uint8_t v = 72;
        v &= ~(uint8_t)32;
        if (v != 72) failures++;
    }


    {
        uint8_t x = 16;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t buf[8] = {147,7,164,197,116,32,129,145};
        uint8_t *p = buf;
        p += 2;
        if (*p != 164) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint32_t a = 55707095UL;
        uint32_t b = 2893819497UL;
        uint32_t r = a - b;
        if (r != 1456854894UL) failures++;
    }


    {
        uint16_t x = 137;
        x = x + 84;
        if (x != 221) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(99,155) != 65480) failures++;
    }


    {
        uint8_t v = 114;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {47,52,226,143,99,34,200,208};
        uint8_t *p = buf;
        p += 7;
        if (*p != 208) failures++;
    }


    {
        uint8_t a[6] = {30,221,223,182,78,252};
        if (a[1] != 221) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)108) + (uint16_t)31629;
        if (r != 31737) failures++;
    }


    {
        uint16_t x = 125;
        x = x + 106;
        if (x != 231) failures++;
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
        for (uint16_t j = 0; j < 1; j += 1) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {15,21,32313,223};
        if (s.c != (uint16_t)32313) failures++;
    }


    {
        uint16_t r = call6(155,57,144,166,108,13);
        if (r != 643) failures++;
    }


    {
        uint16_t r = add2(185,78) + add2(78,238) + add2(185,238);
        if (r != 1002) failures++;
    }


    {
        uint8_t x = 212;
        x <<= 3;
        if (x != 160) failures++;
    }


    {
        uint8_t buf[8] = {222,200,96,199,39,254,21,146};
        uint8_t *p = buf;
        p += 2;
        if (*p != 96) failures++;
    }


    {
        uint8_t m[4][4] = {{54,64,62,235},{180,8,227,180},{147,185,109,199},{24,54,48,166}};
        if (m[0][1] != 64) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint16_t r = 62280 + 18425 + 696 + 21307 + 11312 + 43388 + 4232 + 12872;
        if (r != 43440) failures++;
    }


    {
        uint16_t r = 28714 + 59979 + 56387 + 26322 + 53040 + 7815 + 7388 + 19883;
        if (r != 62920) failures++;
    }


    {
        uint8_t a[6] = {236,46,68,21,183,225};
        if (a[1] != 46) failures++;
    }


    {
        volatile uint8_t port = 122;
        uint8_t r = port;
        if (r != 122) failures++;
    }


    {
        uint8_t x = 220;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 45;
        if (buf[4] != 45) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 149;
        if (buf[0] != 149) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,176,30002,31};
        if (s.a != (uint8_t)205) failures++;
    }


    {
        uint8_t v = 33;
        v |= 2;
        if (v != 35) failures++;
    }


    {
        if (((uint16_t)0) != 0) failures++;
    }


    {
        volatile int16_t a = 1738;
        volatile int16_t b = 22396;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        g16 = 48576;
        if (read_g16() != 48576) failures++;
    }


    {
        uint8_t src[5] = {213,17,148,179,160};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 213) failures++;
    }


    {
        volatile int16_t a = 13178;
        volatile int16_t b = -32389;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)94) != 94) failures++;
    }


    {
        uint32_t a = 1398610831UL;
        uint32_t b = 3694131681UL;
        uint32_t r = a ^ b;
        if (r != 2406670958UL) failures++;
    }


    {
        uint16_t r = add2(101,204) + add2(204,36) + add2(101,36);
        if (r != 682) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)105) + (uint16_t)46958;
        if (r != 47063) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t m[3][4] = {{83,198,13,133},{15,82,98,187},{94,51,69,51}};
        if (m[1][0] != 15) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 7442;
        if (read_g16() != 7442) failures++;
    }


    {
        int8_t a = 56;
        int8_t b = -62;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 82;
        int8_t b = -81;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 3711215174UL;
        uint32_t b = 3935745735UL;
        uint32_t r = a & b;
        if (r != 3356796486UL) failures++;
    }


    {
        volatile int16_t a = 7656;
        volatile int16_t b = 24839;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 12264;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 57;
        uint8_t r = port;
        if (r != 57) failures++;
    }


    {
        uint32_t a = 3295742903UL;
        uint32_t b = 3596233310UL;
        uint32_t r = a & b;
        if (r != 3293579798UL) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 67;
        if (buf[5] != 67) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)122) + (uint16_t)560;
        if (r != 682) failures++;
    }


    {
        volatile int16_t a = 21916;
        volatile int16_t b = -15776;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(136,215,86,199,156,33);
        if (r != 825) failures++;
    }


    {
        uint16_t x = 15992;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 18833;
        volatile int16_t b = 2112;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 96;
        x = x + 158;
        if (x != 254) failures++;
    }


    {
        if (((uint16_t)(((11 - 157) ^ (102 & 108)) & 33)) != 0) failures++;
    }


    {
        volatile int16_t a = -12589;
        volatile int16_t b = -10466;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 113;
        x = x + 32;
        if (x != 145) failures++;
    }


    {
        uint8_t v = 97;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 13: result = 159; break;
        case 17: result = 15; break;
        case 11: result = 106; break;
        case 2: result = 18; break;
        case 6: result = 79; break;
        case 1: result = 50; break;
        case 7: result = 25; break;
        case 0: result = 210; break;
        default: result = 241; break;
        }
        if (result != 25) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {93,207,56182,53};
        if (s.a != (uint8_t)93) failures++;
    }


    {
        uint16_t x = 110;
        x = x + 77;
        if (x != 187) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 154;
        if (buf[4] != 154) failures++;
    }


    {
        g16 = 55155;
        if (read_g16() != 55155) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)6) % (int16_t)((int8_t)-56);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint16_t r = call6(46,1,73,51,232,126);
        if (r != 529) failures++;
    }


    {
        uint8_t src[16] = {15,190,151,238,191,145,173,101,86,20,191,26,49,251,190,215};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[5] != 145) failures++;
    }


    {
        uint8_t a[6] = {37,118,196,69,50,249};
        if (a[4] != 50) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(118,254) != 65400) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {213,136,44281,246};
        if (s.b != (uint8_t)136) failures++;
    }


    {
        uint8_t x = 52;
        x <<= 0;
        if (x != 52) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-55) / (int16_t)((int8_t)-34);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t r = 19289 + 21920 + 52351 + 23439 + 32640 + 23939 + 63688 + 686;
        if (r != 41344) failures++;
    }


    {
        uint8_t m[2][3] = {{179,224,115},{34,218,241}};
        if (m[1][1] != 218) failures++;
    }


    {
        uint16_t x = 219;
        x = x + 213;
        if (x != 432) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-83) % (int16_t)((int8_t)123);
        if ((uint16_t)r != (uint16_t)65453) failures++;
    }


    {
        uint8_t v = 237;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t a[6] = {19,180,186,164,122,243};
        if (a[5] != 243) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(174,119) != 293) failures++;
    }


    {
        uint16_t r = call6(64,229,103,240,219,7);
        if (r != 862) failures++;
    }


    {
        g16 = 16388;
        if (read_g16() != 16388) failures++;
    }


    {
        int8_t a = 54;
        int8_t b = 31;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 177;
        if (buf[2] != 177) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)24) + (uint16_t)15479;
        if (r != 15503) failures++;
    }


    {
        uint8_t v = 65;
        v ^= 1;
        if (v != 64) failures++;
    }


    {
        uint8_t buf[8] = {215,204,255,32,96,93,178,117};
        uint8_t *p = buf;
        p += 5;
        if (*p != 93) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)84) % (int16_t)((int8_t)61);
        if ((uint16_t)r != (uint16_t)23) failures++;
    }


    {
        uint8_t a[6] = {202,37,183,192,52,100};
        if (a[2] != 183) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-67) / (int16_t)((int8_t)-34);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)42) + (uint16_t)8571;
        if (r != 8613) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint8_t src[15] = {223,229,20,212,26,48,153,201,192,170,188,178,32,170,94};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[1] != 229) failures++;
    }


    {
        uint16_t x = 58247;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint16_t x = 104;
        x = x + 146;
        if (x != 250) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {146,77,41915,241};
        if (s.c != (uint16_t)41915) failures++;
    }


    {
        uint8_t x = 17;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        volatile uint8_t port = 79;
        uint8_t r = port;
        if (r != 79) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)169) + (uint16_t)31552;
        if (r != 31721) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)115) / (int16_t)((int8_t)15);
        if ((uint16_t)r != (uint16_t)7) failures++;
    }


    {
        if (((uint16_t)(237 & ((228 & 138) & (150 + 201)))) != 0) failures++;
    }


    {
        uint8_t v = 50;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint32_t a = 2278017797UL;
        uint32_t b = 246366805UL;
        uint32_t r = a | b;
        if (r != 2414857045UL) failures++;
    }


    {
        volatile int16_t a = 25447;
        volatile int16_t b = 2976;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 148;
        x <<= 2;
        if (x != 80) failures++;
    }


    {
        uint8_t buf[8] = {121,213,42,197,32,188,238,69};
        uint8_t *p = buf;
        p += 2;
        if (*p != 42) failures++;
    }


    {
        g16 = 16823;
        if (read_g16() != 16823) failures++;
    }


    {
        uint16_t r = 39886 + 54519 + 20248 + 21012 + 59530 + 35157 + 30409 + 25714;
        if (r != 24331) failures++;
    }


    {
        if (((uint16_t)25) != 25) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(158,98) != 60) failures++;
    }


    {
        uint16_t x = 23711;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(170,82,166,20,51,124);
        if (r != 613) failures++;
    }


    {
        uint16_t x = 177;
        x = x + 110;
        if (x != 287) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(236,222,88,48,25,121);
        if (r != 740) failures++;
    }


    {
        uint8_t src[4] = {212,79,121,157};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[1] != 79) failures++;
    }


    {
        uint8_t v = 65;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 31) failures++;
    }


    {
        uint16_t r = add2(204,11) + add2(11,249) + add2(204,249);
        if (r != 928) failures++;
    }


    {
        if (((uint16_t)((73 - 225) | 101)) != 65389) failures++;
    }


    {
        uint16_t r = call6(86,118,255,60,48,154);
        if (r != 721) failures++;
    }


    {
        uint8_t src[6] = {8,255,207,118,118,221};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 221) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = 2392;
        volatile int16_t b = 196;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)77) != 77) failures++;
    }


    {
        uint32_t a = 816627927UL;
        uint32_t b = 3848083083UL;
        uint32_t r = a | b;
        if (r != 4127053535UL) failures++;
    }


    {
        uint8_t buf[8] = {154,249,36,94,238,165,36,223};
        uint8_t *p = buf;
        p += 4;
        if (*p != 238) failures++;
    }


    {
        uint8_t a[6] = {230,12,72,135,171,65};
        if (a[3] != 135) failures++;
    }


    {
        volatile int16_t a = 28543;
        volatile int16_t b = 6868;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)125) % (int16_t)((int8_t)-6);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        g16 = 16308;
        if (read_g16() != 16308) failures++;
    }


    {
        uint8_t x = 183;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        int8_t a = -81;
        int8_t b = -76;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 45449;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 59009;
        if (read_g16() != 59009) failures++;
    }


    {
        uint8_t v = 198;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(123,91,170,105,231,73);
        if (r != 793) failures++;
    }


    {
        volatile uint8_t port = 124;
        uint8_t r = port;
        if (r != 124) failures++;
    }


    {
        uint16_t r = add2(219,190) + add2(190,193) + add2(219,193);
        if (r != 1204) failures++;
    }


    {
        uint16_t r = call6(132,187,202,221,2,118);
        if (r != 862) failures++;
    }


    {
        uint8_t m[2][4] = {{117,79,176,227},{164,1,238,172}};
        if (m[1][0] != 164) failures++;
    }


    {
        uint8_t x = 11;
        x <<= 4;
        if (x != 176) failures++;
    }


    {
        uint16_t r = 35502 + 23899 + 64425 + 23894 + 20865 + 31713 + 16422 + 22685;
        if (r != 42797) failures++;
    }


    {
        uint8_t src[9] = {205,242,28,195,214,132,166,69,38};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[3] != 195) failures++;
    }


    {
        uint16_t r = add2(224,154) + add2(154,47) + add2(224,47);
        if (r != 850) failures++;
    }


    {
        g16 = 22012;
        if (read_g16() != 22012) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(234,246) != 480) failures++;
    }


    {
        uint16_t x = 122;
        x = x + 75;
        if (x != 197) failures++;
    }


    {
        uint8_t v = 219;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {253,20,191,67,98,113};
        if (a[0] != 253) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-95) % (int16_t)((int8_t)-110);
        if ((uint16_t)r != (uint16_t)65441) failures++;
    }


    {
        uint8_t v = 254;
        v |= 16;
        if (v != 254) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {200,101,27988,98};
        if (s.a != (uint8_t)200) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 153;
        if (buf[6] != 153) failures++;
    }


    {
        int8_t a = 112;
        int8_t b = 18;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 51260 + 42041 + 16565 + 32409 + 20005 + 10091 + 48774 + 30490;
        if (r != 55027) failures++;
    }


    {
        g16 = 13940;
        if (read_g16() != 13940) failures++;
    }


    {
        uint32_t a = 3227108583UL;
        uint32_t b = 2424856208UL;
        uint32_t r = a ^ b;
        if (r != 1355913847UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint16_t r = call6(100,155,210,125,226,158);
        if (r != 974) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)169) + (uint16_t)36921;
        if (r != 37090) failures++;
    }


    {
        uint8_t m[4][4] = {{130,248,188,187},{81,234,254,33},{68,73,222,25},{184,201,252,108}};
        if (m[1][2] != 254) failures++;
    }


    {
        uint16_t x = 27461;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[8] = {87,65,134,126,222,70,195,193};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[3] != 126) failures++;
    }


    {
        uint16_t x = 8368;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(116,81) + add2(81,126) + add2(116,126);
        if (r != 646) failures++;
    }


    {
        uint16_t x = 255;
        x = x + 174;
        if (x != 429) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {104,200,52076,33};
        if (s.c != (uint16_t)52076) failures++;
    }


    {
        uint8_t src[13] = {92,216,89,167,115,183,19,94,223,174,234,56,9};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[7] != 94) failures++;
    }


    {
        uint16_t x = 7473;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-80) % (int16_t)((int8_t)-95);
        if ((uint16_t)r != (uint16_t)65456) failures++;
    }


    {
        uint8_t src[7] = {130,123,97,126,226,11,18};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[0] != 130) failures++;
    }


    {
        uint8_t buf[8] = {111,244,168,70,84,254,245,121};
        uint8_t *p = buf;
        p += 1;
        if (*p != 244) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint8_t v = 32;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 73;
        if (buf[2] != 73) failures++;
    }


    {
        uint8_t v = 118;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 114;
        x <<= 3;
        if (x != 144) failures++;
    }


    {
        uint16_t r = call6(43,136,156,48,123,120);
        if (r != 626) failures++;
    }


    {
        uint16_t x = 33700;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 47555;
        if (read_g16() != 47555) failures++;
    }


    {
        uint8_t a[6] = {122,159,7,32,143,40};
        if (a[5] != 40) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)74) % (int16_t)((int8_t)-66);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {238,107,57260,248};
        if (s.a != (uint8_t)238) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -25540;
        volatile int16_t b = -18618;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[3][2] = {{5,104},{177,78},{66,83}};
        if (m[2][1] != 83) failures++;
    }


    {
        int8_t a = -96;
        int8_t b = 100;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 11790;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -89;
        int8_t b = 6;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t r = add2(237,173) + add2(173,62) + add2(237,62);
        if (r != 944) failures++;
    }


    {
        uint16_t r = 60894 + 18071 + 40548 + 42628 + 65276 + 37859 + 22182 + 26011;
        if (r != 51325) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(114,232) != 65418) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[2][3] = {{202,120,199},{158,165,143}};
        if (m[0][2] != 199) failures++;
    }


    {
        uint8_t buf[8] = {55,224,65,71,245,219,71,236};
        uint8_t *p = buf;
        p += 1;
        if (*p != 224) failures++;
    }


    {
        int8_t a = -57;
        int8_t b = -2;
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
        uint8_t a[6] = {144,172,70,161,223,151};
        if (a[0] != 144) failures++;
    }


    {
        uint8_t x = 27;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        g16 = 55747;
        if (read_g16() != 55747) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(17,134) != 151) failures++;
    }


    {
        if (((uint16_t)(((107 - 86) | (153 - 160)) - 206)) != 65327) failures++;
    }


    {
        g16 = 30853;
        if (read_g16() != 30853) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        volatile uint8_t port = 229;
        uint8_t r = port;
        if (r != 229) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(81,3) != 78) failures++;
    }


    {
        uint8_t buf[8] = {3,113,224,223,87,151,159,153};
        uint8_t *p = buf;
        p += 0;
        if (*p != 3) failures++;
    }


    {
        uint16_t r = add2(120,226) + add2(226,115) + add2(120,115);
        if (r != 922) failures++;
    }


    {
        uint16_t x = 5223;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 38433;
        if (read_g16() != 38433) failures++;
    }


    {
        volatile uint8_t port = 72;
        uint8_t r = port;
        if (r != 72) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        volatile uint8_t port = 32;
        uint8_t r = port;
        if (r != 32) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)235) + (uint16_t)22064;
        if (r != 22299) failures++;
    }


    {
        uint16_t x = 19367;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {74,91,3,30,46,124};
        if (a[3] != 30) failures++;
    }


    {
        uint8_t m[4][4] = {{183,93,215,249},{48,12,131,108},{196,175,9,93},{88,102,149,91}};
        if (m[3][0] != 88) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint32_t a = 2307054365UL;
        uint32_t b = 3089874521UL;
        uint32_t r = a - b;
        if (r != 3512147140UL) failures++;
    }


    {
        volatile int16_t a = -18075;
        volatile int16_t b = -3182;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][3] = {{38,104,190},{11,104,152},{255,209,18},{244,232,113}};
        if (m[1][1] != 104) failures++;
    }


    {
        uint8_t a[6] = {192,177,53,97,101,22};
        if (a[1] != 177) failures++;
    }


    {
        uint8_t buf[8] = {103,88,132,211,146,95,52,2};
        uint8_t *p = buf;
        p += 4;
        if (*p != 146) failures++;
    }


    {
        uint8_t v = 133;
        int r = (v & 64) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t x = 42764;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 208;
        if (buf[12] != 208) failures++;
    }


    {
        volatile uint8_t port = 178;
        uint8_t r = port;
        if (r != 178) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 14: result = 47; break;
        case 15: result = 221; break;
        case 12: result = 44; break;
        case 4: result = 185; break;
        case 5: result = 105; break;
        default: result = 27; break;
        }
        if (result != 27) failures++;
    }


    {
        volatile uint8_t port = 2;
        uint8_t r = port;
        if (r != 2) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint32_t a = 4006452927UL;
        uint32_t b = 952345849UL;
        uint32_t r = a & b;
        if (r != 683771065UL) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 18: result = 20; break;
        case 5: result = 154; break;
        case 9: result = 147; break;
        case 19: result = 145; break;
        case 15: result = 199; break;
        default: result = 15; break;
        }
        if (result != 15) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 30;
        int r = (v & 32) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int8_t a = -92;
        int8_t b = -17;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 235;
        x = x + 22;
        if (x != 257) failures++;
    }


    {
        uint16_t r = 37219 + 34761 + 44023 + 54450 + 35705 + 23643 + 24958 + 28176;
        if (r != 20791) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 13: result = 255; break;
        case 7: result = 26; break;
        case 0: result = 173; break;
        case 5: result = 145; break;
        case 8: result = 16; break;
        case 6: result = 101; break;
        default: result = 246; break;
        }
        if (result != 26) failures++;
    }


    {
        uint8_t src[12] = {182,189,165,186,23,59,91,167,130,97,171,127};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[7] != 167) failures++;
    }


    {
        if (((uint16_t)(((115 ^ 252) | (217 ^ 252)) & ((51 & 71) | (252 + 140)))) != 139) failures++;
    }


    {
        if (((uint16_t)90) != 90) failures++;
    }


    {
        uint8_t m[3][2] = {{184,35},{109,182},{39,223}};
        if (m[0][1] != 35) failures++;
    }


    {
        uint16_t x = 225;
        x = x + 89;
        if (x != 314) failures++;
    }


    {
        uint8_t buf[8] = {122,30,109,81,91,23,2,80};
        uint8_t *p = buf;
        p += 1;
        if (*p != 30) failures++;
    }


    {
        uint8_t x = 173;
        x <<= 0;
        if (x != 173) failures++;
    }


    {
        uint8_t v = 120;
        v ^= 128;
        if (v != 248) failures++;
    }


    {
        g16 = 24131;
        if (read_g16() != 24131) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {24,71,56244,121};
        if (s.a != (uint8_t)24) failures++;
    }


    {
        volatile uint8_t port = 139;
        uint8_t r = port;
        if (r != 139) failures++;
    }


    {
        uint32_t a = 3726440258UL;
        uint32_t b = 288020263UL;
        uint32_t r = a ^ b;
        if (r != 3476430949UL) failures++;
    }


    {
        uint16_t r = add2(45,179) + add2(179,79) + add2(45,79);
        if (r != 606) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 209;
        if (buf[14] != 209) failures++;
    }


    {
        uint8_t v = 27;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t x = 33;
        x <<= 2;
        if (x != 132) failures++;
    }


    {
        uint16_t x = 51027;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(96,107) != 203) failures++;
    }


    {
        if (((uint16_t)(((235 & 159) + 248) - 217)) != 170) failures++;
    }


    {
        g16 = 26584;
        if (read_g16() != 26584) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint16_t r = 46392 + 13428 + 8070 + 50037 + 10845 + 61135 + 35617 + 48787;
        if (r != 12167) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {251,38,45049,15};
        if (s.b != (uint8_t)38) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint16_t r = add2(192,230) + add2(230,144) + add2(192,144);
        if (r != 1132) failures++;
    }


    {
        if (((uint16_t)(((247 - 240) ^ (41 ^ 161)) - 38)) != 105) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)174) + (uint16_t)55978;
        if (r != 56152) failures++;
    }


    {
        uint16_t r = add2(19,244) + add2(244,94) + add2(19,94);
        if (r != 714) failures++;
    }


    {
        uint8_t v = 170;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 59374;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 157;
        x = x + 47;
        if (x != 204) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {94,143,17660,53};
        if (s.b != (uint8_t)143) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 11: result = 98; break;
        case 10: result = 194; break;
        case 16: result = 252; break;
        default: result = 158; break;
        }
        if (result != 194) failures++;
    }


    {
        uint8_t v = 43;
        v &= ~(uint8_t)16;
        if (v != 43) failures++;
    }


    {
        uint16_t r = 49200 + 35644 + 6376 + 12656 + 16069 + 29258 + 60809 + 6934;
        if (r != 20338) failures++;
    }


    {
        volatile uint8_t port = 118;
        uint8_t r = port;
        if (r != 118) failures++;
    }


    {
        uint16_t r = 49733 + 8511 + 41980 + 56086 + 15423 + 37993 + 7083 + 1529;
        if (r != 21730) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 88;
        if (buf[6] != 88) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {139,191,46674,173};
        if (s.d != (uint8_t)173) failures++;
    }


    {
        uint32_t a = 2000963874UL;
        uint32_t b = 2225379343UL;
        uint32_t r = a + b;
        if (r != 4226343217UL) failures++;
    }


    {
        volatile int16_t a = 22730;
        volatile int16_t b = 6086;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)129) + (uint16_t)55189;
        if (r != 55318) failures++;
    }


    {
        uint16_t r = call6(173,175,193,67,71,87);
        if (r != 766) failures++;
    }


    {
        uint32_t a = 1459669209UL;
        uint32_t b = 3587938078UL;
        uint32_t r = a | b;
        if (r != 3621511135UL) failures++;
    }


    {
        uint8_t buf[8] = {130,182,130,187,123,26,191,9};
        uint8_t *p = buf;
        p += 2;
        if (*p != 130) failures++;
    }


    {
        volatile int16_t a = 27016;
        volatile int16_t b = -27023;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[1] = {34};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 34) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)114) + (uint16_t)59615;
        if (r != 59729) failures++;
    }


    {
        uint16_t r = call6(48,118,20,7,66,87);
        if (r != 346) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 201;
        if (buf[3] != 201) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 101;
        if (buf[7] != 101) failures++;
    }


    {
        uint8_t m[2][2] = {{37,243},{189,123}};
        if (m[0][1] != 243) failures++;
    }


    {
        volatile uint8_t port = 213;
        uint8_t r = port;
        if (r != 213) failures++;
    }


    {
        g16 = 44046;
        if (read_g16() != 44046) failures++;
    }


    {
        uint32_t a = 161021476UL;
        uint32_t b = 3016856777UL;
        uint32_t r = a ^ b;
        if (r != 3125373677UL) failures++;
    }


    {
        int8_t a = 118;
        int8_t b = 8;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 244;
        x = x + 167;
        if (x != 411) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 9;
        if (buf[12] != 9) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {186,123,59207,133};
        if (s.c != (uint16_t)59207) failures++;
    }


    {
        uint16_t r = 4054 + 53689 + 2725 + 22837 + 58193 + 1212 + 62462 + 59769;
        if (r != 2797) failures++;
    }


    {
        uint8_t src[6] = {215,201,187,243,185,171};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[2] != 187) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)22) % (int16_t)((int8_t)99);
        if ((uint16_t)r != (uint16_t)22) failures++;
    }


    {
        uint8_t v = 163;
        v ^= 16;
        if (v != 179) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(100,81) != 181) failures++;
    }


    {
        uint16_t r = add2(94,163) + add2(163,49) + add2(94,49);
        if (r != 612) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(244,140) != 384) failures++;
    }


    {
        if (((uint16_t)((81 | 183) + ((181 - 209) + (213 + 139)))) != 571) failures++;
    }


    {
        uint16_t r = call6(68,104,129,243,42,183);
        if (r != 769) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(8,1) != 7) failures++;
    }


    {
        uint32_t a = 1393841790UL;
        uint32_t b = 2701109943UL;
        uint32_t r = a ^ b;
        if (r != 4092329161UL) failures++;
    }


    {
        uint8_t v = 110;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 168;
        x <<= 1;
        if (x != 80) failures++;
    }


    {
        uint8_t src[10] = {222,209,117,135,90,51,190,136,140,145};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[3] != 135) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {43,68,63307,50};
        if (s.c != (uint16_t)63307) failures++;
    }


    {
        uint16_t r = 7389 + 52654 + 10346 + 64183 + 12787 + 32377 + 61759 + 49188;
        if (r != 28539) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)149) + (uint16_t)45516;
        if (r != 45665) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)195) + (uint16_t)60366;
        if (r != 60561) failures++;
    }


    {
        uint8_t m[4][4] = {{252,112,218,186},{223,244,38,201},{246,149,112,64},{225,239,253,153}};
        if (m[0][3] != 186) failures++;
    }


    {
        uint8_t buf[8] = {147,249,56,170,193,36,133,18};
        uint8_t *p = buf;
        p += 0;
        if (*p != 147) failures++;
    }


    {
        uint16_t x = 14488;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)51) != 51) failures++;
    }


    {
        uint16_t r = 7115 + 26647 + 60678 + 11277 + 60388 + 42003 + 14413 + 15487;
        if (r != 41400) failures++;
    }


    {
        g16 = 57772;
        if (read_g16() != 57772) failures++;
    }


    {
        uint16_t r = call6(38,38,119,17,173,87);
        if (r != 472) failures++;
    }


    {
        uint16_t r = add2(2,137) + add2(137,59) + add2(2,59);
        if (r != 396) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)160) + (uint16_t)9390;
        if (r != 9550) failures++;
    }


    {
        if (((uint16_t)((171 & (223 - 42)) ^ 121)) != 216) failures++;
    }


    {
        uint32_t a = 975300376UL;
        uint32_t b = 322419627UL;
        uint32_t r = a + b;
        if (r != 1297720003UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)89) / (int16_t)((int8_t)84);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        g16 = 52540;
        if (read_g16() != 52540) failures++;
    }


    {
        uint32_t a = 2118258279UL;
        uint32_t b = 1112115866UL;
        uint32_t r = a & b;
        if (r != 1111493122UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {29,200,48791,139};
        if (s.c != (uint16_t)48791) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint8_t x = 59;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t x = 70;
        x = x + 127;
        if (x != 197) failures++;
    }


    {
        uint16_t r = call6(244,174,116,247,174,134);
        if (r != 1089) failures++;
    }


    {
        uint8_t m[3][2] = {{229,172},{242,53},{136,61}};
        if (m[2][0] != 136) failures++;
    }


    {
        uint8_t buf[8] = {120,25,92,117,117,215,38,246};
        uint8_t *p = buf;
        p += 0;
        if (*p != 120) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 17: result = 168; break;
        case 14: result = 181; break;
        case 10: result = 125; break;
        case 4: result = 165; break;
        case 8: result = 0; break;
        case 15: result = 211; break;
        case 0: result = 131; break;
        default: result = 141; break;
        }
        if (result != 211) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)254) + (uint16_t)5858;
        if (r != 6112) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 19: result = 56; break;
        case 4: result = 217; break;
        case 18: result = 247; break;
        case 8: result = 160; break;
        case 3: result = 84; break;
        default: result = 125; break;
        }
        if (result != 84) failures++;
    }


    {
        volatile uint8_t port = 116;
        uint8_t r = port;
        if (r != 116) failures++;
    }


    {
        uint8_t src[16] = {7,6,111,218,232,177,14,78,226,38,152,67,166,210,125,213};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[4] != 232) failures++;
    }


    {
        uint8_t x = 138;
        x <<= 1;
        if (x != 20) failures++;
    }


    {
        volatile uint8_t port = 4;
        uint8_t r = port;
        if (r != 4) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {34,31,61982,129};
        if (s.a != (uint8_t)34) failures++;
    }


    {
        uint16_t r = add2(32,44) + add2(44,89) + add2(32,89);
        if (r != 330) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 82;
        if (buf[12] != 82) failures++;
    }


    {
        volatile uint8_t port = 204;
        uint8_t r = port;
        if (r != 204) failures++;
    }


    {
        int8_t a = -33;
        int8_t b = 57;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 164;
        x = x + 145;
        if (x != 309) failures++;
    }


    {
        volatile int16_t a = -31254;
        volatile int16_t b = -14531;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 6;
        uint8_t r = port;
        if (r != 6) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint8_t x = 148;
        x <<= 2;
        if (x != 80) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 196;
        uint8_t r = port;
        if (r != 196) failures++;
    }


    {
        g16 = 27887;
        if (read_g16() != 27887) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 172; break;
        case 16: result = 22; break;
        case 12: result = 219; break;
        case 1: result = 60; break;
        default: result = 250; break;
        }
        if (result != 172) failures++;
    }


    {
        uint8_t x = 199;
        x <<= 3;
        if (x != 56) failures++;
    }


    {
        uint8_t a[6] = {143,227,143,93,129,166};
        if (a[5] != 166) failures++;
    }


    {
        int8_t a = -46;
        int8_t b = 4;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 11;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t a[6] = {104,242,48,14,157,170};
        if (a[3] != 14) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)95) + (uint16_t)5997;
        if (r != 6092) failures++;
    }


    {
        uint8_t src[5] = {113,251,80,11,66};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[1] != 251) failures++;
    }


    {
        uint8_t x = 66;
        x <<= 5;
        if (x != 64) failures++;
    }


    {
        uint8_t x = 126;
        x <<= 1;
        if (x != 252) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)20) / (int16_t)((int8_t)-116);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)245) + (uint16_t)14804;
        if (r != 15049) failures++;
    }


    {
        uint32_t a = 639722033UL;
        uint32_t b = 4163106704UL;
        uint32_t r = a - b;
        if (r != 771582625UL) failures++;
    }


    {
        uint8_t v = 215;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)49) + (uint16_t)4091;
        if (r != 4140) failures++;
    }


    {
        uint8_t m[4][3] = {{196,55,22},{129,3,209},{93,119,125},{79,60,76}};
        if (m[3][2] != 76) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)22) / (int16_t)((int8_t)-40);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {172,232,50,109,147,163};
        if (a[1] != 232) failures++;
    }


    {
        uint16_t r = 15845 + 37349 + 58529 + 27070 + 56333 + 2970 + 11744 + 58043;
        if (r != 5739) failures++;
    }


    {
        uint16_t r = 18808 + 52048 + 63025 + 8774 + 60746 + 2713 + 58939 + 7745;
        if (r != 10654) failures++;
    }


    {
        uint8_t x = 63;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t x = 135;
        x = x + 34;
        if (x != 169) failures++;
    }


    {
        uint8_t x = 87;
        x <<= 2;
        if (x != 92) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)114) % (int16_t)((int8_t)-95);
        if ((uint16_t)r != (uint16_t)19) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)68) + (uint16_t)29274;
        if (r != 29342) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {121,210,38924,70};
        if (s.a != (uint8_t)121) failures++;
    }


    {
        uint16_t r = call6(48,104,109,77,41,39);
        if (r != 418) failures++;
    }


    {
        uint16_t x = 229;
        x = x + 84;
        if (x != 313) failures++;
    }


    {
        uint8_t buf[8] = {48,209,111,252,99,214,31,24};
        uint8_t *p = buf;
        p += 0;
        if (*p != 48) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 47060;
        if (read_g16() != 47060) failures++;
    }


    {
        uint16_t r = add2(254,70) + add2(70,81) + add2(254,81);
        if (r != 810) failures++;
    }


    {
        uint16_t r = add2(93,197) + add2(197,27) + add2(93,27);
        if (r != 634) failures++;
    }


    {
        uint8_t v = 161;
        v |= 8;
        if (v != 169) failures++;
    }


    {
        uint8_t v = 207;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 17) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(191,6) != 197) failures++;
    }


    {
        uint8_t buf[8] = {122,193,152,157,75,202,174,29};
        uint8_t *p = buf;
        p += 3;
        if (*p != 157) failures++;
    }


    {
        if (((uint16_t)(((247 + 17) + (25 + 195)) | (70 | 170))) != 494) failures++;
    }


    {
        g16 = 6484;
        if (read_g16() != 6484) failures++;
    }


    {
        if (((uint16_t)114) != 114) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 14: result = 108; break;
        case 0: result = 32; break;
        case 19: result = 72; break;
        case 15: result = 205; break;
        case 16: result = 51; break;
        default: result = 79; break;
        }
        if (result != 79) failures++;
    }


    {
        volatile uint8_t port = 138;
        uint8_t r = port;
        if (r != 138) failures++;
    }


    {
        g16 = 11922;
        if (read_g16() != 11922) failures++;
    }


    {
        uint8_t buf[8] = {210,142,214,62,139,206,94,77};
        uint8_t *p = buf;
        p += 7;
        if (*p != 77) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint8_t buf[8] = {47,20,156,214,196,250,195,162};
        uint8_t *p = buf;
        p += 7;
        if (*p != 162) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {206,85,30658,100};
        if (s.a != (uint8_t)206) failures++;
    }


    {
        uint16_t x = 31;
        x = x + 163;
        if (x != 194) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)26) + (uint16_t)52596;
        if (r != 52622) failures++;
    }


    {
        uint8_t m[3][3] = {{186,137,204},{157,2,120},{76,85,64}};
        if (m[2][1] != 85) failures++;
    }


    {
        uint8_t v = 2;
        v |= 1;
        if (v != 3) failures++;
    }


    {
        volatile uint8_t port = 155;
        uint8_t r = port;
        if (r != 155) failures++;
    }


    {
        uint8_t v = 204;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t x = 31018;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 190;
        uint8_t r = port;
        if (r != 190) failures++;
    }


    {
        uint8_t v = 60;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint8_t buf[8] = {43,23,186,100,253,24,190,15};
        uint8_t *p = buf;
        p += 4;
        if (*p != 253) failures++;
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
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(216,174) != 390) failures++;
    }


    {
        uint16_t r = 58586 + 36026 + 57379 + 2028 + 39850 + 60335 + 40830 + 64790;
        if (r != 32144) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 2: result = 44; break;
        case 0: result = 157; break;
        case 16: result = 205; break;
        case 5: result = 114; break;
        case 3: result = 110; break;
        case 18: result = 203; break;
        case 15: result = 49; break;
        case 17: result = 60; break;
        default: result = 226; break;
        }
        if (result != 203) failures++;
    }


    {
        uint8_t a[6] = {219,100,226,103,84,82};
        if (a[1] != 100) failures++;
    }


    {
        uint32_t a = 1642070253UL;
        uint32_t b = 4232202085UL;
        uint32_t r = a & b;
        if (r != 1614807141UL) failures++;
    }


    {
        uint16_t x = 59;
        x = x + 64;
        if (x != 123) failures++;
    }


    {
        uint8_t src[10] = {111,113,156,153,150,2,82,185,115,107};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[4] != 150) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint8_t v = 158;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 170;
        if (buf[0] != 170) failures++;
    }


    {
        int8_t a = 70;
        int8_t b = -118;
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
        uint8_t a[6] = {144,221,41,94,195,93};
        if (a[4] != 195) failures++;
    }


    {
        uint16_t x = 2692;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 253;
        x = x + 61;
        if (x != 314) failures++;
    }


    {
        int8_t a = 9;
        int8_t b = -120;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 9;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 119) failures++;
    }


    {
        uint8_t v = 207;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 56145;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 46;
        if (buf[6] != 46) failures++;
    }


    {
        volatile uint8_t port = 217;
        uint8_t r = port;
        if (r != 217) failures++;
    }


    {
        if (((uint16_t)(((121 ^ 171) ^ (201 ^ 70)) + 211)) != 304) failures++;
    }


    {
        uint16_t r = add2(117,7) + add2(7,50) + add2(117,50);
        if (r != 348) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = call6(198,28,240,58,112,10);
        if (r != 646) failures++;
    }


    {
        uint8_t a[6] = {193,7,118,122,135,205};
        if (a[5] != 205) failures++;
    }


    {
        int8_t a = 43;
        int8_t b = 110;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) % (int16_t)((int8_t)8);
        if ((uint16_t)r != (uint16_t)65530) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)87) % (int16_t)((int8_t)-24);
        if ((uint16_t)r != (uint16_t)15) failures++;
    }


    {
        uint8_t v = 230;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(198,88,12,137,232,51);
        if (r != 718) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 8: result = 60; break;
        case 18: result = 113; break;
        case 0: result = 52; break;
        case 1: result = 165; break;
        case 6: result = 178; break;
        default: result = 97; break;
        }
        if (result != 113) failures++;
    }


    {
        uint8_t m[3][2] = {{253,124},{212,55},{250,86}};
        if (m[0][0] != 253) failures++;
    }


    {
        uint16_t x = 126;
        x = x + 97;
        if (x != 223) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        volatile int16_t a = 21228;
        volatile int16_t b = 28243;
        int r = (a != b);
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
        uint16_t r = (uint16_t)((uint8_t)195) + (uint16_t)35085;
        if (r != 35280) failures++;
    }


    {
        uint8_t m[2][3] = {{255,87,147},{238,163,76}};
        if (m[0][2] != 147) failures++;
    }


    {
        uint16_t x = 62996;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)((235 + (18 - 34)) - (89 ^ (151 - 0)))) != 13) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {157,184,18541,26};
        if (s.c != (uint16_t)18541) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint16_t r = add2(238,142) + add2(142,254) + add2(238,254);
        if (r != 1268) failures++;
    }


    {
        uint8_t a[6] = {91,143,64,70,32,184};
        if (a[5] != 184) failures++;
    }


    {
        int8_t a = 47;
        int8_t b = -37;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(21,199,209,161,45,108);
        if (r != 743) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(103,102) != 1) failures++;
    }


    {
        if (((uint16_t)41) != 41) failures++;
    }


    {
        uint8_t buf[8] = {53,69,90,196,45,248,85,229};
        uint8_t *p = buf;
        p += 1;
        if (*p != 69) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 9: result = 151; break;
        case 1: result = 131; break;
        case 14: result = 131; break;
        case 18: result = 102; break;
        case 15: result = 242; break;
        case 3: result = 158; break;
        default: result = 37; break;
        }
        if (result != 131) failures++;
    }


    {
        uint8_t v = 104;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 24) failures++;
    }


    {
        g16 = 20404;
        if (read_g16() != 20404) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {195,141,48372,88};
        if (s.c != (uint16_t)48372) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t v = 149;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = 4221;
        volatile int16_t b = 11017;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(158,22,145,208,120,161);
        if (r != 814) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(57,79) != 136) failures++;
    }


    {
        uint16_t r = call6(32,60,193,107,209,85);
        if (r != 686) failures++;
    }


    {
        uint8_t a[6] = {19,218,90,88,174,61};
        if (a[3] != 88) failures++;
    }


    {
        uint8_t v = 50;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 14) failures++;
    }


    {
        uint8_t src[3] = {214,148,235};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 148) failures++;
    }


    {
        int8_t a = 120;
        int8_t b = -29;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[3][2] = {{20,59},{158,8},{8,88}};
        if (m[0][1] != 59) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 17: result = 255; break;
        case 11: result = 217; break;
        case 14: result = 1; break;
        case 1: result = 229; break;
        case 8: result = 145; break;
        case 18: result = 107; break;
        case 13: result = 227; break;
        default: result = 38; break;
        }
        if (result != 255) failures++;
    }


    {
        uint8_t v = 251;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 96;
        x = x + 38;
        if (x != 134) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)251) + (uint16_t)1735;
        if (r != 1986) failures++;
    }


    {
        uint16_t x = 173;
        x = x + 161;
        if (x != 334) failures++;
    }


    {
        g16 = 60271;
        if (read_g16() != 60271) failures++;
    }


    {
        uint8_t v = 142;
        v &= ~(uint8_t)8;
        if (v != 134) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(108,194) != 65450) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 98;
        if (buf[2] != 98) failures++;
    }


    {
        uint16_t r = 44502 + 50974 + 9303 + 50489 + 44876 + 35025 + 30890 + 23245;
        if (r != 27160) failures++;
    }


    {
        uint16_t r = add2(157,192) + add2(192,91) + add2(157,91);
        if (r != 880) failures++;
    }


    {
        uint8_t m[2][4] = {{185,243,235,6},{219,25,7,253}};
        if (m[1][0] != 219) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 11;
        if (buf[7] != 11) failures++;
    }


    {
        if (((uint16_t)211) != 211) failures++;
    }


    {
        uint16_t x = 25;
        x = x + 43;
        if (x != 68) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)161) + (uint16_t)9769;
        if (r != 9930) failures++;
    }


    {
        uint8_t buf[8] = {26,245,154,105,172,232,193,58};
        uint8_t *p = buf;
        p += 3;
        if (*p != 105) failures++;
    }


    {
        g16 = 421;
        if (read_g16() != 421) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(8,92) != 65452) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)93) + (uint16_t)21951;
        if (r != 22044) failures++;
    }


    {
        uint32_t a = 317700822UL;
        uint32_t b = 1145282244UL;
        uint32_t r = a + b;
        if (r != 1462983066UL) failures++;
    }


    {
        uint16_t r = 38338 + 3198 + 64735 + 33752 + 8034 + 27414 + 2182 + 58621;
        if (r != 39666) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {33,52,54989,5};
        if (s.d != (uint8_t)5) failures++;
    }


    {
        volatile uint8_t port = 138;
        uint8_t r = port;
        if (r != 138) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 59;
        if (buf[2] != 59) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)53) + (uint16_t)43977;
        if (r != 44030) failures++;
    }


    {
        uint8_t v = 149;
        v |= 8;
        if (v != 157) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)106) + (uint16_t)38368;
        if (r != 38474) failures++;
    }


    {
        uint8_t a[6] = {49,117,103,242,221,52};
        if (a[0] != 49) failures++;
    }


    {
        uint32_t a = 2169722975UL;
        uint32_t b = 56612807UL;
        uint32_t r = a | b;
        if (r != 2204098527UL) failures++;
    }


    {
        uint16_t r = add2(7,192) + add2(192,122) + add2(7,122);
        if (r != 642) failures++;
    }


    {
        uint8_t v = 33;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)126) / (int16_t)((int8_t)-16);
        if ((uint16_t)r != (uint16_t)65529) failures++;
    }


    {
        uint16_t r = 49749 + 46457 + 46387 + 2376 + 38577 + 44225 + 34370 + 12834;
        if (r != 12831) failures++;
    }


    {
        uint32_t a = 524683877UL;
        uint32_t b = 4147266656UL;
        uint32_t r = a + b;
        if (r != 376983237UL) failures++;
    }


    {
        uint8_t src[16] = {6,42,168,142,183,255,218,254,107,94,161,86,227,51,87,150};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[7] != 254) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(80,123) != 65493) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)215) + (uint16_t)40354;
        if (r != 40569) failures++;
    }


    {
        uint8_t a[6] = {131,111,133,130,239,81};
        if (a[5] != 81) failures++;
    }


    {
        uint16_t x = 229;
        x = x + 81;
        if (x != 310) failures++;
    }


    {
        if (((uint16_t)(((141 & 74) - (217 - 155)) | (251 | (134 + 150)))) != 65535) failures++;
    }


    {
        uint16_t r = add2(95,230) + add2(230,74) + add2(95,74);
        if (r != 798) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 67;
        if (buf[1] != 67) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {60,68,34278,75};
        if (s.d != (uint8_t)75) failures++;
    }


    {
        uint16_t r = 15407 + 47718 + 60378 + 64937 + 41479 + 12020 + 5747 + 985;
        if (r != 52063) failures++;
    }


    {
        volatile uint8_t port = 190;
        uint8_t r = port;
        if (r != 190) failures++;
    }

    return failures;
}