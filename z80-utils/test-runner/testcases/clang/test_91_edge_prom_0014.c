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
        uint8_t buf[8] = {26,250,92,178,90,206,34,136};
        uint8_t *p = buf;
        p += 1;
        if (*p != 250) failures++;
    }


    {
        uint16_t x = 86;
        x = x + 74;
        if (x != 160) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {70,134,9568,58};
        if (s.d != (uint8_t)58) failures++;
    }


    {
        if (((uint16_t)(198 ^ 20)) != 210) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 30;
        x <<= 2;
        if (x != 120) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(92,78) != 14) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)118) + (uint16_t)11350;
        if (r != 11468) failures++;
    }


    {
        uint16_t x = 49;
        x = x + 144;
        if (x != 193) failures++;
    }


    {
        uint8_t x = 117;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)14) % (int16_t)((int8_t)54);
        if ((uint16_t)r != (uint16_t)14) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 251;
        x = x + 244;
        if (x != 495) failures++;
    }


    {
        uint16_t r = call6(84,41,16,82,94,130);
        if (r != 447) failures++;
    }


    {
        volatile int16_t a = 7902;
        volatile int16_t b = 20814;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(221,70,242,209,212,122);
        if (r != 1076) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t v = 75;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 28;
        x = x + 160;
        if (x != 188) failures++;
    }


    {
        if (((uint16_t)(((225 - 221) - 24) - ((108 - 117) - (180 - 6)))) != 163) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t m[3][3] = {{106,236,123},{107,7,23},{124,136,177}};
        if (m[1][2] != 23) failures++;
    }


    {
        uint8_t x = 75;
        x <<= 2;
        if (x != 44) failures++;
    }


    {
        if (((uint16_t)6) != 6) failures++;
    }


    {
        uint8_t m[4][4] = {{219,47,136,87},{96,196,190,235},{230,115,24,161},{73,218,251,132}};
        if (m[0][3] != 87) failures++;
    }


    {
        if (((uint16_t)((20 ^ (60 ^ 171)) - ((230 - 143) | 125))) != 4) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 14: result = 125; break;
        case 13: result = 128; break;
        case 11: result = 164; break;
        case 1: result = 168; break;
        case 6: result = 18; break;
        case 16: result = 244; break;
        case 4: result = 141; break;
        case 3: result = 140; break;
        default: result = 61; break;
        }
        if (result != 140) failures++;
    }


    {
        uint8_t src[8] = {10,163,241,210,241,34,10,87};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[3] != 210) failures++;
    }


    {
        uint16_t x = 42045;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {99,189,9757,231};
        if (s.b != (uint8_t)189) failures++;
    }


    {
        uint16_t x = 139;
        x = x + 225;
        if (x != 364) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 14: result = 143; break;
        case 6: result = 57; break;
        case 7: result = 108; break;
        case 0: result = 17; break;
        case 8: result = 224; break;
        default: result = 181; break;
        }
        if (result != 181) failures++;
    }


    {
        uint16_t x = 214;
        x = x + 53;
        if (x != 267) failures++;
    }


    {
        uint16_t r = 22467 + 5688 + 60761 + 41543 + 61070 + 60753 + 28419 + 17350;
        if (r != 35907) failures++;
    }


    {
        uint16_t x = 28611;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 251;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t v = 224;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 4: result = 150; break;
        case 1: result = 239; break;
        case 15: result = 122; break;
        case 17: result = 0; break;
        case 7: result = 111; break;
        case 12: result = 100; break;
        case 10: result = 154; break;
        case 8: result = 186; break;
        default: result = 237; break;
        }
        if (result != 186) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint16_t r = 20497 + 40301 + 31578 + 36032 + 14842 + 40778 + 38546 + 56036;
        if (r != 16466) failures++;
    }


    {
        uint8_t src[10] = {210,69,62,38,203,84,234,93,44,95};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[7] != 93) failures++;
    }


    {
        uint16_t x = 59305;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {75,125,230,162,148,237};
        if (a[2] != 230) failures++;
    }


    {
        uint8_t m[2][2] = {{110,240},{56,129}};
        if (m[1][0] != 56) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 1) sum += j;
        if (sum != 1) failures++;
    }


    {
        uint8_t a[6] = {43,196,46,248,168,15};
        if (a[1] != 196) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 12: result = 177; break;
        case 5: result = 142; break;
        case 8: result = 39; break;
        case 9: result = 228; break;
        case 14: result = 191; break;
        default: result = 20; break;
        }
        if (result != 142) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-63) % (int16_t)((int8_t)-22);
        if ((uint16_t)r != (uint16_t)65517) failures++;
    }


    {
        uint8_t v = 239;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = 28825 + 64693 + 21615 + 22304 + 4264 + 22123 + 12961 + 45165;
        if (r != 25342) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint8_t a[6] = {165,125,156,97,37,14};
        if (a[3] != 97) failures++;
    }


    {
        int8_t a = 6;
        int8_t b = 117;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(1,204) + add2(204,221) + add2(1,221);
        if (r != 852) failures++;
    }


    {
        uint8_t src[9] = {3,238,252,118,226,195,15,136,106};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[3] != 118) failures++;
    }


    {
        if (((uint16_t)(((135 & 34) - (242 & 69)) + 76)) != 14) failures++;
    }


    {
        uint16_t r = add2(9,225) + add2(225,214) + add2(9,214);
        if (r != 896) failures++;
    }


    {
        volatile uint8_t port = 198;
        uint8_t r = port;
        if (r != 198) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-83) / (int16_t)((int8_t)-109);
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
        uint16_t r = call6(222,220,214,53,23,63);
        if (r != 795) failures++;
    }


    {
        uint16_t x = 117;
        x = x + 195;
        if (x != 312) failures++;
    }


    {
        volatile uint8_t port = 163;
        uint8_t r = port;
        if (r != 163) failures++;
    }


    {
        uint16_t x = 241;
        x = x + 220;
        if (x != 461) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 11: result = 129; break;
        case 8: result = 255; break;
        case 4: result = 141; break;
        case 18: result = 46; break;
        case 5: result = 195; break;
        default: result = 182; break;
        }
        if (result != 141) failures++;
    }


    {
        uint8_t m[2][4] = {{29,220,71,62},{144,206,95,63}};
        if (m[0][2] != 71) failures++;
    }


    {
        uint16_t x = 24423;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(64,136) + add2(136,79) + add2(64,79);
        if (r != 558) failures++;
    }


    {
        uint8_t m[4][4] = {{203,22,225,174},{9,63,48,111},{63,89,235,2},{219,148,138,233}};
        if (m[3][0] != 219) failures++;
    }


    {
        uint16_t x = 219;
        x = x + 38;
        if (x != 257) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 16;
        if (buf[6] != 16) failures++;
    }


    {
        uint8_t v = 119;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 19491;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 36894;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)71) + (uint16_t)10654;
        if (r != 10725) failures++;
    }


    {
        uint8_t a[6] = {116,187,183,107,75,149};
        if (a[3] != 107) failures++;
    }


    {
        g16 = 10701;
        if (read_g16() != 10701) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(83,237) != 320) failures++;
    }


    {
        uint16_t x = 246;
        x = x + 56;
        if (x != 302) failures++;
    }


    {
        int8_t a = 106;
        int8_t b = 18;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 150;
        if (buf[5] != 150) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(225,219) != 444) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint16_t x = 941;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 64;
        if (buf[8] != 64) failures++;
    }


    {
        volatile int16_t a = -4824;
        volatile int16_t b = 4370;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 3043464099UL;
        uint32_t b = 2856108133UL;
        uint32_t r = a - b;
        if (r != 187355966UL) failures++;
    }


    {
        uint8_t a[6] = {253,158,154,157,111,135};
        if (a[0] != 253) failures++;
    }


    {
        uint8_t m[4][4] = {{219,205,109,134},{222,191,194,147},{204,124,111,20},{28,17,170,151}};
        if (m[1][3] != 147) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 14: result = 132; break;
        case 11: result = 99; break;
        case 4: result = 202; break;
        case 17: result = 249; break;
        case 13: result = 58; break;
        case 18: result = 209; break;
        case 12: result = 15; break;
        case 7: result = 0; break;
        default: result = 81; break;
        }
        if (result != 209) failures++;
    }


    {
        uint16_t r = add2(50,146) + add2(146,13) + add2(50,13);
        if (r != 418) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint16_t r = 34747 + 60967 + 39794 + 63341 + 26031 + 64502 + 27976 + 38069;
        if (r != 27747) failures++;
    }


    {
        uint8_t m[3][4] = {{64,74,7,71},{12,18,43,74},{137,211,47,233}};
        if (m[1][2] != 43) failures++;
    }


    {
        uint16_t r = add2(99,40) + add2(40,91) + add2(99,91);
        if (r != 460) failures++;
    }


    {
        uint16_t r = call6(56,249,253,59,224,32);
        if (r != 873) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)122) + (uint16_t)7191;
        if (r != 7313) failures++;
    }


    {
        uint16_t x = 2245;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(209,127) + add2(127,106) + add2(209,106);
        if (r != 884) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)79) % (int16_t)((int8_t)101);
        if ((uint16_t)r != (uint16_t)79) failures++;
    }


    {
        uint8_t a[6] = {185,230,155,82,203,147};
        if (a[4] != 203) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 2: result = 232; break;
        case 6: result = 34; break;
        case 16: result = 135; break;
        case 7: result = 27; break;
        default: result = 116; break;
        }
        if (result != 34) failures++;
    }


    {
        if (((uint16_t)9) != 9) failures++;
    }


    {
        volatile uint8_t port = 243;
        uint8_t r = port;
        if (r != 243) failures++;
    }


    {
        volatile int16_t a = -2707;
        volatile int16_t b = 16503;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {107,246,255,236,104,9};
        if (a[1] != 246) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {89,255,57614,121};
        if (s.a != (uint8_t)89) failures++;
    }


    {
        uint16_t r = call6(236,47,188,64,128,7);
        if (r != 670) failures++;
    }


    {
        uint16_t r = call6(53,110,131,118,194,195);
        if (r != 801) failures++;
    }


    {
        uint32_t a = 3746816648UL;
        uint32_t b = 188163610UL;
        uint32_t r = a ^ b;
        if (r != 3563373714UL) failures++;
    }


    {
        if (((uint16_t)47) != 47) failures++;
    }


    {
        int8_t a = 8;
        int8_t b = 84;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(155,227,95,60,91,29);
        if (r != 657) failures++;
    }


    {
        int8_t a = -93;
        int8_t b = 62;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 3966225862UL;
        uint32_t b = 3097388436UL;
        uint32_t r = a - b;
        if (r != 868837426UL) failures++;
    }


    {
        uint8_t x = 97;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t x = 64;
        x <<= 0;
        if (x != 64) failures++;
    }


    {
        volatile uint8_t port = 33;
        uint8_t r = port;
        if (r != 33) failures++;
    }


    {
        volatile uint8_t port = 140;
        uint8_t r = port;
        if (r != 140) failures++;
    }


    {
        uint8_t v = 120;
        v &= ~(uint8_t)16;
        if (v != 104) failures++;
    }


    {
        uint16_t r = 40674 + 30772 + 2856 + 38568 + 20624 + 3724 + 994 + 54719;
        if (r != 61859) failures++;
    }


    {
        uint16_t x = 7;
        x = x + 56;
        if (x != 63) failures++;
    }


    {
        uint16_t x = 45218;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 88;
        uint8_t r = port;
        if (r != 88) failures++;
    }


    {
        uint32_t a = 103088328UL;
        uint32_t b = 2600630264UL;
        uint32_t r = a - b;
        if (r != 1797425360UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 1) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 5: result = 182; break;
        case 10: result = 76; break;
        case 8: result = 176; break;
        case 3: result = 249; break;
        default: result = 155; break;
        }
        if (result != 249) failures++;
    }


    {
        uint16_t x = 18415;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(138 | (27 | (202 ^ 93)))) != 159) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 88;
        if (buf[8] != 88) failures++;
    }


    {
        uint16_t r = add2(161,255) + add2(255,37) + add2(161,37);
        if (r != 906) failures++;
    }


    {
        uint8_t src[10] = {155,83,40,69,82,83,137,149,70,15};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[6] != 137) failures++;
    }


    {
        uint16_t r = add2(81,16) + add2(16,64) + add2(81,64);
        if (r != 322) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)112) + (uint16_t)23702;
        if (r != 23814) failures++;
    }


    {
        uint8_t buf[8] = {195,140,127,167,210,127,161,73};
        uint8_t *p = buf;
        p += 2;
        if (*p != 127) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 250;
        if (buf[8] != 250) failures++;
    }


    {
        uint16_t r = call6(236,116,99,51,74,242);
        if (r != 818) failures++;
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
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        g16 = 31568;
        if (read_g16() != 31568) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)143) + (uint16_t)2288;
        if (r != 2431) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {172,141,16599,63};
        if (s.a != (uint8_t)172) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 1) sum += j;
        if (sum != 36) failures++;
    }


    {
        volatile int16_t a = -18272;
        volatile int16_t b = 24561;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[10] = {64,208,207,218,91,32,109,89,210,106};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[9] != 106) failures++;
    }


    {
        uint8_t a[6] = {110,92,158,44,233,100};
        if (a[1] != 92) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 31175;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(248,136) != 112) failures++;
    }


    {
        int8_t a = -65;
        int8_t b = 52;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(((17 ^ 77) - (65 + 74)) | 145)) != 65489) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 119;
        if (buf[1] != 119) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)99) % (int16_t)((int8_t)121);
        if ((uint16_t)r != (uint16_t)99) failures++;
    }


    {
        uint8_t v = 190;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 249;
        uint8_t r = port;
        if (r != 249) failures++;
    }


    {
        uint8_t a[6] = {133,214,47,32,221,146};
        if (a[5] != 146) failures++;
    }


    {
        volatile int16_t a = -18914;
        volatile int16_t b = -1786;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)9) != 9) failures++;
    }


    {
        uint8_t m[4][3] = {{79,179,240},{80,161,136},{30,43,200},{243,154,215}};
        if (m[0][2] != 240) failures++;
    }


    {
        uint16_t x = 20687;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint16_t x = 35;
        x = x + 145;
        if (x != 180) failures++;
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
        uint8_t a[6] = {110,97,105,93,7,154};
        if (a[5] != 154) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t src[2] = {217,33};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 33) failures++;
    }


    {
        uint8_t a[6] = {91,248,113,52,199,41};
        if (a[1] != 248) failures++;
    }


    {
        uint8_t buf[8] = {26,230,153,38,123,193,14,202};
        uint8_t *p = buf;
        p += 4;
        if (*p != 123) failures++;
    }


    {
        uint16_t x = 116;
        x = x + 133;
        if (x != 249) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 175;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 89;
        if (buf[10] != 89) failures++;
    }


    {
        uint8_t a[6] = {121,120,136,118,197,214};
        if (a[1] != 120) failures++;
    }


    {
        uint32_t a = 788125153UL;
        uint32_t b = 3131072772UL;
        uint32_t r = a ^ b;
        if (r != 2488906981UL) failures++;
    }


    {
        uint8_t v = 138;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        uint16_t r = add2(211,210) + add2(210,53) + add2(211,53);
        if (r != 948) failures++;
    }


    {
        volatile uint8_t port = 191;
        uint8_t r = port;
        if (r != 191) failures++;
    }


    {
        int8_t a = -53;
        int8_t b = 108;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 61;
        uint8_t r = port;
        if (r != 61) failures++;
    }


    {
        volatile int16_t a = 27110;
        volatile int16_t b = -20983;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 97;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t src[16] = {165,61,150,210,202,190,136,116,161,54,170,71,244,177,20,54};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[6] != 136) failures++;
    }


    {
        uint8_t buf[8] = {200,90,236,106,87,86,58,77};
        uint8_t *p = buf;
        p += 2;
        if (*p != 236) failures++;
    }


    {
        uint8_t x = 75;
        x <<= 2;
        if (x != 44) failures++;
    }


    {
        uint16_t r = call6(34,140,39,38,144,103);
        if (r != 498) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 10: result = 90; break;
        case 15: result = 132; break;
        case 2: result = 51; break;
        case 3: result = 53; break;
        case 9: result = 248; break;
        default: result = 100; break;
        }
        if (result != 132) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)120) + (uint16_t)41983;
        if (r != 42103) failures++;
    }


    {
        uint32_t a = 472021262UL;
        uint32_t b = 3192424992UL;
        uint32_t r = a | b;
        if (r != 3194682158UL) failures++;
    }


    {
        uint8_t x = 238;
        x <<= 3;
        if (x != 112) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 1050;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 38129 + 16132 + 37992 + 49026 + 58126 + 48206 + 36704 + 48782;
        if (r != 5417) failures++;
    }


    {
        uint8_t x = 209;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 207;
        x = x + 175;
        if (x != 382) failures++;
    }


    {
        uint8_t m[3][2] = {{76,133},{224,16},{147,204}};
        if (m[1][1] != 16) failures++;
    }


    {
        int8_t a = 93;
        int8_t b = -55;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)11) + (uint16_t)45776;
        if (r != 45787) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-37) % (int16_t)((int8_t)102);
        if ((uint16_t)r != (uint16_t)65499) failures++;
    }


    {
        uint8_t m[3][3] = {{28,225,80},{31,177,209},{77,5,248}};
        if (m[0][1] != 225) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {217,40,20898,97};
        if (s.d != (uint8_t)97) failures++;
    }


    {
        volatile int16_t a = 20395;
        volatile int16_t b = 5787;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(92,249) + add2(249,218) + add2(92,218);
        if (r != 1118) failures++;
    }


    {
        uint8_t m[3][3] = {{9,51,201},{16,235,38},{79,252,4}};
        if (m[2][1] != 252) failures++;
    }


    {
        volatile uint8_t port = 14;
        uint8_t r = port;
        if (r != 14) failures++;
    }


    {
        uint8_t v = 138;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 83;
        if (buf[10] != 83) failures++;
    }


    {
        if (((uint16_t)(((143 ^ 127) + 137) + ((189 - 41) ^ (142 & 159)))) != 403) failures++;
    }


    {
        uint16_t r = add2(161,176) + add2(176,159) + add2(161,159);
        if (r != 992) failures++;
    }


    {
        g16 = 56117;
        if (read_g16() != 56117) failures++;
    }


    {
        uint8_t buf[8] = {93,184,245,247,210,245,51,13};
        uint8_t *p = buf;
        p += 6;
        if (*p != 51) failures++;
    }


    {
        volatile int16_t a = -13422;
        volatile int16_t b = -8146;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 142;
        v ^= 32;
        if (v != 174) failures++;
    }


    {
        volatile uint8_t port = 12;
        uint8_t r = port;
        if (r != 12) failures++;
    }


    {
        uint8_t a[6] = {23,221,217,53,205,145};
        if (a[4] != 205) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint16_t r = call6(25,233,126,162,88,40);
        if (r != 674) failures++;
    }


    {
        uint8_t v = 10;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 118) failures++;
    }


    {
        uint8_t v = 171;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)48) / (int16_t)((int8_t)73);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)63) / (int16_t)((int8_t)93);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {189,154,201,91,165,11};
        if (a[3] != 91) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)12) + (uint16_t)60526;
        if (r != 60538) failures++;
    }


    {
        uint8_t buf[8] = {53,220,64,198,138,48,141,228};
        uint8_t *p = buf;
        p += 2;
        if (*p != 64) failures++;
    }


    {
        uint8_t v = 162;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(105,180) != 285) failures++;
    }


    {
        uint8_t buf[8] = {106,136,65,214,69,185,207,80};
        uint8_t *p = buf;
        p += 4;
        if (*p != 69) failures++;
    }


    {
        if (((uint16_t)168) != 168) failures++;
    }


    {
        uint16_t x = 58142;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(176,109) + add2(109,77) + add2(176,77);
        if (r != 724) failures++;
    }


    {
        uint8_t src[13] = {112,124,114,130,246,26,35,1,178,36,29,30,83};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[8] != 178) failures++;
    }


    {
        if (((uint16_t)43) != 43) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-46) / (int16_t)((int8_t)62);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 50617;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(174,248) + add2(248,24) + add2(174,24);
        if (r != 892) failures++;
    }


    {
        uint8_t src[14] = {179,241,66,184,20,101,151,238,143,247,83,67,221,255};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[13] != 255) failures++;
    }


    {
        uint16_t r = 13904 + 58845 + 64484 + 40911 + 6770 + 28451 + 64304 + 37525;
        if (r != 53050) failures++;
    }


    {
        uint32_t a = 2751213295UL;
        uint32_t b = 4049669449UL;
        uint32_t r = a & b;
        if (r != 2707426377UL) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 1: result = 33; break;
        case 2: result = 254; break;
        case 7: result = 115; break;
        case 14: result = 241; break;
        case 10: result = 167; break;
        default: result = 163; break;
        }
        if (result != 254) failures++;
    }


    {
        volatile int16_t a = -22794;
        volatile int16_t b = 25113;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)14) % (int16_t)((int8_t)110);
        if ((uint16_t)r != (uint16_t)14) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-124) % (int16_t)((int8_t)88);
        if ((uint16_t)r != (uint16_t)65500) failures++;
    }


    {
        uint8_t x = 221;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        if (((uint16_t)171) != 171) failures++;
    }


    {
        volatile int16_t a = -12198;
        volatile int16_t b = 3053;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 29946;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 144;
        x = x + 25;
        if (x != 169) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 4;
        if (buf[5] != 4) failures++;
    }


    {
        uint32_t a = 3368492993UL;
        uint32_t b = 867969915UL;
        uint32_t r = a - b;
        if (r != 2500523078UL) failures++;
    }


    {
        uint32_t a = 4053560707UL;
        uint32_t b = 2011547947UL;
        uint32_t r = a & b;
        if (r != 1904494851UL) failures++;
    }


    {
        uint16_t x = 2;
        x = x + 153;
        if (x != 155) failures++;
    }


    {
        if (((uint16_t)229) != 229) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)78) + (uint16_t)25248;
        if (r != 25326) failures++;
    }


    {
        uint32_t a = 4104058354UL;
        uint32_t b = 1986419900UL;
        uint32_t r = a - b;
        if (r != 2117638454UL) failures++;
    }


    {
        uint8_t m[2][2] = {{94,148},{127,36}};
        if (m[1][1] != 36) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 21;
        if (buf[5] != 21) failures++;
    }


    {
        uint16_t x = 53350;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 80;
        int r = (v & 2) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)77) % (int16_t)((int8_t)68);
        if ((uint16_t)r != (uint16_t)9) failures++;
    }


    {
        uint8_t buf[8] = {61,193,251,111,233,134,186,232};
        uint8_t *p = buf;
        p += 7;
        if (*p != 232) failures++;
    }


    {
        uint8_t a[6] = {1,20,45,209,105,42};
        if (a[4] != 105) failures++;
    }


    {
        uint8_t m[2][3] = {{190,47,252},{111,23,157}};
        if (m[1][0] != 111) failures++;
    }


    {
        uint8_t src[3] = {17,5,248};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 248) failures++;
    }


    {
        uint8_t buf[8] = {239,180,0,28,20,139,40,47};
        uint8_t *p = buf;
        p += 2;
        if (*p != 0) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 21;
        if (buf[13] != 21) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 206;
        if (buf[10] != 206) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(208,60) != 148) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 118;
        if (buf[8] != 118) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {157,214,6,226,131,112,75,195};
        uint8_t *p = buf;
        p += 6;
        if (*p != 75) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)100) + (uint16_t)22301;
        if (r != 22401) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-41) / (int16_t)((int8_t)19);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint16_t x = 203;
        x = x + 69;
        if (x != 272) failures++;
    }


    {
        volatile int16_t a = -12715;
        volatile int16_t b = 13198;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(52,103,158,138,200,137);
        if (r != 788) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)227) + (uint16_t)32693;
        if (r != 32920) failures++;
    }


    {
        uint16_t x = 121;
        x = x + 254;
        if (x != 375) failures++;
    }


    {
        uint8_t v = 123;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = 8409 + 41047 + 33995 + 34136 + 44994 + 27074 + 39561 + 18959;
        if (r != 51567) failures++;
    }


    {
        volatile int16_t a = 10900;
        volatile int16_t b = -26938;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 40;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t src[1] = {107};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 107) failures++;
    }


    {
        if (((uint16_t)202) != 202) failures++;
    }


    {
        uint16_t x = 95;
        x = x + 162;
        if (x != 257) failures++;
    }


    {
        uint8_t v = 115;
        v &= ~(uint8_t)4;
        if (v != 115) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 89;
        if (buf[10] != 89) failures++;
    }


    {
        uint16_t x = 207;
        x = x + 143;
        if (x != 350) failures++;
    }


    {
        int8_t a = 97;
        int8_t b = -30;
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
        uint16_t r = call6(55,119,59,158,79,42);
        if (r != 512) failures++;
    }


    {
        volatile int16_t a = -32599;
        volatile int16_t b = -8227;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 18: result = 175; break;
        case 19: result = 255; break;
        case 11: result = 123; break;
        case 8: result = 39; break;
        case 5: result = 3; break;
        default: result = 106; break;
        }
        if (result != 255) failures++;
    }


    {
        volatile uint8_t port = 14;
        uint8_t r = port;
        if (r != 14) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(21,56) != 65501) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 72;
        if (buf[6] != 72) failures++;
    }


    {
        uint8_t a[6] = {184,22,113,116,143,226};
        if (a[0] != 184) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(173,150) != 23) failures++;
    }


    {
        int8_t a = 67;
        int8_t b = 7;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 110;
        int8_t b = 78;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 172;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 16: result = 243; break;
        case 3: result = 228; break;
        case 7: result = 157; break;
        case 2: result = 204; break;
        case 17: result = 245; break;
        case 13: result = 18; break;
        case 1: result = 50; break;
        case 4: result = 239; break;
        default: result = 52; break;
        }
        if (result != 243) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 15: result = 149; break;
        case 8: result = 181; break;
        case 6: result = 110; break;
        default: result = 87; break;
        }
        if (result != 110) failures++;
    }


    {
        uint8_t src[15] = {42,58,101,35,129,222,164,131,43,158,41,97,123,31,233};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[10] != 41) failures++;
    }


    {
        uint16_t r = call6(18,42,81,100,70,175);
        if (r != 486) failures++;
    }


    {
        uint8_t v = 157;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t a[6] = {132,120,69,23,176,53};
        if (a[0] != 132) failures++;
    }


    {
        uint8_t m[2][2] = {{150,85},{172,70}};
        if (m[1][0] != 172) failures++;
    }


    {
        uint16_t r = 3755 + 1704 + 40155 + 5633 + 17001 + 35406 + 60012 + 41413;
        if (r != 8471) failures++;
    }


    {
        uint16_t x = 237;
        x = x + 165;
        if (x != 402) failures++;
    }


    {
        uint8_t v = 226;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = -15216;
        volatile int16_t b = 8831;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[7] = {145,46,109,170,12,188,157};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[2] != 109) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint16_t x = 24855;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {235,83,4340,222};
        if (s.b != (uint8_t)83) failures++;
    }


    {
        uint8_t m[4][3] = {{168,109,183},{86,146,47},{41,100,212},{29,35,113}};
        if (m[2][1] != 100) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)81) / (int16_t)((int8_t)-12);
        if ((uint16_t)r != (uint16_t)65530) failures++;
    }


    {
        uint8_t v = 74;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        int8_t a = 22;
        int8_t b = -52;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[2] = {86,234};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 234) failures++;
    }


    {
        uint8_t a[6] = {141,176,137,170,172,37};
        if (a[5] != 37) failures++;
    }


    {
        uint8_t v = 150;
        v |= 2;
        if (v != 150) failures++;
    }


    {
        uint16_t x = 158;
        x = x + 254;
        if (x != 412) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 18;
        if (buf[8] != 18) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 1) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint8_t src[14] = {209,49,38,6,88,89,64,92,203,112,206,229,199,234};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[12] != 199) failures++;
    }


    {
        uint8_t v = 1;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = 45563 + 21077 + 11436 + 57914 + 62405 + 23031 + 56555 + 9807;
        if (r != 25644) failures++;
    }


    {
        uint32_t a = 391791695UL;
        uint32_t b = 3383137655UL;
        uint32_t r = a & b;
        if (r != 16909383UL) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 14: result = 187; break;
        case 8: result = 162; break;
        case 13: result = 10; break;
        default: result = 254; break;
        }
        if (result != 10) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {171,55,40946,0};
        if (s.d != (uint8_t)0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)72) + (uint16_t)48135;
        if (r != 48207) failures++;
    }


    {
        uint8_t m[2][4] = {{50,97,90,90},{188,255,108,171}};
        if (m[0][2] != 90) failures++;
    }


    {
        uint16_t x = 88;
        x = x + 60;
        if (x != 148) failures++;
    }


    {
        if (((uint16_t)((228 + (217 | 144)) + (121 - (16 + 163)))) != 387) failures++;
    }


    {
        volatile uint8_t port = 6;
        uint8_t r = port;
        if (r != 6) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 7: result = 100; break;
        case 3: result = 55; break;
        case 8: result = 64; break;
        default: result = 170; break;
        }
        if (result != 100) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {253,3,2283,59};
        if (s.a != (uint8_t)253) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-5) % (int16_t)((int8_t)-95);
        if ((uint16_t)r != (uint16_t)65531) failures++;
    }


    {
        uint8_t v = 196;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 28) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)166) + (uint16_t)31281;
        if (r != 31447) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)123) / (int16_t)((int8_t)-104);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t r = 3293 + 9525 + 17920 + 6402 + 60271 + 22282 + 14245 + 27234;
        if (r != 30100) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 11: result = 177; break;
        case 9: result = 124; break;
        case 2: result = 170; break;
        case 1: result = 129; break;
        case 5: result = 5; break;
        case 16: result = 251; break;
        case 17: result = 48; break;
        default: result = 136; break;
        }
        if (result != 170) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(216,133) != 83) failures++;
    }


    {
        uint8_t src[6] = {132,77,127,157,15,118};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[2] != 127) failures++;
    }


    {
        uint16_t r = add2(93,146) + add2(146,201) + add2(93,201);
        if (r != 880) failures++;
    }


    {
        uint8_t m[3][3] = {{195,15,146},{185,218,3},{139,5,252}};
        if (m[2][0] != 139) failures++;
    }


    {
        uint8_t a[6] = {39,62,174,36,39,49};
        if (a[1] != 62) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 163;
        if (buf[11] != 163) failures++;
    }


    {
        uint8_t a[6] = {4,24,52,35,189,123};
        if (a[3] != 35) failures++;
    }


    {
        uint8_t buf[8] = {31,70,62,122,71,174,99,136};
        uint8_t *p = buf;
        p += 6;
        if (*p != 99) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {147,214,5704,29};
        if (s.c != (uint16_t)5704) failures++;
    }


    {
        uint8_t m[4][4] = {{18,162,189,85},{67,92,98,120},{233,169,216,176},{128,163,33,55}};
        if (m[1][1] != 92) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {150,229,41094,50};
        if (s.d != (uint8_t)50) failures++;
    }


    {
        uint8_t buf[8] = {3,233,64,75,229,231,119,94};
        uint8_t *p = buf;
        p += 3;
        if (*p != 75) failures++;
    }


    {
        uint8_t buf[8] = {29,27,91,19,81,175,111,212};
        uint8_t *p = buf;
        p += 5;
        if (*p != 175) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)((69 + 37) | ((158 + 159) | (10 | 216)))) != 511) failures++;
    }


    {
        uint8_t m[2][4] = {{253,59,28,189},{61,26,74,81}};
        if (m[1][1] != 26) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint16_t r = add2(110,150) + add2(150,55) + add2(110,55);
        if (r != 630) failures++;
    }


    {
        uint16_t r = call6(126,207,196,92,172,8);
        if (r != 801) failures++;
    }


    {
        uint8_t a[6] = {221,27,194,173,95,161};
        if (a[5] != 161) failures++;
    }


    {
        uint8_t v = 197;
        v ^= 16;
        if (v != 213) failures++;
    }


    {
        uint8_t a[6] = {162,108,110,245,195,174};
        if (a[3] != 245) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-115) / (int16_t)((int8_t)93);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t r = add2(3,254) + add2(254,240) + add2(3,240);
        if (r != 994) failures++;
    }


    {
        uint8_t m[2][2] = {{160,49},{16,92}};
        if (m[1][1] != 92) failures++;
    }


    {
        uint8_t src[2] = {216,9};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 9) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 3988203139UL;
        uint32_t b = 2892492385UL;
        uint32_t r = a & b;
        if (r != 2888246785UL) failures++;
    }


    {
        uint8_t src[5] = {220,49,44,173,63};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 173) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {141,191,208,147,174,47};
        if (a[0] != 141) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 16: result = 190; break;
        case 0: result = 0; break;
        case 3: result = 173; break;
        case 4: result = 18; break;
        case 18: result = 38; break;
        case 17: result = 28; break;
        case 12: result = 59; break;
        case 7: result = 214; break;
        default: result = 154; break;
        }
        if (result != 154) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)22) + (uint16_t)64332;
        if (r != 64354) failures++;
    }


    {
        uint32_t a = 1787815479UL;
        uint32_t b = 4183213907UL;
        uint32_t r = a ^ b;
        if (r != 2480480612UL) failures++;
    }


    {
        g16 = 57300;
        if (read_g16() != 57300) failures++;
    }


    {
        uint16_t x = 31805;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 66;
        int8_t b = 25;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 25028 + 33916 + 43914 + 51057 + 12060 + 30242 + 17265 + 48310;
        if (r != 65184) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 81;
        if (buf[13] != 81) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[2][4] = {{151,148,108,87},{240,134,182,106}};
        if (m[0][2] != 108) failures++;
    }


    {
        uint8_t v = 144;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t buf[8] = {139,199,106,112,152,39,64,119};
        uint8_t *p = buf;
        p += 2;
        if (*p != 106) failures++;
    }


    {
        uint8_t a[6] = {110,46,40,65,3,174};
        if (a[2] != 40) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {165,128,60639,255};
        if (s.a != (uint8_t)165) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,110,32421,95};
        if (s.b != (uint8_t)110) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 10: result = 148; break;
        case 18: result = 234; break;
        case 9: result = 161; break;
        default: result = 155; break;
        }
        if (result != 161) failures++;
    }


    {
        uint32_t a = 478026804UL;
        uint32_t b = 571709454UL;
        uint32_t r = a - b;
        if (r != 4201284646UL) failures++;
    }


    {
        uint8_t v = 2;
        v |= 2;
        if (v != 2) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)0) + (uint16_t)14804;
        if (r != 14804) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 1) sum += j;
        if (sum != 66) failures++;
    }


    {
        uint16_t x = 25256;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)4) + (uint16_t)48861;
        if (r != 48865) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(65,22) != 43) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 135;
        x = x + 102;
        if (x != 237) failures++;
    }


    {
        uint16_t x = 57932;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {39,42,60681,206};
        if (s.c != (uint16_t)60681) failures++;
    }


    {
        uint8_t m[3][2] = {{185,64},{26,172},{252,166}};
        if (m[0][0] != 185) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 131;
        if (buf[1] != 131) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {112,191,25082,50};
        if (s.d != (uint8_t)50) failures++;
    }


    {
        volatile int16_t a = -15075;
        volatile int16_t b = 24190;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        g16 = 7092;
        if (read_g16() != 7092) failures++;
    }


    {
        uint8_t v = 84;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint8_t src[11] = {108,227,164,110,72,239,75,24,205,81,71};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[9] != 81) failures++;
    }


    {
        uint16_t x = 37893;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 16: result = 208; break;
        case 12: result = 252; break;
        case 0: result = 82; break;
        case 3: result = 11; break;
        case 1: result = 209; break;
        case 19: result = 231; break;
        case 4: result = 115; break;
        case 13: result = 146; break;
        default: result = 197; break;
        }
        if (result != 82) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)124) + (uint16_t)30178;
        if (r != 30302) failures++;
    }


    {
        uint8_t v = 137;
        int r = (v & 2) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-114) % (int16_t)((int8_t)-106);
        if ((uint16_t)r != (uint16_t)65528) failures++;
    }


    {
        uint8_t m[2][3] = {{218,163,3},{42,68,53}};
        if (m[1][0] != 42) failures++;
    }


    {
        volatile uint8_t port = 217;
        uint8_t r = port;
        if (r != 217) failures++;
    }


    {
        uint8_t buf[8] = {171,101,88,183,20,39,148,251};
        uint8_t *p = buf;
        p += 4;
        if (*p != 20) failures++;
    }


    {
        int8_t a = 27;
        int8_t b = -119;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {170,82,205,11,40,13};
        if (a[1] != 82) failures++;
    }


    {
        uint16_t r = 1141 + 17401 + 28799 + 3861 + 27553 + 47632 + 22167 + 21796;
        if (r != 39278) failures++;
    }


    {
        uint8_t x = 107;
        x <<= 3;
        if (x != 88) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {35,31,60604,248};
        if (s.a != (uint8_t)35) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[3][2] = {{57,63},{236,60},{174,219}};
        if (m[1][0] != 236) failures++;
    }


    {
        uint8_t buf[8] = {131,230,243,123,3,143,251,75};
        uint8_t *p = buf;
        p += 6;
        if (*p != 251) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)45) + (uint16_t)28000;
        if (r != 28045) failures++;
    }


    {
        int8_t a = 81;
        int8_t b = -126;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 21;
        v |= 4;
        if (v != 21) failures++;
    }


    {
        uint16_t r = call6(56,90,31,137,119,189);
        if (r != 622) failures++;
    }


    {
        uint32_t a = 2872431684UL;
        uint32_t b = 822022137UL;
        uint32_t r = a + b;
        if (r != 3694453821UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(207,187) != 20) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 189;
        if (buf[2] != 189) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 13: result = 141; break;
        case 0: result = 159; break;
        case 10: result = 59; break;
        default: result = 49; break;
        }
        if (result != 159) failures++;
    }


    {
        g16 = 43034;
        if (read_g16() != 43034) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {167,105,9311,149};
        if (s.a != (uint8_t)167) failures++;
    }


    {
        uint8_t src[8] = {93,132,31,242,52,193,35,69};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[0] != 93) failures++;
    }


    {
        uint8_t v = 73;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 7) failures++;
    }


    {
        uint32_t a = 1852655286UL;
        uint32_t b = 328866237UL;
        uint32_t r = a + b;
        if (r != 2181521523UL) failures++;
    }


    {
        uint8_t v = 157;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t src[4] = {76,195,4,211};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 76) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        volatile int16_t a = 27763;
        volatile int16_t b = -15111;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {105,50,29770,110};
        if (s.c != (uint16_t)29770) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-6) / (int16_t)((int8_t)-111);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 252;
        x = x + 4;
        if (x != 256) failures++;
    }


    {
        uint16_t x = 251;
        x = x + 236;
        if (x != 487) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 96;
        if (buf[10] != 96) failures++;
    }


    {
        uint8_t src[14] = {73,139,139,158,98,138,104,21,68,57,101,11,220,120};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[4] != 98) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)218) + (uint16_t)40610;
        if (r != 40828) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        int8_t a = -101;
        int8_t b = 31;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint8_t src[13] = {73,158,90,78,5,162,49,199,52,46,76,115,251};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[2] != 90) failures++;
    }


    {
        uint8_t v = 218;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        uint16_t r = add2(81,16) + add2(16,240) + add2(81,240);
        if (r != 674) failures++;
    }


    {
        if (((uint16_t)174) != 174) failures++;
    }


    {
        volatile int16_t a = -6586;
        volatile int16_t b = 25044;
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
        uint16_t r = add2(233,194) + add2(194,25) + add2(233,25);
        if (r != 904) failures++;
    }


    {
        uint16_t x = 255;
        x = x + 53;
        if (x != 308) failures++;
    }


    {
        uint8_t v = 255;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        volatile uint8_t port = 112;
        uint8_t r = port;
        if (r != 112) failures++;
    }


    {
        uint8_t buf[8] = {130,190,168,111,48,188,145,232};
        uint8_t *p = buf;
        p += 6;
        if (*p != 145) failures++;
    }


    {
        uint16_t x = 239;
        x = x + 90;
        if (x != 329) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        volatile uint8_t port = 173;
        uint8_t r = port;
        if (r != 173) failures++;
    }


    {
        uint32_t a = 3357800907UL;
        uint32_t b = 174990486UL;
        uint32_t r = a | b;
        if (r != 3396337119UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 142;
        uint8_t r = port;
        if (r != 142) failures++;
    }


    {
        uint8_t m[3][3] = {{181,100,95},{207,235,18},{136,183,31}};
        if (m[1][0] != 207) failures++;
    }


    {
        uint8_t src[3] = {16,45,25};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 25) failures++;
    }


    {
        uint16_t r = 29915 + 47628 + 8990 + 15139 + 54782 + 20612 + 51436 + 64317;
        if (r != 30675) failures++;
    }


    {
        volatile uint8_t port = 167;
        uint8_t r = port;
        if (r != 167) failures++;
    }


    {
        volatile int16_t a = -16056;
        volatile int16_t b = 8036;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 100;
        if (buf[12] != 100) failures++;
    }


    {
        int8_t a = -18;
        int8_t b = -66;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 1875313218UL;
        uint32_t b = 2633900471UL;
        uint32_t r = a - b;
        if (r != 3536380043UL) failures++;
    }


    {
        uint32_t a = 716011244UL;
        uint32_t b = 2406936296UL;
        uint32_t r = a ^ b;
        if (r != 2782631940UL) failures++;
    }


    {
        uint16_t x = 8;
        x = x + 187;
        if (x != 195) failures++;
    }


    {
        uint16_t r = call6(40,145,54,155,185,35);
        if (r != 614) failures++;
    }


    {
        uint16_t r = 32791 + 36746 + 12814 + 22549 + 28423 + 25548 + 41984 + 24756;
        if (r != 29003) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)57) / (int16_t)((int8_t)49);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t r = add2(178,138) + add2(138,201) + add2(178,201);
        if (r != 1034) failures++;
    }


    {
        volatile uint8_t port = 57;
        uint8_t r = port;
        if (r != 57) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {214,240,1295,206};
        if (s.a != (uint8_t)214) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)67) + (uint16_t)23631;
        if (r != 23698) failures++;
    }


    {
        uint8_t src[12] = {54,226,119,12,107,178,91,150,150,231,39,1};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[11] != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)13) / (int16_t)((int8_t)6);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        volatile uint8_t port = 24;
        uint8_t r = port;
        if (r != 24) failures++;
    }


    {
        volatile int16_t a = -29487;
        volatile int16_t b = -21521;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 1) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint16_t r = 20433 + 20185 + 18281 + 61319 + 9131 + 30387 + 41358 + 17635;
        if (r != 22121) failures++;
    }


    {
        int8_t a = -83;
        int8_t b = -90;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][3] = {{209,80,169},{208,182,176},{6,216,242},{200,244,83}};
        if (m[3][0] != 200) failures++;
    }


    {
        uint8_t v = 248;
        v ^= 4;
        if (v != 252) failures++;
    }


    {
        g16 = 60602;
        if (read_g16() != 60602) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(197,200) != 65533) failures++;
    }


    {
        uint8_t a[6] = {122,144,94,34,59,185};
        if (a[0] != 122) failures++;
    }


    {
        uint16_t x = 216;
        x = x + 218;
        if (x != 434) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)216) + (uint16_t)37122;
        if (r != 37338) failures++;
    }


    {
        uint16_t r = add2(49,240) + add2(240,22) + add2(49,22);
        if (r != 622) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)111) + (uint16_t)22177;
        if (r != 22288) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 1) sum += j;
        if (sum != 1) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 60691 + 50473 + 9355 + 40645 + 27152 + 38866 + 33907 + 55639;
        if (r != 54584) failures++;
    }


    {
        uint8_t a[6] = {3,108,253,245,96,236};
        if (a[5] != 236) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 6;
        if (buf[2] != 6) failures++;
    }


    {
        uint8_t m[3][4] = {{155,137,199,237},{141,105,62,175},{183,86,0,52}};
        if (m[0][2] != 199) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 138;
        if (buf[10] != 138) failures++;
    }


    {
        uint32_t a = 1556643504UL;
        uint32_t b = 3053864362UL;
        uint32_t r = a | b;
        if (r != 4274945978UL) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 187;
        if (buf[14] != 187) failures++;
    }


    {
        if (((uint16_t)(((35 - 176) - (65 & 164)) - ((227 - 27) & (6 - 97)))) != 65267) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 174;
        if (buf[14] != 174) failures++;
    }


    {
        uint8_t v = 25;
        v &= ~(uint8_t)4;
        if (v != 25) failures++;
    }


    {
        uint8_t v = 174;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-107) / (int16_t)((int8_t)107);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t x = 11;
        x <<= 0;
        if (x != 11) failures++;
    }


    {
        uint8_t x = 19;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint8_t m[4][4] = {{166,128,152,48},{110,208,147,12},{246,214,230,229},{18,16,68,180}};
        if (m[3][2] != 68) failures++;
    }


    {
        volatile uint8_t port = 183;
        uint8_t r = port;
        if (r != 183) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)140) + (uint16_t)30033;
        if (r != 30173) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = call6(82,214,49,239,87,3);
        if (r != 674) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(185,116) != 301) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 98;
        if (buf[10] != 98) failures++;
    }


    {
        uint16_t r = call6(1,148,137,192,90,5);
        if (r != 573) failures++;
    }


    {
        volatile uint8_t port = 44;
        uint8_t r = port;
        if (r != 44) failures++;
    }


    {
        volatile int16_t a = -15290;
        volatile int16_t b = 29113;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 168;
        if (buf[9] != 168) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {72,187,48656,179};
        if (s.d != (uint8_t)179) failures++;
    }


    {
        if (((uint16_t)52) != 52) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(4,77) != 65463) failures++;
    }


    {
        g16 = 41838;
        if (read_g16() != 41838) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)214) + (uint16_t)14295;
        if (r != 14509) failures++;
    }


    {
        uint8_t x = 13;
        x <<= 3;
        if (x != 104) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        if (((uint16_t)((38 + 160) + ((61 | 168) + (35 + 34)))) != 456) failures++;
    }


    {
        uint8_t buf[8] = {39,190,35,191,38,44,95,186};
        uint8_t *p = buf;
        p += 4;
        if (*p != 38) failures++;
    }


    {
        uint8_t x = 238;
        x <<= 4;
        if (x != 224) failures++;
    }


    {
        uint8_t m[3][2] = {{35,164},{102,10},{242,135}};
        if (m[2][0] != 242) failures++;
    }


    {
        uint32_t a = 1756592825UL;
        uint32_t b = 3345506197UL;
        uint32_t r = a ^ b;
        if (r != 2950374700UL) failures++;
    }


    {
        uint8_t x = 71;
        x <<= 2;
        if (x != 28) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(213,250) != 463) failures++;
    }


    {
        uint8_t m[4][3] = {{161,242,52},{96,131,174},{223,59,63},{143,178,113}};
        if (m[0][1] != 242) failures++;
    }


    {
        uint16_t x = 40555;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -12636;
        volatile int16_t b = 1358;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 213;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint16_t x = 212;
        x = x + 189;
        if (x != 401) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(((61 | 110) | (117 - 184)) & ((238 + 80) | (100 & 226)))) != 382) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 62;
        if (buf[6] != 62) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        if (((uint16_t)(((198 + 215) | 14) & 234)) != 138) failures++;
    }


    {
        volatile uint8_t port = 207;
        uint8_t r = port;
        if (r != 207) failures++;
    }


    {
        uint16_t x = 176;
        x = x + 15;
        if (x != 191) failures++;
    }


    {
        uint32_t a = 3888146108UL;
        uint32_t b = 1640923761UL;
        uint32_t r = a - b;
        if (r != 2247222347UL) failures++;
    }


    {
        uint16_t r = add2(46,13) + add2(13,115) + add2(46,115);
        if (r != 348) failures++;
    }


    {
        g16 = 41127;
        if (read_g16() != 41127) failures++;
    }


    {
        int8_t a = 96;
        int8_t b = -119;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 12: result = 179; break;
        case 15: result = 8; break;
        case 1: result = 85; break;
        case 2: result = 203; break;
        case 14: result = 22; break;
        default: result = 215; break;
        }
        if (result != 215) failures++;
    }


    {
        uint32_t a = 1220898525UL;
        uint32_t b = 1781681233UL;
        uint32_t r = a + b;
        if (r != 3002579758UL) failures++;
    }


    {
        uint8_t buf[8] = {126,62,108,2,159,192,47,203};
        uint8_t *p = buf;
        p += 7;
        if (*p != 203) failures++;
    }


    {
        uint8_t x = 146;
        x <<= 4;
        if (x != 32) failures++;
    }


    {
        uint32_t a = 3688215905UL;
        uint32_t b = 18044358UL;
        uint32_t r = a & b;
        if (r != 17896768UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(151,138) != 289) failures++;
    }


    {
        volatile int16_t a = -217;
        volatile int16_t b = 6210;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {239,21,173,143,117,244};
        if (a[4] != 117) failures++;
    }


    {
        uint32_t a = 387452567UL;
        uint32_t b = 574130651UL;
        uint32_t r = a - b;
        if (r != 4108289212UL) failures++;
    }


    {
        uint8_t v = 11;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 11707;
        volatile int16_t b = 19392;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)248) + (uint16_t)57957;
        if (r != 58205) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {51,103,58603,203};
        if (s.a != (uint8_t)51) failures++;
    }


    {
        uint8_t m[4][2] = {{191,202},{120,46},{17,88},{201,83}};
        if (m[2][0] != 17) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(219,164) != 383) failures++;
    }


    {
        uint8_t a[6] = {99,153,113,164,38,84};
        if (a[5] != 84) failures++;
    }


    {
        uint8_t v = 243;
        v ^= 2;
        if (v != 241) failures++;
    }


    {
        uint16_t x = 16230;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 145;
        x = x + 217;
        if (x != 362) failures++;
    }


    {
        uint8_t buf[8] = {106,118,240,64,61,227,6,142};
        uint8_t *p = buf;
        p += 5;
        if (*p != 227) failures++;
    }


    {
        g16 = 4869;
        if (read_g16() != 4869) failures++;
    }


    {
        uint16_t x = 209;
        x = x + 206;
        if (x != 415) failures++;
    }


    {
        g16 = 23886;
        if (read_g16() != 23886) failures++;
    }


    {
        uint16_t r = call6(160,20,208,139,237,183);
        if (r != 947) failures++;
    }


    {
        uint8_t m[4][2] = {{80,222},{213,203},{10,7},{208,164}};
        if (m[3][1] != 164) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 12: result = 33; break;
        case 15: result = 198; break;
        case 6: result = 229; break;
        case 10: result = 49; break;
        case 0: result = 136; break;
        case 11: result = 15; break;
        case 1: result = 150; break;
        case 16: result = 46; break;
        default: result = 64; break;
        }
        if (result != 15) failures++;
    }


    {
        uint16_t x = 7;
        x = x + 162;
        if (x != 169) failures++;
    }

    return failures;
}