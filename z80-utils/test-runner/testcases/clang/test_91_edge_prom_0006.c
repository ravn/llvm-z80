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
        uint16_t r = call6(73,132,125,168,248,0);
        if (r != 746) failures++;
    }


    {
        uint8_t buf[8] = {200,83,52,146,48,32,185,104};
        uint8_t *p = buf;
        p += 1;
        if (*p != 83) failures++;
    }


    {
        uint16_t x = 25599;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 44962;
        if (read_g16() != 44962) failures++;
    }


    {
        if (((uint16_t)62) != 62) failures++;
    }


    {
        volatile int16_t a = 29130;
        volatile int16_t b = 9952;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {0,126,233,252,251,107};
        if (a[1] != 126) failures++;
    }


    {
        uint8_t v = 75;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)25) + (uint16_t)32829;
        if (r != 32854) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)59) % (int16_t)((int8_t)110);
        if ((uint16_t)r != (uint16_t)59) failures++;
    }


    {
        volatile uint8_t port = 34;
        uint8_t r = port;
        if (r != 34) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 53;
        if (buf[1] != 53) failures++;
    }


    {
        uint32_t a = 3087328605UL;
        uint32_t b = 3849364252UL;
        uint32_t r = a - b;
        if (r != 3532931649UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(163,107) != 270) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 78;
        uint8_t r = port;
        if (r != 78) failures++;
    }


    {
        uint16_t r = add2(148,245) + add2(245,83) + add2(148,83);
        if (r != 952) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-33) / (int16_t)((int8_t)-23);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t v = 1;
        v ^= 32;
        if (v != 33) failures++;
    }


    {
        uint8_t v = 139;
        v ^= 2;
        if (v != 137) failures++;
    }


    {
        uint16_t r = call6(123,147,117,194,200,215);
        if (r != 996) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(233,54) != 179) failures++;
    }


    {
        uint8_t v = 235;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t m[2][3] = {{123,179,153},{54,207,126}};
        if (m[1][2] != 126) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 13: result = 1; break;
        case 16: result = 199; break;
        case 2: result = 45; break;
        case 11: result = 119; break;
        case 15: result = 18; break;
        case 9: result = 53; break;
        default: result = 78; break;
        }
        if (result != 119) failures++;
    }


    {
        uint8_t v = 114;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int8_t a = 59;
        int8_t b = 25;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 11: result = 250; break;
        case 2: result = 14; break;
        case 3: result = 48; break;
        case 8: result = 246; break;
        case 17: result = 204; break;
        case 18: result = 37; break;
        case 5: result = 35; break;
        default: result = 182; break;
        }
        if (result != 246) failures++;
    }


    {
        volatile int16_t a = 22500;
        volatile int16_t b = 32434;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {238,96,142,100,80,126};
        if (a[3] != 100) failures++;
    }


    {
        uint16_t r = add2(16,126) + add2(126,58) + add2(16,58);
        if (r != 400) failures++;
    }


    {
        uint8_t buf[8] = {55,78,71,94,4,46,165,226};
        uint8_t *p = buf;
        p += 4;
        if (*p != 4) failures++;
    }


    {
        uint16_t r = 50345 + 16818 + 47853 + 35073 + 64249 + 13735 + 25501 + 49682;
        if (r != 41112) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {187,99,32895,118};
        if (s.b != (uint8_t)99) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 13: result = 105; break;
        case 2: result = 227; break;
        case 17: result = 252; break;
        case 18: result = 149; break;
        case 15: result = 30; break;
        default: result = 181; break;
        }
        if (result != 30) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)150) + (uint16_t)34834;
        if (r != 34984) failures++;
    }


    {
        uint8_t buf[8] = {50,31,113,25,202,100,199,255};
        uint8_t *p = buf;
        p += 1;
        if (*p != 31) failures++;
    }


    {
        uint8_t v = 3;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t x = 147;
        x = x + 172;
        if (x != 319) failures++;
    }


    {
        uint16_t x = 199;
        x = x + 249;
        if (x != 448) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {112,7,41338,19};
        if (s.d != (uint8_t)19) failures++;
    }


    {
        volatile int16_t a = 18105;
        volatile int16_t b = -1011;
        int r = (a <= b);
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
        uint16_t r = 26160 + 46309 + 32897 + 53069 + 34320 + 26954 + 13869 + 18614;
        if (r != 55584) failures++;
    }


    {
        uint32_t a = 807604344UL;
        uint32_t b = 2518406108UL;
        uint32_t r = a - b;
        if (r != 2584165532UL) failures++;
    }


    {
        uint8_t x = 87;
        x <<= 1;
        if (x != 174) failures++;
    }


    {
        g16 = 27921;
        if (read_g16() != 27921) failures++;
    }


    {
        if (((uint16_t)((219 ^ (163 ^ 169)) + 15)) != 224) failures++;
    }


    {
        uint8_t v = 71;
        v ^= 1;
        if (v != 70) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-93) % (int16_t)((int8_t)16);
        if ((uint16_t)r != (uint16_t)65523) failures++;
    }


    {
        uint8_t buf[8] = {98,40,252,137,19,122,196,240};
        uint8_t *p = buf;
        p += 5;
        if (*p != 122) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 1) sum += j;
        if (sum != 28) failures++;
    }


    {
        uint32_t a = 2859200378UL;
        uint32_t b = 442515402UL;
        uint32_t r = a | b;
        if (r != 3127640058UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-29) % (int16_t)((int8_t)-64);
        if ((uint16_t)r != (uint16_t)65507) failures++;
    }


    {
        volatile uint8_t port = 249;
        uint8_t r = port;
        if (r != 249) failures++;
    }


    {
        uint8_t src[5] = {148,250,210,216,190};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 148) failures++;
    }


    {
        g16 = 24361;
        if (read_g16() != 24361) failures++;
    }


    {
        g16 = 42489;
        if (read_g16() != 42489) failures++;
    }


    {
        if (((uint16_t)11) != 11) failures++;
    }


    {
        uint32_t a = 3366876934UL;
        uint32_t b = 1056593353UL;
        uint32_t r = a - b;
        if (r != 2310283581UL) failures++;
    }


    {
        uint16_t x = 3421;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 8: result = 32; break;
        case 6: result = 82; break;
        case 3: result = 74; break;
        case 7: result = 6; break;
        case 10: result = 160; break;
        case 19: result = 203; break;
        case 13: result = 220; break;
        default: result = 206; break;
        }
        if (result != 6) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 0: result = 29; break;
        case 13: result = 233; break;
        case 9: result = 103; break;
        default: result = 113; break;
        }
        if (result != 233) failures++;
    }


    {
        uint8_t m[4][4] = {{67,149,38,63},{6,227,85,25},{237,254,20,101},{205,169,175,88}};
        if (m[1][1] != 227) failures++;
    }


    {
        uint16_t x = 38;
        x = x + 219;
        if (x != 257) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)114) % (int16_t)((int8_t)30);
        if ((uint16_t)r != (uint16_t)24) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {50,36,30699,58};
        if (s.b != (uint8_t)36) failures++;
    }


    {
        uint8_t m[2][3] = {{207,61,205},{97,58,231}};
        if (m[1][1] != 58) failures++;
    }


    {
        g16 = 65007;
        if (read_g16() != 65007) failures++;
    }


    {
        uint8_t src[1] = {48};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 48) failures++;
    }


    {
        if (((uint16_t)119) != 119) failures++;
    }


    {
        uint8_t buf[8] = {215,70,187,58,243,38,97,177};
        uint8_t *p = buf;
        p += 4;
        if (*p != 243) failures++;
    }


    {
        uint16_t x = 51473;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {156,10,39550,42};
        if (s.c != (uint16_t)39550) failures++;
    }


    {
        uint8_t m[2][4] = {{48,54,204,76},{153,62,209,130}};
        if (m[0][1] != 54) failures++;
    }


    {
        if (((uint16_t)((126 - (5 - 168)) + 153)) != 442) failures++;
    }


    {
        uint8_t a[6] = {67,61,157,150,225,60};
        if (a[1] != 61) failures++;
    }


    {
        uint8_t buf[8] = {250,84,216,167,12,164,140,86};
        uint8_t *p = buf;
        p += 3;
        if (*p != 167) failures++;
    }


    {
        uint8_t a[6] = {36,89,170,156,162,52};
        if (a[5] != 52) failures++;
    }


    {
        volatile int16_t a = 2742;
        volatile int16_t b = 11692;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(5,137,195,96,192,118);
        if (r != 743) failures++;
    }


    {
        uint16_t r = 30122 + 13943 + 11476 + 6000 + 5256 + 33448 + 35363 + 53170;
        if (r != 57706) failures++;
    }


    {
        int8_t a = 76;
        int8_t b = -104;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 46934;
        if (read_g16() != 46934) failures++;
    }


    {
        volatile uint8_t port = 247;
        uint8_t r = port;
        if (r != 247) failures++;
    }


    {
        uint16_t x = 39837;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 70;
        int8_t b = -35;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 198;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint8_t v = 59;
        v |= 4;
        if (v != 63) failures++;
    }


    {
        uint16_t x = 240;
        x = x + 56;
        if (x != 296) failures++;
    }


    {
        uint8_t src[9] = {111,132,120,105,67,254,98,160,136};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[0] != 111) failures++;
    }


    {
        uint8_t a[6] = {6,99,197,72,221,162};
        if (a[5] != 162) failures++;
    }


    {
        uint16_t r = 11823 + 60977 + 30025 + 42049 + 55978 + 14803 + 22932 + 49293;
        if (r != 25736) failures++;
    }


    {
        int8_t a = 111;
        int8_t b = -8;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 250;
        if (buf[8] != 250) failures++;
    }


    {
        uint16_t x = 234;
        x = x + 85;
        if (x != 319) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {197,40,51280,158};
        if (s.c != (uint16_t)51280) failures++;
    }


    {
        uint8_t m[2][3] = {{186,52,27},{250,146,115}};
        if (m[1][1] != 146) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(134,93) != 41) failures++;
    }


    {
        g16 = 62958;
        if (read_g16() != 62958) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {15,210,13730,253};
        if (s.c != (uint16_t)13730) failures++;
    }


    {
        uint32_t a = 2401914238UL;
        uint32_t b = 1641322696UL;
        uint32_t r = a + b;
        if (r != 4043236934UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        g16 = 31427;
        if (read_g16() != 31427) failures++;
    }


    {
        volatile uint8_t port = 167;
        uint8_t r = port;
        if (r != 167) failures++;
    }


    {
        g16 = 27490;
        if (read_g16() != 27490) failures++;
    }


    {
        uint8_t a[6] = {62,9,109,224,236,102};
        if (a[2] != 109) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(204,36) != 240) failures++;
    }


    {
        g16 = 12355;
        if (read_g16() != 12355) failures++;
    }


    {
        uint8_t buf[8] = {187,88,45,178,171,255,1,243};
        uint8_t *p = buf;
        p += 5;
        if (*p != 255) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)2) % (int16_t)((int8_t)96);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        volatile int16_t a = -1873;
        volatile int16_t b = -21257;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(122,237) != 359) failures++;
    }


    {
        uint8_t m[4][2] = {{23,65},{183,51},{157,161},{130,75}};
        if (m[2][0] != 157) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 38;
        if (buf[6] != 38) failures++;
    }


    {
        uint8_t m[2][4] = {{212,142,92,144},{140,92,101,87}};
        if (m[0][0] != 212) failures++;
    }


    {
        uint8_t x = 105;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 43411;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 2504;
        if (read_g16() != 2504) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 16: result = 219; break;
        case 6: result = 57; break;
        case 1: result = 157; break;
        case 13: result = 32; break;
        case 9: result = 95; break;
        case 12: result = 200; break;
        default: result = 204; break;
        }
        if (result != 219) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)117) + (uint16_t)21228;
        if (r != 21345) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {86,98,34681,191};
        if (s.a != (uint8_t)86) failures++;
    }


    {
        volatile int16_t a = 26658;
        volatile int16_t b = -20344;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 126;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(119,77) + add2(77,129) + add2(119,129);
        if (r != 650) failures++;
    }


    {
        uint16_t r = add2(74,5) + add2(5,158) + add2(74,158);
        if (r != 474) failures++;
    }


    {
        uint32_t a = 2940391923UL;
        uint32_t b = 4180065587UL;
        uint32_t r = a & b;
        if (r != 2835517747UL) failures++;
    }


    {
        uint8_t v = 138;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 54) failures++;
    }


    {
        uint8_t v = 132;
        v ^= 32;
        if (v != 164) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 55;
        if (buf[12] != 55) failures++;
    }


    {
        uint8_t v = 49;
        v ^= 128;
        if (v != 177) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 189;
        x = x + 220;
        if (x != 409) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 231;
        if (buf[12] != 231) failures++;
    }


    {
        if (((uint16_t)(((248 ^ 9) & 144) | (37 + (12 & 253)))) != 177) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t a[6] = {247,78,161,36,121,66};
        if (a[2] != 161) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 17: result = 125; break;
        case 3: result = 55; break;
        case 4: result = 194; break;
        case 13: result = 255; break;
        default: result = 185; break;
        }
        if (result != 125) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 186;
        if (buf[0] != 186) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)188) + (uint16_t)18214;
        if (r != 18402) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-37) / (int16_t)((int8_t)57);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {145,19,93,152,227,37,183,247};
        uint8_t *p = buf;
        p += 7;
        if (*p != 247) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)231) + (uint16_t)63175;
        if (r != 63406) failures++;
    }


    {
        g16 = 16109;
        if (read_g16() != 16109) failures++;
    }


    {
        uint8_t v = 90;
        v &= ~(uint8_t)1;
        if (v != 90) failures++;
    }


    {
        uint8_t src[4] = {86,39,36,68};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[2] != 36) failures++;
    }


    {
        uint16_t r = 32113 + 42045 + 29733 + 50678 + 11052 + 503 + 54949 + 61210;
        if (r != 20139) failures++;
    }


    {
        uint16_t r = add2(111,228) + add2(228,212) + add2(111,212);
        if (r != 1102) failures++;
    }


    {
        uint8_t a[6] = {2,79,220,62,252,137};
        if (a[0] != 2) failures++;
    }


    {
        volatile uint8_t port = 220;
        uint8_t r = port;
        if (r != 220) failures++;
    }


    {
        uint8_t buf[8] = {253,78,91,129,71,126,51,139};
        uint8_t *p = buf;
        p += 5;
        if (*p != 126) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 345678943UL;
        uint32_t b = 3858596422UL;
        uint32_t r = a - b;
        if (r != 782049817UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 244;
        x = x + 137;
        if (x != 381) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 2) sum += j;
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
        uint16_t r = add2(1,173) + add2(173,33) + add2(1,33);
        if (r != 414) failures++;
    }


    {
        uint16_t r = 26428 + 23822 + 10978 + 14014 + 25637 + 59255 + 20811 + 30696;
        if (r != 15033) failures++;
    }


    {
        uint16_t r = call6(216,36,1,195,152,178);
        if (r != 778) failures++;
    }


    {
        uint8_t a[6] = {45,136,102,180,217,196};
        if (a[2] != 102) failures++;
    }


    {
        volatile int16_t a = -14762;
        volatile int16_t b = 4445;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 49;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 7) failures++;
    }


    {
        g16 = 58167;
        if (read_g16() != 58167) failures++;
    }


    {
        uint32_t a = 2894366604UL;
        uint32_t b = 1764016543UL;
        uint32_t r = a ^ b;
        if (r != 3315615251UL) failures++;
    }


    {
        volatile uint8_t port = 60;
        uint8_t r = port;
        if (r != 60) failures++;
    }


    {
        uint8_t a[6] = {131,19,241,62,165,156};
        if (a[4] != 165) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(236,147) != 89) failures++;
    }


    {
        uint16_t r = call6(174,165,95,159,210,0);
        if (r != 803) failures++;
    }


    {
        uint8_t m[4][4] = {{216,65,98,129},{65,58,157,132},{236,232,52,37},{115,76,163,149}};
        if (m[3][1] != 76) failures++;
    }


    {
        uint8_t v = 61;
        v &= ~(uint8_t)128;
        if (v != 61) failures++;
    }


    {
        if (((uint16_t)(229 + ((211 + 27) | (149 | 78)))) != 484) failures++;
    }


    {
        int8_t a = -46;
        int8_t b = 92;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 22;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 210;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 64177;
        if (read_g16() != 64177) failures++;
    }


    {
        volatile int16_t a = 20882;
        volatile int16_t b = -29766;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 18: result = 178; break;
        case 19: result = 91; break;
        case 15: result = 202; break;
        case 5: result = 237; break;
        case 7: result = 70; break;
        case 11: result = 7; break;
        case 8: result = 164; break;
        case 6: result = 127; break;
        default: result = 180; break;
        }
        if (result != 70) failures++;
    }


    {
        uint8_t v = 93;
        v &= ~(uint8_t)64;
        if (v != 29) failures++;
    }


    {
        uint8_t v = 206;
        int r = (v & 1) ? 1 : 0;
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
        for (uint16_t j = 0; j < 12; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        if (((uint16_t)(((214 ^ 114) & (213 ^ 166)) & 216)) != 0) failures++;
    }


    {
        uint16_t r = call6(69,178,23,186,6,74);
        if (r != 536) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {12,103,51756,163};
        if (s.a != (uint8_t)12) failures++;
    }


    {
        uint16_t r = 22290 + 7492 + 13716 + 57361 + 3138 + 22178 + 20475 + 26909;
        if (r != 42487) failures++;
    }


    {
        uint8_t x = 201;
        x <<= 3;
        if (x != 72) failures++;
    }


    {
        uint8_t a[6] = {210,142,54,10,112,143};
        if (a[5] != 143) failures++;
    }


    {
        uint8_t src[10] = {226,8,155,193,250,93,154,42,59,161};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[1] != 8) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(160,138) != 298) failures++;
    }


    {
        uint8_t x = 99;
        x <<= 0;
        if (x != 99) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t r = 2467 + 54858 + 15475 + 59587 + 17031 + 24147 + 18075 + 48398;
        if (r != 43430) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        uint8_t v = 89;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(134,237) != 65433) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {203,81,58995,65};
        if (s.a != (uint8_t)203) failures++;
    }


    {
        uint8_t v = 254;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)203) + (uint16_t)21957;
        if (r != 22160) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 9: result = 64; break;
        case 4: result = 141; break;
        case 0: result = 255; break;
        case 10: result = 119; break;
        case 19: result = 30; break;
        case 16: result = 174; break;
        case 7: result = 133; break;
        default: result = 86; break;
        }
        if (result != 255) failures++;
    }


    {
        uint16_t x = 237;
        x = x + 96;
        if (x != 333) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-10) % (int16_t)((int8_t)104);
        if ((uint16_t)r != (uint16_t)65526) failures++;
    }


    {
        if (((uint16_t)(10 | ((127 - 186) - (124 ^ 40)))) != 65403) failures++;
    }


    {
        uint16_t r = add2(194,115) + add2(115,197) + add2(194,197);
        if (r != 1012) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(16,96) != 65456) failures++;
    }


    {
        if (((uint16_t)19) != 19) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)76) % (int16_t)((int8_t)-21);
        if ((uint16_t)r != (uint16_t)13) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint32_t a = 3745720796UL;
        uint32_t b = 1661478475UL;
        uint32_t r = a - b;
        if (r != 2084242321UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)11) + (uint16_t)22410;
        if (r != 22421) failures++;
    }


    {
        uint8_t src[8] = {19,162,235,113,107,86,229,54};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[5] != 86) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 5: result = 238; break;
        case 12: result = 244; break;
        case 2: result = 231; break;
        case 14: result = 178; break;
        default: result = 165; break;
        }
        if (result != 231) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {47,218,10704,182};
        if (s.a != (uint8_t)47) failures++;
    }


    {
        uint8_t x = 172;
        x <<= 0;
        if (x != 172) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 49;
        if (buf[15] != 49) failures++;
    }


    {
        uint16_t x = 210;
        x = x + 60;
        if (x != 270) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)17) + (uint16_t)42176;
        if (r != 42193) failures++;
    }


    {
        uint8_t v = 152;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(60,7) != 67) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)26) % (int16_t)((int8_t)-47);
        if ((uint16_t)r != (uint16_t)26) failures++;
    }


    {
        uint8_t buf[8] = {241,91,3,111,209,196,39,9};
        uint8_t *p = buf;
        p += 2;
        if (*p != 3) failures++;
    }


    {
        if (((uint16_t)249) != 249) failures++;
    }


    {
        volatile int16_t a = -14577;
        volatile int16_t b = 25164;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 106;
        uint8_t r = port;
        if (r != 106) failures++;
    }


    {
        uint16_t r = add2(184,21) + add2(21,143) + add2(184,143);
        if (r != 696) failures++;
    }


    {
        uint16_t r = 17708 + 19064 + 23841 + 37137 + 42896 + 13241 + 27233 + 55120;
        if (r != 39632) failures++;
    }


    {
        uint16_t r = add2(173,158) + add2(158,66) + add2(173,66);
        if (r != 794) failures++;
    }


    {
        uint16_t x = 37594;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 24853;
        volatile int16_t b = 17455;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 172;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        g16 = 15855;
        if (read_g16() != 15855) failures++;
    }


    {
        uint8_t buf[8] = {240,235,179,196,172,53,3,241};
        uint8_t *p = buf;
        p += 4;
        if (*p != 172) failures++;
    }


    {
        volatile int16_t a = -30210;
        volatile int16_t b = -13442;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(192,12,123,37,218,183);
        if (r != 765) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 13: result = 143; break;
        case 16: result = 43; break;
        case 17: result = 183; break;
        default: result = 49; break;
        }
        if (result != 143) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(8,108) != 65436) failures++;
    }


    {
        uint16_t r = add2(173,2) + add2(2,114) + add2(173,114);
        if (r != 578) failures++;
    }


    {
        volatile uint8_t port = 125;
        uint8_t r = port;
        if (r != 125) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-86) % (int16_t)((int8_t)66);
        if ((uint16_t)r != (uint16_t)65516) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {44,2,23966,61};
        if (s.b != (uint8_t)2) failures++;
    }


    {
        if (((uint16_t)28) != 28) failures++;
    }


    {
        uint8_t x = 180;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint8_t v = 219;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 6: result = 112; break;
        case 11: result = 33; break;
        case 13: result = 184; break;
        case 18: result = 89; break;
        case 9: result = 34; break;
        default: result = 110; break;
        }
        if (result != 184) failures++;
    }


    {
        uint16_t x = 246;
        x = x + 54;
        if (x != 300) failures++;
    }


    {
        uint16_t x = 128;
        x = x + 88;
        if (x != 216) failures++;
    }


    {
        uint16_t r = 7807 + 41318 + 15047 + 4509 + 380 + 46425 + 45586 + 58601;
        if (r != 23065) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t v = 166;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)5) % (int16_t)((int8_t)103);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        uint16_t x = 45868;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {34,49,193,183,164,123};
        if (a[3] != 183) failures++;
    }


    {
        g16 = 59965;
        if (read_g16() != 59965) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {123,141,32535,166};
        if (s.d != (uint8_t)166) failures++;
    }


    {
        uint8_t a[6] = {18,64,77,83,122,62};
        if (a[1] != 64) failures++;
    }


    {
        uint16_t r = 28539 + 4052 + 26838 + 61714 + 43029 + 62547 + 7626 + 33589;
        if (r != 5790) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(64,139) != 203) failures++;
    }


    {
        uint16_t x = 50226;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 201;
        x = x + 115;
        if (x != 316) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(39,73) != 65502) failures++;
    }


    {
        if (((uint16_t)(((151 + 95) + 85) + 2)) != 333) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {227,72,11630,192};
        if (s.a != (uint8_t)227) failures++;
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {214,22,55344,247};
        if (s.b != (uint8_t)22) failures++;
    }


    {
        uint16_t r = 43465 + 5596 + 43542 + 15648 + 27999 + 52231 + 12969 + 25887;
        if (r != 30729) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {206,5,30874,144};
        if (s.c != (uint16_t)30874) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(218,131) != 349) failures++;
    }


    {
        volatile uint8_t port = 212;
        uint8_t r = port;
        if (r != 212) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)207) + (uint16_t)27095;
        if (r != 27302) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 167;
        if (buf[10] != 167) failures++;
    }


    {
        uint8_t x = 46;
        x <<= 5;
        if (x != 192) failures++;
    }


    {
        uint16_t r = call6(120,229,42,220,188,109);
        if (r != 908) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 178;
        x = x + 103;
        if (x != 281) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 106;
        if (buf[14] != 106) failures++;
    }


    {
        uint8_t m[3][4] = {{36,92,114,76},{40,64,167,133},{75,57,177,19}};
        if (m[1][0] != 40) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)96) % (int16_t)((int8_t)-88);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 55777;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 4900 + 9560 + 62237 + 33055 + 12272 + 61536 + 11137 + 60919;
        if (r != 59008) failures++;
    }


    {
        uint16_t x = 102;
        x = x + 176;
        if (x != 278) failures++;
    }


    {
        if (((uint16_t)230) != 230) failures++;
    }


    {
        uint8_t buf[8] = {183,60,185,65,205,67,99,74};
        uint8_t *p = buf;
        p += 3;
        if (*p != 65) failures++;
    }


    {
        uint32_t a = 3771088293UL;
        uint32_t b = 2705750655UL;
        uint32_t r = a & b;
        if (r != 2688956453UL) failures++;
    }


    {
        if (((uint16_t)((135 + (78 | 47)) ^ ((42 ^ 154) ^ (104 - 227)))) != 65475) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {225,4,46235,214};
        if (s.c != (uint16_t)46235) failures++;
    }


    {
        uint8_t v = 233;
        v |= 128;
        if (v != 233) failures++;
    }


    {
        volatile int16_t a = -9428;
        volatile int16_t b = 32025;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 6485;
        volatile int16_t b = 31852;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        uint8_t m[3][3] = {{204,111,235},{14,125,92},{112,182,231}};
        if (m[2][1] != 182) failures++;
    }


    {
        uint8_t v = 241;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {35,66,55991,69};
        if (s.c != (uint16_t)55991) failures++;
    }


    {
        uint16_t r = call6(186,225,77,36,200,93);
        if (r != 817) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 15: result = 235; break;
        case 18: result = 246; break;
        case 19: result = 39; break;
        case 3: result = 151; break;
        case 4: result = 227; break;
        default: result = 203; break;
        }
        if (result != 227) failures++;
    }


    {
        uint8_t m[4][3] = {{29,55,190},{86,186,4},{132,91,188},{185,135,176}};
        if (m[2][0] != 132) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(210,181) != 29) failures++;
    }


    {
        if (((uint16_t)((186 | 134) & ((116 - 244) & 231))) != 128) failures++;
    }


    {
        uint16_t r = 2666 + 12884 + 22157 + 16997 + 19686 + 24652 + 65388 + 165;
        if (r != 33523) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)50) + (uint16_t)26290;
        if (r != 26340) failures++;
    }


    {
        uint8_t a[6] = {6,225,210,26,43,107};
        if (a[5] != 107) failures++;
    }


    {
        uint16_t r = add2(145,77) + add2(77,160) + add2(145,160);
        if (r != 764) failures++;
    }


    {
        if (((uint16_t)(225 + ((21 & 102) ^ 17))) != 246) failures++;
    }


    {
        volatile uint8_t port = 226;
        uint8_t r = port;
        if (r != 226) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        volatile int16_t a = -31403;
        volatile int16_t b = -20146;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][4] = {{108,93,52,136},{17,130,27,78},{227,175,16,43},{96,53,88,144}};
        if (m[1][0] != 17) failures++;
    }


    {
        uint8_t src[6] = {177,164,129,2,195,187};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 187) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(244,67) != 311) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(127,167) != 294) failures++;
    }


    {
        uint16_t r = add2(76,154) + add2(154,36) + add2(76,36);
        if (r != 532) failures++;
    }


    {
        volatile int16_t a = 2777;
        volatile int16_t b = -4404;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-24) % (int16_t)((int8_t)-113);
        if ((uint16_t)r != (uint16_t)65512) failures++;
    }


    {
        uint16_t r = add2(245,236) + add2(236,194) + add2(245,194);
        if (r != 1350) failures++;
    }


    {
        if (((uint16_t)(172 ^ 50)) != 158) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {73,243,29489,200};
        if (s.c != (uint16_t)29489) failures++;
    }


    {
        uint8_t m[4][2] = {{250,57},{237,98},{133,8},{46,116}};
        if (m[2][0] != 133) failures++;
    }


    {
        volatile int16_t a = -3100;
        volatile int16_t b = 9637;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 224;
        v |= 64;
        if (v != 224) failures++;
    }


    {
        int8_t a = -122;
        int8_t b = 42;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 19: result = 4; break;
        case 8: result = 47; break;
        case 12: result = 164; break;
        default: result = 121; break;
        }
        if (result != 4) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {122,143,11023,245};
        if (s.d != (uint8_t)245) failures++;
    }


    {
        volatile int16_t a = -18537;
        volatile int16_t b = -22274;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 9: result = 56; break;
        case 6: result = 132; break;
        case 7: result = 26; break;
        case 2: result = 124; break;
        case 0: result = 115; break;
        case 5: result = 187; break;
        case 19: result = 111; break;
        case 8: result = 189; break;
        default: result = 147; break;
        }
        if (result != 132) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        if (((uint16_t)(124 + ((220 | 185) + (254 ^ 57)))) != 576) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        volatile uint8_t port = 139;
        uint8_t r = port;
        if (r != 139) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(38,117,48,0,185,128);
        if (r != 516) failures++;
    }


    {
        uint16_t r = 33513 + 16260 + 12638 + 26511 + 51422 + 60073 + 52382 + 284;
        if (r != 56475) failures++;
    }


    {
        volatile uint8_t port = 88;
        uint8_t r = port;
        if (r != 88) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 1) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t v = 90;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 5: result = 223; break;
        case 3: result = 29; break;
        case 0: result = 12; break;
        case 13: result = 246; break;
        default: result = 29; break;
        }
        if (result != 246) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 2) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint32_t a = 3174447558UL;
        uint32_t b = 1463776048UL;
        uint32_t r = a - b;
        if (r != 1710671510UL) failures++;
    }


    {
        if (((uint16_t)(((135 & 45) | (178 & 225)) - ((70 | 6) | (251 + 119)))) != 65327) failures++;
    }


    {
        uint16_t r = 35214 + 64672 + 28219 + 6697 + 4999 + 23289 + 37409 + 61944;
        if (r != 299) failures++;
    }


    {
        uint16_t r = 26006 + 23516 + 63696 + 32129 + 50924 + 54484 + 12744 + 62230;
        if (r != 63585) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(174,203) != 377) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-126) / (int16_t)((int8_t)-67);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t x = 23416;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][3] = {{226,68,137},{112,212,228},{125,236,65}};
        if (m[0][2] != 137) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 3;
        uint8_t r = port;
        if (r != 3) failures++;
    }


    {
        uint32_t a = 870441434UL;
        uint32_t b = 1022377855UL;
        uint32_t r = a + b;
        if (r != 1892819289UL) failures++;
    }


    {
        g16 = 16765;
        if (read_g16() != 16765) failures++;
    }


    {
        uint8_t m[2][2] = {{73,3},{49,233}};
        if (m[0][0] != 73) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t r = call6(74,141,200,39,11,127);
        if (r != 592) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(137,39) != 98) failures++;
    }


    {
        g16 = 25322;
        if (read_g16() != 25322) failures++;
    }


    {
        volatile int16_t a = 19092;
        volatile int16_t b = 22113;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 254;
        if (buf[15] != 254) failures++;
    }


    {
        volatile int16_t a = 8720;
        volatile int16_t b = 13426;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[3][2] = {{1,59},{3,44},{83,132}};
        if (m[0][1] != 59) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 1) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint16_t r = 9840 + 29826 + 39274 + 46636 + 60711 + 38919 + 2882 + 54482;
        if (r != 20426) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)29) / (int16_t)((int8_t)113);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        int8_t a = -109;
        int8_t b = -55;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -6643;
        volatile int16_t b = 16337;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 37327 + 56948 + 55476 + 5489 + 54723 + 39961 + 48400 + 20640;
        if (r != 56820) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {195,195,5,247,219,82};
        if (a[2] != 5) failures++;
    }


    {
        uint8_t v = 23;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 105) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(83,29) != 112) failures++;
    }


    {
        volatile int16_t a = -20694;
        volatile int16_t b = -16366;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 189;
        x = x + 110;
        if (x != 299) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint8_t src[3] = {37,181,71};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 71) failures++;
    }


    {
        g16 = 29603;
        if (read_g16() != 29603) failures++;
    }


    {
        uint8_t x = 167;
        x <<= 0;
        if (x != 167) failures++;
    }


    {
        volatile int16_t a = -18308;
        volatile int16_t b = -30382;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[2][2] = {{141,192},{131,7}};
        if (m[1][0] != 131) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        volatile uint8_t port = 11;
        uint8_t r = port;
        if (r != 11) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 15246;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 46847 + 28595 + 60165 + 36833 + 23535 + 50483 + 36575 + 28028;
        if (r != 48917) failures++;
    }


    {
        uint16_t x = 40550;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(((207 - 92) - (174 + 162)) ^ 119)) != 65364) failures++;
    }


    {
        uint16_t r = 52429 + 8793 + 36058 + 64043 + 10663 + 37511 + 48730 + 43206;
        if (r != 39289) failures++;
    }


    {
        int8_t a = -98;
        int8_t b = 36;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 1) sum += j;
        if (sum != 0) failures++;
    }


    {
        int8_t a = -64;
        int8_t b = 69;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 62561 + 39348 + 23776 + 45131 + 55832 + 4542 + 7463 + 62501;
        if (r != 39010) failures++;
    }


    {
        uint16_t r = 62750 + 42865 + 6652 + 55862 + 29173 + 29333 + 33557 + 11499;
        if (r != 9547) failures++;
    }


    {
        uint8_t m[4][2] = {{6,65},{192,170},{237,32},{133,47}};
        if (m[3][0] != 133) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 231;
        if (buf[10] != 231) failures++;
    }


    {
        if (((uint16_t)(((165 - 168) | 249) | 4)) != 65533) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {218,229,43607,169};
        if (s.d != (uint8_t)169) failures++;
    }


    {
        g16 = 52913;
        if (read_g16() != 52913) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)69) + (uint16_t)29790;
        if (r != 29859) failures++;
    }


    {
        volatile uint8_t port = 28;
        uint8_t r = port;
        if (r != 28) failures++;
    }


    {
        int8_t a = -4;
        int8_t b = 119;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 41;
        uint8_t r = port;
        if (r != 41) failures++;
    }


    {
        volatile uint8_t port = 152;
        uint8_t r = port;
        if (r != 152) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t v = 155;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int8_t a = -27;
        int8_t b = 25;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 11;
        int8_t b = 28;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 165;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t v = 1;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 18: result = 187; break;
        case 15: result = 49; break;
        case 0: result = 217; break;
        default: result = 197; break;
        }
        if (result != 197) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)222) + (uint16_t)6596;
        if (r != 6818) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {5,13,34493,92};
        if (s.d != (uint8_t)92) failures++;
    }


    {
        uint8_t v = 206;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 18) failures++;
    }


    {
        uint8_t v = 33;
        v |= 32;
        if (v != 33) failures++;
    }


    {
        uint16_t x = 182;
        x = x + 76;
        if (x != 258) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)101) % (int16_t)((int8_t)-11);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint16_t x = 62649;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {145,71,106,129,81,148,100,69};
        uint8_t *p = buf;
        p += 3;
        if (*p != 129) failures++;
    }


    {
        uint8_t x = 255;
        x <<= 3;
        if (x != 248) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(122,117) != 5) failures++;
    }


    {
        uint8_t v = 206;
        v |= 8;
        if (v != 206) failures++;
    }


    {
        uint8_t m[3][3] = {{143,91,206},{1,195,166},{87,170,233}};
        if (m[1][2] != 166) failures++;
    }


    {
        uint8_t v = 123;
        v ^= 8;
        if (v != 115) failures++;
    }


    {
        g16 = 53329;
        if (read_g16() != 53329) failures++;
    }


    {
        uint8_t src[12] = {73,93,197,147,106,88,101,175,95,101,240,166};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[3] != 147) failures++;
    }


    {
        uint8_t src[10] = {8,57,42,192,73,107,65,122,103,57};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[2] != 42) failures++;
    }


    {
        uint8_t buf[8] = {140,89,65,90,117,92,212,194};
        uint8_t *p = buf;
        p += 1;
        if (*p != 89) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 0: result = 1; break;
        case 12: result = 66; break;
        case 10: result = 103; break;
        case 5: result = 89; break;
        case 11: result = 178; break;
        case 6: result = 123; break;
        case 15: result = 11; break;
        case 9: result = 181; break;
        default: result = 130; break;
        }
        if (result != 130) failures++;
    }


    {
        uint16_t r = 29921 + 18410 + 60366 + 51561 + 37149 + 52816 + 37127 + 7435;
        if (r != 32641) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t v = 251;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {109,242,112,119,126,39,172,10};
        uint8_t *p = buf;
        p += 1;
        if (*p != 242) failures++;
    }


    {
        uint8_t buf[8] = {155,120,12,179,142,55,101,233};
        uint8_t *p = buf;
        p += 2;
        if (*p != 12) failures++;
    }


    {
        g16 = 24266;
        if (read_g16() != 24266) failures++;
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
        if (fn(91,134) != 225) failures++;
    }


    {
        uint16_t r = add2(92,79) + add2(79,148) + add2(92,148);
        if (r != 638) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)114) + (uint16_t)1124;
        if (r != 1238) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 14;
        if (buf[14] != 14) failures++;
    }


    {
        if (((uint16_t)((11 - (92 - 4)) + (163 & (213 & 47)))) != 65460) failures++;
    }


    {
        uint32_t a = 2348059931UL;
        uint32_t b = 2747258484UL;
        uint32_t r = a - b;
        if (r != 3895768743UL) failures++;
    }


    {
        uint16_t x = 36823;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {164,4,149,28,210,153};
        if (a[1] != 4) failures++;
    }


    {
        uint8_t src[7] = {11,17,158,108,86,48,208};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[6] != 208) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(187,70) != 257) failures++;
    }


    {
        uint8_t a[6] = {175,228,38,104,243,53};
        if (a[0] != 175) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 128;
        if (x != 326) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 247;
        if (buf[15] != 247) failures++;
    }


    {
        uint8_t buf[8] = {234,56,191,162,185,18,12,14};
        uint8_t *p = buf;
        p += 2;
        if (*p != 191) failures++;
    }


    {
        uint32_t a = 644542621UL;
        uint32_t b = 2909812745UL;
        uint32_t r = a + b;
        if (r != 3554355366UL) failures++;
    }


    {
        uint8_t m[2][4] = {{67,22,126,122},{203,195,152,99}};
        if (m[0][2] != 126) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 1: result = 118; break;
        case 0: result = 193; break;
        case 5: result = 127; break;
        case 10: result = 35; break;
        case 14: result = 109; break;
        case 4: result = 139; break;
        case 2: result = 180; break;
        case 12: result = 226; break;
        default: result = 146; break;
        }
        if (result != 146) failures++;
    }


    {
        uint8_t v = 254;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
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
        uint16_t x = 9411;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -7668;
        volatile int16_t b = 23327;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {108,14,208,53,214,235,102,217};
        uint8_t *p = buf;
        p += 1;
        if (*p != 14) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {194,110,53745,111};
        if (s.d != (uint8_t)111) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)98) % (int16_t)((int8_t)-128);
        if ((uint16_t)r != (uint16_t)98) failures++;
    }


    {
        uint16_t r = 11913 + 60834 + 23260 + 44397 + 44133 + 56092 + 31217 + 41897;
        if (r != 51599) failures++;
    }


    {
        g16 = 64330;
        if (read_g16() != 64330) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 133;
        if (buf[6] != 133) failures++;
    }


    {
        uint8_t v = 119;
        v ^= 16;
        if (v != 103) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {250,78,23156,64};
        if (s.d != (uint8_t)64) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 128;
        if (buf[11] != 128) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 80;
        if (buf[14] != 80) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t a[6] = {9,24,60,150,28,242};
        if (a[1] != 24) failures++;
    }


    {
        uint16_t r = add2(164,252) + add2(252,23) + add2(164,23);
        if (r != 878) failures++;
    }


    {
        uint8_t x = 107;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)113) % (int16_t)((int8_t)-22);
        if ((uint16_t)r != (uint16_t)3) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)3) + (uint16_t)57195;
        if (r != 57198) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -11959;
        volatile int16_t b = -22373;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 53113 + 32733 + 40304 + 32676 + 3639 + 51440 + 19835 + 35987;
        if (r != 7583) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(86,192,37,75,24,182);
        if (r != 596) failures++;
    }


    {
        int8_t a = -66;
        int8_t b = 28;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 0;
        x <<= 1;
        if (x != 0) failures++;
    }


    {
        uint8_t src[6] = {49,55,154,41,196,122};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[2] != 154) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-105) / (int16_t)((int8_t)-109);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {149,224,47787,141};
        if (s.a != (uint8_t)149) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {111,26,46777,182};
        if (s.b != (uint8_t)26) failures++;
    }


    {
        int8_t a = -44;
        int8_t b = -56;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 115;
        v |= 16;
        if (v != 115) failures++;
    }


    {
        uint8_t m[4][2] = {{250,180},{149,138},{166,169},{39,146}};
        if (m[0][1] != 180) failures++;
    }


    {
        uint16_t r = call6(165,183,245,73,83,105);
        if (r != 854) failures++;
    }


    {
        uint8_t src[3] = {117,2,181};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 2) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-91) % (int16_t)((int8_t)65);
        if ((uint16_t)r != (uint16_t)65510) failures++;
    }


    {
        volatile int16_t a = -22659;
        volatile int16_t b = -18143;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 246;
        uint8_t r = port;
        if (r != 246) failures++;
    }


    {
        uint8_t x = 125;
        x <<= 1;
        if (x != 250) failures++;
    }


    {
        uint32_t a = 2696004766UL;
        uint32_t b = 1057797239UL;
        uint32_t r = a ^ b;
        if (r != 2679992553UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 58364;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        g16 = 59786;
        if (read_g16() != 59786) failures++;
    }


    {
        int8_t a = -89;
        int8_t b = -64;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 33212 + 63706 + 15870 + 33066 + 5724 + 14746 + 11045 + 36754;
        if (r != 17515) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {99,109,43323,88};
        if (s.c != (uint16_t)43323) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)65) + (uint16_t)65283;
        if (r != 65348) failures++;
    }


    {
        uint16_t r = add2(123,89) + add2(89,139) + add2(123,139);
        if (r != 702) failures++;
    }


    {
        uint8_t m[2][2] = {{121,52},{233,182}};
        if (m[1][0] != 233) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)142) + (uint16_t)2883;
        if (r != 3025) failures++;
    }


    {
        uint16_t x = 35752;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 2885;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 19194;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 42258 + 5448 + 25596 + 50964 + 27013 + 5326 + 32144 + 44369;
        if (r != 36510) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint8_t src[12] = {191,104,11,167,15,251,219,69,108,91,80,31};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[2] != 11) failures++;
    }


    {
        uint8_t v = 249;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 6: result = 225; break;
        case 15: result = 206; break;
        case 14: result = 199; break;
        case 4: result = 195; break;
        default: result = 89; break;
        }
        if (result != 199) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = 5352;
        volatile int16_t b = 21164;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint16_t x = 3;
        x = x + 212;
        if (x != 215) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 163;
        if (buf[14] != 163) failures++;
    }


    {
        uint8_t m[2][3] = {{198,219,20},{20,120,70}};
        if (m[1][2] != 70) failures++;
    }


    {
        if (((uint16_t)(((201 ^ 254) | (225 - 3)) & ((94 - 156) + (251 + 232)))) != 165) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {116,45,50478,223};
        if (s.c != (uint16_t)50478) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)91) / (int16_t)((int8_t)48);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(141,66) != 207) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)114) + (uint16_t)37616;
        if (r != 37730) failures++;
    }


    {
        uint16_t x = 40;
        x = x + 146;
        if (x != 186) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        g16 = 57424;
        if (read_g16() != 57424) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = call6(2,42,169,127,245,187);
        if (r != 772) failures++;
    }


    {
        volatile int16_t a = -15417;
        volatile int16_t b = -2866;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -43;
        int8_t b = -104;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)57) / (int16_t)((int8_t)-126);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = call6(202,206,192,39,221,185);
        if (r != 1045) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)207) + (uint16_t)57362;
        if (r != 57569) failures++;
    }


    {
        uint16_t r = add2(155,150) + add2(150,89) + add2(155,89);
        if (r != 788) failures++;
    }


    {
        volatile int16_t a = 1396;
        volatile int16_t b = 11944;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)((53 ^ (125 ^ 14)) - 31)) != 39) failures++;
    }


    {
        uint8_t v = 86;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 3180653417UL;
        uint32_t b = 2970940164UL;
        uint32_t r = a | b;
        if (r != 3180657517UL) failures++;
    }


    {
        uint16_t x = 33665;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {147,172,5294,48};
        if (s.c != (uint16_t)5294) failures++;
    }


    {
        uint8_t x = 254;
        x <<= 5;
        if (x != 192) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)5) / (int16_t)((int8_t)25);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 39307;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 132;
        v |= 64;
        if (v != 196) failures++;
    }


    {
        int8_t a = 51;
        int8_t b = 40;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 247;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(96,75) != 171) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 3: result = 18; break;
        case 9: result = 67; break;
        case 0: result = 94; break;
        case 8: result = 188; break;
        default: result = 130; break;
        }
        if (result != 67) failures++;
    }


    {
        uint8_t buf[8] = {191,114,56,69,124,34,36,237};
        uint8_t *p = buf;
        p += 7;
        if (*p != 237) failures++;
    }


    {
        volatile int16_t a = -11177;
        volatile int16_t b = -32660;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {28,241,14,62,40,68};
        if (a[1] != 241) failures++;
    }


    {
        uint8_t a[6] = {99,60,52,30,208,5};
        if (a[4] != 208) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 134;
        if (buf[8] != 134) failures++;
    }


    {
        uint8_t v = 150;
        v &= ~(uint8_t)32;
        if (v != 150) failures++;
    }


    {
        uint16_t x = 185;
        x = x + 106;
        if (x != 291) failures++;
    }


    {
        uint8_t v = 227;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 58122;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 66;
        int r = (v & 4) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 9: result = 254; break;
        case 17: result = 156; break;
        case 14: result = 100; break;
        case 12: result = 19; break;
        default: result = 124; break;
        }
        if (result != 156) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 2: result = 195; break;
        case 18: result = 122; break;
        case 5: result = 23; break;
        default: result = 155; break;
        }
        if (result != 23) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 187;
        if (buf[8] != 187) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(78,224) != 302) failures++;
    }


    {
        uint16_t x = 52140;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(11,33,28,125,88,74);
        if (r != 359) failures++;
    }


    {
        uint16_t r = call6(136,24,245,243,205,71);
        if (r != 924) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(208,13) != 221) failures++;
    }


    {
        uint16_t r = 19314 + 17101 + 50812 + 15195 + 22341 + 48707 + 47557 + 53161;
        if (r != 12044) failures++;
    }


    {
        g16 = 55799;
        if (read_g16() != 55799) failures++;
    }


    {
        volatile uint8_t port = 245;
        uint8_t r = port;
        if (r != 245) failures++;
    }


    {
        volatile int16_t a = 16036;
        volatile int16_t b = -1553;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[5] = {63,116,22,17,200};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[4] != 200) failures++;
    }


    {
        volatile uint8_t port = 92;
        uint8_t r = port;
        if (r != 92) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 133;
        if (buf[15] != 133) failures++;
    }


    {
        uint8_t v = 0;
        v ^= 8;
        if (v != 8) failures++;
    }


    {
        uint8_t v = 95;
        v ^= 8;
        if (v != 87) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(76,92) != 65520) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)86) % (int16_t)((int8_t)64);
        if ((uint16_t)r != (uint16_t)22) failures++;
    }

    return failures;
}