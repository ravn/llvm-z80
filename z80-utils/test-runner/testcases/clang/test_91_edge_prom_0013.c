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
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 151;
        if (buf[1] != 151) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 14: result = 251; break;
        case 15: result = 148; break;
        case 13: result = 61; break;
        default: result = 84; break;
        }
        if (result != 61) failures++;
    }


    {
        g16 = 51385;
        if (read_g16() != 51385) failures++;
    }


    {
        int8_t a = -91;
        int8_t b = 127;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[12] = {35,191,178,130,254,162,242,23,16,117,30,55};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[9] != 117) failures++;
    }


    {
        uint8_t src[4] = {39,108,0,113};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 39) failures++;
    }


    {
        uint8_t m[3][4] = {{186,95,89,86},{145,33,146,233},{217,159,223,121}};
        if (m[1][2] != 146) failures++;
    }


    {
        uint8_t m[2][3] = {{101,107,16},{234,252,176}};
        if (m[0][2] != 16) failures++;
    }


    {
        g16 = 54523;
        if (read_g16() != 54523) failures++;
    }


    {
        uint16_t r = 47048 + 57030 + 10093 + 29114 + 43898 + 51454 + 53443 + 10513;
        if (r != 40449) failures++;
    }


    {
        uint16_t r = call6(100,89,97,101,228,143);
        if (r != 758) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-69) / (int16_t)((int8_t)-61);
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
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 149;
        if (buf[1] != 149) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t a[6] = {118,238,100,89,54,41};
        if (a[1] != 238) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)242) + (uint16_t)14672;
        if (r != 14914) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 908457523UL;
        uint32_t b = 2802053028UL;
        uint32_t r = a & b;
        if (r != 637658656UL) failures++;
    }


    {
        int8_t a = 47;
        int8_t b = -26;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 10: result = 185; break;
        case 6: result = 184; break;
        case 8: result = 164; break;
        case 16: result = 190; break;
        default: result = 22; break;
        }
        if (result != 185) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 44;
        if (buf[4] != 44) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 12: result = 76; break;
        case 2: result = 89; break;
        case 14: result = 146; break;
        case 5: result = 112; break;
        case 4: result = 170; break;
        case 10: result = 239; break;
        case 17: result = 133; break;
        case 7: result = 2; break;
        default: result = 133; break;
        }
        if (result != 112) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {163,184,12428,196};
        if (s.b != (uint8_t)184) failures++;
    }


    {
        uint32_t a = 2929907509UL;
        uint32_t b = 57730367UL;
        uint32_t r = a + b;
        if (r != 2987637876UL) failures++;
    }


    {
        volatile int16_t a = 3881;
        volatile int16_t b = -22338;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        g16 = 48986;
        if (read_g16() != 48986) failures++;
    }


    {
        uint8_t src[16] = {0,54,164,138,190,172,152,111,8,187,185,201,68,17,138,195};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[7] != 111) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 167;
        if (buf[4] != 167) failures++;
    }


    {
        uint8_t x = 64;
        x <<= 0;
        if (x != 64) failures++;
    }


    {
        volatile uint8_t port = 81;
        uint8_t r = port;
        if (r != 81) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t v = 87;
        v &= ~(uint8_t)4;
        if (v != 83) failures++;
    }


    {
        uint8_t a[6] = {89,150,220,181,118,208};
        if (a[1] != 150) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)69) + (uint16_t)33119;
        if (r != 33188) failures++;
    }


    {
        uint16_t x = 36027;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][3] = {{207,12,87},{144,220,10},{9,229,24}};
        if (m[1][2] != 10) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 249;
        uint8_t r = port;
        if (r != 249) failures++;
    }


    {
        uint32_t a = 1430165459UL;
        uint32_t b = 429968960UL;
        uint32_t r = a ^ b;
        if (r != 1285446035UL) failures++;
    }


    {
        uint16_t r = add2(184,131) + add2(131,88) + add2(184,88);
        if (r != 806) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 1: result = 176; break;
        case 9: result = 18; break;
        case 10: result = 169; break;
        case 2: result = 185; break;
        case 12: result = 143; break;
        case 18: result = 120; break;
        case 17: result = 206; break;
        case 16: result = 126; break;
        default: result = 160; break;
        }
        if (result != 143) failures++;
    }


    {
        uint16_t r = call6(82,20,178,252,54,191);
        if (r != 777) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 93;
        if (buf[2] != 93) failures++;
    }


    {
        uint32_t a = 2119636511UL;
        uint32_t b = 3942015778UL;
        uint32_t r = a + b;
        if (r != 1766684993UL) failures++;
    }


    {
        uint8_t x = 170;
        x <<= 3;
        if (x != 80) failures++;
    }


    {
        uint8_t buf[8] = {160,217,148,145,112,41,2,130};
        uint8_t *p = buf;
        p += 3;
        if (*p != 145) failures++;
    }


    {
        int8_t a = -46;
        int8_t b = -34;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 2080152269UL;
        uint32_t b = 2851978527UL;
        uint32_t r = a - b;
        if (r != 3523141038UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-18) % (int16_t)((int8_t)27);
        if ((uint16_t)r != (uint16_t)65518) failures++;
    }


    {
        uint16_t x = 133;
        x = x + 216;
        if (x != 349) failures++;
    }


    {
        int8_t a = 20;
        int8_t b = -86;
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
        g16 = 19446;
        if (read_g16() != 19446) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 10: result = 200; break;
        case 12: result = 218; break;
        case 3: result = 119; break;
        default: result = 46; break;
        }
        if (result != 218) failures++;
    }


    {
        volatile uint8_t port = 94;
        uint8_t r = port;
        if (r != 94) failures++;
    }


    {
        if (((uint16_t)(30 - ((247 ^ 205) | 245))) != 65311) failures++;
    }


    {
        uint32_t a = 1876665971UL;
        uint32_t b = 1007806790UL;
        uint32_t r = a & b;
        if (r != 739352642UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {31,37,53711,193};
        if (s.c != (uint16_t)53711) failures++;
    }


    {
        uint8_t m[3][4] = {{222,211,24,200},{73,177,98,247},{221,109,183,198}};
        if (m[1][0] != 73) failures++;
    }


    {
        uint32_t a = 485504233UL;
        uint32_t b = 301822732UL;
        uint32_t r = a ^ b;
        if (r != 218974181UL) failures++;
    }


    {
        uint8_t v = 65;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint32_t a = 269252312UL;
        uint32_t b = 331245108UL;
        uint32_t r = a | b;
        if (r != 331249404UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)33) + (uint16_t)12717;
        if (r != 12750) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)81) % (int16_t)((int8_t)-23);
        if ((uint16_t)r != (uint16_t)12) failures++;
    }


    {
        uint8_t v = 15;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        int8_t a = -116;
        int8_t b = -108;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 164;
        if (buf[6] != 164) failures++;
    }


    {
        uint32_t a = 3585391435UL;
        uint32_t b = 1417750997UL;
        uint32_t r = a | b;
        if (r != 3585457119UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {240,48,25863,88};
        if (s.c != (uint16_t)25863) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {113,238,34420,111};
        if (s.a != (uint8_t)113) failures++;
    }


    {
        if (((uint16_t)(35 & ((231 | 2) + (54 + 179)))) != 0) failures++;
    }


    {
        uint16_t r = call6(47,10,82,51,57,19);
        if (r != 266) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        volatile int16_t a = -5214;
        volatile int16_t b = 28052;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 236;
        uint8_t r = port;
        if (r != 236) failures++;
    }


    {
        volatile int16_t a = -29720;
        volatile int16_t b = 21562;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(129,9) != 120) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(154,3) != 151) failures++;
    }


    {
        uint8_t buf[8] = {87,226,52,77,128,158,4,73};
        uint8_t *p = buf;
        p += 3;
        if (*p != 77) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {39,204,50297,169};
        if (s.b != (uint8_t)204) failures++;
    }


    {
        uint8_t a[6] = {215,243,76,40,53,22};
        if (a[0] != 215) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(80,225,33,10,22,12);
        if (r != 382) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-74) % (int16_t)((int8_t)-83);
        if ((uint16_t)r != (uint16_t)65462) failures++;
    }


    {
        volatile uint8_t port = 72;
        uint8_t r = port;
        if (r != 72) failures++;
    }


    {
        uint16_t x = 13180;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 43385;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = 30037 + 51959 + 23356 + 56121 + 3169 + 65120 + 63382 + 34188;
        if (r != 65188) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 7: result = 194; break;
        case 9: result = 218; break;
        case 2: result = 81; break;
        default: result = 213; break;
        }
        if (result != 213) failures++;
    }


    {
        int8_t a = -1;
        int8_t b = 77;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 27985;
        volatile int16_t b = -20054;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 24;
        x = x + 182;
        if (x != 206) failures++;
    }


    {
        volatile int16_t a = 10829;
        volatile int16_t b = -29250;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-36) % (int16_t)((int8_t)-114);
        if ((uint16_t)r != (uint16_t)65500) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)127) % (int16_t)((int8_t)-59);
        if ((uint16_t)r != (uint16_t)9) failures++;
    }


    {
        uint8_t buf[8] = {5,148,8,112,10,239,9,29};
        uint8_t *p = buf;
        p += 7;
        if (*p != 29) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(249,211) != 38) failures++;
    }


    {
        uint8_t src[5] = {114,252,72,205,126};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 72) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)101) + (uint16_t)5558;
        if (r != 5659) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)25) + (uint16_t)56440;
        if (r != 56465) failures++;
    }


    {
        uint16_t r = add2(122,34) + add2(34,178) + add2(122,178);
        if (r != 668) failures++;
    }


    {
        uint16_t r = add2(244,227) + add2(227,106) + add2(244,106);
        if (r != 1154) failures++;
    }


    {
        uint8_t buf[8] = {124,74,72,230,79,77,41,123};
        uint8_t *p = buf;
        p += 1;
        if (*p != 74) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        volatile int16_t a = 28688;
        volatile int16_t b = -6326;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 10501;
        volatile int16_t b = 13045;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint16_t r = 60292 + 22651 + 40621 + 56200 + 4324 + 28765 + 44734 + 54834;
        if (r != 50277) failures++;
    }


    {
        uint16_t r = call6(51,239,158,212,150,17);
        if (r != 827) failures++;
    }


    {
        uint16_t r = add2(161,196) + add2(196,153) + add2(161,153);
        if (r != 1020) failures++;
    }


    {
        volatile uint8_t port = 125;
        uint8_t r = port;
        if (r != 125) failures++;
    }


    {
        g16 = 44353;
        if (read_g16() != 44353) failures++;
    }


    {
        uint16_t x = 58;
        x = x + 253;
        if (x != 311) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 159;
        if (buf[0] != 159) failures++;
    }


    {
        uint8_t buf[8] = {164,227,108,2,127,68,125,142};
        uint8_t *p = buf;
        p += 2;
        if (*p != 108) failures++;
    }


    {
        uint8_t src[7] = {241,126,210,150,26,49,231};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[6] != 231) failures++;
    }


    {
        uint16_t x = 23;
        x = x + 255;
        if (x != 278) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 2: result = 94; break;
        case 5: result = 232; break;
        case 3: result = 89; break;
        case 15: result = 2; break;
        case 6: result = 0; break;
        case 1: result = 130; break;
        case 0: result = 213; break;
        default: result = 160; break;
        }
        if (result != 232) failures++;
    }


    {
        int8_t a = 103;
        int8_t b = -5;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)36) + (uint16_t)1133;
        if (r != 1169) failures++;
    }


    {
        uint8_t v = 19;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t m[4][3] = {{226,94,213},{29,205,12},{138,98,156},{11,174,52}};
        if (m[3][2] != 52) failures++;
    }


    {
        uint8_t m[2][4] = {{44,182,198,71},{120,63,40,104}};
        if (m[0][2] != 198) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {227,2,48300,81};
        if (s.a != (uint8_t)227) failures++;
    }


    {
        uint16_t x = 9568;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(163,73,159,104,19,68);
        if (r != 586) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)158) + (uint16_t)40130;
        if (r != 40288) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = 13975;
        volatile int16_t b = -30748;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 59;
        v |= 1;
        if (v != 59) failures++;
    }


    {
        uint16_t x = 30052;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -15789;
        volatile int16_t b = -9946;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 15;
        int8_t b = -88;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2540288734UL;
        uint32_t b = 55657141UL;
        uint32_t r = a & b;
        if (r != 54592148UL) failures++;
    }


    {
        uint8_t x = 117;
        x <<= 1;
        if (x != 234) failures++;
    }


    {
        uint16_t r = 46006 + 58258 + 11326 + 44977 + 60942 + 44614 + 15964 + 9372;
        if (r != 29315) failures++;
    }


    {
        uint16_t r = call6(245,52,12,211,146,148);
        if (r != 814) failures++;
    }


    {
        uint8_t v = 39;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        int8_t a = 56;
        int8_t b = -72;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {15,17,49,228,5,215,129,144};
        uint8_t *p = buf;
        p += 1;
        if (*p != 17) failures++;
    }


    {
        uint32_t a = 167969531UL;
        uint32_t b = 426505458UL;
        uint32_t r = a + b;
        if (r != 594474989UL) failures++;
    }


    {
        uint32_t a = 831749003UL;
        uint32_t b = 2437702830UL;
        uint32_t r = a + b;
        if (r != 3269451833UL) failures++;
    }


    {
        uint8_t x = 103;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)171) + (uint16_t)39743;
        if (r != 39914) failures++;
    }


    {
        uint8_t a[6] = {95,137,80,178,29,245};
        if (a[5] != 245) failures++;
    }


    {
        uint16_t r = add2(187,134) + add2(134,222) + add2(187,222);
        if (r != 1086) failures++;
    }


    {
        uint8_t src[13] = {170,160,54,55,226,176,218,17,131,11,74,234,219};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[7] != 17) failures++;
    }


    {
        uint8_t v = 215;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint32_t a = 2471126553UL;
        uint32_t b = 1899608889UL;
        uint32_t r = a & b;
        if (r != 285745689UL) failures++;
    }


    {
        uint8_t buf[8] = {94,4,104,31,201,101,168,154};
        uint8_t *p = buf;
        p += 5;
        if (*p != 101) failures++;
    }


    {
        int8_t a = -113;
        int8_t b = -11;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 24095;
        if (read_g16() != 24095) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-103) / (int16_t)((int8_t)-77);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-72) / (int16_t)((int8_t)-98);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 140;
        uint8_t r = port;
        if (r != 140) failures++;
    }


    {
        int8_t a = 110;
        int8_t b = -5;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 36;
        uint8_t r = port;
        if (r != 36) failures++;
    }


    {
        uint32_t a = 3779083763UL;
        uint32_t b = 3155751540UL;
        uint32_t r = a ^ b;
        if (r != 1566101383UL) failures++;
    }


    {
        uint8_t x = 189;
        x <<= 0;
        if (x != 189) failures++;
    }


    {
        uint8_t v = 74;
        v ^= 32;
        if (v != 106) failures++;
    }


    {
        uint16_t r = call6(1,112,10,219,203,10);
        if (r != 555) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 2) sum += j;
        if (sum != 42) failures++;
    }


    {
        uint8_t buf[8] = {192,204,151,89,87,91,107,202};
        uint8_t *p = buf;
        p += 3;
        if (*p != 89) failures++;
    }


    {
        g16 = 35189;
        if (read_g16() != 35189) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(((212 | 3) ^ (214 + 79)) - (133 ^ (181 & 232)))) != 461) failures++;
    }


    {
        uint16_t x = 75;
        x = x + 248;
        if (x != 323) failures++;
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
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(99,191) != 65444) failures++;
    }


    {
        uint8_t v = 95;
        v &= ~(uint8_t)128;
        if (v != 95) failures++;
    }


    {
        uint16_t r = add2(182,168) + add2(168,47) + add2(182,47);
        if (r != 794) failures++;
    }


    {
        uint32_t a = 2533807115UL;
        uint32_t b = 1363971949UL;
        uint32_t r = a + b;
        if (r != 3897779064UL) failures++;
    }


    {
        int8_t a = -126;
        int8_t b = 77;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(19,27,13,11,218,148);
        if (r != 436) failures++;
    }


    {
        uint16_t r = call6(67,147,6,230,92,95);
        if (r != 637) failures++;
    }


    {
        uint8_t a[6] = {189,59,113,70,157,182};
        if (a[0] != 189) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 13: result = 116; break;
        case 12: result = 5; break;
        case 17: result = 42; break;
        case 14: result = 133; break;
        default: result = 73; break;
        }
        if (result != 133) failures++;
    }


    {
        int8_t a = -10;
        int8_t b = 79;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(152,176) + add2(176,98) + add2(152,98);
        if (r != 852) failures++;
    }


    {
        uint8_t v = 179;
        v |= 16;
        if (v != 179) failures++;
    }


    {
        volatile int16_t a = -26495;
        volatile int16_t b = -2188;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        g16 = 40836;
        if (read_g16() != 40836) failures++;
    }


    {
        uint32_t a = 2983499272UL;
        uint32_t b = 3825810392UL;
        uint32_t r = a + b;
        if (r != 2514342368UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)81) + (uint16_t)32283;
        if (r != 32364) failures++;
    }


    {
        volatile int16_t a = -22962;
        volatile int16_t b = 13614;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint32_t a = 498522327UL;
        uint32_t b = 3051534041UL;
        uint32_t r = a - b;
        if (r != 1741955582UL) failures++;
    }


    {
        uint8_t a[6] = {200,75,219,192,98,103};
        if (a[3] != 192) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(61,247) != 65350) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)121) % (int16_t)((int8_t)25);
        if ((uint16_t)r != (uint16_t)21) failures++;
    }


    {
        int8_t a = 23;
        int8_t b = 5;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 6;
        x <<= 2;
        if (x != 24) failures++;
    }


    {
        uint8_t x = 33;
        x <<= 3;
        if (x != 8) failures++;
    }


    {
        uint8_t x = 161;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint16_t x = 12605;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 9889 + 29466 + 17552 + 12551 + 5072 + 51498 + 952 + 6629;
        if (r != 2537) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(248,10) != 258) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {128,69,27939,245};
        if (s.c != (uint16_t)27939) failures++;
    }


    {
        uint8_t a[6] = {63,117,135,55,91,209};
        if (a[2] != 135) failures++;
    }


    {
        uint8_t v = 173;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 19) failures++;
    }


    {
        uint8_t v = 255;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        volatile int16_t a = 30853;
        volatile int16_t b = -4816;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)58) + (uint16_t)46801;
        if (r != 46859) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 78;
        if (buf[12] != 78) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-84) % (int16_t)((int8_t)120);
        if ((uint16_t)r != (uint16_t)65452) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 5: result = 135; break;
        case 15: result = 151; break;
        case 17: result = 20; break;
        case 13: result = 243; break;
        case 4: result = 106; break;
        default: result = 32; break;
        }
        if (result != 106) failures++;
    }


    {
        uint8_t a[6] = {16,83,156,106,195,133};
        if (a[3] != 106) failures++;
    }


    {
        uint16_t r = add2(92,149) + add2(149,210) + add2(92,210);
        if (r != 902) failures++;
    }


    {
        uint16_t r = add2(217,99) + add2(99,153) + add2(217,153);
        if (r != 938) failures++;
    }


    {
        uint16_t r = add2(174,160) + add2(160,132) + add2(174,132);
        if (r != 932) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 354390225UL;
        uint32_t b = 2085817283UL;
        uint32_t r = a & b;
        if (r != 336789697UL) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 11: result = 164; break;
        case 0: result = 222; break;
        case 15: result = 159; break;
        case 6: result = 199; break;
        case 10: result = 17; break;
        case 2: result = 147; break;
        default: result = 192; break;
        }
        if (result != 222) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        volatile int16_t a = -15215;
        volatile int16_t b = 25360;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 1912527074UL;
        uint32_t b = 1757275868UL;
        uint32_t r = a + b;
        if (r != 3669802942UL) failures++;
    }


    {
        uint16_t x = 52022;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {212,53,61117,252};
        if (s.c != (uint16_t)61117) failures++;
    }


    {
        uint8_t x = 25;
        x <<= 4;
        if (x != 144) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {160,122,41996,12};
        if (s.d != (uint8_t)12) failures++;
    }


    {
        uint16_t r = add2(139,193) + add2(193,79) + add2(139,79);
        if (r != 822) failures++;
    }


    {
        uint16_t r = 58690 + 44946 + 41214 + 15217 + 8580 + 46189 + 52046 + 50553;
        if (r != 55291) failures++;
    }


    {
        uint8_t a[6] = {237,4,91,206,132,54};
        if (a[5] != 54) failures++;
    }


    {
        uint16_t r = call6(2,2,104,51,46,144);
        if (r != 349) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {10,159,49203,94};
        if (s.d != (uint8_t)94) failures++;
    }


    {
        volatile uint8_t port = 206;
        uint8_t r = port;
        if (r != 206) failures++;
    }


    {
        uint8_t buf[8] = {12,254,189,30,166,247,189,74};
        uint8_t *p = buf;
        p += 1;
        if (*p != 254) failures++;
    }


    {
        uint16_t r = add2(122,32) + add2(32,219) + add2(122,219);
        if (r != 746) failures++;
    }


    {
        volatile int16_t a = 6695;
        volatile int16_t b = 24583;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 171;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint32_t a = 2315119288UL;
        uint32_t b = 2427233428UL;
        uint32_t r = a & b;
        if (r != 2158796944UL) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 14: result = 175; break;
        case 3: result = 95; break;
        case 7: result = 18; break;
        case 19: result = 237; break;
        case 15: result = 251; break;
        default: result = 32; break;
        }
        if (result != 18) failures++;
    }


    {
        uint16_t r = 47429 + 42618 + 12613 + 63303 + 24266 + 9939 + 29708 + 19112;
        if (r != 52380) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 13: result = 182; break;
        case 14: result = 140; break;
        case 18: result = 67; break;
        case 6: result = 237; break;
        case 1: result = 44; break;
        default: result = 21; break;
        }
        if (result != 140) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(28,244) != 272) failures++;
    }


    {
        uint8_t src[11] = {13,101,133,146,113,142,72,60,192,201,158};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[0] != 13) failures++;
    }


    {
        uint16_t r = 35760 + 36353 + 27432 + 49284 + 1061 + 16729 + 2656 + 18954;
        if (r != 57157) failures++;
    }


    {
        volatile int16_t a = -12924;
        volatile int16_t b = -2760;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[5] = {253,183,34,59,162};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[1] != 183) failures++;
    }


    {
        uint16_t r = call6(56,97,178,241,185,69);
        if (r != 826) failures++;
    }


    {
        if (((uint16_t)(169 ^ (152 + (200 ^ 71)))) != 398) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 15: result = 69; break;
        case 12: result = 162; break;
        case 4: result = 79; break;
        default: result = 167; break;
        }
        if (result != 69) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(181,215) != 65502) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)133) + (uint16_t)4516;
        if (r != 4649) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-126) / (int16_t)((int8_t)-14);
        if ((uint16_t)r != (uint16_t)9) failures++;
    }


    {
        if (((uint16_t)(((13 & 47) | (70 - 139)) & ((47 ^ 236) | (63 | 107)))) != 191) failures++;
    }


    {
        uint16_t r = call6(150,157,224,126,199,33);
        if (r != 889) failures++;
    }


    {
        uint8_t a[6] = {179,250,50,145,235,109};
        if (a[4] != 235) failures++;
    }


    {
        uint16_t x = 105;
        x = x + 97;
        if (x != 202) failures++;
    }


    {
        if (((uint16_t)104) != 104) failures++;
    }


    {
        uint8_t a[6] = {111,179,166,192,94,64};
        if (a[3] != 192) failures++;
    }


    {
        if (((uint16_t)((136 & (17 ^ 166)) | ((144 | 208) ^ (90 - 88)))) != 210) failures++;
    }


    {
        uint32_t a = 3272384352UL;
        uint32_t b = 1922040013UL;
        uint32_t r = a | b;
        if (r != 4087128045UL) failures++;
    }


    {
        if (((uint16_t)(((165 - 186) - (220 + 127)) & ((33 - 225) & (164 - 139)))) != 0) failures++;
    }


    {
        if (((uint16_t)(((160 ^ 253) ^ 29) - ((208 & 3) | (53 | 205)))) != 65347) failures++;
    }


    {
        uint16_t r = add2(166,198) + add2(198,104) + add2(166,104);
        if (r != 936) failures++;
    }


    {
        uint8_t buf[8] = {242,94,133,24,142,183,23,238};
        uint8_t *p = buf;
        p += 5;
        if (*p != 183) failures++;
    }


    {
        if (((uint16_t)(((162 | 216) & (97 ^ 253)) ^ 206)) != 86) failures++;
    }


    {
        int8_t a = -117;
        int8_t b = 49;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 18343;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {245,136,39256,244};
        if (s.b != (uint8_t)136) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)54) + (uint16_t)59863;
        if (r != 59917) failures++;
    }


    {
        uint32_t a = 715663866UL;
        uint32_t b = 935880525UL;
        uint32_t r = a ^ b;
        if (r != 492850871UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(203,122) != 81) failures++;
    }


    {
        uint16_t r = call6(41,190,110,211,73,168);
        if (r != 793) failures++;
    }


    {
        uint8_t buf[8] = {36,63,45,141,129,44,212,72};
        uint8_t *p = buf;
        p += 7;
        if (*p != 72) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)100) / (int16_t)((int8_t)-71);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t x = 18161;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 28246;
        volatile int16_t b = -15553;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)224) + (uint16_t)61778;
        if (r != 62002) failures++;
    }


    {
        uint16_t x = 181;
        x = x + 186;
        if (x != 367) failures++;
    }


    {
        uint16_t r = 9179 + 29257 + 58449 + 9368 + 58164 + 39672 + 47762 + 49933;
        if (r != 39640) failures++;
    }


    {
        uint16_t r = call6(120,28,219,14,67,146);
        if (r != 594) failures++;
    }


    {
        volatile int16_t a = 26705;
        volatile int16_t b = -20551;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {232,7,159,110,149,74};
        if (a[4] != 149) failures++;
    }


    {
        uint8_t buf[8] = {153,109,237,35,144,51,48,132};
        uint8_t *p = buf;
        p += 6;
        if (*p != 48) failures++;
    }


    {
        uint8_t v = 144;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 86;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[3][3] = {{211,188,3},{71,234,181},{235,8,24}};
        if (m[1][1] != 234) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)120) + (uint16_t)34132;
        if (r != 34252) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {173,60,31065,251};
        if (s.a != (uint8_t)173) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        volatile uint8_t port = 199;
        uint8_t r = port;
        if (r != 199) failures++;
    }


    {
        uint16_t r = 20952 + 5408 + 47505 + 11144 + 20308 + 53935 + 32239 + 39488;
        if (r != 34371) failures++;
    }


    {
        g16 = 36495;
        if (read_g16() != 36495) failures++;
    }


    {
        uint16_t r = 62034 + 51915 + 52345 + 39194 + 6182 + 10917 + 24762 + 55145;
        if (r != 40350) failures++;
    }


    {
        uint16_t r = call6(113,96,156,144,253,253);
        if (r != 1015) failures++;
    }


    {
        if (((uint16_t)(((152 + 129) ^ (65 ^ 3)) - ((205 - 229) & (28 | 253)))) != 115) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        g16 = 29093;
        if (read_g16() != 29093) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 19: result = 90; break;
        case 5: result = 183; break;
        case 2: result = 191; break;
        case 11: result = 229; break;
        case 18: result = 167; break;
        case 13: result = 8; break;
        case 1: result = 85; break;
        default: result = 202; break;
        }
        if (result != 183) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        if (((uint16_t)(172 - 132)) != 40) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 1) sum += j;
        if (sum != 28) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[9] = {193,29,222,237,139,252,234,193,146};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 222) failures++;
    }


    {
        uint32_t a = 4232993488UL;
        uint32_t b = 2477412325UL;
        uint32_t r = a ^ b;
        if (r != 1877218613UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)171) + (uint16_t)1681;
        if (r != 1852) failures++;
    }


    {
        uint16_t r = 61318 + 47209 + 4191 + 27767 + 42104 + 64240 + 36957 + 47572;
        if (r != 3678) failures++;
    }


    {
        uint16_t r = 15901 + 18530 + 2146 + 14102 + 905 + 19147 + 1522 + 53237;
        if (r != 59954) failures++;
    }


    {
        if (((uint16_t)((45 & 85) ^ 111)) != 106) failures++;
    }


    {
        if (((uint16_t)94) != 94) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)124) / (int16_t)((int8_t)-51);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 5: result = 76; break;
        case 19: result = 235; break;
        case 0: result = 90; break;
        case 6: result = 46; break;
        case 16: result = 182; break;
        default: result = 224; break;
        }
        if (result != 76) failures++;
    }


    {
        uint8_t a[6] = {187,80,164,69,97,69};
        if (a[5] != 69) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = call6(98,84,204,86,31,247);
        if (r != 750) failures++;
    }


    {
        uint32_t a = 2919499649UL;
        uint32_t b = 4045805241UL;
        uint32_t r = a & b;
        if (r != 2684618369UL) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 1;
        if (buf[6] != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-67) / (int16_t)((int8_t)-106);
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
        volatile int16_t a = 21300;
        volatile int16_t b = -660;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 64052 + 28896 + 35794 + 36906 + 5767 + 35286 + 54575 + 13141;
        if (r != 12273) failures++;
    }


    {
        uint8_t src[9] = {73,250,0,211,174,223,98,69,199};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[8] != 199) failures++;
    }


    {
        uint16_t r = 34735 + 47414 + 29229 + 236 + 19166 + 29939 + 47333 + 59119;
        if (r != 5027) failures++;
    }


    {
        uint8_t m[3][4] = {{176,229,233,79},{81,231,143,224},{218,105,35,50}};
        if (m[1][2] != 143) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)13) + (uint16_t)26240;
        if (r != 26253) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        volatile int16_t a = -5088;
        volatile int16_t b = 15959;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {59,96,26301,237};
        if (s.c != (uint16_t)26301) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 11: result = 244; break;
        case 6: result = 4; break;
        case 4: result = 255; break;
        case 7: result = 228; break;
        default: result = 212; break;
        }
        if (result != 228) failures++;
    }


    {
        uint8_t v = 74;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        uint8_t m[2][4] = {{99,183,10,57},{129,216,230,222}};
        if (m[1][2] != 230) failures++;
    }


    {
        int8_t a = 3;
        int8_t b = -115;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 122;
        int8_t b = -53;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 59558;
        if (read_g16() != 59558) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(79,179) != 65436) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 117;
        if (buf[10] != 117) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 16: result = 35; break;
        case 7: result = 68; break;
        case 3: result = 232; break;
        case 8: result = 32; break;
        default: result = 6; break;
        }
        if (result != 6) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)141) + (uint16_t)59015;
        if (r != 59156) failures++;
    }


    {
        uint8_t v = 184;
        v ^= 8;
        if (v != 176) failures++;
    }


    {
        uint8_t x = 129;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 5: result = 48; break;
        case 11: result = 97; break;
        case 8: result = 74; break;
        case 1: result = 69; break;
        case 10: result = 249; break;
        case 19: result = 194; break;
        default: result = 88; break;
        }
        if (result != 48) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)18) + (uint16_t)33745;
        if (r != 33763) failures++;
    }


    {
        uint8_t v = 113;
        v ^= 128;
        if (v != 241) failures++;
    }


    {
        g16 = 28103;
        if (read_g16() != 28103) failures++;
    }


    {
        if (((uint16_t)(86 & ((149 + 188) + (182 & 132)))) != 84) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)124) % (int16_t)((int8_t)-3);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t r = add2(32,229) + add2(229,83) + add2(32,83);
        if (r != 688) failures++;
    }


    {
        g16 = 22676;
        if (read_g16() != 22676) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 86;
        x = x + 95;
        if (x != 181) failures++;
    }


    {
        uint8_t v = 148;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        g16 = 31138;
        if (read_g16() != 31138) failures++;
    }


    {
        uint8_t v = 199;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 25) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-39) / (int16_t)((int8_t)88);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t m[4][3] = {{152,45,231},{88,246,163},{51,174,13},{191,126,62}};
        if (m[0][1] != 45) failures++;
    }


    {
        volatile int16_t a = -28274;
        volatile int16_t b = -19364;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 41;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 9;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 55) failures++;
    }


    {
        uint8_t a[6] = {126,70,126,168,237,122};
        if (a[5] != 122) failures++;
    }


    {
        volatile uint8_t port = 230;
        uint8_t r = port;
        if (r != 230) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-81) % (int16_t)((int8_t)113);
        if ((uint16_t)r != (uint16_t)65455) failures++;
    }


    {
        if (((uint16_t)((64 ^ (224 ^ 41)) & ((85 ^ 56) - 90))) != 1) failures++;
    }


    {
        uint16_t x = 171;
        x = x + 203;
        if (x != 374) failures++;
    }


    {
        uint16_t x = 209;
        x = x + 143;
        if (x != 352) failures++;
    }


    {
        uint8_t m[3][3] = {{133,219,166},{96,161,200},{82,168,94}};
        if (m[0][2] != 166) failures++;
    }


    {
        uint8_t x = 125;
        x <<= 3;
        if (x != 232) failures++;
    }


    {
        uint8_t m[2][3] = {{106,75,71},{45,213,72}};
        if (m[1][0] != 45) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 97;
        v |= 1;
        if (v != 97) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {237,92,60417,240};
        if (s.c != (uint16_t)60417) failures++;
    }


    {
        int8_t a = 107;
        int8_t b = 111;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 221;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t x = 47473;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)((125 | (86 + 181)) - ((250 - 148) & 165))) != 347) failures++;
    }


    {
        if (((uint16_t)(((252 & 92) - 111) & ((52 & 249) | (182 ^ 148)))) != 32) failures++;
    }


    {
        uint16_t x = 79;
        x = x + 248;
        if (x != 327) failures++;
    }


    {
        if (((uint16_t)(((7 ^ 223) + (121 - 233)) & ((66 | 146) - (41 ^ 142)))) != 40) failures++;
    }


    {
        uint8_t v = 138;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint32_t a = 100789789UL;
        uint32_t b = 1855553939UL;
        uint32_t r = a ^ b;
        if (r != 1754820494UL) failures++;
    }


    {
        uint8_t v = 113;
        v |= 16;
        if (v != 113) failures++;
    }


    {
        uint8_t x = 50;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t r = 6384 + 49337 + 3009 + 40451 + 47260 + 61096 + 33921 + 62122;
        if (r != 41436) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(14,205) != 65345) failures++;
    }


    {
        volatile int16_t a = 704;
        volatile int16_t b = -31210;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)((22 + (37 ^ 146)) & ((13 - 196) | (181 - 36)))) != 201) failures++;
    }


    {
        uint8_t src[5] = {169,106,61,81,136};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 81) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-69) / (int16_t)((int8_t)87);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        if (((uint16_t)53) != 53) failures++;
    }


    {
        uint8_t m[3][4] = {{211,90,180,217},{152,202,87,234},{28,152,100,13}};
        if (m[2][2] != 100) failures++;
    }


    {
        uint8_t v = 229;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 11) failures++;
    }


    {
        int8_t a = 18;
        int8_t b = 19;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-2) % (int16_t)((int8_t)87);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint16_t x = 32491;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 243;
        uint8_t r = port;
        if (r != 243) failures++;
    }


    {
        g16 = 23573;
        if (read_g16() != 23573) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(117,4) + add2(4,150) + add2(117,150);
        if (r != 542) failures++;
    }


    {
        uint32_t a = 1701551793UL;
        uint32_t b = 679575082UL;
        uint32_t r = a & b;
        if (r != 536944160UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(78,242) != 320) failures++;
    }


    {
        volatile int16_t a = -27888;
        volatile int16_t b = 20053;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -11;
        int8_t b = 51;
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
        int16_t r = (int16_t)((int8_t)25) / (int16_t)((int8_t)-47);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 41141;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)66) % (int16_t)((int8_t)92);
        if ((uint16_t)r != (uint16_t)66) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint16_t r = call6(253,226,175,15,121,51);
        if (r != 841) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(240,182) != 422) failures++;
    }


    {
        uint8_t src[8] = {179,184,183,190,251,190,124,49};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 184) failures++;
    }


    {
        uint32_t a = 2686153028UL;
        uint32_t b = 2749209190UL;
        uint32_t r = a | b;
        if (r != 2749365094UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint8_t v = 219;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {240,218,30843,35};
        if (s.a != (uint8_t)240) failures++;
    }


    {
        uint8_t v = 237;
        v ^= 1;
        if (v != 236) failures++;
    }


    {
        volatile uint8_t port = 251;
        uint8_t r = port;
        if (r != 251) failures++;
    }


    {
        uint8_t src[10] = {126,69,162,146,241,34,187,191,43,236};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[3] != 146) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {156,226,30047,42};
        if (s.c != (uint16_t)30047) failures++;
    }


    {
        uint8_t a[6] = {175,217,235,233,226,214};
        if (a[2] != 235) failures++;
    }


    {
        volatile int16_t a = -25284;
        volatile int16_t b = 10553;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 214;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 216; break;
        case 10: result = 148; break;
        case 13: result = 248; break;
        case 5: result = 203; break;
        default: result = 241; break;
        }
        if (result != 216) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t m[3][3] = {{146,236,209},{239,224,109},{250,111,13}};
        if (m[1][0] != 239) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 210;
        if (buf[11] != 210) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 246;
        if (buf[12] != 246) failures++;
    }


    {
        int8_t a = 19;
        int8_t b = -15;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 49584;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 26423 + 55750 + 23025 + 54924 + 47464 + 53399 + 58389 + 10290;
        if (r != 1984) failures++;
    }


    {
        int8_t a = -47;
        int8_t b = -76;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[9] = {110,67,20,106,6,190,175,150,67};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[3] != 106) failures++;
    }


    {
        volatile int16_t a = -26721;
        volatile int16_t b = 13680;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 0: result = 6; break;
        case 16: result = 85; break;
        case 2: result = 24; break;
        case 15: result = 124; break;
        case 1: result = 156; break;
        case 11: result = 200; break;
        default: result = 39; break;
        }
        if (result != 200) failures++;
    }


    {
        volatile uint8_t port = 109;
        uint8_t r = port;
        if (r != 109) failures++;
    }


    {
        g16 = 38067;
        if (read_g16() != 38067) failures++;
    }


    {
        g16 = 7748;
        if (read_g16() != 7748) failures++;
    }


    {
        if (((uint16_t)141) != 141) failures++;
    }


    {
        uint8_t m[4][2] = {{80,172},{72,77},{65,178},{8,159}};
        if (m[1][1] != 77) failures++;
    }


    {
        g16 = 28789;
        if (read_g16() != 28789) failures++;
    }


    {
        uint8_t m[2][4] = {{219,239,7,117},{189,31,231,132}};
        if (m[1][3] != 132) failures++;
    }


    {
        volatile int16_t a = -19541;
        volatile int16_t b = -2083;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 213;
        uint8_t r = port;
        if (r != 213) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 203;
        if (buf[15] != 203) failures++;
    }


    {
        uint8_t src[3] = {69,192,170};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 170) failures++;
    }


    {
        uint16_t x = 65264;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[4][4] = {{217,176,142,169},{229,221,235,100},{29,222,82,80},{195,42,215,59}};
        if (m[1][3] != 100) failures++;
    }


    {
        volatile uint8_t port = 169;
        uint8_t r = port;
        if (r != 169) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)31) / (int16_t)((int8_t)44);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t x = 227;
        x <<= 0;
        if (x != 227) failures++;
    }


    {
        volatile uint8_t port = 139;
        uint8_t r = port;
        if (r != 139) failures++;
    }


    {
        if (((uint16_t)(((55 ^ 99) ^ 27) - ((8 ^ 223) & 176))) != 65471) failures++;
    }


    {
        uint8_t v = 144;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = 10347 + 51534 + 47247 + 38018 + 1919 + 5958 + 10718 + 28754;
        if (r != 63423) failures++;
    }


    {
        uint8_t a[6] = {55,133,41,96,151,44};
        if (a[3] != 96) failures++;
    }


    {
        g16 = 20873;
        if (read_g16() != 20873) failures++;
    }


    {
        uint16_t r = 15852 + 17834 + 59893 + 21328 + 15850 + 49369 + 5965 + 13471;
        if (r != 2954) failures++;
    }


    {
        uint8_t src[1] = {199};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 199) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint16_t x = 67;
        x = x + 181;
        if (x != 248) failures++;
    }


    {
        uint16_t x = 10;
        x = x + 35;
        if (x != 45) failures++;
    }


    {
        uint32_t a = 1041820913UL;
        uint32_t b = 3329238607UL;
        uint32_t r = a + b;
        if (r != 76092224UL) failures++;
    }


    {
        uint32_t a = 2268564567UL;
        uint32_t b = 1680895032UL;
        uint32_t r = a | b;
        if (r != 3879201919UL) failures++;
    }


    {
        uint8_t a[6] = {34,14,171,4,92,156};
        if (a[1] != 14) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)129) + (uint16_t)58909;
        if (r != 59038) failures++;
    }


    {
        uint16_t x = 243;
        x = x + 53;
        if (x != 296) failures++;
    }


    {
        uint16_t x = 60634;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 73;
        x = x + 232;
        if (x != 305) failures++;
    }


    {
        uint16_t r = add2(24,44) + add2(44,144) + add2(24,144);
        if (r != 424) failures++;
    }


    {
        volatile uint8_t port = 65;
        uint8_t r = port;
        if (r != 65) failures++;
    }


    {
        uint8_t v = 168;
        v &= ~(uint8_t)8;
        if (v != 160) failures++;
    }


    {
        uint8_t a[6] = {138,12,107,119,56,16};
        if (a[0] != 138) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 13: result = 75; break;
        case 17: result = 16; break;
        case 12: result = 198; break;
        case 14: result = 64; break;
        case 3: result = 223; break;
        case 18: result = 33; break;
        case 11: result = 3; break;
        case 6: result = 140; break;
        default: result = 170; break;
        }
        if (result != 64) failures++;
    }


    {
        uint16_t r = add2(44,242) + add2(242,247) + add2(44,247);
        if (r != 1066) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(167,10) != 177) failures++;
    }


    {
        uint32_t a = 2091642394UL;
        uint32_t b = 3815912861UL;
        uint32_t r = a ^ b;
        if (r != 2681854855UL) failures++;
    }


    {
        uint16_t x = 62248;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[16] = {193,184,172,29,110,136,35,216,175,116,227,172,215,62,207,42};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[9] != 116) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)95) + (uint16_t)45424;
        if (r != 45519) failures++;
    }


    {
        uint16_t x = 145;
        x = x + 230;
        if (x != 375) failures++;
    }


    {
        uint8_t m[2][3] = {{172,239,136},{168,192,147}};
        if (m[1][2] != 147) failures++;
    }


    {
        uint16_t r = add2(235,38) + add2(38,21) + add2(235,21);
        if (r != 588) failures++;
    }


    {
        uint16_t r = add2(52,135) + add2(135,123) + add2(52,123);
        if (r != 620) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {209,241,63635,205};
        if (s.d != (uint8_t)205) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 9: result = 231; break;
        case 10: result = 211; break;
        case 14: result = 200; break;
        default: result = 226; break;
        }
        if (result != 200) failures++;
    }


    {
        uint8_t buf[8] = {40,239,96,162,228,140,163,162};
        uint8_t *p = buf;
        p += 1;
        if (*p != 239) failures++;
    }


    {
        if (((uint16_t)(168 - (162 ^ 30))) != 65516) failures++;
    }


    {
        uint8_t x = 45;
        x <<= 5;
        if (x != 160) failures++;
    }


    {
        volatile uint8_t port = 47;
        uint8_t r = port;
        if (r != 47) failures++;
    }


    {
        uint16_t r = call6(81,0,92,76,238,197);
        if (r != 684) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 212;
        x = x + 111;
        if (x != 323) failures++;
    }


    {
        volatile uint8_t port = 206;
        uint8_t r = port;
        if (r != 206) failures++;
    }


    {
        uint8_t v = 3;
        v &= ~(uint8_t)32;
        if (v != 3) failures++;
    }


    {
        if (((uint16_t)((70 | 96) - 16)) != 86) failures++;
    }


    {
        uint8_t src[10] = {17,205,210,251,233,40,254,190,86,91};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[1] != 205) failures++;
    }


    {
        uint16_t x = 1485;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 9078 + 1940 + 63736 + 57573 + 64637 + 36472 + 33092 + 59628;
        if (r != 64012) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 2) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint32_t a = 1096964436UL;
        uint32_t b = 377664207UL;
        uint32_t r = a + b;
        if (r != 1474628643UL) failures++;
    }


    {
        uint8_t x = 202;
        x <<= 5;
        if (x != 64) failures++;
    }


    {
        g16 = 6776;
        if (read_g16() != 6776) failures++;
    }


    {
        uint16_t x = 49448;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[5] = {174,24,4,196,170};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 174) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {31,122,22825,189};
        if (s.c != (uint16_t)22825) failures++;
    }


    {
        int8_t a = 17;
        int8_t b = 18;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)((223 & (79 ^ 222)) & 0)) != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        if (((uint16_t)(((182 + 26) & 133) | 144)) != 144) failures++;
    }


    {
        uint8_t m[3][3] = {{4,112,142},{253,141,111},{81,93,109}};
        if (m[0][2] != 142) failures++;
    }


    {
        uint16_t x = 75;
        x = x + 18;
        if (x != 93) failures++;
    }


    {
        g16 = 35817;
        if (read_g16() != 35817) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)114) % (int16_t)((int8_t)-9);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint8_t x = 27;
        x <<= 2;
        if (x != 108) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 244;
        uint8_t r = port;
        if (r != 244) failures++;
    }


    {
        if (((uint16_t)(((234 + 188) - (201 + 61)) ^ 85)) != 245) failures++;
    }


    {
        uint8_t x = 19;
        x <<= 3;
        if (x != 152) failures++;
    }


    {
        uint16_t r = 38955 + 52146 + 49279 + 56694 + 40411 + 46563 + 27529 + 43993;
        if (r != 27890) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {97,179,20226,50};
        if (s.d != (uint8_t)50) failures++;
    }


    {
        uint16_t r = 58789 + 39490 + 13978 + 20295 + 56895 + 3496 + 42590 + 2297;
        if (r != 41222) failures++;
    }


    {
        uint8_t a[6] = {31,109,69,39,30,61};
        if (a[0] != 31) failures++;
    }


    {
        uint8_t v = 82;
        v &= ~(uint8_t)32;
        if (v != 82) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 98;
        if (buf[1] != 98) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 143;
        if (buf[5] != 143) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t buf[8] = {143,184,67,111,209,189,203,17};
        uint8_t *p = buf;
        p += 7;
        if (*p != 17) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {230,177,47181,214};
        if (s.c != (uint16_t)47181) failures++;
    }


    {
        uint16_t x = 70;
        x = x + 153;
        if (x != 223) failures++;
    }


    {
        uint8_t v = 202;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = 36664 + 8741 + 18433 + 29581 + 8374 + 1067 + 48904 + 13327;
        if (r != 34019) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 1089662235UL;
        uint32_t b = 3009325108UL;
        uint32_t r = a & b;
        if (r != 5416976UL) failures++;
    }


    {
        volatile uint8_t port = 181;
        uint8_t r = port;
        if (r != 181) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 1) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint16_t x = 4193;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 170;
        v ^= 4;
        if (v != 174) failures++;
    }


    {
        g16 = 17209;
        if (read_g16() != 17209) failures++;
    }


    {
        uint32_t a = 956374715UL;
        uint32_t b = 141025452UL;
        uint32_t r = a ^ b;
        if (r != 828833303UL) failures++;
    }


    {
        uint8_t buf[8] = {158,59,138,161,139,112,104,87};
        uint8_t *p = buf;
        p += 7;
        if (*p != 87) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 78;
        if (buf[6] != 78) failures++;
    }


    {
        uint8_t src[10] = {105,48,58,102,51,38,147,86,187,92};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[0] != 105) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(140,236) != 376) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {147,247,27402,146};
        if (s.c != (uint16_t)27402) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 10: result = 117; break;
        case 0: result = 179; break;
        case 15: result = 90; break;
        case 18: result = 2; break;
        default: result = 182; break;
        }
        if (result != 90) failures++;
    }


    {
        g16 = 28088;
        if (read_g16() != 28088) failures++;
    }


    {
        uint16_t r = call6(74,172,35,66,174,8);
        if (r != 529) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 6: result = 4; break;
        case 13: result = 131; break;
        case 18: result = 90; break;
        case 15: result = 47; break;
        case 12: result = 144; break;
        default: result = 162; break;
        }
        if (result != 144) failures++;
    }


    {
        uint8_t a[6] = {138,246,2,131,196,197};
        if (a[0] != 138) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {231,172,186,170,4,7};
        if (a[4] != 4) failures++;
    }


    {
        uint8_t buf[8] = {204,44,165,65,123,102,105,143};
        uint8_t *p = buf;
        p += 7;
        if (*p != 143) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)3) % (int16_t)((int8_t)60);
        if ((uint16_t)r != (uint16_t)3) failures++;
    }


    {
        uint16_t x = 25556;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 47;
        int8_t b = 118;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 180;
        uint8_t r = port;
        if (r != 180) failures++;
    }


    {
        uint16_t r = add2(190,250) + add2(250,83) + add2(190,83);
        if (r != 1046) failures++;
    }


    {
        volatile uint8_t port = 28;
        uint8_t r = port;
        if (r != 28) failures++;
    }


    {
        uint8_t src[11] = {205,237,3,171,155,90,127,245,3,195,127};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[8] != 3) failures++;
    }


    {
        uint8_t buf[8] = {129,62,43,52,204,156,205,195};
        uint8_t *p = buf;
        p += 4;
        if (*p != 204) failures++;
    }


    {
        uint16_t x = 57622;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {57,171,45379,13};
        if (s.b != (uint8_t)171) failures++;
    }


    {
        uint8_t x = 139;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        uint16_t r = call6(148,56,6,25,105,40);
        if (r != 380) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        int8_t a = 92;
        int8_t b = 28;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 555819115UL;
        uint32_t b = 2149660652UL;
        uint32_t r = a + b;
        if (r != 2705479767UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(188,69) != 119) failures++;
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
        for (uint8_t j = 0; j < 5; j++) buf[j] = 61;
        if (buf[4] != 61) failures++;
    }


    {
        uint8_t x = 82;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 106;
        int8_t b = 22;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint32_t a = 2229015378UL;
        uint32_t b = 2570893838UL;
        uint32_t r = a & b;
        if (r != 2149323266UL) failures++;
    }


    {
        int8_t a = -92;
        int8_t b = 49;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[11] = {221,147,177,5,191,219,95,73,139,29,176};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[8] != 139) failures++;
    }


    {
        uint8_t src[14] = {77,41,186,163,219,215,127,230,79,163,105,131,197,120};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[3] != 163) failures++;
    }


    {
        uint8_t v = 117;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(135,95) + add2(95,219) + add2(135,219);
        if (r != 898) failures++;
    }


    {
        uint16_t x = 62;
        x = x + 167;
        if (x != 229) failures++;
    }


    {
        uint8_t v = 121;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 173;
        int r = (v & 64) ? 1 : 0;
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
        uint8_t v = 138;
        v ^= 128;
        if (v != 10) failures++;
    }


    {
        int8_t a = -52;
        int8_t b = 107;
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
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 253;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }

    return failures;
}