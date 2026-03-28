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
        uint32_t a = 1565996902UL;
        uint32_t b = 401894965UL;
        uint32_t r = a ^ b;
        if (r != 1252219219UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)35) / (int16_t)((int8_t)-18);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {235,2,44836,140};
        if (s.c != (uint16_t)44836) failures++;
    }


    {
        volatile int16_t a = -25898;
        volatile int16_t b = -5902;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 126;
        x <<= 3;
        if (x != 240) failures++;
    }


    {
        uint8_t m[3][4] = {{160,77,109,37},{18,10,28,122},{95,75,241,129}};
        if (m[0][2] != 109) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)253) + (uint16_t)53869;
        if (r != 54122) failures++;
    }


    {
        uint8_t x = 255;
        x <<= 4;
        if (x != 240) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)0) + (uint16_t)24777;
        if (r != 24777) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 48;
        if (buf[0] != 48) failures++;
    }


    {
        uint16_t x = 106;
        x = x + 118;
        if (x != 224) failures++;
    }


    {
        uint16_t r = 53546 + 19568 + 54177 + 7334 + 13211 + 62855 + 31559 + 35674;
        if (r != 15780) failures++;
    }


    {
        uint8_t v = 128;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint16_t x = 36069;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t r = add2(96,193) + add2(193,213) + add2(96,213);
        if (r != 1004) failures++;
    }


    {
        uint8_t m[2][4] = {{20,250,6,110},{241,10,248,160}};
        if (m[0][3] != 110) failures++;
    }


    {
        uint16_t r = 41160 + 57430 + 49003 + 62390 + 11331 + 29020 + 18355 + 42329;
        if (r != 48874) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)1) + (uint16_t)44000;
        if (r != 44001) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 3: result = 51; break;
        case 10: result = 219; break;
        case 1: result = 6; break;
        case 16: result = 237; break;
        case 9: result = 93; break;
        case 17: result = 162; break;
        case 19: result = 167; break;
        case 15: result = 216; break;
        default: result = 144; break;
        }
        if (result != 219) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {114,18,49599,79};
        if (s.a != (uint8_t)114) failures++;
    }


    {
        int8_t a = 23;
        int8_t b = 31;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)240) + (uint16_t)58739;
        if (r != 58979) failures++;
    }


    {
        uint16_t x = 253;
        x = x + 111;
        if (x != 364) failures++;
    }


    {
        int8_t a = -68;
        int8_t b = 112;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(190,167) != 23) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 7: result = 54; break;
        case 10: result = 239; break;
        case 1: result = 26; break;
        default: result = 62; break;
        }
        if (result != 239) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 1;
        if (buf[3] != 1) failures++;
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
        if (fn(31,52) != 83) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 4183023023UL;
        uint32_t b = 3975712684UL;
        uint32_t r = a & b;
        if (r != 3897593260UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 2) sum += j;
        if (sum != 42) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 7: result = 184; break;
        case 12: result = 114; break;
        case 13: result = 21; break;
        case 6: result = 129; break;
        case 2: result = 255; break;
        case 17: result = 243; break;
        case 4: result = 132; break;
        case 8: result = 130; break;
        default: result = 224; break;
        }
        if (result != 224) failures++;
    }


    {
        uint16_t x = 77;
        x = x + 125;
        if (x != 202) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)120) % (int16_t)((int8_t)-62);
        if ((uint16_t)r != (uint16_t)58) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 13;
        if (buf[6] != 13) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)6) + (uint16_t)16658;
        if (r != 16664) failures++;
    }


    {
        volatile uint8_t port = 197;
        uint8_t r = port;
        if (r != 197) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 85;
        x = x + 87;
        if (x != 172) failures++;
    }


    {
        uint16_t r = call6(254,155,131,90,5,28);
        if (r != 663) failures++;
    }


    {
        uint8_t x = 211;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        uint16_t r = 54661 + 62941 + 33258 + 41157 + 36130 + 48866 + 23686 + 4033;
        if (r != 42588) failures++;
    }


    {
        uint32_t a = 277586517UL;
        uint32_t b = 2183293619UL;
        uint32_t r = a ^ b;
        if (r != 2460600550UL) failures++;
    }


    {
        uint8_t v = 252;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        if (((uint16_t)24) != 24) failures++;
    }


    {
        uint16_t r = call6(187,151,177,130,222,249);
        if (r != 1116) failures++;
    }


    {
        uint32_t a = 3423746251UL;
        uint32_t b = 2496517525UL;
        uint32_t r = a + b;
        if (r != 1625296480UL) failures++;
    }


    {
        uint16_t x = 15145;
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
        case 10: result = 216; break;
        case 15: result = 190; break;
        case 17: result = 218; break;
        case 1: result = 5; break;
        case 5: result = 133; break;
        case 19: result = 252; break;
        default: result = 8; break;
        }
        if (result != 190) failures++;
    }


    {
        uint8_t m[4][3] = {{149,98,7},{253,55,75},{208,100,56},{165,88,83}};
        if (m[0][1] != 98) failures++;
    }


    {
        uint16_t r = call6(52,80,111,38,147,205);
        if (r != 633) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint16_t x = 26973;
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
        case 11: result = 39; break;
        case 15: result = 246; break;
        case 4: result = 237; break;
        case 2: result = 140; break;
        case 9: result = 69; break;
        case 12: result = 105; break;
        default: result = 198; break;
        }
        if (result != 198) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {129,42,4683,143};
        if (s.b != (uint8_t)42) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(43,186) != 229) failures++;
    }


    {
        int8_t a = 58;
        int8_t b = -78;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 63006;
        if (read_g16() != 63006) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(38,214) != 252) failures++;
    }


    {
        uint8_t m[3][3] = {{174,26,77},{39,53,40},{134,161,235}};
        if (m[2][2] != 235) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(244,111,77,37,58,102);
        if (r != 629) failures++;
    }


    {
        uint16_t r = call6(189,172,198,40,212,128);
        if (r != 939) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 1) sum += j;
        if (sum != 45) failures++;
    }


    {
        if (((uint16_t)(((202 & 231) & (15 + 106)) - ((110 - 204) - (178 ^ 237)))) != 253) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        volatile uint8_t port = 115;
        uint8_t r = port;
        if (r != 115) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)94) % (int16_t)((int8_t)81);
        if ((uint16_t)r != (uint16_t)13) failures++;
    }


    {
        uint8_t a[6] = {110,240,172,133,199,132};
        if (a[2] != 172) failures++;
    }


    {
        uint8_t x = 243;
        x <<= 0;
        if (x != 243) failures++;
    }


    {
        uint16_t x = 9445;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 232;
        x = x + 8;
        if (x != 240) failures++;
    }


    {
        if (((uint16_t)190) != 190) failures++;
    }


    {
        volatile int16_t a = -2073;
        volatile int16_t b = -22910;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {149,166,54,228,188,163};
        if (a[2] != 54) failures++;
    }


    {
        uint16_t r = 61016 + 9566 + 39400 + 24106 + 19059 + 47934 + 43782 + 16357;
        if (r != 64612) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        volatile int16_t a = -20787;
        volatile int16_t b = 24128;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 62862;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 27;
        uint8_t r = port;
        if (r != 27) failures++;
    }


    {
        uint8_t a[6] = {253,52,158,56,165,43};
        if (a[5] != 43) failures++;
    }


    {
        volatile uint8_t port = 239;
        uint8_t r = port;
        if (r != 239) failures++;
    }


    {
        uint8_t src[16] = {111,113,11,140,230,249,201,226,216,44,130,224,148,102,44,24};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[3] != 140) failures++;
    }


    {
        uint16_t r = call6(140,241,62,41,28,151);
        if (r != 663) failures++;
    }


    {
        uint16_t x = 65125;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)68) + (uint16_t)11171;
        if (r != 11239) failures++;
    }


    {
        uint32_t a = 2084442008UL;
        uint32_t b = 2025743445UL;
        uint32_t r = a + b;
        if (r != 4110185453UL) failures++;
    }


    {
        uint8_t a[6] = {25,247,220,30,102,190};
        if (a[5] != 190) failures++;
    }


    {
        if (((uint16_t)(((82 ^ 8) & (43 + 177)) + ((189 ^ 38) - (52 + 200)))) != 65527) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)61) + (uint16_t)8689;
        if (r != 8750) failures++;
    }


    {
        uint8_t m[2][2] = {{31,238},{90,242}};
        if (m[1][1] != 242) failures++;
    }


    {
        uint8_t v = 178;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        uint16_t x = 143;
        x = x + 212;
        if (x != 355) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {175,37,6623,23};
        if (s.d != (uint8_t)23) failures++;
    }


    {
        uint16_t x = 212;
        x = x + 190;
        if (x != 402) failures++;
    }


    {
        uint8_t x = 162;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint8_t v = 8;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        volatile int16_t a = 12878;
        volatile int16_t b = 31804;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint8_t m[4][3] = {{77,94,64},{58,107,208},{187,155,120},{98,54,209}};
        if (m[0][0] != 77) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 8: result = 38; break;
        case 6: result = 82; break;
        case 12: result = 152; break;
        case 18: result = 125; break;
        case 10: result = 230; break;
        case 2: result = 224; break;
        case 7: result = 142; break;
        case 4: result = 2; break;
        default: result = 188; break;
        }
        if (result != 230) failures++;
    }


    {
        uint16_t x = 107;
        x = x + 14;
        if (x != 121) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(119,23) != 142) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)22) + (uint16_t)25100;
        if (r != 25122) failures++;
    }


    {
        uint16_t r = 25089 + 58870 + 5117 + 20099 + 45755 + 25635 + 61082 + 9932;
        if (r != 54971) failures++;
    }


    {
        g16 = 29671;
        if (read_g16() != 29671) failures++;
    }


    {
        uint8_t a[6] = {52,154,125,37,159,172};
        if (a[2] != 125) failures++;
    }


    {
        g16 = 11626;
        if (read_g16() != 11626) failures++;
    }


    {
        uint16_t r = call6(173,60,125,76,70,160);
        if (r != 664) failures++;
    }


    {
        volatile uint8_t port = 252;
        uint8_t r = port;
        if (r != 252) failures++;
    }


    {
        uint16_t r = 62747 + 53705 + 7732 + 42793 + 22960 + 21379 + 47709 + 63320;
        if (r != 60201) failures++;
    }


    {
        uint32_t a = 1201274556UL;
        uint32_t b = 2570536024UL;
        uint32_t r = a & b;
        if (r != 17908760UL) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 15: result = 235; break;
        case 14: result = 130; break;
        case 19: result = 100; break;
        case 6: result = 248; break;
        case 10: result = 121; break;
        case 16: result = 243; break;
        case 7: result = 95; break;
        case 17: result = 87; break;
        default: result = 35; break;
        }
        if (result != 235) failures++;
    }


    {
        uint8_t src[1] = {156};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 156) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 1) sum += j;
        if (sum != 21) failures++;
    }


    {
        uint32_t a = 3392979358UL;
        uint32_t b = 4267552593UL;
        uint32_t r = a - b;
        if (r != 3420394061UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 190;
        if (buf[13] != 190) failures++;
    }


    {
        uint8_t v = 74;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {202,148,55,151,93,251,157,107};
        uint8_t *p = buf;
        p += 2;
        if (*p != 55) failures++;
    }


    {
        uint8_t buf[8] = {229,229,233,159,230,65,250,52};
        uint8_t *p = buf;
        p += 0;
        if (*p != 229) failures++;
    }


    {
        uint8_t v = 247;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(60,138) + add2(138,192) + add2(60,192);
        if (r != 780) failures++;
    }


    {
        if (((uint16_t)((25 & (110 | 156)) | ((111 | 239) - (22 + 253)))) != 65500) failures++;
    }


    {
        uint16_t x = 226;
        x = x + 98;
        if (x != 324) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 3: result = 103; break;
        case 9: result = 252; break;
        case 12: result = 173; break;
        case 5: result = 41; break;
        case 10: result = 239; break;
        case 0: result = 63; break;
        case 19: result = 231; break;
        case 13: result = 43; break;
        default: result = 60; break;
        }
        if (result != 173) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(233,25) != 258) failures++;
    }


    {
        volatile uint8_t port = 7;
        uint8_t r = port;
        if (r != 7) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 53; break;
        case 9: result = 105; break;
        case 5: result = 18; break;
        case 2: result = 196; break;
        case 18: result = 79; break;
        default: result = 105; break;
        }
        if (result != 53) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 2) sum += j;
        if (sum != 42) failures++;
    }


    {
        uint8_t src[15] = {11,73,205,177,56,117,232,24,141,52,195,211,130,73,195};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[3] != 177) failures++;
    }


    {
        uint8_t v = 112;
        v ^= 2;
        if (v != 114) failures++;
    }


    {
        uint8_t src[14] = {130,61,234,240,154,21,26,247,19,92,231,100,52,204};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[5] != 21) failures++;
    }


    {
        uint8_t buf[8] = {125,226,243,173,240,217,12,223};
        uint8_t *p = buf;
        p += 2;
        if (*p != 243) failures++;
    }


    {
        g16 = 29047;
        if (read_g16() != 29047) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 81;
        uint8_t r = port;
        if (r != 81) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)203) + (uint16_t)30253;
        if (r != 30456) failures++;
    }


    {
        uint16_t r = 48887 + 56035 + 57968 + 34847 + 24134 + 6131 + 46372 + 33951;
        if (r != 46181) failures++;
    }


    {
        uint8_t m[3][4] = {{126,3,204,248},{224,68,167,66},{224,165,179,74}};
        if (m[1][0] != 224) failures++;
    }


    {
        uint8_t v = 160;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(158,153) + add2(153,187) + add2(158,187);
        if (r != 996) failures++;
    }


    {
        uint8_t src[7] = {220,211,43,116,161,47,234};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[1] != 211) failures++;
    }


    {
        uint8_t buf[8] = {223,210,16,226,21,239,108,178};
        uint8_t *p = buf;
        p += 7;
        if (*p != 178) failures++;
    }


    {
        uint8_t v = 127;
        int r = (v & 8) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t x = 14309;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 59681 + 43070 + 62590 + 64580 + 40252 + 23520 + 61467 + 33887;
        if (r != 61367) failures++;
    }


    {
        if (((uint16_t)61) != 61) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 19;
        if (buf[4] != 19) failures++;
    }


    {
        g16 = 62069;
        if (read_g16() != 62069) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint16_t x = 52;
        x = x + 212;
        if (x != 264) failures++;
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
        case 17: result = 0; break;
        case 14: result = 13; break;
        case 13: result = 113; break;
        case 3: result = 73; break;
        case 19: result = 220; break;
        case 1: result = 112; break;
        case 10: result = 27; break;
        case 8: result = 49; break;
        default: result = 25; break;
        }
        if (result != 220) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = 30764;
        volatile int16_t b = 9161;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[14] = {253,11,236,114,130,79,142,142,25,193,108,230,229,215};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[12] != 229) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = 28569 + 61808 + 8348 + 36877 + 2465 + 13605 + 38123 + 60471;
        if (r != 53658) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(79,69) != 10) failures++;
    }


    {
        uint8_t src[11] = {79,71,68,91,202,111,10,61,215,145,81};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[0] != 79) failures++;
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
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        volatile uint8_t port = 146;
        uint8_t r = port;
        if (r != 146) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 14: result = 87; break;
        case 16: result = 36; break;
        case 17: result = 46; break;
        case 9: result = 165; break;
        case 13: result = 92; break;
        case 11: result = 152; break;
        case 2: result = 248; break;
        default: result = 201; break;
        }
        if (result != 36) failures++;
    }


    {
        uint8_t m[4][4] = {{231,99,181,158},{0,193,41,123},{229,162,27,64},{62,202,242,185}};
        if (m[2][2] != 27) failures++;
    }


    {
        uint16_t x = 28423;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(189,175) + add2(175,40) + add2(189,40);
        if (r != 808) failures++;
    }


    {
        uint16_t r = 6536 + 9521 + 29724 + 5630 + 25776 + 1644 + 10701 + 6963;
        if (r != 30959) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {11,54,709,169};
        if (s.a != (uint8_t)11) failures++;
    }


    {
        g16 = 12596;
        if (read_g16() != 12596) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(36,86) != 65486) failures++;
    }


    {
        int8_t a = -74;
        int8_t b = 110;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        g16 = 54515;
        if (read_g16() != 54515) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 211;
        if (buf[10] != 211) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t v = 120;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = 24819;
        volatile int16_t b = 6578;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2914955706UL;
        uint32_t b = 4075756636UL;
        uint32_t r = a | b;
        if (r != 4294950398UL) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 12: result = 25; break;
        case 7: result = 219; break;
        case 17: result = 2; break;
        case 0: result = 229; break;
        case 9: result = 107; break;
        case 6: result = 56; break;
        default: result = 199; break;
        }
        if (result != 229) failures++;
    }


    {
        volatile int16_t a = -19236;
        volatile int16_t b = -9780;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(94,61) + add2(61,40) + add2(94,40);
        if (r != 390) failures++;
    }


    {
        int8_t a = 79;
        int8_t b = 77;
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
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 2: result = 114; break;
        case 18: result = 57; break;
        case 4: result = 84; break;
        case 11: result = 0; break;
        case 6: result = 119; break;
        case 14: result = 23; break;
        default: result = 203; break;
        }
        if (result != 57) failures++;
    }


    {
        uint8_t m[3][2] = {{119,105},{175,181},{182,108}};
        if (m[2][1] != 108) failures++;
    }


    {
        uint8_t a[6] = {179,202,181,36,202,65};
        if (a[4] != 202) failures++;
    }


    {
        if (((uint16_t)123) != 123) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 230;
        if (buf[3] != 230) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 11: result = 81; break;
        case 16: result = 31; break;
        case 18: result = 36; break;
        case 2: result = 147; break;
        case 10: result = 137; break;
        default: result = 101; break;
        }
        if (result != 147) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {197,184,560,2};
        if (s.d != (uint8_t)2) failures++;
    }


    {
        uint8_t v = 82;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 230;
        uint8_t r = port;
        if (r != 230) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {228,219,56604,201};
        if (s.c != (uint16_t)56604) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)12) + (uint16_t)21268;
        if (r != 21280) failures++;
    }


    {
        int8_t a = 40;
        int8_t b = 3;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {2,61,40597,67};
        if (s.b != (uint8_t)61) failures++;
    }


    {
        uint8_t m[3][2] = {{72,68},{134,69},{120,100}};
        if (m[2][1] != 100) failures++;
    }


    {
        uint32_t a = 2544829319UL;
        uint32_t b = 1628233139UL;
        uint32_t r = a ^ b;
        if (r != 4137934388UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)52) + (uint16_t)64813;
        if (r != 64865) failures++;
    }


    {
        volatile int16_t a = 24;
        volatile int16_t b = 20102;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {2,141,18514,173};
        if (s.c != (uint16_t)18514) failures++;
    }


    {
        int8_t a = 39;
        int8_t b = -86;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {21,168,231,85,168,50,189,133};
        uint8_t *p = buf;
        p += 2;
        if (*p != 231) failures++;
    }


    {
        uint32_t a = 3744930009UL;
        uint32_t b = 4264176975UL;
        uint32_t r = a + b;
        if (r != 3714139688UL) failures++;
    }


    {
        uint8_t v = 53;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)32) + (uint16_t)43708;
        if (r != 43740) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 253;
        if (buf[5] != 253) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 11: result = 220; break;
        case 12: result = 178; break;
        case 13: result = 113; break;
        case 7: result = 30; break;
        default: result = 74; break;
        }
        if (result != 178) failures++;
    }


    {
        uint16_t r = 7662 + 55771 + 31757 + 42519 + 8813 + 21286 + 14243 + 43521;
        if (r != 28964) failures++;
    }


    {
        uint8_t x = 236;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)83) % (int16_t)((int8_t)50);
        if ((uint16_t)r != (uint16_t)33) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 19: result = 231; break;
        case 13: result = 32; break;
        case 1: result = 218; break;
        case 15: result = 212; break;
        case 12: result = 15; break;
        case 5: result = 48; break;
        case 7: result = 58; break;
        default: result = 6; break;
        }
        if (result != 48) failures++;
    }


    {
        uint16_t r = 34874 + 26920 + 970 + 58326 + 21163 + 45272 + 38946 + 6766;
        if (r != 36629) failures++;
    }


    {
        uint8_t v = 38;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        g16 = 31929;
        if (read_g16() != 31929) failures++;
    }


    {
        uint16_t r = call6(246,99,35,74,171,198);
        if (r != 823) failures++;
    }


    {
        uint16_t r = call6(40,126,209,226,164,27);
        if (r != 792) failures++;
    }


    {
        if (((uint16_t)70) != 70) failures++;
    }


    {
        uint16_t x = 61;
        x = x + 129;
        if (x != 190) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t x = 27;
        x <<= 1;
        if (x != 54) failures++;
    }


    {
        if (((uint16_t)(82 + ((76 ^ 132) + (60 | 203)))) != 537) failures++;
    }


    {
        uint8_t a[6] = {28,101,218,65,165,195};
        if (a[4] != 165) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 11: result = 148; break;
        case 15: result = 71; break;
        case 9: result = 158; break;
        case 6: result = 60; break;
        case 3: result = 189; break;
        case 14: result = 152; break;
        case 2: result = 71; break;
        case 5: result = 126; break;
        default: result = 240; break;
        }
        if (result != 240) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 9: result = 214; break;
        case 4: result = 239; break;
        case 18: result = 71; break;
        case 11: result = 8; break;
        case 14: result = 156; break;
        default: result = 205; break;
        }
        if (result != 205) failures++;
    }


    {
        volatile int16_t a = -23861;
        volatile int16_t b = -7103;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 41187 + 59568 + 43284 + 55856 + 27988 + 44685 + 46273 + 55564;
        if (r != 46725) failures++;
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
        volatile uint8_t port = 182;
        uint8_t r = port;
        if (r != 182) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-73) % (int16_t)((int8_t)-57);
        if ((uint16_t)r != (uint16_t)65520) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint16_t r = 49961 + 6058 + 29093 + 3367 + 36034 + 44947 + 44240 + 17776;
        if (r != 34868) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)68) / (int16_t)((int8_t)34);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint8_t m[4][3] = {{250,199,47},{78,70,162},{223,23,28},{242,2,201}};
        if (m[2][2] != 28) failures++;
    }


    {
        uint8_t v = 242;
        v |= 32;
        if (v != 242) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(128,14) != 142) failures++;
    }


    {
        uint8_t v = 89;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 89;
        x = x + 187;
        if (x != 276) failures++;
    }


    {
        uint8_t a[6] = {41,25,7,76,97,253};
        if (a[0] != 41) failures++;
    }


    {
        uint16_t x = 4847;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 59;
        v |= 128;
        if (v != 187) failures++;
    }


    {
        uint8_t v = 158;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 131;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t m[3][4] = {{178,85,68,107},{244,142,145,105},{219,189,212,67}};
        if (m[0][3] != 107) failures++;
    }


    {
        int8_t a = 78;
        int8_t b = -121;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-88) % (int16_t)((int8_t)90);
        if ((uint16_t)r != (uint16_t)65448) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(178,245) != 423) failures++;
    }


    {
        uint16_t r = add2(233,197) + add2(197,144) + add2(233,144);
        if (r != 1148) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint16_t x = 41;
        x = x + 5;
        if (x != 46) failures++;
    }


    {
        uint16_t r = 19941 + 50110 + 15111 + 31287 + 35336 + 42851 + 17307 + 10817;
        if (r != 26152) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        if (((uint16_t)((64 & 82) ^ (61 ^ 174))) != 211) failures++;
    }


    {
        uint8_t v = 99;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint16_t r = add2(203,244) + add2(244,78) + add2(203,78);
        if (r != 1050) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 7;
        if (buf[9] != 7) failures++;
    }


    {
        uint16_t x = 189;
        x = x + 96;
        if (x != 285) failures++;
    }


    {
        uint8_t v = 17;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 186;
        if (buf[9] != 186) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t r = 2667 + 7146 + 4443 + 40519 + 839 + 26405 + 59541 + 65107;
        if (r != 10059) failures++;
    }


    {
        uint8_t a[6] = {31,34,100,177,160,96};
        if (a[3] != 177) failures++;
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
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 20;
        if (buf[6] != 20) failures++;
    }


    {
        uint16_t x = 63911;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {100,17,22897,156};
        if (s.b != (uint8_t)17) failures++;
    }


    {
        uint8_t v = 129;
        v ^= 1;
        if (v != 128) failures++;
    }


    {
        uint16_t r = call6(24,63,185,8,82,199);
        if (r != 561) failures++;
    }


    {
        uint8_t src[11] = {201,113,9,72,13,14,97,169,47,180,157};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[0] != 201) failures++;
    }


    {
        uint8_t src[7] = {198,221,64,208,242,3,39};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[4] != 242) failures++;
    }


    {
        uint16_t r = call6(7,196,37,122,244,157);
        if (r != 763) failures++;
    }


    {
        uint16_t r = call6(165,114,223,19,196,232);
        if (r != 949) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 18: result = 189; break;
        case 13: result = 131; break;
        case 16: result = 179; break;
        case 15: result = 3; break;
        default: result = 187; break;
        }
        if (result != 179) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)204) + (uint16_t)47841;
        if (r != 48045) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-34) % (int16_t)((int8_t)118);
        if ((uint16_t)r != (uint16_t)65502) failures++;
    }


    {
        uint16_t x = 173;
        x = x + 249;
        if (x != 422) failures++;
    }


    {
        volatile int16_t a = 4515;
        volatile int16_t b = -32451;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(199,176) + add2(176,107) + add2(199,107);
        if (r != 964) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)1) + (uint16_t)53508;
        if (r != 53509) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 10: result = 206; break;
        case 11: result = 48; break;
        case 16: result = 225; break;
        default: result = 149; break;
        }
        if (result != 149) failures++;
    }


    {
        uint32_t a = 2361973429UL;
        uint32_t b = 2649073941UL;
        uint32_t r = a ^ b;
        if (r != 288191392UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(77,69) != 146) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)194) + (uint16_t)23884;
        if (r != 24078) failures++;
    }


    {
        uint8_t src[3] = {167,80,42};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 80) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(209,237) != 446) failures++;
    }


    {
        g16 = 43937;
        if (read_g16() != 43937) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 153;
        if (buf[4] != 153) failures++;
    }


    {
        uint16_t r = 2245 + 22886 + 6804 + 45913 + 27358 + 11719 + 24237 + 12914;
        if (r != 23004) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-84) % (int16_t)((int8_t)113);
        if ((uint16_t)r != (uint16_t)65452) failures++;
    }


    {
        uint8_t x = 121;
        x <<= 3;
        if (x != 200) failures++;
    }


    {
        g16 = 42492;
        if (read_g16() != 42492) failures++;
    }


    {
        volatile uint8_t port = 59;
        uint8_t r = port;
        if (r != 59) failures++;
    }


    {
        uint8_t v = 111;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 75;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 32237;
        if (read_g16() != 32237) failures++;
    }


    {
        uint8_t m[3][2] = {{223,41},{238,78},{205,75}};
        if (m[0][0] != 223) failures++;
    }


    {
        uint8_t x = 43;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t x = 238;
        x = x + 22;
        if (x != 260) failures++;
    }


    {
        uint32_t a = 3190560424UL;
        uint32_t b = 3793054815UL;
        uint32_t r = a ^ b;
        if (r != 1547270903UL) failures++;
    }


    {
        volatile int16_t a = 4937;
        volatile int16_t b = -16668;
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
        int16_t r = (int16_t)((int8_t)-79) / (int16_t)((int8_t)81);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        if (((uint16_t)(((35 | 112) & (244 - 146)) ^ ((238 | 209) ^ 30))) != 131) failures++;
    }


    {
        uint8_t x = 111;
        x <<= 3;
        if (x != 120) failures++;
    }


    {
        uint16_t r = 27088 + 58129 + 48951 + 38912 + 37733 + 16576 + 42098 + 38942;
        if (r != 46285) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)89) / (int16_t)((int8_t)115);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 6804 + 18901 + 52951 + 41534 + 5195 + 19741 + 3540 + 50888;
        if (r != 2946) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t x = 47374;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(112,85) != 27) failures++;
    }


    {
        uint16_t x = 11015;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 1) sum += j;
        if (sum != 153) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(250,14) != 264) failures++;
    }


    {
        uint16_t x = 203;
        x = x + 130;
        if (x != 333) failures++;
    }


    {
        uint16_t r = 55729 + 32426 + 2434 + 54767 + 2001 + 2919 + 65424 + 37752;
        if (r != 56844) failures++;
    }


    {
        uint8_t src[1] = {73};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 73) failures++;
    }


    {
        uint16_t r = call6(216,176,255,17,160,66);
        if (r != 890) failures++;
    }


    {
        uint16_t r = call6(127,210,195,92,153,205);
        if (r != 982) failures++;
    }


    {
        uint16_t r = 10461 + 32140 + 33318 + 23715 + 46310 + 18570 + 7271 + 19888;
        if (r != 60601) failures++;
    }


    {
        uint16_t r = call6(145,195,179,125,199,144);
        if (r != 987) failures++;
    }


    {
        uint32_t a = 39666548UL;
        uint32_t b = 503681731UL;
        uint32_t r = a - b;
        if (r != 3830952113UL) failures++;
    }


    {
        uint8_t src[10] = {133,107,120,140,54,99,99,48,157,139};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[3] != 140) failures++;
    }


    {
        g16 = 38416;
        if (read_g16() != 38416) failures++;
    }


    {
        if (((uint16_t)((252 - (47 + 226)) & 177)) != 161) failures++;
    }


    {
        if (((uint16_t)(((163 & 215) | (189 ^ 103)) | ((186 + 13) + (10 | 79)))) != 479) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 137;
        if (buf[14] != 137) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)107) % (int16_t)((int8_t)96);
        if ((uint16_t)r != (uint16_t)11) failures++;
    }


    {
        uint16_t x = 115;
        x = x + 34;
        if (x != 149) failures++;
    }


    {
        uint8_t v = 161;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        volatile int16_t a = -2620;
        volatile int16_t b = 25577;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 3021119079UL;
        uint32_t b = 5288268UL;
        uint32_t r = a - b;
        if (r != 3015830811UL) failures++;
    }


    {
        uint8_t a[6] = {147,208,116,235,45,93};
        if (a[5] != 93) failures++;
    }


    {
        uint16_t r = 23030 + 57583 + 29471 + 996 + 62366 + 5114 + 50667 + 34860;
        if (r != 1943) failures++;
    }


    {
        uint32_t a = 172584960UL;
        uint32_t b = 3472815283UL;
        uint32_t r = a ^ b;
        if (r != 3300361395UL) failures++;
    }


    {
        if (((uint16_t)43) != 43) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 12: result = 205; break;
        case 14: result = 201; break;
        case 2: result = 223; break;
        case 10: result = 73; break;
        case 18: result = 63; break;
        case 15: result = 113; break;
        default: result = 41; break;
        }
        if (result != 201) failures++;
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
        for (uint8_t j = 0; j < 1; j++) buf[j] = 245;
        if (buf[0] != 245) failures++;
    }


    {
        uint16_t r = add2(218,187) + add2(187,243) + add2(218,243);
        if (r != 1296) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint16_t x = 103;
        x = x + 27;
        if (x != 130) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = 32720 + 44299 + 54823 + 15970 + 14901 + 54898 + 47893 + 17077;
        if (r != 20437) failures++;
    }


    {
        if (((uint16_t)22) != 22) failures++;
    }


    {
        g16 = 3935;
        if (read_g16() != 3935) failures++;
    }


    {
        uint16_t r = add2(163,93) + add2(93,248) + add2(163,248);
        if (r != 1008) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 2) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint8_t m[4][2] = {{185,150},{54,115},{9,41},{42,64}};
        if (m[2][0] != 9) failures++;
    }


    {
        uint32_t a = 447717456UL;
        uint32_t b = 195886200UL;
        uint32_t r = a | b;
        if (r != 464518264UL) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 240;
        if (buf[14] != 240) failures++;
    }


    {
        uint8_t x = 0;
        x <<= 4;
        if (x != 0) failures++;
    }


    {
        volatile int16_t a = 22646;
        volatile int16_t b = -6881;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint8_t a[6] = {212,93,219,94,12,152};
        if (a[5] != 152) failures++;
    }


    {
        uint8_t buf[8] = {72,89,139,138,135,210,150,176};
        uint8_t *p = buf;
        p += 5;
        if (*p != 210) failures++;
    }


    {
        int8_t a = 32;
        int8_t b = 124;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 226;
        if (buf[1] != 226) failures++;
    }


    {
        g16 = 20784;
        if (read_g16() != 20784) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-52) % (int16_t)((int8_t)-66);
        if ((uint16_t)r != (uint16_t)65484) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-63) % (int16_t)((int8_t)45);
        if ((uint16_t)r != (uint16_t)65518) failures++;
    }


    {
        g16 = 9402;
        if (read_g16() != 9402) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)114) / (int16_t)((int8_t)-20);
        if ((uint16_t)r != (uint16_t)65531) failures++;
    }


    {
        uint16_t r = call6(192,132,35,221,192,80);
        if (r != 852) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 12: result = 124; break;
        case 16: result = 70; break;
        case 17: result = 179; break;
        default: result = 154; break;
        }
        if (result != 70) failures++;
    }


    {
        uint8_t x = 44;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(183,241) != 424) failures++;
    }


    {
        if (((uint16_t)((82 & 194) | (115 - 67))) != 114) failures++;
    }


    {
        uint16_t r = 54726 + 40499 + 2724 + 2774 + 12621 + 33342 + 28859 + 39659;
        if (r != 18596) failures++;
    }


    {
        volatile int16_t a = 12180;
        volatile int16_t b = 5763;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 56092;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 29;
        int8_t b = 110;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-88) / (int16_t)((int8_t)80);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t x = 113;
        x = x + 159;
        if (x != 272) failures++;
    }


    {
        uint8_t m[4][3] = {{150,164,210},{33,237,86},{67,25,14},{188,169,149}};
        if (m[3][2] != 149) failures++;
    }


    {
        uint16_t r = call6(34,69,187,212,211,111);
        if (r != 824) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(85,128) != 213) failures++;
    }


    {
        uint8_t src[3] = {138,246,17};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 246) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {6,106,3001,232};
        if (s.c != (uint16_t)3001) failures++;
    }


    {
        uint8_t buf[8] = {175,70,179,233,76,186,137,207};
        uint8_t *p = buf;
        p += 7;
        if (*p != 207) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint16_t r = add2(149,106) + add2(106,55) + add2(149,55);
        if (r != 620) failures++;
    }


    {
        uint8_t src[9] = {81,209,128,90,77,225,92,23,96};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[1] != 209) failures++;
    }


    {
        volatile int16_t a = 28930;
        volatile int16_t b = 14434;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 7: result = 156; break;
        case 5: result = 99; break;
        case 15: result = 41; break;
        case 12: result = 239; break;
        case 18: result = 63; break;
        case 6: result = 208; break;
        case 1: result = 166; break;
        case 11: result = 171; break;
        default: result = 94; break;
        }
        if (result != 171) failures++;
    }


    {
        uint8_t v = 78;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {48,117,36407,138};
        if (s.b != (uint8_t)117) failures++;
    }


    {
        if (((uint16_t)(((150 ^ 13) ^ (191 | 60)) - ((163 & 137) & (196 - 0)))) != 65444) failures++;
    }


    {
        uint8_t v = 125;
        v |= 64;
        if (v != 125) failures++;
    }


    {
        uint8_t a[6] = {68,96,229,119,214,213};
        if (a[0] != 68) failures++;
    }


    {
        volatile int16_t a = 25343;
        volatile int16_t b = -31623;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 248;
        v |= 32;
        if (v != 248) failures++;
    }


    {
        uint8_t src[7] = {157,89,60,167,168,12,110};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[6] != 110) failures++;
    }


    {
        if (((uint16_t)224) != 224) failures++;
    }


    {
        volatile int16_t a = 5293;
        volatile int16_t b = -18247;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)50) / (int16_t)((int8_t)42);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 87;
        if (buf[7] != 87) failures++;
    }


    {
        volatile uint8_t port = 147;
        uint8_t r = port;
        if (r != 147) failures++;
    }


    {
        uint8_t a[6] = {134,78,110,144,84,232};
        if (a[1] != 78) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {17,191,33392,176};
        if (s.d != (uint8_t)176) failures++;
    }


    {
        uint16_t r = add2(48,150) + add2(150,88) + add2(48,88);
        if (r != 572) failures++;
    }


    {
        uint8_t v = 164;
        v |= 1;
        if (v != 165) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {152,31,24954,126};
        if (s.b != (uint8_t)31) failures++;
    }


    {
        uint16_t r = add2(75,17) + add2(17,24) + add2(75,24);
        if (r != 232) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)157) + (uint16_t)34231;
        if (r != 34388) failures++;
    }


    {
        uint8_t m[3][2] = {{70,13},{184,45},{149,201}};
        if (m[1][1] != 45) failures++;
    }


    {
        uint32_t a = 4144361859UL;
        uint32_t b = 2642651190UL;
        uint32_t r = a & b;
        if (r != 2499911682UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {161,112,16807,195};
        if (s.a != (uint8_t)161) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 13: result = 9; break;
        case 0: result = 96; break;
        case 17: result = 113; break;
        case 5: result = 102; break;
        case 12: result = 28; break;
        case 15: result = 114; break;
        case 10: result = 18; break;
        default: result = 142; break;
        }
        if (result != 28) failures++;
    }


    {
        uint32_t a = 719534022UL;
        uint32_t b = 4095233710UL;
        uint32_t r = a + b;
        if (r != 519800436UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(209,223) != 65522) failures++;
    }


    {
        uint8_t buf[8] = {135,69,27,226,197,202,103,71};
        uint8_t *p = buf;
        p += 2;
        if (*p != 27) failures++;
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
        uint8_t buf[8] = {109,136,185,216,49,168,43,86};
        uint8_t *p = buf;
        p += 4;
        if (*p != 49) failures++;
    }


    {
        uint8_t a[6] = {190,43,228,142,59,125};
        if (a[1] != 43) failures++;
    }


    {
        uint8_t v = 63;
        v |= 16;
        if (v != 63) failures++;
    }


    {
        uint16_t r = 64911 + 33258 + 1461 + 56151 + 24079 + 15207 + 60399 + 25399;
        if (r != 18721) failures++;
    }


    {
        uint8_t a[6] = {139,106,243,160,215,128};
        if (a[3] != 160) failures++;
    }


    {
        uint16_t r = add2(229,198) + add2(198,159) + add2(229,159);
        if (r != 1172) failures++;
    }


    {
        volatile uint8_t port = 200;
        uint8_t r = port;
        if (r != 200) failures++;
    }


    {
        int8_t a = -88;
        int8_t b = 85;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint16_t r = add2(164,145) + add2(145,8) + add2(164,8);
        if (r != 634) failures++;
    }


    {
        uint32_t a = 839303925UL;
        uint32_t b = 3443675895UL;
        uint32_t r = a + b;
        if (r != 4282979820UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)71) / (int16_t)((int8_t)-38);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t x = 1557;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {145,57,139,229};
        if (s.b != (uint8_t)57) failures++;
    }


    {
        int8_t a = -54;
        int8_t b = 45;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-63) % (int16_t)((int8_t)-4);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        uint8_t v = 217;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {77,219,8742,181};
        if (s.b != (uint8_t)219) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 106;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)(((108 ^ 56) | 19) | 4)) != 87) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint8_t src[2] = {116,96};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 116) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t m[3][4] = {{96,202,55,30},{28,213,10,181},{0,206,182,169}};
        if (m[0][2] != 55) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(74,248) != 65362) failures++;
    }


    {
        uint8_t a[6] = {107,164,170,57,79,64};
        if (a[1] != 164) failures++;
    }


    {
        uint8_t src[10] = {85,12,25,33,132,50,199,244,189,43};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[4] != 132) failures++;
    }


    {
        uint8_t x = 17;
        x <<= 1;
        if (x != 34) failures++;
    }


    {
        uint16_t r = call6(98,43,218,56,35,131);
        if (r != 581) failures++;
    }


    {
        uint8_t buf[8] = {255,185,210,238,171,251,166,115};
        uint8_t *p = buf;
        p += 6;
        if (*p != 166) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-118) % (int16_t)((int8_t)41);
        if ((uint16_t)r != (uint16_t)65500) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)43) % (int16_t)((int8_t)74);
        if ((uint16_t)r != (uint16_t)43) failures++;
    }


    {
        uint8_t x = 191;
        x <<= 0;
        if (x != 191) failures++;
    }


    {
        uint16_t r = 3296 + 55025 + 16655 + 8435 + 50594 + 57202 + 27872 + 48952;
        if (r != 5887) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(22,128) != 150) failures++;
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
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 3: result = 121; break;
        case 8: result = 143; break;
        case 7: result = 145; break;
        case 4: result = 14; break;
        case 15: result = 189; break;
        case 9: result = 207; break;
        default: result = 195; break;
        }
        if (result != 145) failures++;
    }


    {
        volatile int16_t a = -26756;
        volatile int16_t b = -31176;
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
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 1) sum += j;
        if (sum != 66) failures++;
    }


    {
        uint8_t buf[8] = {169,6,95,85,108,1,176,158};
        uint8_t *p = buf;
        p += 2;
        if (*p != 95) failures++;
    }


    {
        volatile int16_t a = -30844;
        volatile int16_t b = 22875;
        int r = (a == b);
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
        volatile int16_t a = 12291;
        volatile int16_t b = -24415;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 539968785UL;
        uint32_t b = 2334780752UL;
        uint32_t r = a - b;
        if (r != 2500155329UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)11) % (int16_t)((int8_t)74);
        if ((uint16_t)r != (uint16_t)11) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {94,253,12926,252};
        if (s.c != (uint16_t)12926) failures++;
    }


    {
        if (((uint16_t)(((132 & 236) - 209) | ((81 + 79) ^ (73 - 135)))) != 65523) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {50,66,54171,171};
        if (s.b != (uint8_t)66) failures++;
    }


    {
        uint8_t v = 9;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-117) / (int16_t)((int8_t)88);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 8: result = 135; break;
        case 17: result = 48; break;
        case 10: result = 69; break;
        default: result = 129; break;
        }
        if (result != 48) failures++;
    }


    {
        volatile uint8_t port = 112;
        uint8_t r = port;
        if (r != 112) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)88) + (uint16_t)43697;
        if (r != 43785) failures++;
    }


    {
        uint8_t buf[8] = {217,146,106,3,192,81,196,190};
        uint8_t *p = buf;
        p += 7;
        if (*p != 190) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(84,51) != 135) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-42) / (int16_t)((int8_t)109);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t x = 76;
        x <<= 2;
        if (x != 48) failures++;
    }


    {
        uint8_t v = 163;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-112) % (int16_t)((int8_t)27);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-49) % (int16_t)((int8_t)66);
        if ((uint16_t)r != (uint16_t)65487) failures++;
    }


    {
        uint16_t r = 9747 + 65008 + 17644 + 62757 + 65505 + 21164 + 5154 + 47879;
        if (r != 32714) failures++;
    }


    {
        uint8_t m[3][3] = {{0,116,16},{240,12,2},{51,167,140}};
        if (m[0][2] != 16) failures++;
    }


    {
        uint8_t buf[8] = {189,193,59,151,103,219,13,50};
        uint8_t *p = buf;
        p += 4;
        if (*p != 103) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 16: result = 105; break;
        case 12: result = 244; break;
        case 3: result = 69; break;
        case 1: result = 5; break;
        case 15: result = 224; break;
        case 18: result = 141; break;
        case 8: result = 253; break;
        default: result = 118; break;
        }
        if (result != 253) failures++;
    }


    {
        uint32_t a = 2898946103UL;
        uint32_t b = 947396808UL;
        uint32_t r = a | b;
        if (r != 3170527487UL) failures++;
    }


    {
        uint8_t x = 220;
        x <<= 1;
        if (x != 184) failures++;
    }


    {
        uint8_t v = 4;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(170,222) != 65484) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 33;
        if (buf[5] != 33) failures++;
    }


    {
        uint8_t a[6] = {246,8,94,41,0,196};
        if (a[0] != 246) failures++;
    }


    {
        int8_t a = 117;
        int8_t b = -91;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 17571;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 166;
        uint8_t r = port;
        if (r != 166) failures++;
    }


    {
        int8_t a = -88;
        int8_t b = -88;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 21;
        uint8_t r = port;
        if (r != 21) failures++;
    }


    {
        uint16_t x = 900;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 23529;
        volatile int16_t b = -22360;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 168;
        uint8_t r = port;
        if (r != 168) failures++;
    }


    {
        uint16_t r = add2(203,63) + add2(63,150) + add2(203,150);
        if (r != 832) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 7: result = 100; break;
        case 19: result = 120; break;
        case 13: result = 143; break;
        case 16: result = 54; break;
        case 8: result = 41; break;
        default: result = 115; break;
        }
        if (result != 115) failures++;
    }


    {
        g16 = 28678;
        if (read_g16() != 28678) failures++;
    }


    {
        uint8_t v = 58;
        v &= ~(uint8_t)2;
        if (v != 56) failures++;
    }


    {
        uint16_t x = 224;
        x = x + 55;
        if (x != 279) failures++;
    }


    {
        int8_t a = 73;
        int8_t b = 19;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t v = 40;
        v |= 1;
        if (v != 41) failures++;
    }


    {
        uint8_t a[6] = {245,245,162,145,82,178};
        if (a[3] != 145) failures++;
    }


    {
        uint16_t x = 200;
        x = x + 221;
        if (x != 421) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)214) + (uint16_t)7809;
        if (r != 8023) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 99;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 13;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t a[6] = {163,125,123,178,78,76};
        if (a[3] != 178) failures++;
    }


    {
        uint16_t x = 134;
        x = x + 5;
        if (x != 139) failures++;
    }


    {
        uint8_t a[6] = {106,248,14,167,136,65};
        if (a[5] != 65) failures++;
    }


    {
        uint8_t v = 187;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {113,119,15976,9};
        if (s.a != (uint8_t)113) failures++;
    }


    {
        uint8_t m[3][4] = {{237,115,27,134},{248,106,228,100},{193,134,23,116}};
        if (m[1][2] != 228) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)135) + (uint16_t)28396;
        if (r != 28531) failures++;
    }


    {
        if (((uint16_t)(((175 & 127) ^ (107 + 9)) + 136)) != 227) failures++;
    }


    {
        uint8_t v = 237;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 229;
        uint8_t r = port;
        if (r != 229) failures++;
    }


    {
        uint16_t r = add2(56,244) + add2(244,59) + add2(56,59);
        if (r != 718) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-48) / (int16_t)((int8_t)-118);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 44358 + 17165 + 62340 + 8071 + 46215 + 12657 + 40685 + 48370;
        if (r != 17717) failures++;
    }


    {
        uint16_t x = 43;
        x = x + 26;
        if (x != 69) failures++;
    }


    {
        if (((uint16_t)(((242 - 175) - (229 + 112)) - ((179 | 66) + (140 + 195)))) != 64684) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)145) + (uint16_t)53313;
        if (r != 53458) failures++;
    }


    {
        int8_t a = -44;
        int8_t b = 28;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 211;
        int r = (v & 4) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)48) + (uint16_t)52893;
        if (r != 52941) failures++;
    }


    {
        int8_t a = 14;
        int8_t b = -94;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 195;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 48;
        if (buf[0] != 48) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -1967;
        volatile int16_t b = 20219;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 4517;
        if (read_g16() != 4517) failures++;
    }


    {
        uint8_t a[6] = {92,234,170,129,91,94};
        if (a[5] != 94) failures++;
    }


    {
        uint8_t a[6] = {107,78,114,170,87,177};
        if (a[1] != 78) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 5: result = 66; break;
        case 18: result = 223; break;
        case 10: result = 135; break;
        case 12: result = 78; break;
        case 15: result = 10; break;
        case 2: result = 5; break;
        case 8: result = 237; break;
        case 11: result = 25; break;
        default: result = 199; break;
        }
        if (result != 10) failures++;
    }


    {
        uint16_t r = call6(32,103,177,75,135,185);
        if (r != 707) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        volatile int16_t a = -4642;
        volatile int16_t b = 4072;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(250,83) + add2(83,18) + add2(250,18);
        if (r != 702) failures++;
    }


    {
        g16 = 18017;
        if (read_g16() != 18017) failures++;
    }


    {
        uint32_t a = 1516053345UL;
        uint32_t b = 2073649626UL;
        uint32_t r = a + b;
        if (r != 3589702971UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)131) + (uint16_t)51612;
        if (r != 51743) failures++;
    }


    {
        g16 = 49847;
        if (read_g16() != 49847) failures++;
    }


    {
        g16 = 8605;
        if (read_g16() != 8605) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(12,48) != 65500) failures++;
    }


    {
        uint8_t v = 16;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 17: result = 102; break;
        case 19: result = 46; break;
        case 15: result = 12; break;
        default: result = 73; break;
        }
        if (result != 102) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)112) / (int16_t)((int8_t)-3);
        if ((uint16_t)r != (uint16_t)65499) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {33,138,20537,66};
        if (s.c != (uint16_t)20537) failures++;
    }


    {
        uint8_t a[6] = {187,170,29,184,254,16};
        if (a[3] != 184) failures++;
    }


    {
        uint8_t v = 208;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 238;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 59645;
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
        uint8_t v = 34;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 30) failures++;
    }


    {
        volatile uint8_t port = 153;
        uint8_t r = port;
        if (r != 153) failures++;
    }


    {
        uint8_t src[15] = {50,112,233,75,37,249,238,189,155,7,128,218,210,36,80};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[7] != 189) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 1: result = 217; break;
        case 5: result = 154; break;
        case 8: result = 8; break;
        case 17: result = 112; break;
        case 3: result = 10; break;
        case 0: result = 191; break;
        default: result = 106; break;
        }
        if (result != 106) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 42;
        if (buf[5] != 42) failures++;
    }


    {
        int8_t a = -31;
        int8_t b = 34;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 22;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 106) failures++;
    }


    {
        uint8_t buf[8] = {220,129,67,21,106,244,77,42};
        uint8_t *p = buf;
        p += 7;
        if (*p != 42) failures++;
    }


    {
        uint16_t x = 31396;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -317;
        volatile int16_t b = -9402;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 30946 + 62995 + 20813 + 3327 + 17635 + 35954 + 3923 + 9613;
        if (r != 54134) failures++;
    }


    {
        uint16_t r = call6(11,202,178,98,192,7);
        if (r != 688) failures++;
    }


    {
        g16 = 20468;
        if (read_g16() != 20468) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)65) + (uint16_t)8927;
        if (r != 8992) failures++;
    }


    {
        if (((uint16_t)(((93 & 227) + 107) & (99 + 113))) != 132) failures++;
    }


    {
        uint8_t v = 62;
        v ^= 64;
        if (v != 126) failures++;
    }


    {
        uint8_t v = 148;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {33,124,241,158,32,59,27,5};
        uint8_t *p = buf;
        p += 7;
        if (*p != 5) failures++;
    }


    {
        uint8_t src[4] = {223,235,239,214};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[2] != 239) failures++;
    }


    {
        volatile uint8_t port = 32;
        uint8_t r = port;
        if (r != 32) failures++;
    }


    {
        uint16_t r = 60465 + 60999 + 18075 + 39257 + 62898 + 45925 + 10422 + 64105;
        if (r != 34466) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 8: result = 159; break;
        case 0: result = 105; break;
        case 4: result = 189; break;
        case 3: result = 134; break;
        case 10: result = 143; break;
        default: result = 158; break;
        }
        if (result != 134) failures++;
    }


    {
        uint16_t r = call6(206,145,164,230,28,209);
        if (r != 982) failures++;
    }

    return failures;
}