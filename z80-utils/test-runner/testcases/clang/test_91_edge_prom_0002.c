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
        for (uint16_t j = 0; j < 20; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {45,39,27065,126};
        if (s.d != (uint8_t)126) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 18: result = 10; break;
        case 10: result = 19; break;
        case 11: result = 24; break;
        case 4: result = 252; break;
        case 7: result = 50; break;
        case 17: result = 3; break;
        case 5: result = 37; break;
        default: result = 129; break;
        }
        if (result != 252) failures++;
    }


    {
        uint8_t a[6] = {61,209,42,219,84,75};
        if (a[3] != 219) failures++;
    }


    {
        uint16_t r = call6(67,199,216,151,193,90);
        if (r != 916) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {199,188,35524,88};
        if (s.c != (uint16_t)35524) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 16: result = 112; break;
        case 17: result = 72; break;
        case 15: result = 11; break;
        case 11: result = 216; break;
        default: result = 118; break;
        }
        if (result != 112) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)178) + (uint16_t)54648;
        if (r != 54826) failures++;
    }


    {
        uint8_t v = 247;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = 25425 + 28282 + 8612 + 26933 + 20256 + 63215 + 4337 + 37153;
        if (r != 17605) failures++;
    }


    {
        uint8_t v = 186;
        v ^= 128;
        if (v != 58) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(153,113) != 40) failures++;
    }


    {
        uint8_t a[6] = {41,249,108,45,216,116};
        if (a[4] != 216) failures++;
    }


    {
        uint8_t a[6] = {186,231,30,126,7,254};
        if (a[1] != 231) failures++;
    }


    {
        uint32_t a = 3731501017UL;
        uint32_t b = 2932110778UL;
        uint32_t r = a ^ b;
        if (r != 1890474595UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {135,218,4903,117};
        if (s.d != (uint8_t)117) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(203,65) != 268) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {1,124,1781,204};
        if (s.d != (uint8_t)204) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)67) + (uint16_t)42441;
        if (r != 42508) failures++;
    }


    {
        uint8_t m[2][4] = {{25,36,98,77},{162,44,97,164}};
        if (m[1][3] != 164) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {239,13,57978,1};
        if (s.c != (uint16_t)57978) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 3: result = 65; break;
        case 9: result = 218; break;
        case 19: result = 224; break;
        case 15: result = 38; break;
        case 4: result = 50; break;
        case 16: result = 23; break;
        case 14: result = 157; break;
        case 6: result = 249; break;
        default: result = 221; break;
        }
        if (result != 65) failures++;
    }


    {
        uint16_t r = call6(143,159,159,210,101,166);
        if (r != 938) failures++;
    }


    {
        uint8_t buf[8] = {159,32,111,52,245,19,149,225};
        uint8_t *p = buf;
        p += 6;
        if (*p != 149) failures++;
    }


    {
        uint8_t x = 156;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(228,164) != 64) failures++;
    }


    {
        if (((uint16_t)(((106 | 41) - (145 - 162)) - ((16 & 148) & (245 & 192)))) != 124) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)108) % (int16_t)((int8_t)97);
        if ((uint16_t)r != (uint16_t)11) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)135) + (uint16_t)29756;
        if (r != 29891) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(167,3) != 164) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)19) / (int16_t)((int8_t)18);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t x = 113;
        x = x + 246;
        if (x != 359) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)180) + (uint16_t)22999;
        if (r != 23179) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-95) % (int16_t)((int8_t)-6);
        if ((uint16_t)r != (uint16_t)65531) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)92) + (uint16_t)33725;
        if (r != 33817) failures++;
    }


    {
        uint8_t v = 165;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 27) failures++;
    }


    {
        uint8_t m[2][4] = {{57,60,195,112},{57,96,103,4}};
        if (m[0][3] != 112) failures++;
    }


    {
        int8_t a = -44;
        int8_t b = -4;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(168,200) != 65504) failures++;
    }


    {
        g16 = 32886;
        if (read_g16() != 32886) failures++;
    }


    {
        uint8_t m[3][4] = {{84,206,22,224},{213,20,184,182},{176,97,196,124}};
        if (m[2][0] != 176) failures++;
    }


    {
        uint16_t x = 27616;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 13130;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 196;
        uint8_t r = port;
        if (r != 196) failures++;
    }


    {
        volatile uint8_t port = 106;
        uint8_t r = port;
        if (r != 106) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-39) % (int16_t)((int8_t)-15);
        if ((uint16_t)r != (uint16_t)65527) failures++;
    }


    {
        uint16_t r = add2(31,43) + add2(43,7) + add2(31,7);
        if (r != 162) failures++;
    }


    {
        uint8_t a[6] = {174,30,22,143,144,234};
        if (a[5] != 234) failures++;
    }


    {
        volatile uint8_t port = 44;
        uint8_t r = port;
        if (r != 44) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 8: result = 21; break;
        case 12: result = 160; break;
        case 5: result = 233; break;
        case 14: result = 22; break;
        default: result = 232; break;
        }
        if (result != 232) failures++;
    }


    {
        uint32_t a = 1279178620UL;
        uint32_t b = 2985186473UL;
        uint32_t r = a ^ b;
        if (r != 4258325461UL) failures++;
    }


    {
        uint32_t a = 3764768705UL;
        uint32_t b = 1035273035UL;
        uint32_t r = a | b;
        if (r != 4260745163UL) failures++;
    }


    {
        uint8_t v = 165;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(246,18) + add2(18,138) + add2(246,138);
        if (r != 804) failures++;
    }


    {
        uint32_t a = 2118406128UL;
        uint32_t b = 3214382418UL;
        uint32_t r = a & b;
        if (r != 1040450896UL) failures++;
    }


    {
        uint16_t r = call6(209,74,209,184,49,93);
        if (r != 818) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint8_t src[9] = {54,147,74,199,19,157,13,88,199};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[7] != 88) failures++;
    }


    {
        volatile int16_t a = 28546;
        volatile int16_t b = 12412;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 7;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        volatile int16_t a = 4130;
        volatile int16_t b = 6064;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {136,101,74,45,131,191};
        if (a[3] != 45) failures++;
    }


    {
        g16 = 24381;
        if (read_g16() != 24381) failures++;
    }


    {
        uint8_t x = 181;
        x <<= 4;
        if (x != 80) failures++;
    }


    {
        uint16_t r = call6(129,38,165,113,50,148);
        if (r != 643) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {172,235,44641,216};
        if (s.a != (uint8_t)172) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint16_t x = 3316;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 2719920334UL;
        uint32_t b = 2576150145UL;
        uint32_t r = a - b;
        if (r != 143770189UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint32_t a = 4146499201UL;
        uint32_t b = 1279048986UL;
        uint32_t r = a & b;
        if (r != 1143245824UL) failures++;
    }


    {
        uint8_t src[14] = {1,129,216,146,255,174,99,158,229,126,73,150,111,159};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[12] != 111) failures++;
    }


    {
        uint16_t x = 107;
        x = x + 119;
        if (x != 226) failures++;
    }


    {
        uint16_t r = add2(140,41) + add2(41,23) + add2(140,23);
        if (r != 408) failures++;
    }


    {
        uint32_t a = 3772067765UL;
        uint32_t b = 3778702585UL;
        uint32_t r = a & b;
        if (r != 3759156401UL) failures++;
    }


    {
        volatile int16_t a = -6388;
        volatile int16_t b = 12221;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 59108 + 40595 + 52409 + 38984 + 35182 + 14077 + 34957 + 31218;
        if (r != 44386) failures++;
    }


    {
        uint32_t a = 780395154UL;
        uint32_t b = 1558199206UL;
        uint32_t r = a | b;
        if (r != 2128870326UL) failures++;
    }


    {
        uint16_t x = 41665;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 12;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        volatile uint8_t port = 155;
        uint8_t r = port;
        if (r != 155) failures++;
    }


    {
        g16 = 45169;
        if (read_g16() != 45169) failures++;
    }


    {
        int8_t a = 19;
        int8_t b = -77;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 6: result = 195; break;
        case 13: result = 19; break;
        case 0: result = 12; break;
        default: result = 199; break;
        }
        if (result != 19) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)204) + (uint16_t)38920;
        if (r != 39124) failures++;
    }


    {
        uint16_t r = add2(81,172) + add2(172,201) + add2(81,201);
        if (r != 908) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)37) + (uint16_t)30214;
        if (r != 30251) failures++;
    }


    {
        g16 = 53026;
        if (read_g16() != 53026) failures++;
    }


    {
        uint8_t v = 110;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 245;
        uint8_t r = port;
        if (r != 245) failures++;
    }


    {
        uint16_t x = 244;
        x = x + 195;
        if (x != 439) failures++;
    }


    {
        uint16_t x = 147;
        x = x + 92;
        if (x != 239) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-92) / (int16_t)((int8_t)-64);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)101) + (uint16_t)22318;
        if (r != 22419) failures++;
    }


    {
        uint8_t buf[8] = {47,20,243,186,44,198,141,231};
        uint8_t *p = buf;
        p += 2;
        if (*p != 243) failures++;
    }


    {
        uint32_t a = 2077879076UL;
        uint32_t b = 1974559197UL;
        uint32_t r = a & b;
        if (r != 1905352964UL) failures++;
    }


    {
        uint8_t buf[8] = {237,204,42,67,106,226,233,29};
        uint8_t *p = buf;
        p += 0;
        if (*p != 237) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(59,47) != 106) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-117) % (int16_t)((int8_t)62);
        if ((uint16_t)r != (uint16_t)65481) failures++;
    }


    {
        uint8_t buf[8] = {108,203,219,125,87,41,235,105};
        uint8_t *p = buf;
        p += 7;
        if (*p != 105) failures++;
    }


    {
        uint8_t a[6] = {74,63,51,94,116,92};
        if (a[3] != 94) failures++;
    }


    {
        uint16_t r = add2(217,110) + add2(110,243) + add2(217,243);
        if (r != 1140) failures++;
    }


    {
        uint16_t x = 228;
        x = x + 15;
        if (x != 243) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {164,4,6474,174};
        if (s.c != (uint16_t)6474) failures++;
    }


    {
        uint16_t r = add2(57,153) + add2(153,149) + add2(57,149);
        if (r != 718) failures++;
    }


    {
        uint8_t a[6] = {94,11,219,107,169,82};
        if (a[4] != 169) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-69) / (int16_t)((int8_t)45);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 1) sum += j;
        if (sum != 105) failures++;
    }


    {
        uint32_t a = 3596629501UL;
        uint32_t b = 821560931UL;
        uint32_t r = a & b;
        if (r != 274728033UL) failures++;
    }


    {
        uint8_t src[7] = {107,63,82,212,84,79,201};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[4] != 84) failures++;
    }


    {
        uint8_t v = 242;
        v ^= 16;
        if (v != 226) failures++;
    }


    {
        uint16_t x = 200;
        x = x + 51;
        if (x != 251) failures++;
    }


    {
        uint32_t a = 1146550180UL;
        uint32_t b = 1120257062UL;
        uint32_t r = a | b;
        if (r != 1188558758UL) failures++;
    }


    {
        uint16_t x = 33902;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 16;
        x <<= 1;
        if (x != 32) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 108;
        if (buf[15] != 108) failures++;
    }


    {
        uint16_t x = 197;
        x = x + 48;
        if (x != 245) failures++;
    }


    {
        uint32_t a = 3485991321UL;
        uint32_t b = 2350146990UL;
        uint32_t r = a & b;
        if (r != 2348810632UL) failures++;
    }


    {
        volatile int16_t a = 7488;
        volatile int16_t b = -24559;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 169;
        if (buf[6] != 169) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(231,201) != 432) failures++;
    }


    {
        uint16_t x = 38110;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)52) % (int16_t)((int8_t)-124);
        if ((uint16_t)r != (uint16_t)52) failures++;
    }


    {
        int8_t a = -28;
        int8_t b = 118;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {43,215,194,196,73,29,238,236};
        uint8_t *p = buf;
        p += 2;
        if (*p != 194) failures++;
    }


    {
        uint8_t src[4] = {110,241,179,144};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[2] != 179) failures++;
    }


    {
        uint8_t a[6] = {185,199,202,156,34,222};
        if (a[0] != 185) failures++;
    }


    {
        uint32_t a = 3007532074UL;
        uint32_t b = 1996953739UL;
        uint32_t r = a | b;
        if (r != 4148649131UL) failures++;
    }


    {
        uint8_t a[6] = {70,67,59,4,17,4};
        if (a[1] != 67) failures++;
    }


    {
        uint8_t v = 19;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 45) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)220) + (uint16_t)59766;
        if (r != 59986) failures++;
    }


    {
        uint8_t x = 148;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        uint32_t a = 2392382197UL;
        uint32_t b = 2583450361UL;
        uint32_t r = a ^ b;
        if (r != 392464396UL) failures++;
    }


    {
        uint16_t r = add2(56,37) + add2(37,156) + add2(56,156);
        if (r != 498) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(19,225) != 65330) failures++;
    }


    {
        uint8_t v = 195;
        v ^= 1;
        if (v != 194) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)243) + (uint16_t)9141;
        if (r != 9384) failures++;
    }


    {
        volatile int16_t a = 23711;
        volatile int16_t b = -20970;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 7;
        x = x + 23;
        if (x != 30) failures++;
    }


    {
        volatile uint8_t port = 162;
        uint8_t r = port;
        if (r != 162) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(68,117) != 185) failures++;
    }


    {
        uint8_t buf[8] = {33,250,185,175,61,178,154,80};
        uint8_t *p = buf;
        p += 3;
        if (*p != 175) failures++;
    }


    {
        uint16_t x = 120;
        x = x + 68;
        if (x != 188) failures++;
    }


    {
        uint16_t x = 46631;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 391;
        if (read_g16() != 391) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {202,185,13385,87};
        if (s.c != (uint16_t)13385) failures++;
    }


    {
        uint32_t a = 2819178698UL;
        uint32_t b = 4171105856UL;
        uint32_t r = a + b;
        if (r != 2695317258UL) failures++;
    }


    {
        uint16_t r = add2(244,177) + add2(177,245) + add2(244,245);
        if (r != 1332) failures++;
    }


    {
        volatile uint8_t port = 47;
        uint8_t r = port;
        if (r != 47) failures++;
    }


    {
        if (((uint16_t)(((72 - 206) & (157 + 224)) & 40)) != 40) failures++;
    }


    {
        volatile int16_t a = 2548;
        volatile int16_t b = -20861;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[5] = {71,172,167,152,135};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 167) failures++;
    }


    {
        uint8_t x = 200;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        uint8_t m[2][2] = {{49,107},{27,148}};
        if (m[1][1] != 148) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 12733;
        if (read_g16() != 12733) failures++;
    }


    {
        uint16_t x = 30600;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)((204 & (84 | 19)) - 2)) != 66) failures++;
    }


    {
        int8_t a = -31;
        int8_t b = 57;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -10150;
        volatile int16_t b = -26186;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 81244732UL;
        uint32_t b = 2233791576UL;
        uint32_t r = a ^ b;
        if (r != 2180204132UL) failures++;
    }


    {
        uint8_t a[6] = {224,121,83,76,229,148};
        if (a[4] != 229) failures++;
    }


    {
        uint8_t v = 177;
        v &= ~(uint8_t)64;
        if (v != 177) failures++;
    }


    {
        uint32_t a = 4236226854UL;
        uint32_t b = 77440468UL;
        uint32_t r = a ^ b;
        if (r != 4175566066UL) failures++;
    }


    {
        uint8_t v = 46;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(32,93,198,143,41,142);
        if (r != 649) failures++;
    }


    {
        uint16_t r = add2(85,9) + add2(9,81) + add2(85,81);
        if (r != 350) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-76) / (int16_t)((int8_t)-52);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t r = add2(236,134) + add2(134,185) + add2(236,185);
        if (r != 1110) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 93;
        if (buf[6] != 93) failures++;
    }


    {
        uint8_t a[6] = {11,97,117,12,180,249};
        if (a[0] != 11) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(147,40) != 107) failures++;
    }


    {
        g16 = 11481;
        if (read_g16() != 11481) failures++;
    }


    {
        uint16_t x = 54026;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 50;
        int8_t b = -13;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(198,117,184,25,205,140);
        if (r != 869) failures++;
    }


    {
        volatile uint8_t port = 38;
        uint8_t r = port;
        if (r != 38) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        volatile int16_t a = 13851;
        volatile int16_t b = -14335;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -27200;
        volatile int16_t b = -16784;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 41;
        uint8_t r = port;
        if (r != 41) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = add2(250,123) + add2(123,185) + add2(250,185);
        if (r != 1116) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-106) % (int16_t)((int8_t)97);
        if ((uint16_t)r != (uint16_t)65527) failures++;
    }


    {
        uint16_t x = 56207;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(47,96) + add2(96,220) + add2(47,220);
        if (r != 726) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 15: result = 209; break;
        case 8: result = 31; break;
        case 7: result = 3; break;
        case 18: result = 192; break;
        case 12: result = 5; break;
        case 16: result = 229; break;
        case 0: result = 104; break;
        default: result = 22; break;
        }
        if (result != 192) failures++;
    }


    {
        g16 = 11999;
        if (read_g16() != 11999) failures++;
    }


    {
        uint8_t x = 24;
        x <<= 2;
        if (x != 96) failures++;
    }


    {
        uint16_t r = add2(219,147) + add2(147,88) + add2(219,88);
        if (r != 908) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)226) + (uint16_t)39350;
        if (r != 39576) failures++;
    }


    {
        uint8_t a[6] = {9,216,153,31,44,117};
        if (a[5] != 117) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[2][2] = {{243,92},{247,162}};
        if (m[1][0] != 247) failures++;
    }


    {
        uint16_t x = 172;
        x = x + 20;
        if (x != 192) failures++;
    }


    {
        uint8_t src[10] = {14,74,104,125,82,236,167,67,34,24};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[6] != 167) failures++;
    }


    {
        volatile uint8_t port = 250;
        uint8_t r = port;
        if (r != 250) failures++;
    }


    {
        uint32_t a = 942561543UL;
        uint32_t b = 1397106882UL;
        uint32_t r = a | b;
        if (r != 2070837703UL) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 171;
        if (buf[12] != 171) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)127) % (int16_t)((int8_t)-31);
        if ((uint16_t)r != (uint16_t)3) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 1: result = 45; break;
        case 4: result = 44; break;
        case 16: result = 232; break;
        case 3: result = 40; break;
        case 18: result = 210; break;
        default: result = 193; break;
        }
        if (result != 193) failures++;
    }


    {
        uint8_t a[6] = {250,52,84,194,233,163};
        if (a[0] != 250) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {45,140,47368,7};
        if (s.b != (uint8_t)140) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[4][3] = {{216,4,92},{232,160,193},{154,45,246},{173,186,208}};
        if (m[3][0] != 173) failures++;
    }


    {
        volatile uint8_t port = 146;
        uint8_t r = port;
        if (r != 146) failures++;
    }


    {
        uint16_t x = 36181;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 17: result = 8; break;
        case 2: result = 6; break;
        case 9: result = 115; break;
        case 8: result = 171; break;
        default: result = 10; break;
        }
        if (result != 171) failures++;
    }


    {
        g16 = 1152;
        if (read_g16() != 1152) failures++;
    }


    {
        uint8_t buf[8] = {209,130,193,217,24,145,109,8};
        uint8_t *p = buf;
        p += 7;
        if (*p != 8) failures++;
    }


    {
        uint8_t m[2][3] = {{42,153,81},{20,127,238}};
        if (m[1][2] != 238) failures++;
    }


    {
        uint16_t x = 174;
        x = x + 201;
        if (x != 375) failures++;
    }


    {
        volatile int16_t a = 28472;
        volatile int16_t b = 15289;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {54,128,205,42,232,227};
        if (a[4] != 232) failures++;
    }


    {
        uint8_t buf[8] = {3,92,165,239,187,16,51,94};
        uint8_t *p = buf;
        p += 1;
        if (*p != 92) failures++;
    }


    {
        uint32_t a = 1838990959UL;
        uint32_t b = 3941870540UL;
        uint32_t r = a ^ b;
        if (r != 2271800739UL) failures++;
    }


    {
        uint8_t x = 212;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint16_t r = add2(64,149) + add2(149,48) + add2(64,48);
        if (r != 522) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint8_t x = 62;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint16_t r = add2(197,63) + add2(63,136) + add2(197,136);
        if (r != 792) failures++;
    }


    {
        int8_t a = -98;
        int8_t b = -4;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(154,194) != 348) failures++;
    }


    {
        uint32_t a = 1379637963UL;
        uint32_t b = 2851875549UL;
        uint32_t r = a & b;
        if (r != 3672777UL) failures++;
    }


    {
        volatile uint8_t port = 223;
        uint8_t r = port;
        if (r != 223) failures++;
    }


    {
        uint8_t buf[8] = {60,219,240,191,224,10,101,116};
        uint8_t *p = buf;
        p += 1;
        if (*p != 219) failures++;
    }


    {
        g16 = 30301;
        if (read_g16() != 30301) failures++;
    }


    {
        uint16_t r = call6(222,25,240,205,50,35);
        if (r != 777) failures++;
    }


    {
        uint16_t r = call6(44,190,233,97,255,22);
        if (r != 841) failures++;
    }


    {
        uint8_t m[4][4] = {{18,194,62,125},{139,53,77,1},{1,217,29,106},{207,194,39,56}};
        if (m[1][2] != 77) failures++;
    }


    {
        uint16_t r = add2(246,89) + add2(89,177) + add2(246,177);
        if (r != 1024) failures++;
    }


    {
        uint8_t m[3][3] = {{47,23,230},{188,198,15},{223,175,40}};
        if (m[0][1] != 23) failures++;
    }


    {
        volatile uint8_t port = 245;
        uint8_t r = port;
        if (r != 245) failures++;
    }


    {
        if (((uint16_t)(((138 - 230) | (244 - 56)) | ((202 + 80) | (9 ^ 186)))) != 65471) failures++;
    }


    {
        int8_t a = -107;
        int8_t b = -99;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(159,10) + add2(10,186) + add2(159,186);
        if (r != 710) failures++;
    }


    {
        uint16_t x = 19048;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -35;
        int8_t b = 78;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 190;
        if (buf[2] != 190) failures++;
    }


    {
        int8_t a = 77;
        int8_t b = -78;
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
        volatile int16_t a = 19261;
        volatile int16_t b = -22536;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)93) + (uint16_t)5597;
        if (r != 5690) failures++;
    }


    {
        uint8_t src[6] = {158,142,181,172,223,54};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 54) failures++;
    }


    {
        int8_t a = -93;
        int8_t b = 109;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 3597720470UL;
        uint32_t b = 3534704558UL;
        uint32_t r = a & b;
        if (r != 3525331846UL) failures++;
    }


    {
        volatile int16_t a = -29664;
        volatile int16_t b = 14172;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 252;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint8_t a[6] = {43,192,216,2,242,184};
        if (a[1] != 192) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 1) sum += j;
        if (sum != 153) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {85,221,54595,210};
        if (s.b != (uint8_t)221) failures++;
    }


    {
        uint16_t x = 15064;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 45364;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 72;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(((137 | 23) + (30 - 233)) - ((163 - 195) ^ (29 ^ 163)))) != 118) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {182,163,34029,148};
        if (s.d != (uint8_t)148) failures++;
    }


    {
        uint16_t r = 16989 + 64019 + 41407 + 47225 + 63698 + 36711 + 10646 + 17975;
        if (r != 36526) failures++;
    }


    {
        uint16_t r = call6(21,195,216,63,150,137);
        if (r != 782) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-95) % (int16_t)((int8_t)47);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        volatile uint8_t port = 70;
        uint8_t r = port;
        if (r != 70) failures++;
    }


    {
        volatile int16_t a = 28914;
        volatile int16_t b = -28617;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -67;
        int8_t b = 35;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {194,76,20757,21};
        if (s.c != (uint16_t)20757) failures++;
    }


    {
        if (((uint16_t)241) != 241) failures++;
    }


    {
        volatile uint8_t port = 171;
        uint8_t r = port;
        if (r != 171) failures++;
    }


    {
        uint8_t buf[8] = {89,25,3,114,129,23,100,29};
        uint8_t *p = buf;
        p += 5;
        if (*p != 23) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)159) + (uint16_t)49847;
        if (r != 50006) failures++;
    }


    {
        uint8_t buf[8] = {118,194,129,123,235,120,34,197};
        uint8_t *p = buf;
        p += 7;
        if (*p != 197) failures++;
    }


    {
        uint16_t x = 120;
        x = x + 130;
        if (x != 250) failures++;
    }


    {
        uint8_t a[6] = {68,238,84,32,224,105};
        if (a[4] != 224) failures++;
    }


    {
        uint8_t a[6] = {197,109,242,197,216,233};
        if (a[5] != 233) failures++;
    }


    {
        uint32_t a = 1239362495UL;
        uint32_t b = 1867788159UL;
        uint32_t r = a + b;
        if (r != 3107150654UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint16_t r = call6(134,90,87,118,77,88);
        if (r != 594) failures++;
    }


    {
        uint8_t a[6] = {19,194,184,98,49,150};
        if (a[4] != 49) failures++;
    }


    {
        if (((uint16_t)(((137 + 195) & (210 & 90)) + ((13 | 214) | (136 & 199)))) != 287) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t x = 3;
        x <<= 1;
        if (x != 6) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 120;
        if (buf[9] != 120) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 26;
        uint8_t r = port;
        if (r != 26) failures++;
    }


    {
        int8_t a = -90;
        int8_t b = 57;
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
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 1: result = 55; break;
        case 13: result = 69; break;
        case 19: result = 237; break;
        case 0: result = 173; break;
        case 8: result = 83; break;
        default: result = 229; break;
        }
        if (result != 55) failures++;
    }


    {
        uint32_t a = 3493081047UL;
        uint32_t b = 422254484UL;
        uint32_t r = a + b;
        if (r != 3915335531UL) failures++;
    }


    {
        uint16_t r = add2(166,153) + add2(153,19) + add2(166,19);
        if (r != 676) failures++;
    }


    {
        uint16_t r = 42818 + 37246 + 11422 + 27044 + 62074 + 13035 + 44222 + 13615;
        if (r != 54868) failures++;
    }


    {
        uint8_t buf[8] = {218,171,182,173,119,127,70,123};
        uint8_t *p = buf;
        p += 5;
        if (*p != 127) failures++;
    }


    {
        g16 = 20118;
        if (read_g16() != 20118) failures++;
    }


    {
        uint16_t x = 19547;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {113,19,90,224,25,211,183,35};
        uint8_t *p = buf;
        p += 1;
        if (*p != 19) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 11;
        if (buf[11] != 11) failures++;
    }


    {
        uint32_t a = 2890375998UL;
        uint32_t b = 1983537895UL;
        uint32_t r = a & b;
        if (r != 604119590UL) failures++;
    }


    {
        uint16_t r = call6(186,254,130,192,125,177);
        if (r != 1064) failures++;
    }


    {
        uint8_t v = 146;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
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
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 3: result = 226; break;
        case 15: result = 145; break;
        case 10: result = 231; break;
        case 6: result = 70; break;
        case 12: result = 39; break;
        default: result = 249; break;
        }
        if (result != 145) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        if (((uint16_t)24) != 24) failures++;
    }


    {
        uint8_t v = 154;
        v |= 16;
        if (v != 154) failures++;
    }


    {
        if (((uint16_t)(((219 & 106) - 8) - (188 & 173))) != 65430) failures++;
    }


    {
        uint8_t buf[8] = {224,88,238,33,200,49,127,251};
        uint8_t *p = buf;
        p += 3;
        if (*p != 33) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 90;
        if (buf[7] != 90) failures++;
    }


    {
        uint8_t src[15] = {22,169,232,57,55,3,203,56,201,1,161,197,20,22,244};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[14] != 244) failures++;
    }


    {
        g16 = 11169;
        if (read_g16() != 11169) failures++;
    }


    {
        uint8_t x = 113;
        x <<= 1;
        if (x != 226) failures++;
    }


    {
        uint8_t v = 255;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 8: result = 209; break;
        case 4: result = 119; break;
        case 19: result = 243; break;
        case 11: result = 147; break;
        case 0: result = 211; break;
        case 15: result = 43; break;
        case 9: result = 18; break;
        case 16: result = 104; break;
        default: result = 94; break;
        }
        if (result != 94) failures++;
    }


    {
        if (((uint16_t)180) != 180) failures++;
    }


    {
        uint8_t x = 189;
        x <<= 2;
        if (x != 244) failures++;
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
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 45082;
        if (read_g16() != 45082) failures++;
    }


    {
        uint8_t m[3][3] = {{37,246,188},{25,237,161},{15,12,193}};
        if (m[0][0] != 37) failures++;
    }


    {
        uint8_t m[3][3] = {{178,254,67},{186,86,230},{79,66,161}};
        if (m[1][1] != 86) failures++;
    }


    {
        uint8_t m[2][3] = {{207,105,248},{113,95,28}};
        if (m[1][0] != 113) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(192,1) != 193) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(18,240) != 65314) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {100,0,50113,197};
        if (s.a != (uint8_t)100) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-78) / (int16_t)((int8_t)18);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        uint8_t a[6] = {205,33,231,252,16,94};
        if (a[4] != 16) failures++;
    }


    {
        uint8_t buf[8] = {66,184,195,58,176,47,143,129};
        uint8_t *p = buf;
        p += 6;
        if (*p != 143) failures++;
    }


    {
        uint16_t r = call6(55,31,8,234,8,32);
        if (r != 368) failures++;
    }


    {
        volatile int16_t a = 4975;
        volatile int16_t b = -15062;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 58540;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)172) + (uint16_t)42049;
        if (r != 42221) failures++;
    }


    {
        uint8_t m[3][2] = {{190,67},{74,191},{140,66}};
        if (m[1][0] != 74) failures++;
    }


    {
        uint8_t buf[8] = {82,255,178,5,237,200,156,235};
        uint8_t *p = buf;
        p += 1;
        if (*p != 255) failures++;
    }


    {
        uint8_t x = 143;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t r = 47610 + 36566 + 52328 + 53690 + 30926 + 43679 + 11032 + 53995;
        if (r != 2146) failures++;
    }


    {
        uint8_t src[8] = {21,117,126,50,107,216,167,36};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 117) failures++;
    }


    {
        uint8_t v = 209;
        v ^= 64;
        if (v != 145) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)239) + (uint16_t)19793;
        if (r != 20032) failures++;
    }


    {
        if (((uint16_t)66) != 66) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 213;
        if (buf[12] != 213) failures++;
    }


    {
        uint16_t r = call6(64,20,8,233,143,111);
        if (r != 579) failures++;
    }


    {
        g16 = 49507;
        if (read_g16() != 49507) failures++;
    }


    {
        int8_t a = 0;
        int8_t b = 73;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {202,118,3213,173};
        if (s.a != (uint8_t)202) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)121) + (uint16_t)10382;
        if (r != 10503) failures++;
    }


    {
        volatile uint8_t port = 226;
        uint8_t r = port;
        if (r != 226) failures++;
    }


    {
        uint16_t x = 3146;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 12: result = 1; break;
        case 1: result = 57; break;
        case 10: result = 106; break;
        case 4: result = 232; break;
        default: result = 36; break;
        }
        if (result != 232) failures++;
    }


    {
        volatile uint8_t port = 192;
        uint8_t r = port;
        if (r != 192) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 4: result = 244; break;
        case 7: result = 124; break;
        case 6: result = 19; break;
        default: result = 89; break;
        }
        if (result != 89) failures++;
    }


    {
        uint32_t a = 2465219150UL;
        uint32_t b = 1722456208UL;
        uint32_t r = a - b;
        if (r != 742762942UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t m[4][4] = {{51,179,64,36},{254,171,34,127},{103,124,66,149},{126,224,178,11}};
        if (m[3][2] != 178) failures++;
    }


    {
        uint16_t r = 1337 + 16955 + 60928 + 48131 + 45324 + 43237 + 38297 + 19605;
        if (r != 11670) failures++;
    }


    {
        uint8_t v = 211;
        v &= ~(uint8_t)1;
        if (v != 210) failures++;
    }


    {
        uint8_t x = 47;
        x <<= 2;
        if (x != 188) failures++;
    }


    {
        volatile uint8_t port = 36;
        uint8_t r = port;
        if (r != 36) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 8: result = 1; break;
        case 6: result = 95; break;
        case 1: result = 238; break;
        case 14: result = 72; break;
        case 9: result = 55; break;
        case 11: result = 5; break;
        case 2: result = 136; break;
        case 17: result = 13; break;
        default: result = 159; break;
        }
        if (result != 1) failures++;
    }


    {
        uint8_t v = 176;
        v ^= 128;
        if (v != 48) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {65,73,31754,121};
        if (s.b != (uint8_t)73) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        if (((uint16_t)(59 - 90)) != 65505) failures++;
    }


    {
        uint8_t m[2][3] = {{120,93,190},{197,80,146}};
        if (m[0][2] != 190) failures++;
    }


    {
        uint16_t x = 41695;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 11371;
        volatile int16_t b = 22582;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 21;
        x <<= 0;
        if (x != 21) failures++;
    }


    {
        uint16_t r = 44378 + 21695 + 32766 + 34241 + 25295 + 9674 + 40961 + 44575;
        if (r != 56977) failures++;
    }


    {
        g16 = 60690;
        if (read_g16() != 60690) failures++;
    }


    {
        volatile int16_t a = 14912;
        volatile int16_t b = -32706;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 5853 + 56274 + 56728 + 1120 + 39832 + 36525 + 17189 + 15478;
        if (r != 32391) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 46;
        v ^= 1;
        if (v != 47) failures++;
    }


    {
        uint8_t src[12] = {58,249,12,149,178,17,122,36,112,144,169,173};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[3] != 149) failures++;
    }


    {
        uint16_t r = call6(60,77,243,85,241,171);
        if (r != 877) failures++;
    }


    {
        uint16_t r = 36327 + 63157 + 3038 + 5254 + 9878 + 57064 + 37967 + 20243;
        if (r != 36320) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)254) + (uint16_t)22167;
        if (r != 22421) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        uint8_t x = 244;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        uint16_t x = 124;
        x = x + 166;
        if (x != 290) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-89) / (int16_t)((int8_t)-81);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        int8_t a = -109;
        int8_t b = -4;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 235;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(177,198) != 65515) failures++;
    }


    {
        uint16_t x = 37;
        x = x + 178;
        if (x != 215) failures++;
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
        volatile uint8_t port = 137;
        uint8_t r = port;
        if (r != 137) failures++;
    }


    {
        uint16_t r = call6(103,112,0,243,10,2);
        if (r != 470) failures++;
    }


    {
        volatile uint8_t port = 3;
        uint8_t r = port;
        if (r != 3) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 147;
        if (buf[1] != 147) failures++;
    }


    {
        uint16_t x = 244;
        x = x + 225;
        if (x != 469) failures++;
    }


    {
        uint8_t v = 244;
        v ^= 8;
        if (v != 252) failures++;
    }


    {
        volatile int16_t a = 10946;
        volatile int16_t b = 23816;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 62;
        if (buf[6] != 62) failures++;
    }


    {
        uint16_t r = call6(232,84,95,196,211,248);
        if (r != 1066) failures++;
    }


    {
        volatile uint8_t port = 47;
        uint8_t r = port;
        if (r != 47) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 1) sum += j;
        if (sum != 1) failures++;
    }


    {
        volatile int16_t a = 25230;
        volatile int16_t b = -24454;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {233,45,125,191,150,75,21,251};
        uint8_t *p = buf;
        p += 5;
        if (*p != 75) failures++;
    }


    {
        uint8_t v = 63;
        int r = (v & 64) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 113;
        uint8_t r = port;
        if (r != 113) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)11) + (uint16_t)46393;
        if (r != 46404) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 3: result = 4; break;
        case 15: result = 68; break;
        case 9: result = 218; break;
        case 14: result = 153; break;
        case 1: result = 51; break;
        case 18: result = 68; break;
        case 2: result = 23; break;
        case 11: result = 245; break;
        default: result = 125; break;
        }
        if (result != 68) failures++;
    }


    {
        uint16_t r = 10864 + 14406 + 18250 + 52231 + 39081 + 49410 + 7295 + 46130;
        if (r != 41059) failures++;
    }


    {
        uint8_t m[2][4] = {{166,0,208,89},{254,82,250,102}};
        if (m[1][3] != 102) failures++;
    }


    {
        uint16_t x = 41671;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 77;
        int8_t b = -46;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)15) / (int16_t)((int8_t)-13);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t x = 19144;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)25) % (int16_t)((int8_t)-40);
        if ((uint16_t)r != (uint16_t)25) failures++;
    }


    {
        if (((uint16_t)(67 | ((221 ^ 178) & (39 + 84)))) != 107) failures++;
    }


    {
        int8_t a = 55;
        int8_t b = -67;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 226;
        uint8_t r = port;
        if (r != 226) failures++;
    }


    {
        uint16_t r = call6(193,82,173,239,11,223);
        if (r != 921) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(13,55) != 68) failures++;
    }


    {
        uint32_t a = 640122362UL;
        uint32_t b = 3877107333UL;
        uint32_t r = a ^ b;
        if (r != 3241184127UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(221,209) != 12) failures++;
    }


    {
        uint32_t a = 3236377516UL;
        uint32_t b = 2863403787UL;
        uint32_t r = a - b;
        if (r != 372973729UL) failures++;
    }


    {
        g16 = 24793;
        if (read_g16() != 24793) failures++;
    }


    {
        if (((uint16_t)((222 + (238 ^ 194)) - ((237 - 20) & 31))) != 241) failures++;
    }


    {
        uint8_t m[4][3] = {{70,90,255},{247,223,136},{27,77,203},{200,4,49}};
        if (m[1][1] != 223) failures++;
    }


    {
        uint8_t a[6] = {224,235,98,240,156,75};
        if (a[1] != 235) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 1: result = 129; break;
        case 15: result = 121; break;
        case 6: result = 64; break;
        case 19: result = 229; break;
        default: result = 220; break;
        }
        if (result != 64) failures++;
    }


    {
        uint8_t buf[8] = {167,23,11,117,214,49,129,194};
        uint8_t *p = buf;
        p += 5;
        if (*p != 49) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 129;
        if (buf[8] != 129) failures++;
    }


    {
        volatile int16_t a = 630;
        volatile int16_t b = -3105;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 456141750UL;
        uint32_t b = 1138741134UL;
        uint32_t r = a + b;
        if (r != 1594882884UL) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 11: result = 153; break;
        case 16: result = 109; break;
        case 12: result = 239; break;
        case 3: result = 171; break;
        case 15: result = 25; break;
        case 9: result = 108; break;
        default: result = 87; break;
        }
        if (result != 109) failures++;
    }


    {
        if (((uint16_t)(((183 - 59) - (199 - 245)) ^ 96)) != 202) failures++;
    }


    {
        uint8_t m[4][3] = {{6,211,129},{159,109,175},{253,198,141},{200,2,142}};
        if (m[0][0] != 6) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {67,182,21536,149};
        if (s.b != (uint8_t)182) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(27,235) != 262) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {246,154,53691,64};
        if (s.d != (uint8_t)64) failures++;
    }


    {
        uint16_t r = add2(41,228) + add2(228,195) + add2(41,195);
        if (r != 928) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 229;
        if (buf[11] != 229) failures++;
    }


    {
        uint8_t x = 81;
        x <<= 4;
        if (x != 16) failures++;
    }


    {
        uint8_t x = 104;
        x <<= 3;
        if (x != 64) failures++;
    }


    {
        g16 = 44847;
        if (read_g16() != 44847) failures++;
    }


    {
        uint16_t r = call6(172,49,4,67,72,200);
        if (r != 564) failures++;
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
        for (uint16_t j = 0; j < 16; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t a[6] = {241,76,99,167,236,129};
        if (a[5] != 129) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-11) % (int16_t)((int8_t)95);
        if ((uint16_t)r != (uint16_t)65525) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        int8_t a = 33;
        int8_t b = 57;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 20690;
        volatile int16_t b = -20247;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 11: result = 205; break;
        case 0: result = 27; break;
        case 7: result = 246; break;
        case 2: result = 132; break;
        case 12: result = 122; break;
        case 17: result = 98; break;
        default: result = 101; break;
        }
        if (result != 101) failures++;
    }


    {
        g16 = 54105;
        if (read_g16() != 54105) failures++;
    }


    {
        volatile uint8_t port = 82;
        uint8_t r = port;
        if (r != 82) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 7: result = 255; break;
        case 4: result = 37; break;
        case 14: result = 178; break;
        default: result = 225; break;
        }
        if (result != 255) failures++;
    }


    {
        int8_t a = -124;
        int8_t b = 108;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 240;
        x <<= 1;
        if (x != 224) failures++;
    }


    {
        uint16_t r = add2(123,17) + add2(17,6) + add2(123,6);
        if (r != 292) failures++;
    }


    {
        uint16_t r = 37815 + 10111 + 35031 + 24416 + 53770 + 26854 + 12794 + 14210;
        if (r != 18393) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 175;
        if (buf[9] != 175) failures++;
    }


    {
        uint8_t buf[8] = {151,83,65,7,134,126,236,160};
        uint8_t *p = buf;
        p += 4;
        if (*p != 134) failures++;
    }


    {
        uint8_t buf[8] = {170,25,220,4,232,129,193,219};
        uint8_t *p = buf;
        p += 3;
        if (*p != 4) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-119) / (int16_t)((int8_t)-74);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t r = 65013 + 33142 + 13484 + 13464 + 23043 + 10327 + 52764 + 50320;
        if (r != 64949) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {163,110,49889,83};
        if (s.c != (uint16_t)49889) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {254,40,52404,208};
        if (s.d != (uint8_t)208) failures++;
    }


    {
        if (((uint16_t)(((75 ^ 64) - (96 ^ 18)) - ((103 | 130) & (28 | 26)))) != 65427) failures++;
    }


    {
        volatile int16_t a = -30356;
        volatile int16_t b = 15750;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 89;
        int8_t b = -78;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[8] = {207,224,193,126,229,254,215,93};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 224) failures++;
    }


    {
        uint16_t x = 232;
        x = x + 151;
        if (x != 383) failures++;
    }


    {
        uint8_t v = 149;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        if (((uint16_t)130) != 130) failures++;
    }


    {
        uint8_t buf[8] = {27,72,68,15,29,187,212,182};
        uint8_t *p = buf;
        p += 1;
        if (*p != 72) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-92) % (int16_t)((int8_t)-111);
        if ((uint16_t)r != (uint16_t)65444) failures++;
    }


    {
        g16 = 36767;
        if (read_g16() != 36767) failures++;
    }


    {
        int8_t a = -83;
        int8_t b = 90;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 37873;
        if (read_g16() != 37873) failures++;
    }


    {
        uint8_t v = 216;
        v ^= 2;
        if (v != 218) failures++;
    }


    {
        uint8_t buf[8] = {49,129,55,83,116,205,150,128};
        uint8_t *p = buf;
        p += 3;
        if (*p != 83) failures++;
    }


    {
        uint16_t r = add2(166,222) + add2(222,124) + add2(166,124);
        if (r != 1024) failures++;
    }


    {
        uint32_t a = 2953145026UL;
        uint32_t b = 1777793354UL;
        uint32_t r = a & b;
        if (r != 537159746UL) failures++;
    }


    {
        uint8_t x = 169;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t buf[8] = {61,66,116,5,89,161,77,234};
        uint8_t *p = buf;
        p += 0;
        if (*p != 61) failures++;
    }


    {
        uint8_t v = 206;
        v &= ~(uint8_t)1;
        if (v != 206) failures++;
    }


    {
        uint8_t v = 146;
        v &= ~(uint8_t)32;
        if (v != 146) failures++;
    }


    {
        uint16_t x = 139;
        x = x + 241;
        if (x != 380) failures++;
    }


    {
        uint16_t r = 18654 + 9072 + 48513 + 64547 + 46795 + 1002 + 28753 + 61380;
        if (r != 16572) failures++;
    }


    {
        uint8_t m[4][4] = {{158,21,55,225},{103,45,181,237},{47,119,124,89},{101,106,11,172}};
        if (m[2][3] != 89) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 185;
        if (buf[12] != 185) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)246) + (uint16_t)14611;
        if (r != 14857) failures++;
    }


    {
        uint16_t x = 11770;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 47246;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 8: result = 212; break;
        case 19: result = 171; break;
        case 9: result = 89; break;
        case 11: result = 240; break;
        case 12: result = 224; break;
        case 4: result = 190; break;
        case 1: result = 160; break;
        case 17: result = 16; break;
        default: result = 190; break;
        }
        if (result != 190) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(94,229) != 65401) failures++;
    }


    {
        uint8_t buf[8] = {221,45,73,88,28,125,102,75};
        uint8_t *p = buf;
        p += 2;
        if (*p != 73) failures++;
    }


    {
        uint16_t r = call6(79,109,16,251,51,109);
        if (r != 615) failures++;
    }


    {
        uint16_t r = 227 + 43496 + 108 + 34141 + 61279 + 64700 + 13522 + 48478;
        if (r != 3807) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {45,72,5688,189};
        if (s.c != (uint16_t)5688) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)95) + (uint16_t)21900;
        if (r != 21995) failures++;
    }


    {
        g16 = 61342;
        if (read_g16() != 61342) failures++;
    }


    {
        uint8_t v = 40;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)121) / (int16_t)((int8_t)-2);
        if ((uint16_t)r != (uint16_t)65476) failures++;
    }


    {
        uint32_t a = 825928970UL;
        uint32_t b = 2203333974UL;
        uint32_t r = a ^ b;
        if (r != 2993592412UL) failures++;
    }


    {
        uint16_t r = add2(45,125) + add2(125,208) + add2(45,208);
        if (r != 756) failures++;
    }


    {
        uint32_t a = 563138499UL;
        uint32_t b = 938052483UL;
        uint32_t r = a - b;
        if (r != 3920053312UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {165,220,51600,80};
        if (s.a != (uint8_t)165) failures++;
    }


    {
        uint16_t r = call6(11,168,20,235,55,117);
        if (r != 606) failures++;
    }


    {
        uint16_t x = 28986;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 92;
        uint8_t r = port;
        if (r != 92) failures++;
    }


    {
        uint8_t src[8] = {122,44,0,73,147,244,54,67};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[6] != 54) failures++;
    }


    {
        int8_t a = 61;
        int8_t b = -39;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 83;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 45) failures++;
    }


    {
        uint8_t a[6] = {36,87,21,85,3,105};
        if (a[2] != 21) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 184;
        if (buf[8] != 184) failures++;
    }


    {
        uint8_t m[2][2] = {{147,16},{249,182}};
        if (m[0][0] != 147) failures++;
    }


    {
        g16 = 22433;
        if (read_g16() != 22433) failures++;
    }


    {
        uint32_t a = 1103812695UL;
        uint32_t b = 231191669UL;
        uint32_t r = a | b;
        if (r != 1305476215UL) failures++;
    }


    {
        volatile int16_t a = -16732;
        volatile int16_t b = -28592;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 201;
        x = x + 15;
        if (x != 216) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)172) + (uint16_t)34804;
        if (r != 34976) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t buf[8] = {137,89,192,6,86,252,234,222};
        uint8_t *p = buf;
        p += 3;
        if (*p != 6) failures++;
    }


    {
        uint16_t x = 186;
        x = x + 160;
        if (x != 346) failures++;
    }


    {
        uint8_t a[6] = {86,50,76,139,42,83};
        if (a[3] != 139) failures++;
    }


    {
        uint8_t x = 178;
        x <<= 3;
        if (x != 144) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 9: result = 120; break;
        case 12: result = 114; break;
        case 5: result = 182; break;
        case 0: result = 53; break;
        case 13: result = 153; break;
        default: result = 231; break;
        }
        if (result != 153) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {98,116,42670,231};
        if (s.a != (uint8_t)98) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 15: result = 76; break;
        case 7: result = 116; break;
        case 1: result = 164; break;
        case 19: result = 27; break;
        default: result = 31; break;
        }
        if (result != 116) failures++;
    }


    {
        int8_t a = 17;
        int8_t b = 74;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(((206 + 205) ^ 245) & ((162 ^ 178) + (104 ^ 241)))) != 40) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {238,215,58912,201};
        if (s.d != (uint8_t)201) failures++;
    }


    {
        uint16_t r = call6(231,118,179,0,134,230);
        if (r != 892) failures++;
    }


    {
        uint8_t a[6] = {110,200,202,231,129,252};
        if (a[4] != 129) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t v = 80;
        v &= ~(uint8_t)128;
        if (v != 80) failures++;
    }


    {
        uint16_t r = add2(202,21) + add2(21,191) + add2(202,191);
        if (r != 828) failures++;
    }


    {
        uint8_t a[6] = {95,49,125,227,181,79};
        if (a[4] != 181) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 4: result = 178; break;
        case 10: result = 156; break;
        case 15: result = 53; break;
        case 14: result = 47; break;
        case 0: result = 96; break;
        case 18: result = 38; break;
        case 8: result = 170; break;
        default: result = 88; break;
        }
        if (result != 38) failures++;
    }


    {
        uint8_t v = 82;
        v &= ~(uint8_t)4;
        if (v != 82) failures++;
    }


    {
        uint8_t x = 104;
        x <<= 0;
        if (x != 104) failures++;
    }


    {
        uint8_t m[3][2] = {{9,225},{131,109},{199,23}};
        if (m[2][1] != 23) failures++;
    }


    {
        g16 = 11587;
        if (read_g16() != 11587) failures++;
    }


    {
        uint16_t x = 61844;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)21) / (int16_t)((int8_t)111);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {91,103,62025,191};
        if (s.d != (uint8_t)191) failures++;
    }


    {
        uint8_t m[2][2] = {{162,96},{213,118}};
        if (m[0][1] != 96) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint16_t x = 29497;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 12: result = 119; break;
        case 13: result = 217; break;
        case 7: result = 175; break;
        case 3: result = 95; break;
        case 4: result = 28; break;
        case 14: result = 64; break;
        default: result = 143; break;
        }
        if (result != 217) failures++;
    }


    {
        uint16_t x = 234;
        x = x + 180;
        if (x != 414) failures++;
    }


    {
        uint16_t x = 37188;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 4849;
        if (read_g16() != 4849) failures++;
    }


    {
        uint8_t v = 147;
        v |= 32;
        if (v != 179) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 3: result = 184; break;
        case 16: result = 37; break;
        case 17: result = 247; break;
        case 0: result = 221; break;
        case 6: result = 120; break;
        case 5: result = 126; break;
        case 15: result = 210; break;
        case 1: result = 167; break;
        default: result = 39; break;
        }
        if (result != 221) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 7: result = 126; break;
        case 6: result = 177; break;
        case 19: result = 250; break;
        case 14: result = 64; break;
        case 13: result = 3; break;
        default: result = 90; break;
        }
        if (result != 64) failures++;
    }


    {
        uint8_t src[5] = {156,65,240,117,123};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[4] != 123) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 170;
        if (buf[15] != 170) failures++;
    }


    {
        uint16_t r = call6(176,22,224,143,75,146);
        if (r != 786) failures++;
    }


    {
        uint16_t r = add2(29,141) + add2(141,86) + add2(29,86);
        if (r != 512) failures++;
    }


    {
        uint8_t src[1] = {68};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 68) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 2) sum += j;
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
        uint8_t m[2][3] = {{39,211,83},{188,19,128}};
        if (m[1][0] != 188) failures++;
    }


    {
        uint8_t src[2] = {67,214};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 67) failures++;
    }


    {
        int8_t a = 92;
        int8_t b = -4;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)((184 | 238) + (3 ^ (14 - 78)))) != 193) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(181,101) != 282) failures++;
    }


    {
        uint16_t r = call6(226,221,150,1,118,204);
        if (r != 920) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        g16 = 12257;
        if (read_g16() != 12257) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {186,221,1030,117};
        if (s.d != (uint8_t)117) failures++;
    }


    {
        uint8_t a[6] = {61,22,217,246,213,129};
        if (a[0] != 61) failures++;
    }


    {
        uint8_t v = 112;
        int r = (v & 32) ? 1 : 0;
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
        uint8_t buf[8] = {190,188,99,191,34,123,27,188};
        uint8_t *p = buf;
        p += 7;
        if (*p != 188) failures++;
    }


    {
        uint16_t r = add2(36,200) + add2(200,12) + add2(36,12);
        if (r != 496) failures++;
    }


    {
        g16 = 33818;
        if (read_g16() != 33818) failures++;
    }


    {
        uint8_t v = 100;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 28) failures++;
    }


    {
        int8_t a = 105;
        int8_t b = 49;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 16;
        uint8_t r = port;
        if (r != 16) failures++;
    }


    {
        uint8_t v = 234;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 169;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)29) + (uint16_t)53310;
        if (r != 53339) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)201) + (uint16_t)14301;
        if (r != 14502) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 11: result = 89; break;
        case 4: result = 224; break;
        case 17: result = 240; break;
        default: result = 153; break;
        }
        if (result != 224) failures++;
    }


    {
        uint8_t src[5] = {231,250,137,234,69};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 231) failures++;
    }

    return failures;
}