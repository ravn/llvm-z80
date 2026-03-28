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
        uint8_t src[6] = {152,44,207,106,60,122};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[3] != 106) failures++;
    }


    {
        uint8_t src[15] = {207,226,15,30,46,15,169,3,199,18,219,42,56,74,228};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[6] != 169) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 2: result = 221; break;
        case 10: result = 140; break;
        case 16: result = 41; break;
        case 19: result = 191; break;
        default: result = 34; break;
        }
        if (result != 140) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(((105 ^ 146) ^ (163 + 242)) + (224 + (55 | 28)))) != 653) failures++;
    }


    {
        uint8_t x = 230;
        x <<= 2;
        if (x != 152) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 0: result = 183; break;
        case 15: result = 179; break;
        case 3: result = 139; break;
        case 7: result = 128; break;
        case 14: result = 234; break;
        case 12: result = 19; break;
        case 8: result = 29; break;
        default: result = 175; break;
        }
        if (result != 19) failures++;
    }


    {
        g16 = 55682;
        if (read_g16() != 55682) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        g16 = 22766;
        if (read_g16() != 22766) failures++;
    }


    {
        uint8_t m[2][2] = {{51,56},{70,148}};
        if (m[0][1] != 56) failures++;
    }


    {
        if (((uint16_t)(((122 ^ 244) & 28) + ((229 | 40) - (234 | 23)))) != 65530) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 102;
        if (buf[1] != 102) failures++;
    }


    {
        int8_t a = -26;
        int8_t b = 4;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-31) % (int16_t)((int8_t)-85);
        if ((uint16_t)r != (uint16_t)65505) failures++;
    }


    {
        volatile int16_t a = 27813;
        volatile int16_t b = 10388;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)142) + (uint16_t)42946;
        if (r != 43088) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 172;
        if (buf[4] != 172) failures++;
    }


    {
        uint32_t a = 1504089545UL;
        uint32_t b = 79150397UL;
        uint32_t r = a - b;
        if (r != 1424939148UL) failures++;
    }


    {
        uint8_t a[6] = {9,175,235,183,184,189};
        if (a[5] != 189) failures++;
    }


    {
        uint16_t x = 112;
        x = x + 104;
        if (x != 216) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)32) % (int16_t)((int8_t)4);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(137,180) != 65493) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 154;
        if (buf[4] != 154) failures++;
    }


    {
        volatile uint8_t port = 25;
        uint8_t r = port;
        if (r != 25) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)6) + (uint16_t)52837;
        if (r != 52843) failures++;
    }


    {
        uint16_t r = 9306 + 687 + 14162 + 39390 + 49145 + 31301 + 57586 + 39171;
        if (r != 44140) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {136,250,56126,239};
        if (s.b != (uint8_t)250) failures++;
    }


    {
        uint8_t m[3][2] = {{108,186},{93,142},{67,110}};
        if (m[0][0] != 108) failures++;
    }


    {
        uint32_t a = 1567422125UL;
        uint32_t b = 1999638468UL;
        uint32_t r = a + b;
        if (r != 3567060593UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 57470 + 32137 + 50953 + 22162 + 12240 + 22094 + 42427 + 9086;
        if (r != 51961) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)58) + (uint16_t)14149;
        if (r != 14207) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {62,22,40226,194};
        if (s.a != (uint8_t)62) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 161;
        if (buf[4] != 161) failures++;
    }


    {
        uint8_t src[13] = {1,167,200,149,195,87,111,223,202,71,94,11,38};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[4] != 195) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        int8_t a = 51;
        int8_t b = 94;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 59186;
        if (read_g16() != 59186) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 31;
        if (buf[9] != 31) failures++;
    }


    {
        uint8_t buf[8] = {74,149,71,225,96,96,240,128};
        uint8_t *p = buf;
        p += 6;
        if (*p != 240) failures++;
    }


    {
        uint8_t m[4][3] = {{166,196,68},{86,33,33},{112,235,141},{151,253,226}};
        if (m[1][1] != 33) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 78;
        if (buf[14] != 78) failures++;
    }


    {
        volatile uint8_t port = 72;
        uint8_t r = port;
        if (r != 72) failures++;
    }


    {
        uint8_t a[6] = {82,67,122,41,8,41};
        if (a[1] != 67) failures++;
    }


    {
        uint8_t a[6] = {61,162,62,144,168,251};
        if (a[2] != 62) failures++;
    }


    {
        g16 = 39658;
        if (read_g16() != 39658) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)45) + (uint16_t)42311;
        if (r != 42356) failures++;
    }


    {
        uint8_t a[6] = {91,150,75,12,171,170};
        if (a[1] != 150) failures++;
    }


    {
        uint8_t x = 21;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t src[2] = {253,224};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 253) failures++;
    }


    {
        uint8_t v = 22;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 413610891UL;
        uint32_t b = 3419268541UL;
        uint32_t r = a - b;
        if (r != 1289309646UL) failures++;
    }


    {
        uint8_t v = 14;
        v |= 2;
        if (v != 14) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 18: result = 181; break;
        case 16: result = 183; break;
        case 0: result = 255; break;
        case 6: result = 159; break;
        case 19: result = 24; break;
        case 11: result = 80; break;
        case 5: result = 21; break;
        case 3: result = 237; break;
        default: result = 102; break;
        }
        if (result != 183) failures++;
    }


    {
        uint16_t r = 42270 + 26799 + 42147 + 62532 + 21770 + 50139 + 1453 + 45899;
        if (r != 30865) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {72,207,60,110,64,59,216,159};
        uint8_t *p = buf;
        p += 3;
        if (*p != 110) failures++;
    }


    {
        uint8_t src[2] = {52,117};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 52) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {215,9,32798,202};
        if (s.c != (uint16_t)32798) failures++;
    }


    {
        uint8_t src[1] = {115};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 115) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint16_t r = add2(32,2) + add2(2,244) + add2(32,244);
        if (r != 556) failures++;
    }


    {
        uint16_t x = 31770;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint16_t r = call6(60,194,242,28,120,169);
        if (r != 813) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 3: result = 147; break;
        case 11: result = 107; break;
        case 7: result = 191; break;
        case 0: result = 114; break;
        case 8: result = 222; break;
        case 18: result = 106; break;
        case 13: result = 9; break;
        case 19: result = 138; break;
        default: result = 234; break;
        }
        if (result != 106) failures++;
    }


    {
        uint8_t x = 108;
        x <<= 1;
        if (x != 216) failures++;
    }


    {
        uint8_t v = 114;
        v &= ~(uint8_t)16;
        if (v != 98) failures++;
    }


    {
        int8_t a = 126;
        int8_t b = 111;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(66,137) + add2(137,76) + add2(66,76);
        if (r != 558) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 2) sum += j;
        if (sum != 42) failures++;
    }


    {
        uint16_t x = 7;
        x = x + 12;
        if (x != 19) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-32) / (int16_t)((int8_t)-76);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t x = 70;
        x <<= 0;
        if (x != 70) failures++;
    }


    {
        uint8_t v = 45;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 1757;
        if (read_g16() != 1757) failures++;
    }


    {
        if (((uint16_t)(((245 ^ 162) - 5) + 193)) != 275) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {180,153,27216,166};
        if (s.a != (uint8_t)180) failures++;
    }


    {
        uint8_t src[5] = {215,108,18,135,219};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 215) failures++;
    }


    {
        uint16_t r = call6(125,148,84,221,68,61);
        if (r != 707) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 41;
        if (buf[1] != 41) failures++;
    }


    {
        uint8_t src[15] = {73,186,100,24,227,131,149,187,61,155,65,236,37,194,200};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[2] != 100) failures++;
    }


    {
        uint16_t r = 59129 + 54972 + 54303 + 47726 + 31636 + 38288 + 30894 + 11195;
        if (r != 463) failures++;
    }


    {
        uint8_t a[6] = {48,211,41,122,172,226};
        if (a[5] != 226) failures++;
    }


    {
        uint32_t a = 3673146128UL;
        uint32_t b = 2859714763UL;
        uint32_t r = a | b;
        if (r != 4211067867UL) failures++;
    }


    {
        uint8_t v = 167;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(7,163) + add2(163,11) + add2(7,11);
        if (r != 362) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {110,52,86,74,31,151};
        if (a[2] != 86) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        g16 = 17460;
        if (read_g16() != 17460) failures++;
    }


    {
        volatile int16_t a = 19314;
        volatile int16_t b = 1009;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint16_t r = 22656 + 42401 + 19712 + 52809 + 6885 + 2922 + 4272 + 16816;
        if (r != 37401) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(66,56) != 122) failures++;
    }


    {
        volatile int16_t a = 2886;
        volatile int16_t b = 29481;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        if (((uint16_t)(((150 ^ 152) + 234) ^ ((206 ^ 161) - 234))) != 65405) failures++;
    }


    {
        uint8_t v = 165;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {148,83,50430,174};
        if (s.b != (uint8_t)83) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(62,203) != 65395) failures++;
    }


    {
        uint8_t v = 249;
        v ^= 4;
        if (v != 253) failures++;
    }


    {
        uint8_t x = 54;
        x <<= 0;
        if (x != 54) failures++;
    }


    {
        volatile uint8_t port = 158;
        uint8_t r = port;
        if (r != 158) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 187;
        if (buf[2] != 187) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {194,72,94,89,213,192,133,252};
        uint8_t *p = buf;
        p += 0;
        if (*p != 194) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)126) + (uint16_t)23292;
        if (r != 23418) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint32_t a = 143463137UL;
        uint32_t b = 2113262390UL;
        uint32_t r = a | b;
        if (r != 2113786871UL) failures++;
    }


    {
        if (((uint16_t)128) != 128) failures++;
    }


    {
        int8_t a = -104;
        int8_t b = 75;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {241,21,32371,62};
        if (s.c != (uint16_t)32371) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-30) / (int16_t)((int8_t)118);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {29,157,99,235,152,38};
        if (a[4] != 152) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 65;
        if (buf[3] != 65) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        g16 = 4951;
        if (read_g16() != 4951) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 10: result = 142; break;
        case 6: result = 14; break;
        case 8: result = 17; break;
        case 1: result = 247; break;
        case 3: result = 23; break;
        case 19: result = 95; break;
        default: result = 196; break;
        }
        if (result != 247) failures++;
    }


    {
        uint16_t r = add2(155,134) + add2(134,72) + add2(155,72);
        if (r != 722) failures++;
    }


    {
        uint16_t x = 38721;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 41605;
        if (read_g16() != 41605) failures++;
    }


    {
        volatile int16_t a = -22827;
        volatile int16_t b = -28628;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {26,101,63328,91};
        if (s.a != (uint8_t)26) failures++;
    }


    {
        uint8_t buf[8] = {196,167,53,54,57,68,124,101};
        uint8_t *p = buf;
        p += 3;
        if (*p != 54) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(206,99) != 107) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(33,93) != 126) failures++;
    }


    {
        if (((uint16_t)18) != 18) failures++;
    }


    {
        uint16_t x = 6839;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 3: result = 55; break;
        case 8: result = 74; break;
        case 5: result = 6; break;
        case 2: result = 190; break;
        default: result = 3; break;
        }
        if (result != 190) failures++;
    }


    {
        int8_t a = -105;
        int8_t b = -3;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -67;
        int8_t b = 24;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 140;
        x <<= 2;
        if (x != 48) failures++;
    }


    {
        volatile uint8_t port = 68;
        uint8_t r = port;
        if (r != 68) failures++;
    }


    {
        uint16_t x = 2;
        x = x + 86;
        if (x != 88) failures++;
    }


    {
        uint16_t r = call6(180,203,230,114,75,227);
        if (r != 1029) failures++;
    }


    {
        uint16_t r = add2(25,133) + add2(133,206) + add2(25,206);
        if (r != 728) failures++;
    }


    {
        uint8_t m[4][2] = {{134,233},{123,179},{77,209},{24,135}};
        if (m[1][1] != 179) failures++;
    }


    {
        uint8_t src[6] = {4,31,202,94,37,126};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[0] != 4) failures++;
    }


    {
        uint8_t a[6] = {126,24,151,30,244,201};
        if (a[3] != 30) failures++;
    }


    {
        uint16_t x = 154;
        x = x + 180;
        if (x != 334) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {39,220,14777,95};
        if (s.a != (uint8_t)39) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        uint8_t v = 192;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {180,105,51,61,147,227};
        if (a[2] != 51) failures++;
    }


    {
        g16 = 3186;
        if (read_g16() != 3186) failures++;
    }


    {
        uint8_t m[4][2] = {{43,44},{236,179},{207,211},{144,25}};
        if (m[0][1] != 44) failures++;
    }


    {
        uint16_t x = 5883;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {115,187,110,126,179,146};
        if (a[0] != 115) failures++;
    }


    {
        uint16_t x = 15643;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 199;
        if (buf[0] != 199) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)100) % (int16_t)((int8_t)-56);
        if ((uint16_t)r != (uint16_t)44) failures++;
    }


    {
        volatile uint8_t port = 165;
        uint8_t r = port;
        if (r != 165) failures++;
    }


    {
        uint8_t v = 215;
        v ^= 8;
        if (v != 223) failures++;
    }


    {
        uint8_t m[4][2] = {{61,139},{175,85},{144,174},{198,229}};
        if (m[1][1] != 85) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(99,219) != 318) failures++;
    }


    {
        uint32_t a = 13355157UL;
        uint32_t b = 2093678878UL;
        uint32_t r = a + b;
        if (r != 2107034035UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {237,76,28349,124};
        if (s.d != (uint8_t)124) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 56;
        if (buf[7] != 56) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(125,183) != 65478) failures++;
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
        uint8_t src[5] = {61,45,122,64,88};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 122) failures++;
    }


    {
        uint16_t x = 40347;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {113,11,12424,102};
        if (s.a != (uint8_t)113) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 200;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 8) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)49) % (int16_t)((int8_t)70);
        if ((uint16_t)r != (uint16_t)49) failures++;
    }


    {
        volatile int16_t a = 14;
        volatile int16_t b = 21205;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 3: result = 169; break;
        case 16: result = 241; break;
        case 4: result = 148; break;
        case 12: result = 215; break;
        case 14: result = 34; break;
        default: result = 232; break;
        }
        if (result != 232) failures++;
    }


    {
        int8_t a = -25;
        int8_t b = 76;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -101;
        int8_t b = 102;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        g16 = 12014;
        if (read_g16() != 12014) failures++;
    }


    {
        uint16_t x = 58884;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 92;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {246,1,127,41,176,176,26,92};
        uint8_t *p = buf;
        p += 4;
        if (*p != 176) failures++;
    }


    {
        uint8_t a[6] = {182,164,171,20,47,153};
        if (a[3] != 20) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint16_t r = call6(202,20,204,243,159,48);
        if (r != 876) failures++;
    }


    {
        uint8_t m[3][3] = {{172,69,115},{100,244,99},{180,182,162}};
        if (m[0][2] != 115) failures++;
    }


    {
        uint16_t r = 8379 + 16294 + 17070 + 20882 + 42613 + 57517 + 58416 + 48805;
        if (r != 7832) failures++;
    }


    {
        int8_t a = 49;
        int8_t b = 126;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][2] = {{109,81},{46,226},{107,45},{136,122}};
        if (m[3][1] != 122) failures++;
    }


    {
        uint8_t v = 76;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)135) + (uint16_t)39751;
        if (r != 39886) failures++;
    }


    {
        uint8_t v = 117;
        v &= ~(uint8_t)4;
        if (v != 113) failures++;
    }


    {
        uint8_t x = 202;
        x <<= 2;
        if (x != 40) failures++;
    }


    {
        uint16_t r = call6(59,187,232,46,169,192);
        if (r != 885) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t a[6] = {33,193,106,195,113,204};
        if (a[5] != 204) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t v = 84;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)40) / (int16_t)((int8_t)88);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 34;
        x = x + 248;
        if (x != 282) failures++;
    }


    {
        volatile int16_t a = -31282;
        volatile int16_t b = -742;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)((89 - (107 ^ 161)) ^ 120)) != 65527) failures++;
    }


    {
        uint8_t v = 87;
        v |= 32;
        if (v != 119) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(61,204) != 65393) failures++;
    }


    {
        volatile uint8_t port = 0;
        uint8_t r = port;
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)49) != 49) failures++;
    }


    {
        volatile int16_t a = 1686;
        volatile int16_t b = -15058;
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
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 42239;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(48,114,9,48,182,161);
        if (r != 562) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 49;
        if (buf[0] != 49) failures++;
    }


    {
        uint8_t v = 82;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {34,235,252,134,227,88};
        if (a[5] != 88) failures++;
    }


    {
        uint16_t r = 62356 + 48154 + 1812 + 39913 + 25783 + 58073 + 35625 + 36125;
        if (r != 45697) failures++;
    }


    {
        g16 = 40570;
        if (read_g16() != 40570) failures++;
    }


    {
        volatile int16_t a = 16184;
        volatile int16_t b = -27414;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 150;
        uint8_t r = port;
        if (r != 150) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(233,150) != 83) failures++;
    }


    {
        volatile uint8_t port = 226;
        uint8_t r = port;
        if (r != 226) failures++;
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
        for (uint16_t j = 0; j < 18; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t m[4][2] = {{228,236},{246,4},{208,114},{81,32}};
        if (m[3][0] != 81) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 154;
        x = x + 118;
        if (x != 272) failures++;
    }


    {
        volatile uint8_t port = 242;
        uint8_t r = port;
        if (r != 242) failures++;
    }


    {
        int8_t a = -50;
        int8_t b = 82;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{107,71,6},{81,46,25},{225,26,109},{132,47,154}};
        if (m[1][0] != 81) failures++;
    }


    {
        uint16_t r = add2(31,81) + add2(81,45) + add2(31,45);
        if (r != 314) failures++;
    }


    {
        int8_t a = 69;
        int8_t b = -86;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        g16 = 47205;
        if (read_g16() != 47205) failures++;
    }


    {
        uint8_t v = 75;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 53) failures++;
    }


    {
        uint16_t x = 254;
        x = x + 179;
        if (x != 433) failures++;
    }


    {
        uint8_t buf[8] = {96,162,205,92,219,173,67,136};
        uint8_t *p = buf;
        p += 1;
        if (*p != 162) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)124) + (uint16_t)11576;
        if (r != 11700) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(30,160) != 190) failures++;
    }


    {
        uint16_t x = 174;
        x = x + 214;
        if (x != 388) failures++;
    }


    {
        uint8_t x = 248;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 14: result = 27; break;
        case 15: result = 51; break;
        case 3: result = 66; break;
        case 12: result = 33; break;
        case 19: result = 61; break;
        case 16: result = 39; break;
        case 8: result = 172; break;
        case 0: result = 67; break;
        default: result = 96; break;
        }
        if (result != 39) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        volatile int16_t a = 14222;
        volatile int16_t b = -17431;
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
        uint16_t r = call6(67,111,40,38,214,119);
        if (r != 589) failures++;
    }


    {
        uint8_t src[14] = {75,204,196,21,24,207,15,240,32,109,203,210,167,194};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[10] != 203) failures++;
    }


    {
        g16 = 8336;
        if (read_g16() != 8336) failures++;
    }


    {
        uint16_t r = call6(178,145,232,183,1,73);
        if (r != 812) failures++;
    }


    {
        uint8_t a[6] = {183,40,159,85,153,229};
        if (a[0] != 183) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)20) / (int16_t)((int8_t)-10);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        volatile int16_t a = -29367;
        volatile int16_t b = 32611;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 14: result = 80; break;
        case 11: result = 94; break;
        case 17: result = 34; break;
        case 15: result = 78; break;
        default: result = 23; break;
        }
        if (result != 23) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)74) % (int16_t)((int8_t)-69);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        uint8_t a[6] = {139,1,248,10,88,231};
        if (a[0] != 139) failures++;
    }


    {
        uint8_t src[3] = {169,96,19};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 96) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(208,20) != 228) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(151,92) != 243) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t src[11] = {199,63,195,204,24,230,137,231,20,2,5};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[2] != 195) failures++;
    }


    {
        uint32_t a = 714015407UL;
        uint32_t b = 1651923766UL;
        uint32_t r = a & b;
        if (r != 570819110UL) failures++;
    }


    {
        int8_t a = -64;
        int8_t b = -22;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 195;
        x = x + 239;
        if (x != 434) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-48) / (int16_t)((int8_t)96);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t src[5] = {170,164,145,254,146};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[1] != 164) failures++;
    }


    {
        uint8_t m[2][2] = {{106,128},{144,170}};
        if (m[0][0] != 106) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(43,34) != 77) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)209) + (uint16_t)38916;
        if (r != 39125) failures++;
    }


    {
        uint16_t x = 39107;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(((219 & 173) - 88) + 196)) != 245) failures++;
    }


    {
        uint8_t buf[8] = {72,253,196,90,161,204,216,72};
        uint8_t *p = buf;
        p += 2;
        if (*p != 196) failures++;
    }


    {
        volatile int16_t a = 24378;
        volatile int16_t b = -24296;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(232,242) + add2(242,60) + add2(232,60);
        if (r != 1068) failures++;
    }


    {
        uint16_t r = add2(42,5) + add2(5,128) + add2(42,128);
        if (r != 350) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {234,126,952,220};
        if (s.a != (uint8_t)234) failures++;
    }


    {
        g16 = 42747;
        if (read_g16() != 42747) failures++;
    }


    {
        uint16_t x = 33997;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)68) + (uint16_t)7378;
        if (r != 7446) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {54,46,14546,18};
        if (s.b != (uint8_t)46) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 14: result = 0; break;
        case 11: result = 81; break;
        case 12: result = 67; break;
        case 1: result = 38; break;
        case 9: result = 26; break;
        case 15: result = 146; break;
        case 17: result = 31; break;
        default: result = 161; break;
        }
        if (result != 38) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)73) / (int16_t)((int8_t)103);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 5: result = 1; break;
        case 4: result = 173; break;
        case 15: result = 91; break;
        case 10: result = 233; break;
        case 14: result = 131; break;
        default: result = 129; break;
        }
        if (result != 129) failures++;
    }


    {
        uint16_t x = 215;
        x = x + 211;
        if (x != 426) failures++;
    }


    {
        uint16_t x = 163;
        x = x + 236;
        if (x != 399) failures++;
    }


    {
        uint32_t a = 336076287UL;
        uint32_t b = 562089848UL;
        uint32_t r = a & b;
        if (r != 3448UL) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 107;
        if (buf[1] != 107) failures++;
    }


    {
        uint8_t v = 116;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 12) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-100) % (int16_t)((int8_t)-84);
        if ((uint16_t)r != (uint16_t)65520) failures++;
    }


    {
        uint8_t v = 45;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 10: result = 20; break;
        case 1: result = 10; break;
        case 3: result = 159; break;
        default: result = 119; break;
        }
        if (result != 119) failures++;
    }


    {
        uint16_t r = call6(218,68,90,75,184,187);
        if (r != 822) failures++;
    }


    {
        uint16_t x = 220;
        x = x + 197;
        if (x != 417) failures++;
    }


    {
        if (((uint16_t)(((174 - 172) + 115) ^ ((9 ^ 215) + (235 + 177)))) != 527) failures++;
    }


    {
        if (((uint16_t)40) != 40) failures++;
    }


    {
        uint32_t a = 3478476563UL;
        uint32_t b = 3615682988UL;
        uint32_t r = a & b;
        if (r != 3338686720UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)243) + (uint16_t)36605;
        if (r != 36848) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {137,15,30231,133};
        if (s.a != (uint8_t)137) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 178;
        if (buf[10] != 178) failures++;
    }


    {
        uint8_t a[6] = {8,213,213,105,251,122};
        if (a[5] != 122) failures++;
    }


    {
        uint8_t buf[8] = {196,132,52,53,93,10,38,89};
        uint8_t *p = buf;
        p += 2;
        if (*p != 52) failures++;
    }


    {
        volatile uint8_t port = 80;
        uint8_t r = port;
        if (r != 80) failures++;
    }


    {
        uint8_t x = 28;
        x <<= 4;
        if (x != 192) failures++;
    }


    {
        uint8_t src[3] = {41,228,128};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 41) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(244,125) != 369) failures++;
    }


    {
        volatile int16_t a = 1986;
        volatile int16_t b = -16888;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        int8_t a = 13;
        int8_t b = 3;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 14990;
        if (read_g16() != 14990) failures++;
    }


    {
        uint8_t buf[8] = {32,222,39,162,63,110,38,144};
        uint8_t *p = buf;
        p += 2;
        if (*p != 39) failures++;
    }


    {
        uint16_t r = add2(151,39) + add2(39,42) + add2(151,42);
        if (r != 464) failures++;
    }


    {
        if (((uint16_t)(215 + 200)) != 415) failures++;
    }


    {
        uint8_t src[8] = {156,129,204,63,181,42,171,133};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[7] != 133) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)132) + (uint16_t)11936;
        if (r != 12068) failures++;
    }


    {
        uint8_t v = 255;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 33) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(184,14) != 198) failures++;
    }


    {
        uint16_t x = 94;
        x = x + 33;
        if (x != 127) failures++;
    }


    {
        uint8_t v = 132;
        int r = (v & 4) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-49) / (int16_t)((int8_t)43);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 1) sum += j;
        if (sum != 21) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {200,23,59205,227};
        if (s.a != (uint8_t)200) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 16: result = 20; break;
        case 10: result = 8; break;
        case 12: result = 58; break;
        case 2: result = 80; break;
        case 15: result = 28; break;
        case 0: result = 175; break;
        default: result = 111; break;
        }
        if (result != 8) failures++;
    }


    {
        uint16_t r = 52844 + 701 + 43569 + 30748 + 57783 + 21560 + 22994 + 27680;
        if (r != 61271) failures++;
    }


    {
        uint16_t r = call6(168,126,40,214,77,195);
        if (r != 820) failures++;
    }


    {
        uint16_t r = 39638 + 54091 + 63789 + 7768 + 4000 + 51202 + 22380 + 30226;
        if (r != 10950) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {46,60,21504,72};
        if (s.b != (uint8_t)60) failures++;
    }


    {
        if (((uint16_t)(((60 ^ 70) ^ (126 - 21)) ^ ((174 & 181) | (96 + 247)))) != 484) failures++;
    }


    {
        uint16_t x = 8715;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 35608 + 63168 + 54741 + 42656 + 32723 + 39625 + 62475 + 48963;
        if (r != 52279) failures++;
    }


    {
        uint16_t r = call6(229,238,176,89,60,181);
        if (r != 973) failures++;
    }


    {
        volatile uint8_t port = 32;
        uint8_t r = port;
        if (r != 32) failures++;
    }


    {
        uint8_t v = 227;
        v |= 4;
        if (v != 231) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 1) sum += j;
        if (sum != 3) failures++;
    }


    {
        volatile int16_t a = 29469;
        volatile int16_t b = 13105;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[1] = {55};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 55) failures++;
    }


    {
        uint16_t r = add2(197,248) + add2(248,227) + add2(197,227);
        if (r != 1344) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(143,132) != 11) failures++;
    }


    {
        uint16_t r = call6(66,29,167,93,164,231);
        if (r != 750) failures++;
    }


    {
        uint16_t r = add2(221,169) + add2(169,48) + add2(221,48);
        if (r != 876) failures++;
    }


    {
        uint8_t v = 214;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {171,233,721,48};
        if (s.d != (uint8_t)48) failures++;
    }


    {
        uint8_t x = 149;
        x <<= 2;
        if (x != 84) failures++;
    }


    {
        uint16_t r = add2(66,62) + add2(62,125) + add2(66,125);
        if (r != 506) failures++;
    }


    {
        uint16_t r = 63642 + 55130 + 19475 + 16331 + 9822 + 41481 + 581 + 22726;
        if (r != 32580) failures++;
    }


    {
        uint16_t x = 18494;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[2][4] = {{201,142,53,79},{38,206,67,30}};
        if (m[0][0] != 201) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {48,236,228,26,194,220,140,98};
        uint8_t *p = buf;
        p += 4;
        if (*p != 194) failures++;
    }


    {
        uint16_t x = 27770;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)30) + (uint16_t)56456;
        if (r != 56486) failures++;
    }


    {
        uint8_t v = 83;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = call6(113,37,178,38,23,210);
        if (r != 599) failures++;
    }


    {
        uint8_t a[6] = {139,5,24,229,102,25};
        if (a[5] != 25) failures++;
    }


    {
        uint8_t m[2][4] = {{73,37,187,154},{94,145,58,97}};
        if (m[1][2] != 58) failures++;
    }


    {
        volatile uint8_t port = 24;
        uint8_t r = port;
        if (r != 24) failures++;
    }


    {
        uint8_t v = 28;
        v |= 128;
        if (v != 156) failures++;
    }


    {
        uint16_t r = add2(197,138) + add2(138,133) + add2(197,133);
        if (r != 936) failures++;
    }


    {
        uint16_t r = add2(83,253) + add2(253,64) + add2(83,64);
        if (r != 800) failures++;
    }


    {
        uint8_t v = 93;
        v &= ~(uint8_t)64;
        if (v != 29) failures++;
    }


    {
        uint32_t a = 833007186UL;
        uint32_t b = 3679874923UL;
        uint32_t r = a - b;
        if (r != 1448099559UL) failures++;
    }


    {
        volatile int16_t a = 7712;
        volatile int16_t b = 19606;
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
        uint8_t buf[8] = {65,233,127,192,33,36,43,194};
        uint8_t *p = buf;
        p += 5;
        if (*p != 36) failures++;
    }


    {
        uint32_t a = 1365319922UL;
        uint32_t b = 1120589147UL;
        uint32_t r = a - b;
        if (r != 244730775UL) failures++;
    }


    {
        uint8_t v = 128;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 32) failures++;
    }


    {
        volatile uint8_t port = 153;
        uint8_t r = port;
        if (r != 153) failures++;
    }


    {
        if (((uint16_t)(124 | (58 | (65 - 30)))) != 127) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 1: result = 252; break;
        case 4: result = 172; break;
        case 11: result = 144; break;
        case 16: result = 72; break;
        case 7: result = 234; break;
        case 9: result = 111; break;
        case 18: result = 54; break;
        case 17: result = 181; break;
        default: result = 205; break;
        }
        if (result != 181) failures++;
    }


    {
        uint8_t buf[8] = {28,53,9,126,119,224,37,152};
        uint8_t *p = buf;
        p += 3;
        if (*p != 126) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {0,127,22406,42};
        if (s.b != (uint8_t)127) failures++;
    }


    {
        uint16_t x = 169;
        x = x + 57;
        if (x != 226) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)47) + (uint16_t)16461;
        if (r != 16508) failures++;
    }


    {
        uint16_t r = call6(237,223,185,176,63,32);
        if (r != 916) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {176,37,657,97};
        if (s.d != (uint8_t)97) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint32_t a = 2445168807UL;
        uint32_t b = 767338959UL;
        uint32_t r = a + b;
        if (r != 3212507766UL) failures++;
    }


    {
        uint16_t r = add2(71,88) + add2(88,186) + add2(71,186);
        if (r != 690) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        volatile uint8_t port = 21;
        uint8_t r = port;
        if (r != 21) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        volatile int16_t a = 3962;
        volatile int16_t b = -13850;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 230;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 10) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint8_t x = 171;
        x <<= 4;
        if (x != 176) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 18: result = 238; break;
        case 3: result = 105; break;
        case 7: result = 149; break;
        case 14: result = 137; break;
        case 1: result = 242; break;
        case 2: result = 9; break;
        default: result = 25; break;
        }
        if (result != 242) failures++;
    }


    {
        uint16_t x = 231;
        x = x + 143;
        if (x != 374) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-15) % (int16_t)((int8_t)-85);
        if ((uint16_t)r != (uint16_t)65521) failures++;
    }


    {
        uint16_t r = 61489 + 29033 + 48458 + 36306 + 63767 + 15881 + 53835 + 31054;
        if (r != 12143) failures++;
    }


    {
        volatile int16_t a = 21231;
        volatile int16_t b = 28785;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(14,93) != 65457) failures++;
    }


    {
        uint8_t v = 23;
        v |= 2;
        if (v != 23) failures++;
    }


    {
        uint8_t m[3][2] = {{216,87},{7,231},{114,91}};
        if (m[2][0] != 114) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 14: result = 162; break;
        case 6: result = 196; break;
        case 15: result = 175; break;
        case 8: result = 95; break;
        default: result = 242; break;
        }
        if (result != 175) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        g16 = 14034;
        if (read_g16() != 14034) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(239,108) != 131) failures++;
    }


    {
        int8_t a = -57;
        int8_t b = 46;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 1336;
        if (read_g16() != 1336) failures++;
    }


    {
        uint8_t src[16] = {69,29,248,220,213,24,200,51,158,58,241,240,64,187,208,235};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[2] != 248) failures++;
    }


    {
        volatile int16_t a = -19768;
        volatile int16_t b = -15005;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 53;
        v &= ~(uint8_t)4;
        if (v != 49) failures++;
    }


    {
        if (((uint16_t)230) != 230) failures++;
    }


    {
        uint8_t a[6] = {187,179,212,230,179,138};
        if (a[0] != 187) failures++;
    }


    {
        int8_t a = 14;
        int8_t b = 16;
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
        uint16_t r = call6(9,168,202,74,26,11);
        if (r != 490) failures++;
    }


    {
        uint16_t x = 40;
        x = x + 133;
        if (x != 173) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t a[6] = {243,10,126,59,59,200};
        if (a[1] != 10) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 228;
        if (buf[13] != 228) failures++;
    }


    {
        g16 = 39194;
        if (read_g16() != 39194) failures++;
    }


    {
        uint16_t r = call6(41,189,103,122,242,173);
        if (r != 870) failures++;
    }


    {
        volatile uint8_t port = 123;
        uint8_t r = port;
        if (r != 123) failures++;
    }


    {
        uint16_t r = call6(212,114,175,40,31,118);
        if (r != 690) failures++;
    }


    {
        uint16_t r = 38948 + 26873 + 21125 + 55517 + 19827 + 50752 + 19189 + 35204;
        if (r != 5291) failures++;
    }


    {
        uint16_t x = 65298;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)126) + (uint16_t)17370;
        if (r != 17496) failures++;
    }


    {
        uint8_t buf[8] = {93,163,102,159,172,224,205,227};
        uint8_t *p = buf;
        p += 4;
        if (*p != 172) failures++;
    }


    {
        uint8_t m[4][3] = {{154,111,80},{163,1,236},{242,162,202},{43,235,237}};
        if (m[3][0] != 43) failures++;
    }


    {
        uint8_t m[3][3] = {{89,60,109},{167,136,54},{78,207,164}};
        if (m[1][0] != 167) failures++;
    }


    {
        uint16_t r = add2(97,246) + add2(246,226) + add2(97,226);
        if (r != 1138) failures++;
    }


    {
        uint16_t r = 18247 + 18618 + 62241 + 43253 + 41557 + 52079 + 17401 + 29873;
        if (r != 21125) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(56,106) != 65486) failures++;
    }


    {
        uint8_t buf[8] = {165,215,224,239,185,73,210,15};
        uint8_t *p = buf;
        p += 2;
        if (*p != 224) failures++;
    }


    {
        uint8_t v = 141;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {248,162,229,126,153,45};
        if (a[4] != 153) failures++;
    }


    {
        uint8_t src[16] = {78,186,224,71,9,239,132,176,171,31,77,231,156,192,52,58};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[12] != 156) failures++;
    }


    {
        uint16_t x = 41646;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(115,231) != 346) failures++;
    }


    {
        uint16_t x = 39223;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 23; break;
        case 14: result = 148; break;
        case 1: result = 83; break;
        case 7: result = 73; break;
        case 11: result = 146; break;
        case 19: result = 108; break;
        case 15: result = 232; break;
        case 0: result = 130; break;
        default: result = 225; break;
        }
        if (result != 23) failures++;
    }


    {
        g16 = 44925;
        if (read_g16() != 44925) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {144,37,730,165};
        if (s.d != (uint8_t)165) failures++;
    }


    {
        g16 = 34858;
        if (read_g16() != 34858) failures++;
    }


    {
        uint8_t m[3][3] = {{238,199,155},{249,159,220},{89,94,41}};
        if (m[1][1] != 159) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)210) + (uint16_t)45000;
        if (r != 45210) failures++;
    }


    {
        uint16_t r = 42960 + 28528 + 52141 + 42269 + 19926 + 49013 + 36850 + 1797;
        if (r != 11340) failures++;
    }


    {
        g16 = 38265;
        if (read_g16() != 38265) failures++;
    }


    {
        uint8_t src[13] = {63,217,194,250,106,255,66,200,125,79,57,213,230};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[1] != 217) failures++;
    }


    {
        uint8_t src[13] = {229,125,224,238,130,189,104,170,210,212,153,137,252};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[5] != 189) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)78) + (uint16_t)28993;
        if (r != 29071) failures++;
    }


    {
        uint8_t src[14] = {159,147,72,238,238,4,135,144,114,78,184,177,197,238};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[8] != 114) failures++;
    }


    {
        g16 = 65344;
        if (read_g16() != 65344) failures++;
    }


    {
        volatile int16_t a = 31196;
        volatile int16_t b = 11561;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(((126 + 99) & 158) | 141)) != 141) failures++;
    }


    {
        uint16_t r = add2(9,149) + add2(149,225) + add2(9,225);
        if (r != 766) failures++;
    }


    {
        uint8_t x = 155;
        x <<= 2;
        if (x != 108) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint8_t buf[8] = {201,9,255,193,107,227,76,54};
        uint8_t *p = buf;
        p += 6;
        if (*p != 76) failures++;
    }


    {
        volatile int16_t a = 10116;
        volatile int16_t b = -32519;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -101;
        int8_t b = -93;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 42708;
        if (read_g16() != 42708) failures++;
    }


    {
        uint16_t r = add2(171,224) + add2(224,34) + add2(171,34);
        if (r != 858) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 15: result = 156; break;
        case 11: result = 145; break;
        case 10: result = 225; break;
        case 12: result = 22; break;
        case 19: result = 12; break;
        case 3: result = 43; break;
        default: result = 113; break;
        }
        if (result != 113) failures++;
    }


    {
        uint8_t x = 198;
        x <<= 0;
        if (x != 198) failures++;
    }


    {
        uint8_t v = 224;
        v &= ~(uint8_t)8;
        if (v != 224) failures++;
    }


    {
        g16 = 60289;
        if (read_g16() != 60289) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        int8_t a = -95;
        int8_t b = -105;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 246358556UL;
        uint32_t b = 2158669576UL;
        uint32_t r = a ^ b;
        if (r != 2382728468UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)46) + (uint16_t)58747;
        if (r != 58793) failures++;
    }


    {
        if (((uint16_t)((251 | (119 | 97)) - ((202 & 143) | (137 | 132)))) != 112) failures++;
    }


    {
        volatile uint8_t port = 60;
        uint8_t r = port;
        if (r != 60) failures++;
    }


    {
        uint8_t buf[8] = {71,99,125,100,223,130,109,204};
        uint8_t *p = buf;
        p += 0;
        if (*p != 71) failures++;
    }


    {
        int8_t a = -82;
        int8_t b = 24;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 64720;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {135,18,226,32,127,25,237,96};
        uint8_t *p = buf;
        p += 3;
        if (*p != 32) failures++;
    }


    {
        uint8_t m[2][3] = {{234,242,72},{220,131,103}};
        if (m[1][1] != 131) failures++;
    }


    {
        g16 = 12098;
        if (read_g16() != 12098) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-72) % (int16_t)((int8_t)-105);
        if ((uint16_t)r != (uint16_t)65464) failures++;
    }


    {
        uint8_t m[3][4] = {{164,126,101,63},{63,73,145,36},{237,48,222,85}};
        if (m[0][2] != 101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)109) + (uint16_t)922;
        if (r != 1031) failures++;
    }


    {
        g16 = 31696;
        if (read_g16() != 31696) failures++;
    }


    {
        uint8_t a[6] = {239,124,132,119,135,195};
        if (a[4] != 135) failures++;
    }


    {
        uint8_t v = 75;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t v = 131;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {108,163,49479,111};
        if (s.d != (uint8_t)111) failures++;
    }


    {
        uint16_t r = 46250 + 31301 + 61209 + 65091 + 50776 + 9222 + 32323 + 38445;
        if (r != 6937) failures++;
    }


    {
        uint16_t x = 102;
        x = x + 107;
        if (x != 209) failures++;
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
        uint16_t x = 234;
        x = x + 49;
        if (x != 283) failures++;
    }


    {
        uint8_t m[2][2] = {{34,225},{96,18}};
        if (m[0][0] != 34) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 0: result = 152; break;
        case 6: result = 141; break;
        case 14: result = 141; break;
        case 12: result = 69; break;
        case 4: result = 93; break;
        case 2: result = 158; break;
        case 17: result = 15; break;
        default: result = 7; break;
        }
        if (result != 141) failures++;
    }


    {
        uint8_t a[6] = {192,174,35,112,132,221};
        if (a[4] != 132) failures++;
    }


    {
        int8_t a = 100;
        int8_t b = 122;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint16_t r = call6(90,175,229,61,192,194);
        if (r != 941) failures++;
    }


    {
        uint16_t r = 9344 + 37097 + 26778 + 37885 + 4214 + 3306 + 19841 + 28060;
        if (r != 35453) failures++;
    }


    {
        uint16_t r = add2(159,112) + add2(112,100) + add2(159,100);
        if (r != 742) failures++;
    }


    {
        uint8_t m[2][3] = {{45,180,31},{155,217,186}};
        if (m[0][0] != 45) failures++;
    }


    {
        uint8_t src[14] = {202,210,204,91,248,188,200,151,127,158,128,242,188,50};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[10] != 128) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)161) + (uint16_t)25377;
        if (r != 25538) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)217) + (uint16_t)28169;
        if (r != 28386) failures++;
    }


    {
        uint16_t r = add2(23,117) + add2(117,102) + add2(23,102);
        if (r != 484) failures++;
    }


    {
        int8_t a = -35;
        int8_t b = 106;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(143,125,212,246,114,41);
        if (r != 881) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)229) + (uint16_t)49186;
        if (r != 49415) failures++;
    }


    {
        uint8_t m[4][4] = {{254,155,108,129},{208,7,253,218},{123,98,159,154},{243,190,115,227}};
        if (m[2][2] != 159) failures++;
    }


    {
        uint8_t v = 163;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        g16 = 23296;
        if (read_g16() != 23296) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)165) + (uint16_t)34975;
        if (r != 35140) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(190,144) != 46) failures++;
    }


    {
        uint16_t x = 44901;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 192;
        uint8_t r = port;
        if (r != 192) failures++;
    }


    {
        int8_t a = -4;
        int8_t b = 63;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 16: result = 65; break;
        case 19: result = 246; break;
        case 17: result = 211; break;
        default: result = 97; break;
        }
        if (result != 65) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 147;
        if (buf[1] != 147) failures++;
    }


    {
        uint16_t r = 29383 + 36587 + 648 + 7997 + 2601 + 63823 + 64831 + 34286;
        if (r != 43548) failures++;
    }


    {
        uint8_t src[14] = {54,211,5,205,0,246,157,38,20,41,244,87,47,152};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[5] != 246) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)115) + (uint16_t)9031;
        if (r != 9146) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {61,45,17734,11};
        if (s.d != (uint8_t)11) failures++;
    }


    {
        if (((uint16_t)(166 & 8)) != 0) failures++;
    }


    {
        uint8_t x = 214;
        x <<= 5;
        if (x != 192) failures++;
    }


    {
        if (((uint16_t)((27 & (140 | 41)) - ((24 ^ 78) ^ (236 | 207)))) != 65360) failures++;
    }


    {
        uint16_t x = 3;
        x = x + 15;
        if (x != 18) failures++;
    }


    {
        uint16_t x = 9880;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {184,28,9017,213};
        if (s.b != (uint8_t)28) failures++;
    }


    {
        uint8_t src[10] = {61,255,159,206,27,32,205,135,22,58};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[3] != 206) failures++;
    }


    {
        uint8_t m[4][4] = {{1,16,232,162},{221,223,92,126},{92,111,166,39},{131,160,71,235}};
        if (m[3][0] != 131) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(36,195) != 65377) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 19: result = 5; break;
        case 2: result = 35; break;
        case 11: result = 68; break;
        case 6: result = 105; break;
        case 4: result = 186; break;
        case 1: result = 214; break;
        case 9: result = 47; break;
        case 13: result = 90; break;
        default: result = 87; break;
        }
        if (result != 105) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)6) % (int16_t)((int8_t)34);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 196;
        if (buf[6] != 196) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        volatile uint8_t port = 83;
        uint8_t r = port;
        if (r != 83) failures++;
    }


    {
        volatile uint8_t port = 227;
        uint8_t r = port;
        if (r != 227) failures++;
    }


    {
        uint8_t v = 115;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {98,229,1,96,62,3};
        if (a[2] != 1) failures++;
    }


    {
        uint16_t r = add2(246,89) + add2(89,59) + add2(246,59);
        if (r != 788) failures++;
    }


    {
        uint8_t m[3][2] = {{74,238},{195,188},{25,184}};
        if (m[1][1] != 188) failures++;
    }


    {
        if (((uint16_t)174) != 174) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = 46194 + 60682 + 26619 + 22374 + 15514 + 21332 + 30855 + 29306;
        if (r != 56268) failures++;
    }


    {
        int8_t a = 111;
        int8_t b = -29;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {143,70,6702,208};
        if (s.b != (uint8_t)70) failures++;
    }


    {
        int8_t a = 93;
        int8_t b = 24;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 160;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t a[6] = {130,95,246,149,214,90};
        if (a[1] != 95) failures++;
    }


    {
        uint16_t x = 164;
        x = x + 189;
        if (x != 353) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 93;
        if (buf[13] != 93) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 51185;
        if (read_g16() != 51185) failures++;
    }


    {
        uint16_t r = 59388 + 16078 + 49576 + 50962 + 24144 + 30070 + 27724 + 17552;
        if (r != 13350) failures++;
    }


    {
        uint16_t r = call6(12,227,115,10,223,37);
        if (r != 624) failures++;
    }


    {
        if (((uint16_t)193) != 193) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)24) / (int16_t)((int8_t)10);
        if ((uint16_t)r != (uint16_t)2) failures++;
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
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = 45151 + 23745 + 16556 + 57120 + 25780 + 47937 + 39226 + 55189;
        if (r != 48560) failures++;
    }


    {
        volatile uint8_t port = 53;
        uint8_t r = port;
        if (r != 53) failures++;
    }


    {
        int8_t a = -16;
        int8_t b = -77;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {163,109,19,91,99,232,116,212};
        uint8_t *p = buf;
        p += 4;
        if (*p != 99) failures++;
    }


    {
        uint8_t x = 186;
        x <<= 0;
        if (x != 186) failures++;
    }


    {
        uint8_t a[6] = {239,223,154,31,78,91};
        if (a[2] != 154) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 1) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 103778216UL;
        uint32_t b = 2471197830UL;
        uint32_t r = a ^ b;
        if (r != 2506423086UL) failures++;
    }


    {
        uint16_t r = add2(36,203) + add2(203,90) + add2(36,90);
        if (r != 658) failures++;
    }


    {
        uint8_t v = 43;
        v &= ~(uint8_t)32;
        if (v != 11) failures++;
    }


    {
        uint8_t v = 178;
        v ^= 8;
        if (v != 186) failures++;
    }


    {
        g16 = 43356;
        if (read_g16() != 43356) failures++;
    }


    {
        uint8_t a[6] = {124,180,59,88,176,237};
        if (a[0] != 124) failures++;
    }


    {
        uint8_t m[4][3] = {{103,31,211},{30,244,84},{26,224,11},{101,49,76}};
        if (m[1][1] != 244) failures++;
    }


    {
        uint8_t a[6] = {229,144,191,36,60,241};
        if (a[4] != 60) failures++;
    }


    {
        uint8_t buf[8] = {226,40,97,85,228,135,203,183};
        uint8_t *p = buf;
        p += 5;
        if (*p != 135) failures++;
    }


    {
        uint16_t r = call6(79,96,22,243,138,196);
        if (r != 774) failures++;
    }


    {
        uint16_t x = 52;
        x = x + 75;
        if (x != 127) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {232,35,10898,80};
        if (s.b != (uint8_t)35) failures++;
    }


    {
        int8_t a = -34;
        int8_t b = -123;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 42246;
        if (read_g16() != 42246) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(20,1) != 19) failures++;
    }


    {
        volatile uint8_t port = 53;
        uint8_t r = port;
        if (r != 53) failures++;
    }


    {
        uint8_t src[1] = {154};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 154) failures++;
    }


    {
        uint32_t a = 3214648757UL;
        uint32_t b = 3950244325UL;
        uint32_t r = a & b;
        if (r != 2870190501UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        if (((uint16_t)(68 ^ (53 ^ (170 ^ 27)))) != 192) failures++;
    }


    {
        volatile uint8_t port = 50;
        uint8_t r = port;
        if (r != 50) failures++;
    }


    {
        g16 = 51681;
        if (read_g16() != 51681) failures++;
    }


    {
        if (((uint16_t)(((108 & 7) | 131) - ((168 | 124) + 141))) != 65278) failures++;
    }


    {
        volatile int16_t a = 26988;
        volatile int16_t b = -20158;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 11: result = 80; break;
        case 14: result = 103; break;
        case 6: result = 185; break;
        default: result = 58; break;
        }
        if (result != 58) failures++;
    }


    {
        uint8_t v = 128;
        v ^= 1;
        if (v != 129) failures++;
    }


    {
        uint16_t x = 52096;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -53;
        int8_t b = 103;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 98;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 241;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t src[9] = {2,250,108,182,193,187,88,171,136};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 108) failures++;
    }


    {
        uint16_t x = 17122;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)61) + (uint16_t)7102;
        if (r != 7163) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {224,30,33239,117};
        if (s.d != (uint8_t)117) failures++;
    }


    {
        uint16_t r = add2(55,201) + add2(201,78) + add2(55,78);
        if (r != 668) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)93) % (int16_t)((int8_t)-61);
        if ((uint16_t)r != (uint16_t)32) failures++;
    }

    return failures;
}