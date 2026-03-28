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
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 13: result = 254; break;
        case 5: result = 113; break;
        case 10: result = 158; break;
        case 19: result = 193; break;
        case 0: result = 28; break;
        case 8: result = 133; break;
        case 14: result = 182; break;
        default: result = 91; break;
        }
        if (result != 113) failures++;
    }


    {
        if (((uint16_t)240) != 240) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)84) + (uint16_t)11895;
        if (r != 11979) failures++;
    }


    {
        uint8_t x = 122;
        x <<= 1;
        if (x != 244) failures++;
    }


    {
        int8_t a = -77;
        int8_t b = 86;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint8_t x = 168;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 5: result = 213; break;
        case 6: result = 145; break;
        case 8: result = 33; break;
        case 15: result = 218; break;
        case 4: result = 58; break;
        case 13: result = 14; break;
        case 9: result = 110; break;
        default: result = 231; break;
        }
        if (result != 110) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {247,159,14158,231};
        if (s.a != (uint8_t)247) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {87,8,23167,5};
        if (s.c != (uint16_t)23167) failures++;
    }


    {
        uint16_t r = add2(56,110) + add2(110,14) + add2(56,14);
        if (r != 360) failures++;
    }


    {
        uint16_t x = 205;
        x = x + 201;
        if (x != 406) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 57094 + 47438 + 33958 + 58401 + 986 + 15204 + 65450 + 47808;
        if (r != 64195) failures++;
    }


    {
        volatile uint8_t port = 30;
        uint8_t r = port;
        if (r != 30) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)10) + (uint16_t)59167;
        if (r != 59177) failures++;
    }


    {
        uint16_t r = add2(180,5) + add2(5,240) + add2(180,240);
        if (r != 850) failures++;
    }


    {
        uint16_t x = 28;
        x = x + 50;
        if (x != 78) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)65) / (int16_t)((int8_t)68);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint32_t a = 3962581881UL;
        uint32_t b = 1597425360UL;
        uint32_t r = a | b;
        if (r != 4281792505UL) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 8: result = 196; break;
        case 18: result = 33; break;
        case 14: result = 107; break;
        default: result = 85; break;
        }
        if (result != 85) failures++;
    }


    {
        uint8_t a[6] = {199,148,17,186,14,31};
        if (a[4] != 14) failures++;
    }


    {
        uint8_t x = 184;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        volatile uint8_t port = 95;
        uint8_t r = port;
        if (r != 95) failures++;
    }


    {
        uint16_t r = 785 + 26649 + 19045 + 57054 + 64421 + 38163 + 25630 + 11028;
        if (r != 46167) failures++;
    }


    {
        uint16_t x = 253;
        x = x + 133;
        if (x != 386) failures++;
    }


    {
        g16 = 16765;
        if (read_g16() != 16765) failures++;
    }


    {
        int8_t a = 55;
        int8_t b = -76;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {35,188,118,151,16,5,124,121};
        uint8_t *p = buf;
        p += 6;
        if (*p != 124) failures++;
    }


    {
        uint16_t x = 138;
        x = x + 86;
        if (x != 224) failures++;
    }


    {
        int8_t a = 41;
        int8_t b = -44;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 60046;
        if (read_g16() != 60046) failures++;
    }


    {
        volatile uint8_t port = 219;
        uint8_t r = port;
        if (r != 219) failures++;
    }


    {
        uint16_t r = add2(55,83) + add2(83,46) + add2(55,46);
        if (r != 368) failures++;
    }


    {
        uint8_t x = 103;
        x <<= 0;
        if (x != 103) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 18: result = 43; break;
        case 2: result = 80; break;
        case 12: result = 190; break;
        case 7: result = 36; break;
        case 6: result = 12; break;
        case 3: result = 125; break;
        default: result = 193; break;
        }
        if (result != 36) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)103) % (int16_t)((int8_t)124);
        if ((uint16_t)r != (uint16_t)103) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,70,949,206};
        if (s.a != (uint8_t)205) failures++;
    }


    {
        uint8_t v = 176;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)21) / (int16_t)((int8_t)-124);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 197;
        x = x + 151;
        if (x != 348) failures++;
    }


    {
        uint16_t r = call6(136,66,31,73,143,253);
        if (r != 702) failures++;
    }


    {
        g16 = 49037;
        if (read_g16() != 49037) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 253; break;
        case 3: result = 223; break;
        case 12: result = 249; break;
        case 18: result = 118; break;
        default: result = 210; break;
        }
        if (result != 210) failures++;
    }


    {
        volatile int16_t a = -4000;
        volatile int16_t b = -17242;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 63;
        int8_t b = -10;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(190,248) != 65478) failures++;
    }


    {
        if (((uint16_t)(((170 - 183) + (89 | 217)) + ((76 & 10) + (35 - 151)))) != 96) failures++;
    }


    {
        uint8_t x = 104;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        uint16_t r = add2(41,252) + add2(252,155) + add2(41,155);
        if (r != 896) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(167,246) != 413) failures++;
    }


    {
        uint16_t x = 113;
        x = x + 128;
        if (x != 241) failures++;
    }


    {
        uint16_t r = 36320 + 2537 + 14852 + 3593 + 38145 + 37135 + 26483 + 6167;
        if (r != 34160) failures++;
    }


    {
        int8_t a = -83;
        int8_t b = 56;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint32_t a = 135971335UL;
        uint32_t b = 3390584643UL;
        uint32_t r = a - b;
        if (r != 1040353988UL) failures++;
    }


    {
        uint16_t r = 64245 + 9219 + 33798 + 14463 + 40047 + 4601 + 48116 + 42452;
        if (r != 60333) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {126,211,32682,202};
        if (s.d != (uint8_t)202) failures++;
    }


    {
        uint8_t buf[8] = {164,51,31,249,113,129,230,236};
        uint8_t *p = buf;
        p += 6;
        if (*p != 230) failures++;
    }


    {
        volatile uint8_t port = 11;
        uint8_t r = port;
        if (r != 11) failures++;
    }


    {
        uint8_t x = 10;
        x <<= 4;
        if (x != 160) failures++;
    }


    {
        volatile int16_t a = -10979;
        volatile int16_t b = -2546;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(21,119) != 140) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 0: result = 219; break;
        case 7: result = 1; break;
        case 2: result = 211; break;
        default: result = 192; break;
        }
        if (result != 219) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)56) + (uint16_t)58732;
        if (r != 58788) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t a[6] = {248,107,85,10,246,29};
        if (a[2] != 85) failures++;
    }


    {
        int8_t a = 67;
        int8_t b = 110;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 58357 + 60294 + 43184 + 19880 + 17555 + 44517 + 6188 + 17201;
        if (r != 5032) failures++;
    }


    {
        uint8_t src[4] = {123,210,190,23};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 123) failures++;
    }


    {
        uint8_t v = 37;
        int r = (v & 8) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t x = 50;
        x = x + 181;
        if (x != 231) failures++;
    }


    {
        uint8_t x = 81;
        x <<= 3;
        if (x != 136) failures++;
    }


    {
        uint16_t r = call6(41,181,89,108,236,199);
        if (r != 854) failures++;
    }


    {
        uint16_t x = 8;
        x = x + 75;
        if (x != 83) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)135) + (uint16_t)28264;
        if (r != 28399) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint8_t x = 227;
        x <<= 3;
        if (x != 24) failures++;
    }


    {
        uint16_t x = 40682;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 2778;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 169;
        uint8_t r = port;
        if (r != 169) failures++;
    }


    {
        uint8_t buf[8] = {175,51,61,255,27,81,10,5};
        uint8_t *p = buf;
        p += 4;
        if (*p != 27) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)26) / (int16_t)((int8_t)-10);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        volatile int16_t a = 25908;
        volatile int16_t b = -9951;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 26;
        int r = (v & 8) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {149,225,33,220,179,235,128,39};
        uint8_t *p = buf;
        p += 6;
        if (*p != 128) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 4: result = 116; break;
        case 18: result = 13; break;
        case 6: result = 168; break;
        default: result = 42; break;
        }
        if (result != 116) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)12) / (int16_t)((int8_t)-64);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {147,179,157,94,117,112};
        if (a[3] != 94) failures++;
    }


    {
        uint8_t m[2][3] = {{60,23,15},{241,219,60}};
        if (m[1][0] != 241) failures++;
    }


    {
        uint16_t r = call6(33,225,82,58,157,111);
        if (r != 666) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint8_t src[9] = {239,161,48,239,131,174,101,149,202};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[1] != 161) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(44,227) != 271) failures++;
    }


    {
        volatile int16_t a = -19908;
        volatile int16_t b = 27810;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 129;
        x = x + 2;
        if (x != 131) failures++;
    }


    {
        uint8_t x = 5;
        x <<= 2;
        if (x != 20) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)92) / (int16_t)((int8_t)-42);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint8_t a[6] = {62,238,106,195,205,178};
        if (a[2] != 106) failures++;
    }


    {
        uint8_t v = 22;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 10) failures++;
    }


    {
        uint16_t r = call6(70,10,181,107,218,177);
        if (r != 763) failures++;
    }


    {
        uint8_t v = 6;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {23,247,25,126,188,189,42,125};
        uint8_t *p = buf;
        p += 7;
        if (*p != 125) failures++;
    }


    {
        uint16_t r = call6(38,193,13,102,203,139);
        if (r != 688) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)39) / (int16_t)((int8_t)-54);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        int8_t a = 57;
        int8_t b = 56;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 53072;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 142;
        if (buf[9] != 142) failures++;
    }


    {
        uint8_t m[3][3] = {{138,174,123},{142,248,107},{77,84,177}};
        if (m[0][1] != 174) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 1) sum += j;
        if (sum != 171) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {231,46,47445,76};
        if (s.b != (uint8_t)46) failures++;
    }


    {
        uint8_t v = 159;
        v &= ~(uint8_t)64;
        if (v != 159) failures++;
    }


    {
        uint16_t r = add2(130,85) + add2(85,53) + add2(130,53);
        if (r != 536) failures++;
    }


    {
        uint16_t r = call6(66,49,1,234,29,1);
        if (r != 380) failures++;
    }


    {
        int8_t a = 15;
        int8_t b = 47;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(1,43,202,210,173,156);
        if (r != 785) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)242) + (uint16_t)21010;
        if (r != 21252) failures++;
    }


    {
        uint8_t x = 160;
        x <<= 4;
        if (x != 0) failures++;
    }


    {
        uint16_t x = 160;
        x = x + 154;
        if (x != 314) failures++;
    }


    {
        uint16_t x = 7408;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 13: result = 9; break;
        case 11: result = 115; break;
        case 8: result = 114; break;
        case 7: result = 97; break;
        default: result = 171; break;
        }
        if (result != 115) failures++;
    }


    {
        uint32_t a = 4062841490UL;
        uint32_t b = 4095283481UL;
        uint32_t r = a - b;
        if (r != 4262525305UL) failures++;
    }


    {
        uint8_t src[15] = {193,62,65,57,53,103,130,182,130,73,35,200,251,194,2};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[11] != 200) failures++;
    }


    {
        uint16_t r = add2(238,115) + add2(115,22) + add2(238,22);
        if (r != 750) failures++;
    }


    {
        uint8_t a[6] = {88,77,25,87,58,4};
        if (a[0] != 88) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(179,89) != 90) failures++;
    }


    {
        uint8_t v = 162;
        v &= ~(uint8_t)128;
        if (v != 34) failures++;
    }


    {
        uint8_t src[14] = {178,103,162,161,75,188,181,255,131,210,248,142,239,53};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[11] != 142) failures++;
    }


    {
        int8_t a = -113;
        int8_t b = -13;
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
        uint8_t v = 116;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 59591;
        if (read_g16() != 59591) failures++;
    }


    {
        uint32_t a = 1652462401UL;
        uint32_t b = 3020062110UL;
        uint32_t r = a ^ b;
        if (r != 3598511839UL) failures++;
    }


    {
        g16 = 6385;
        if (read_g16() != 6385) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        volatile uint8_t port = 106;
        uint8_t r = port;
        if (r != 106) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 117;
        if (buf[2] != 117) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)106) + (uint16_t)39523;
        if (r != 39629) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)17) + (uint16_t)17747;
        if (r != 17764) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 115;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        int8_t a = -43;
        int8_t b = 60;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {129,53,172,29,237,191};
        if (a[1] != 53) failures++;
    }


    {
        uint8_t m[4][4] = {{95,63,127,160},{166,223,236,4},{49,88,141,142},{136,235,166,51}};
        if (m[3][3] != 51) failures++;
    }


    {
        uint8_t m[3][3] = {{98,95,4},{188,226,255},{171,234,99}};
        if (m[2][0] != 171) failures++;
    }


    {
        uint8_t src[12] = {134,158,167,47,131,238,150,254,165,147,6,208};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[3] != 47) failures++;
    }


    {
        volatile int16_t a = 5216;
        volatile int16_t b = 31148;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[8] = {217,226,138,146,160,144,128,140};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[7] != 140) failures++;
    }


    {
        uint8_t buf[8] = {232,98,168,65,187,180,66,113};
        uint8_t *p = buf;
        p += 1;
        if (*p != 98) failures++;
    }


    {
        uint8_t x = 53;
        x <<= 3;
        if (x != 168) failures++;
    }


    {
        uint8_t v = 93;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 41;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint8_t v = 146;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(33,113) + add2(113,71) + add2(33,71);
        if (r != 434) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 14: result = 31; break;
        case 9: result = 111; break;
        case 13: result = 208; break;
        case 18: result = 72; break;
        case 5: result = 55; break;
        case 0: result = 159; break;
        case 16: result = 202; break;
        default: result = 202; break;
        }
        if (result != 202) failures++;
    }


    {
        if (((uint16_t)(192 + ((119 ^ 49) - 184))) != 78) failures++;
    }


    {
        uint32_t a = 3295039207UL;
        uint32_t b = 3130117791UL;
        uint32_t r = a + b;
        if (r != 2130189702UL) failures++;
    }


    {
        uint8_t src[11] = {40,51,66,213,65,116,210,156,91,77,147};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[2] != 66) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)98) / (int16_t)((int8_t)-58);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-91) % (int16_t)((int8_t)-124);
        if ((uint16_t)r != (uint16_t)65445) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 126;
        x <<= 4;
        if (x != 224) failures++;
    }


    {
        uint32_t a = 3234746897UL;
        uint32_t b = 2105179621UL;
        uint32_t r = a & b;
        if (r != 1078611969UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-89) / (int16_t)((int8_t)111);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 112;
        x = x + 96;
        if (x != 208) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)248) + (uint16_t)3700;
        if (r != 3948) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-116) / (int16_t)((int8_t)107);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint32_t a = 3805393151UL;
        uint32_t b = 3707957167UL;
        uint32_t r = a - b;
        if (r != 97435984UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)82) / (int16_t)((int8_t)-50);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t m[2][3] = {{227,216,202},{60,172,245}};
        if (m[0][0] != 227) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint8_t a[6] = {161,106,198,69,108,118};
        if (a[2] != 198) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint16_t r = call6(7,25,221,154,231,60);
        if (r != 698) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)49) + (uint16_t)57778;
        if (r != 57827) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 88;
        if (buf[13] != 88) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 55;
        if (buf[9] != 55) failures++;
    }


    {
        volatile uint8_t port = 243;
        uint8_t r = port;
        if (r != 243) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {69,74,65060,149};
        if (s.c != (uint16_t)65060) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {163,203,10446,128};
        if (s.c != (uint16_t)10446) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {242,52,59811,149};
        if (s.c != (uint16_t)59811) failures++;
    }


    {
        if (((uint16_t)(62 + ((225 - 0) & (165 | 197)))) != 287) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)108) + (uint16_t)15666;
        if (r != 15774) failures++;
    }


    {
        g16 = 54956;
        if (read_g16() != 54956) failures++;
    }


    {
        uint8_t x = 230;
        x <<= 4;
        if (x != 96) failures++;
    }


    {
        volatile uint8_t port = 44;
        uint8_t r = port;
        if (r != 44) failures++;
    }


    {
        uint16_t r = 46707 + 52319 + 63555 + 21016 + 22386 + 11815 + 29462 + 8308;
        if (r != 58960) failures++;
    }


    {
        uint8_t m[3][2] = {{212,36},{55,138},{184,102}};
        if (m[1][0] != 55) failures++;
    }


    {
        uint8_t v = 114;
        v &= ~(uint8_t)16;
        if (v != 98) failures++;
    }


    {
        uint16_t r = add2(125,120) + add2(120,161) + add2(125,161);
        if (r != 812) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {23,215,41121,188};
        if (s.c != (uint16_t)41121) failures++;
    }


    {
        uint8_t a[6] = {36,75,81,137,16,82};
        if (a[3] != 137) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)142) != 142) failures++;
    }


    {
        uint8_t a[6] = {175,27,34,252,58,194};
        if (a[3] != 252) failures++;
    }


    {
        uint16_t r = 16365 + 58349 + 43217 + 20829 + 43285 + 46197 + 30755 + 42840;
        if (r != 39693) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(197,78) != 119) failures++;
    }


    {
        uint8_t v = 255;
        v |= 16;
        if (v != 255) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(224,159,31,167,238,40);
        if (r != 859) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(113,114) != 227) failures++;
    }


    {
        uint8_t buf[8] = {20,26,245,86,251,54,184,150};
        uint8_t *p = buf;
        p += 5;
        if (*p != 54) failures++;
    }


    {
        uint8_t v = 155;
        v &= ~(uint8_t)128;
        if (v != 27) failures++;
    }


    {
        uint8_t x = 144;
        x <<= 3;
        if (x != 128) failures++;
    }


    {
        volatile int16_t a = 24824;
        volatile int16_t b = 8403;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {32,177,211,121,230,171};
        if (a[4] != 230) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 110;
        if (buf[2] != 110) failures++;
    }


    {
        uint8_t m[4][3] = {{228,212,93},{130,17,196},{208,80,97},{180,62,187}};
        if (m[1][2] != 196) failures++;
    }


    {
        volatile uint8_t port = 118;
        uint8_t r = port;
        if (r != 118) failures++;
    }


    {
        uint8_t buf[8] = {19,198,142,129,58,215,102,233};
        uint8_t *p = buf;
        p += 4;
        if (*p != 58) failures++;
    }


    {
        volatile uint8_t port = 156;
        uint8_t r = port;
        if (r != 156) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)25) % (int16_t)((int8_t)-121);
        if ((uint16_t)r != (uint16_t)25) failures++;
    }


    {
        uint16_t r = add2(16,219) + add2(219,17) + add2(16,17);
        if (r != 504) failures++;
    }


    {
        volatile uint8_t port = 38;
        uint8_t r = port;
        if (r != 38) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {15,200,29784,26};
        if (s.c != (uint16_t)29784) failures++;
    }


    {
        uint16_t r = add2(82,144) + add2(144,201) + add2(82,201);
        if (r != 854) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)62) % (int16_t)((int8_t)-18);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        uint16_t x = 1;
        x = x + 164;
        if (x != 165) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(27,94) != 121) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)224) + (uint16_t)1324;
        if (r != 1548) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {160,200,248,40,106,35};
        if (a[5] != 35) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)21) / (int16_t)((int8_t)-66);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = call6(209,10,44,24,75,29);
        if (r != 391) failures++;
    }


    {
        volatile uint8_t port = 186;
        uint8_t r = port;
        if (r != 186) failures++;
    }


    {
        volatile int16_t a = 23755;
        volatile int16_t b = 21724;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t src[15] = {131,65,42,195,65,37,35,99,196,136,177,63,28,31,5};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[11] != 63) failures++;
    }


    {
        uint8_t v = 112;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 146;
        x = x + 219;
        if (x != 365) failures++;
    }


    {
        uint8_t src[16] = {189,103,87,253,123,218,201,99,161,243,232,104,11,248,77,93};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[4] != 123) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)98) % (int16_t)((int8_t)-7);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-62) % (int16_t)((int8_t)116);
        if ((uint16_t)r != (uint16_t)65474) failures++;
    }


    {
        int8_t a = -17;
        int8_t b = 41;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 35894;
        if (read_g16() != 35894) failures++;
    }


    {
        uint8_t buf[8] = {172,104,46,218,249,245,116,102};
        uint8_t *p = buf;
        p += 4;
        if (*p != 249) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 9: result = 10; break;
        case 5: result = 148; break;
        case 19: result = 164; break;
        case 14: result = 138; break;
        case 17: result = 196; break;
        default: result = 201; break;
        }
        if (result != 201) failures++;
    }


    {
        uint8_t m[2][4] = {{7,149,109,55},{146,196,155,249}};
        if (m[1][3] != 249) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {212,39,7292,4};
        if (s.c != (uint16_t)7292) failures++;
    }


    {
        uint32_t a = 2890415249UL;
        uint32_t b = 526754879UL;
        uint32_t r = a | b;
        if (r != 3211639999UL) failures++;
    }


    {
        uint16_t x = 3082;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-20) % (int16_t)((int8_t)-38);
        if ((uint16_t)r != (uint16_t)65516) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)14) + (uint16_t)47387;
        if (r != 47401) failures++;
    }


    {
        uint32_t a = 4063752405UL;
        uint32_t b = 4119226868UL;
        uint32_t r = a | b;
        if (r != 4156029429UL) failures++;
    }


    {
        uint32_t a = 2246739086UL;
        uint32_t b = 3588708587UL;
        uint32_t r = a + b;
        if (r != 1540480377UL) failures++;
    }


    {
        g16 = 55078;
        if (read_g16() != 55078) failures++;
    }


    {
        uint8_t v = 161;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 31) failures++;
    }


    {
        uint16_t r = 40137 + 15544 + 63005 + 59338 + 21848 + 28002 + 27312 + 14367;
        if (r != 7409) failures++;
    }


    {
        uint32_t a = 3894227041UL;
        uint32_t b = 2533341002UL;
        uint32_t r = a | b;
        if (r != 4278171499UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {254,228,20953,253};
        if (s.a != (uint8_t)254) failures++;
    }


    {
        volatile int16_t a = -27552;
        volatile int16_t b = 15663;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(30,69) + add2(69,181) + add2(30,181);
        if (r != 560) failures++;
    }


    {
        uint8_t v = 90;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        uint16_t r = add2(224,200) + add2(200,22) + add2(224,22);
        if (r != 892) failures++;
    }


    {
        uint16_t x = 89;
        x = x + 138;
        if (x != 227) failures++;
    }


    {
        uint8_t src[14] = {108,58,110,194,91,70,28,67,106,141,41,77,78,248};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[9] != 141) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 167;
        v |= 32;
        if (v != 167) failures++;
    }


    {
        uint8_t m[4][3] = {{88,129,0},{161,86,208},{10,226,33},{160,83,116}};
        if (m[3][2] != 116) failures++;
    }


    {
        uint16_t x = 1109;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(172,125) != 47) failures++;
    }


    {
        volatile uint8_t port = 6;
        uint8_t r = port;
        if (r != 6) failures++;
    }


    {
        volatile int16_t a = -27164;
        volatile int16_t b = -14211;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 96;
        uint8_t r = port;
        if (r != 96) failures++;
    }


    {
        uint16_t x = 30275;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -18772;
        volatile int16_t b = 27565;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = -19780;
        volatile int16_t b = 28036;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 165771233UL;
        uint32_t b = 417863303UL;
        uint32_t r = a & b;
        if (r != 148903553UL) failures++;
    }


    {
        uint32_t a = 282022138UL;
        uint32_t b = 1042128268UL;
        uint32_t r = a ^ b;
        if (r != 785567094UL) failures++;
    }


    {
        uint16_t r = add2(237,244) + add2(244,228) + add2(237,228);
        if (r != 1418) failures++;
    }


    {
        uint8_t m[4][4] = {{161,26,61,83},{63,163,171,25},{181,228,20,46},{117,146,121,130}};
        if (m[2][0] != 181) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {243,26,47895,15};
        if (s.b != (uint8_t)26) failures++;
    }


    {
        uint8_t src[1] = {98};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 98) failures++;
    }


    {
        uint16_t x = 115;
        x = x + 164;
        if (x != 279) failures++;
    }


    {
        uint8_t v = 94;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {201,62,202,254,63,219,229,20};
        uint8_t *p = buf;
        p += 4;
        if (*p != 63) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        volatile uint8_t port = 66;
        uint8_t r = port;
        if (r != 66) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)239) + (uint16_t)23357;
        if (r != 23596) failures++;
    }


    {
        volatile uint8_t port = 72;
        uint8_t r = port;
        if (r != 72) failures++;
    }


    {
        uint16_t r = 43055 + 44191 + 30618 + 14387 + 24884 + 61307 + 15387 + 59971;
        if (r != 31656) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)69) + (uint16_t)51155;
        if (r != 51224) failures++;
    }


    {
        uint8_t v = 192;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        if (((uint16_t)6) != 6) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(115,23) != 138) failures++;
    }


    {
        uint8_t x = 162;
        x <<= 3;
        if (x != 16) failures++;
    }


    {
        uint8_t buf[8] = {114,234,197,81,227,191,53,146};
        uint8_t *p = buf;
        p += 3;
        if (*p != 81) failures++;
    }


    {
        volatile int16_t a = 10296;
        volatile int16_t b = -28676;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(201,194) != 7) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        uint8_t v = 143;
        v |= 32;
        if (v != 175) failures++;
    }


    {
        uint8_t src[15] = {169,162,186,245,142,254,185,18,90,99,51,252,158,91,167};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[8] != 90) failures++;
    }


    {
        volatile int16_t a = 21631;
        volatile int16_t b = 10537;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t v = 106;
        int r = (v & 4) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(133 + 73)) != 206) failures++;
    }


    {
        uint8_t v = 196;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t src[4] = {14,65,48,217};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 14) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 31763;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(22,4) != 18) failures++;
    }


    {
        uint8_t m[2][4] = {{91,194,226,176},{205,252,217,7}};
        if (m[0][1] != 194) failures++;
    }


    {
        uint8_t m[4][4] = {{246,133,62,158},{135,236,24,86},{32,186,210,69},{88,125,42,50}};
        if (m[1][0] != 135) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {83,61,51734,246};
        if (s.b != (uint8_t)61) failures++;
    }


    {
        uint16_t x = 248;
        x = x + 125;
        if (x != 373) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 2) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 18780;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(145,116) + add2(116,239) + add2(145,239);
        if (r != 1000) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 228;
        if (buf[13] != 228) failures++;
    }


    {
        uint8_t v = 191;
        v &= ~(uint8_t)2;
        if (v != 189) failures++;
    }


    {
        uint8_t a[6] = {236,254,220,33,17,68};
        if (a[4] != 17) failures++;
    }


    {
        uint16_t x = 162;
        x = x + 242;
        if (x != 404) failures++;
    }


    {
        uint8_t buf[8] = {213,150,57,65,216,55,212,27};
        uint8_t *p = buf;
        p += 1;
        if (*p != 150) failures++;
    }


    {
        volatile uint8_t port = 34;
        uint8_t r = port;
        if (r != 34) failures++;
    }


    {
        uint8_t buf[8] = {52,226,31,244,31,97,186,66};
        uint8_t *p = buf;
        p += 1;
        if (*p != 226) failures++;
    }


    {
        g16 = 7885;
        if (read_g16() != 7885) failures++;
    }


    {
        volatile uint8_t port = 254;
        uint8_t r = port;
        if (r != 254) failures++;
    }


    {
        uint16_t r = add2(71,81) + add2(81,149) + add2(71,149);
        if (r != 602) failures++;
    }


    {
        volatile uint8_t port = 208;
        uint8_t r = port;
        if (r != 208) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 16: result = 164; break;
        case 18: result = 222; break;
        case 19: result = 128; break;
        case 7: result = 198; break;
        default: result = 173; break;
        }
        if (result != 173) failures++;
    }


    {
        int8_t a = 73;
        int8_t b = -107;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        if (((uint16_t)(213 - (66 - (92 & 134)))) != 151) failures++;
    }


    {
        int8_t a = -84;
        int8_t b = -24;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)4) + (uint16_t)57263;
        if (r != 57267) failures++;
    }


    {
        uint8_t src[1] = {96};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 96) failures++;
    }


    {
        uint16_t r = call6(106,139,249,54,232,232);
        if (r != 1012) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 15: result = 19; break;
        case 9: result = 72; break;
        case 4: result = 254; break;
        case 2: result = 12; break;
        case 6: result = 196; break;
        case 19: result = 69; break;
        default: result = 7; break;
        }
        if (result != 254) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 128;
        if (buf[5] != 128) failures++;
    }


    {
        uint8_t m[4][3] = {{14,209,32},{77,73,237},{132,71,238},{177,235,24}};
        if (m[1][2] != 237) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(179,185) != 364) failures++;
    }


    {
        volatile int16_t a = 12896;
        volatile int16_t b = 10962;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(121,4,96,177,91,243);
        if (r != 732) failures++;
    }


    {
        if (((uint16_t)(((58 ^ 109) & (240 + 113)) - 31)) != 34) failures++;
    }


    {
        uint8_t buf[8] = {237,158,56,147,21,89,62,59};
        uint8_t *p = buf;
        p += 4;
        if (*p != 21) failures++;
    }


    {
        if (((uint16_t)(53 & ((160 | 56) + (28 + 161)))) != 53) failures++;
    }


    {
        g16 = 55787;
        if (read_g16() != 55787) failures++;
    }


    {
        g16 = 42278;
        if (read_g16() != 42278) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {63,123,21606,251};
        if (s.a != (uint8_t)63) failures++;
    }


    {
        uint8_t a[6] = {133,67,42,192,53,104};
        if (a[1] != 67) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(32,172) != 65396) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        if (((uint16_t)129) != 129) failures++;
    }


    {
        uint8_t m[3][3] = {{69,132,27},{127,222,153},{222,154,105}};
        if (m[0][1] != 132) failures++;
    }


    {
        int8_t a = 74;
        int8_t b = -23;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = -969;
        volatile int16_t b = 27574;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[2] = {149,207};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 149) failures++;
    }


    {
        uint8_t buf[8] = {189,147,220,169,205,52,110,190};
        uint8_t *p = buf;
        p += 6;
        if (*p != 110) failures++;
    }


    {
        uint16_t r = add2(67,225) + add2(225,91) + add2(67,91);
        if (r != 766) failures++;
    }


    {
        uint16_t r = 25731 + 11479 + 32677 + 12402 + 33793 + 12850 + 40841 + 36495;
        if (r != 9660) failures++;
    }


    {
        volatile int16_t a = -13054;
        volatile int16_t b = -28421;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 8;
        x <<= 3;
        if (x != 64) failures++;
    }


    {
        uint16_t r = add2(203,82) + add2(82,199) + add2(203,199);
        if (r != 968) failures++;
    }


    {
        uint16_t r = add2(44,235) + add2(235,111) + add2(44,111);
        if (r != 780) failures++;
    }


    {
        uint16_t r = add2(101,141) + add2(141,77) + add2(101,77);
        if (r != 638) failures++;
    }


    {
        volatile uint8_t port = 97;
        uint8_t r = port;
        if (r != 97) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)60) % (int16_t)((int8_t)-93);
        if ((uint16_t)r != (uint16_t)60) failures++;
    }


    {
        if (((uint16_t)(88 | ((118 | 16) ^ (129 & 185)))) != 255) failures++;
    }


    {
        int8_t a = -71;
        int8_t b = 77;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 41;
        if (buf[5] != 41) failures++;
    }


    {
        uint16_t x = 19444;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 17: result = 84; break;
        case 8: result = 100; break;
        case 9: result = 237; break;
        case 0: result = 193; break;
        case 5: result = 156; break;
        case 15: result = 131; break;
        default: result = 121; break;
        }
        if (result != 131) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(88,111) != 199) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint32_t a = 3377364062UL;
        uint32_t b = 1649037141UL;
        uint32_t r = a + b;
        if (r != 731433907UL) failures++;
    }


    {
        uint8_t v = 223;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        int8_t a = -95;
        int8_t b = -31;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 142;
        v &= ~(uint8_t)32;
        if (v != 142) failures++;
    }


    {
        uint16_t r = 62530 + 5311 + 40276 + 40924 + 65099 + 19472 + 43215 + 27790;
        if (r != 42473) failures++;
    }


    {
        uint16_t r = 9493 + 28502 + 17290 + 37737 + 50267 + 7210 + 9702 + 62514;
        if (r != 26107) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 46;
        if (buf[11] != 46) failures++;
    }


    {
        uint32_t a = 1512091106UL;
        uint32_t b = 1787721978UL;
        uint32_t r = a - b;
        if (r != 4019336424UL) failures++;
    }


    {
        g16 = 48857;
        if (read_g16() != 48857) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 19: result = 124; break;
        case 8: result = 209; break;
        case 12: result = 16; break;
        case 15: result = 243; break;
        case 5: result = 245; break;
        case 2: result = 11; break;
        case 1: result = 203; break;
        default: result = 190; break;
        }
        if (result != 209) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 13: result = 170; break;
        case 18: result = 5; break;
        case 8: result = 22; break;
        case 0: result = 68; break;
        case 6: result = 74; break;
        case 7: result = 207; break;
        case 10: result = 251; break;
        case 12: result = 186; break;
        default: result = 239; break;
        }
        if (result != 68) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 198;
        if (buf[7] != 198) failures++;
    }


    {
        uint8_t src[15] = {184,207,64,205,178,104,85,123,52,18,131,71,61,49,177};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[1] != 207) failures++;
    }


    {
        if (((uint16_t)((24 & (8 - 214)) & 35)) != 0) failures++;
    }


    {
        uint16_t x = 61;
        x = x + 122;
        if (x != 183) failures++;
    }


    {
        volatile uint8_t port = 117;
        uint8_t r = port;
        if (r != 117) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)71) + (uint16_t)37337;
        if (r != 37408) failures++;
    }


    {
        uint16_t x = 105;
        x = x + 129;
        if (x != 234) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 211;
        if (buf[15] != 211) failures++;
    }


    {
        uint8_t v = 141;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 51) failures++;
    }


    {
        volatile uint8_t port = 119;
        uint8_t r = port;
        if (r != 119) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {159,120,11822,242};
        if (s.b != (uint8_t)120) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint16_t r = 54724 + 36309 + 43518 + 16755 + 63017 + 54513 + 29852 + 3158;
        if (r != 39702) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-128) % (int16_t)((int8_t)-11);
        if ((uint16_t)r != (uint16_t)65529) failures++;
    }


    {
        uint8_t m[4][3] = {{92,212,68},{249,203,26},{125,29,126},{68,38,241}};
        if (m[3][1] != 38) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {121,100,11487,229};
        if (s.d != (uint8_t)229) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[3][3] = {{232,121,158},{175,248,115},{72,7,126}};
        if (m[1][2] != 115) failures++;
    }


    {
        uint16_t r = 42699 + 38907 + 24860 + 57865 + 11561 + 41982 + 35789 + 27190;
        if (r != 18709) failures++;
    }


    {
        uint16_t r = call6(216,108,114,149,0,144);
        if (r != 731) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t x = 229;
        x <<= 2;
        if (x != 148) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-87) % (int16_t)((int8_t)-3);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 22;
        if (buf[10] != 22) failures++;
    }


    {
        volatile int16_t a = 12729;
        volatile int16_t b = -28869;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {206,191,46,88,124,21,4,122};
        uint8_t *p = buf;
        p += 4;
        if (*p != 124) failures++;
    }


    {
        uint16_t r = call6(105,139,36,97,126,80);
        if (r != 583) failures++;
    }


    {
        uint8_t v = 77;
        v |= 64;
        if (v != 77) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 189;
        if (buf[10] != 189) failures++;
    }


    {
        uint8_t v = 213;
        int r = (v & 8) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 235;
        uint8_t r = port;
        if (r != 235) failures++;
    }


    {
        volatile uint8_t port = 218;
        uint8_t r = port;
        if (r != 218) failures++;
    }


    {
        uint8_t x = 27;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        uint16_t r = 8722 + 57261 + 62448 + 24759 + 21059 + 48521 + 11332 + 9196;
        if (r != 46690) failures++;
    }


    {
        uint16_t r = call6(187,6,28,121,193,49);
        if (r != 584) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 53;
        if (buf[3] != 53) failures++;
    }


    {
        uint8_t v = 211;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint16_t r = add2(95,3) + add2(3,201) + add2(95,201);
        if (r != 598) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t v = 144;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t x = 75;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)89) + (uint16_t)7293;
        if (r != 7382) failures++;
    }


    {
        uint16_t x = 8171;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 60370;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 221;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 199;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t r = 61584 + 59714 + 34516 + 38053 + 5494 + 11777 + 38794 + 43209;
        if (r != 30997) failures++;
    }


    {
        g16 = 713;
        if (read_g16() != 713) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 34;
        if (buf[3] != 34) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)64) / (int16_t)((int8_t)8);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(23,233) != 65326) failures++;
    }


    {
        uint8_t x = 187;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        g16 = 10318;
        if (read_g16() != 10318) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {248,140,48565,149};
        if (s.b != (uint8_t)140) failures++;
    }


    {
        uint32_t a = 1059460310UL;
        uint32_t b = 4204942075UL;
        uint32_t r = a & b;
        if (r != 975311058UL) failures++;
    }


    {
        int8_t a = -89;
        int8_t b = -63;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {170,157,4,184,36,103};
        if (a[3] != 184) failures++;
    }


    {
        uint16_t x = 17;
        x = x + 85;
        if (x != 102) failures++;
    }


    {
        volatile int16_t a = 22617;
        volatile int16_t b = -16700;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 720052785UL;
        uint32_t b = 299107214UL;
        uint32_t r = a | b;
        if (r != 1006576575UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t a[6] = {4,105,231,254,184,171};
        if (a[1] != 105) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(210,90) != 300) failures++;
    }


    {
        uint8_t x = 154;
        x <<= 4;
        if (x != 160) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 0: result = 123; break;
        case 4: result = 218; break;
        case 19: result = 240; break;
        case 17: result = 92; break;
        case 7: result = 45; break;
        default: result = 250; break;
        }
        if (result != 218) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {20,230,42010,83};
        if (s.a != (uint8_t)20) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {242,71,25690,185};
        if (s.d != (uint8_t)185) failures++;
    }


    {
        uint16_t x = 153;
        x = x + 10;
        if (x != 163) failures++;
    }


    {
        uint16_t r = add2(93,129) + add2(129,155) + add2(93,155);
        if (r != 754) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)177) + (uint16_t)29619;
        if (r != 29796) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {187,135,27202,243};
        if (s.a != (uint8_t)187) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 4115047458UL;
        uint32_t b = 1307949344UL;
        uint32_t r = a | b;
        if (r != 4260871458UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(102,199) != 65439) failures++;
    }


    {
        uint8_t m[2][2] = {{166,245},{248,189}};
        if (m[1][1] != 189) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {173,253,32788,143};
        if (s.b != (uint8_t)253) failures++;
    }


    {
        g16 = 20545;
        if (read_g16() != 20545) failures++;
    }


    {
        uint8_t v = 187;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 144;
        uint8_t r = port;
        if (r != 144) failures++;
    }


    {
        uint8_t a[6] = {146,33,59,219,13,232};
        if (a[3] != 219) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 224;
        if (buf[11] != 224) failures++;
    }


    {
        uint8_t a[6] = {66,115,175,44,171,198};
        if (a[3] != 44) failures++;
    }


    {
        uint32_t a = 1391356071UL;
        uint32_t b = 3613872909UL;
        uint32_t r = a - b;
        if (r != 2072450458UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)113) + (uint16_t)911;
        if (r != 1024) failures++;
    }


    {
        uint8_t x = 24;
        x <<= 4;
        if (x != 128) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)21) / (int16_t)((int8_t)-11);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t m[2][4] = {{45,224,74,243},{226,93,109,243}};
        if (m[1][0] != 226) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        volatile int16_t a = 28349;
        volatile int16_t b = 4300;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 1: result = 245; break;
        case 5: result = 110; break;
        case 15: result = 92; break;
        case 12: result = 126; break;
        case 18: result = 195; break;
        case 6: result = 194; break;
        case 3: result = 103; break;
        case 2: result = 34; break;
        default: result = 188; break;
        }
        if (result != 34) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 18: result = 67; break;
        case 9: result = 224; break;
        case 3: result = 246; break;
        default: result = 25; break;
        }
        if (result != 224) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t x = 183;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t r = add2(100,0) + add2(0,17) + add2(100,17);
        if (r != 234) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 5: result = 136; break;
        case 13: result = 232; break;
        case 17: result = 232; break;
        default: result = 66; break;
        }
        if (result != 232) failures++;
    }


    {
        uint16_t r = add2(0,140) + add2(140,170) + add2(0,170);
        if (r != 620) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 254;
        if (buf[5] != 254) failures++;
    }


    {
        uint8_t x = 171;
        x <<= 1;
        if (x != 86) failures++;
    }


    {
        uint8_t m[2][4] = {{154,82,159,48},{89,12,80,160}};
        if (m[1][3] != 160) failures++;
    }


    {
        uint8_t buf[8] = {43,7,187,137,95,201,173,98};
        uint8_t *p = buf;
        p += 6;
        if (*p != 173) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(228,187) != 415) failures++;
    }


    {
        uint8_t src[6] = {35,95,8,72,127,4};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[3] != 72) failures++;
    }


    {
        g16 = 63976;
        if (read_g16() != 63976) failures++;
    }


    {
        uint8_t v = 174;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t a[6] = {98,72,80,136,186,157};
        if (a[0] != 98) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 173;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 19) failures++;
    }


    {
        uint8_t a[6] = {43,14,118,163,156,157};
        if (a[2] != 118) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)((137 & (55 ^ 6)) & 244)) != 0) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[4][2] = {{218,122},{160,200},{128,152},{37,124}};
        if (m[1][1] != 200) failures++;
    }


    {
        uint8_t a[6] = {2,9,111,155,126,46};
        if (a[4] != 126) failures++;
    }


    {
        uint16_t r = add2(149,111) + add2(111,183) + add2(149,183);
        if (r != 886) failures++;
    }


    {
        int8_t a = 26;
        int8_t b = 65;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {121,191,238,25,66,87};
        if (a[2] != 238) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {147,189,2359,24};
        if (s.b != (uint8_t)189) failures++;
    }


    {
        uint8_t v = 64;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 64) failures++;
    }


    {
        uint8_t v = 33;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t src[16] = {251,132,235,42,200,174,117,235,252,21,235,253,239,210,220,110};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[10] != 235) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(99,156) != 65479) failures++;
    }


    {
        uint8_t a[6] = {169,197,11,81,106,140};
        if (a[4] != 106) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 228;
        x = x + 23;
        if (x != 251) failures++;
    }


    {
        uint16_t r = add2(247,88) + add2(88,241) + add2(247,241);
        if (r != 1152) failures++;
    }


    {
        uint32_t a = 7650670UL;
        uint32_t b = 812763936UL;
        uint32_t r = a - b;
        if (r != 3489854030UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 1) sum += j;
        if (sum != 105) failures++;
    }


    {
        uint8_t v = 83;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t v = 226;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 19151;
        if (read_g16() != 19151) failures++;
    }


    {
        uint8_t src[3] = {185,181,161};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 185) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(192,199) != 65529) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 34;
        if (buf[6] != 34) failures++;
    }


    {
        uint8_t a[6] = {179,141,241,199,127,176};
        if (a[5] != 176) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(133,166) != 65503) failures++;
    }


    {
        if (((uint16_t)(((123 + 151) + (197 + 92)) - (42 - (215 ^ 56)))) != 760) failures++;
    }


    {
        uint8_t v = 40;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        g16 = 4834;
        if (read_g16() != 4834) failures++;
    }


    {
        volatile uint8_t port = 144;
        uint8_t r = port;
        if (r != 144) failures++;
    }


    {
        uint8_t a[6] = {181,190,229,20,43,114};
        if (a[2] != 229) failures++;
    }


    {
        uint8_t a[6] = {179,183,212,240,125,61};
        if (a[3] != 240) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 198;
        if (buf[1] != 198) failures++;
    }


    {
        uint8_t a[6] = {111,63,9,25,96,112};
        if (a[4] != 96) failures++;
    }


    {
        uint8_t a[6] = {9,152,69,131,139,42};
        if (a[3] != 131) failures++;
    }


    {
        uint16_t r = add2(79,207) + add2(207,228) + add2(79,228);
        if (r != 1028) failures++;
    }


    {
        uint16_t r = 19359 + 1965 + 38296 + 27713 + 30665 + 53335 + 44150 + 30975;
        if (r != 49850) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 175;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint8_t v = 232;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 163;
        uint8_t r = port;
        if (r != 163) failures++;
    }


    {
        uint8_t m[2][2] = {{137,63},{99,230}};
        if (m[0][1] != 63) failures++;
    }


    {
        volatile uint8_t port = 235;
        uint8_t r = port;
        if (r != 235) failures++;
    }


    {
        volatile uint8_t port = 220;
        uint8_t r = port;
        if (r != 220) failures++;
    }


    {
        uint8_t src[13] = {39,58,180,75,167,125,59,82,52,129,121,166,150};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[6] != 59) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 145;
        if (buf[5] != 145) failures++;
    }


    {
        uint8_t src[1] = {189};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 189) failures++;
    }


    {
        volatile int16_t a = -13934;
        volatile int16_t b = -25965;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 27531 + 17356 + 49637 + 58490 + 42742 + 36327 + 22959 + 2733;
        if (r != 61167) failures++;
    }


    {
        uint16_t r = add2(83,45) + add2(45,114) + add2(83,114);
        if (r != 484) failures++;
    }


    {
        uint16_t x = 255;
        x = x + 255;
        if (x != 510) failures++;
    }


    {
        uint16_t x = 135;
        x = x + 24;
        if (x != 159) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[15] = {112,156,57,14,24,202,99,134,29,77,139,150,243,35,227};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[6] != 99) failures++;
    }


    {
        if (((uint16_t)((25 + 122) + ((75 ^ 191) & (157 ^ 212)))) != 211) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-116) % (int16_t)((int8_t)46);
        if ((uint16_t)r != (uint16_t)65512) failures++;
    }


    {
        uint16_t x = 194;
        x = x + 95;
        if (x != 289) failures++;
    }


    {
        g16 = 33055;
        if (read_g16() != 33055) failures++;
    }


    {
        volatile uint8_t port = 107;
        uint8_t r = port;
        if (r != 107) failures++;
    }


    {
        uint32_t a = 4113762743UL;
        uint32_t b = 1794926558UL;
        uint32_t r = a - b;
        if (r != 2318836185UL) failures++;
    }


    {
        int8_t a = 47;
        int8_t b = -43;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(((231 & 101) & (1 + 136)) | 183)) != 183) failures++;
    }


    {
        uint8_t v = 45;
        v ^= 2;
        if (v != 47) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(26,46) != 65516) failures++;
    }


    {
        uint16_t x = 228;
        x = x + 77;
        if (x != 305) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(68,17) != 51) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(66,173) != 239) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint16_t r = 31213 + 45804 + 31399 + 13639 + 64757 + 20652 + 61574 + 55280;
        if (r != 62174) failures++;
    }


    {
        uint8_t src[16] = {56,253,1,114,58,204,255,47,50,234,91,31,95,96,189,72};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[8] != 50) failures++;
    }


    {
        uint16_t x = 51470;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 174;
        if (buf[0] != 174) failures++;
    }


    {
        uint8_t src[11] = {214,168,239,34,78,60,93,49,109,199,222};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[2] != 239) failures++;
    }


    {
        uint8_t buf[8] = {215,234,144,193,52,108,181,166};
        uint8_t *p = buf;
        p += 3;
        if (*p != 193) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 4: result = 26; break;
        case 17: result = 160; break;
        case 13: result = 174; break;
        case 8: result = 59; break;
        default: result = 177; break;
        }
        if (result != 160) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)208) + (uint16_t)62615;
        if (r != 62823) failures++;
    }


    {
        uint16_t x = 188;
        x = x + 81;
        if (x != 269) failures++;
    }


    {
        uint16_t r = add2(81,150) + add2(150,171) + add2(81,171);
        if (r != 804) failures++;
    }


    {
        g16 = 34223;
        if (read_g16() != 34223) failures++;
    }


    {
        uint8_t v = 82;
        v |= 128;
        if (v != 210) failures++;
    }


    {
        volatile int16_t a = 29081;
        volatile int16_t b = -30025;
        int r = (a >= b);
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
        int8_t a = 103;
        int8_t b = -32;
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
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(171,214) != 65493) failures++;
    }


    {
        uint8_t v = 248;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 15;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 113) failures++;
    }


    {
        uint32_t a = 3984706135UL;
        uint32_t b = 2216715082UL;
        uint32_t r = a | b;
        if (r != 3986811743UL) failures++;
    }


    {
        uint8_t m[3][2] = {{173,248},{73,99},{69,246}};
        if (m[2][0] != 69) failures++;
    }


    {
        g16 = 55825;
        if (read_g16() != 55825) failures++;
    }


    {
        if (((uint16_t)(8 ^ ((202 + 63) ^ (12 | 167)))) != 430) failures++;
    }


    {
        uint32_t a = 4124313929UL;
        uint32_t b = 3253177603UL;
        uint32_t r = a + b;
        if (r != 3082524236UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {73,173,28749,4};
        if (s.b != (uint8_t)173) failures++;
    }


    {
        uint16_t r = call6(164,162,142,4,183,129);
        if (r != 784) failures++;
    }


    {
        g16 = 34867;
        if (read_g16() != 34867) failures++;
    }

    return failures;
}