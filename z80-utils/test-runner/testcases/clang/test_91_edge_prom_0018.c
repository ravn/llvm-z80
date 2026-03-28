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
        volatile uint8_t port = 88;
        uint8_t r = port;
        if (r != 88) failures++;
    }


    {
        uint8_t a[6] = {158,51,179,251,135,93};
        if (a[3] != 251) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)108) % (int16_t)((int8_t)75);
        if ((uint16_t)r != (uint16_t)33) failures++;
    }


    {
        uint32_t a = 2440388803UL;
        uint32_t b = 3384741515UL;
        uint32_t r = a ^ b;
        if (r != 1489661512UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)112) / (int16_t)((int8_t)118);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {60,4,9833,104};
        if (s.b != (uint8_t)4) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 15: result = 18; break;
        case 14: result = 46; break;
        case 5: result = 155; break;
        case 10: result = 83; break;
        case 2: result = 42; break;
        case 8: result = 221; break;
        case 6: result = 41; break;
        default: result = 149; break;
        }
        if (result != 42) failures++;
    }


    {
        g16 = 36811;
        if (read_g16() != 36811) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 19: result = 79; break;
        case 2: result = 64; break;
        case 5: result = 108; break;
        default: result = 186; break;
        }
        if (result != 108) failures++;
    }


    {
        uint8_t v = 85;
        int r = (v & 32) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(230,76,96,206,17,208);
        if (r != 833) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint16_t x = 48027;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 52243;
        if (read_g16() != 52243) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 13: result = 203; break;
        case 2: result = 155; break;
        case 18: result = 103; break;
        case 4: result = 109; break;
        case 12: result = 75; break;
        default: result = 229; break;
        }
        if (result != 103) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {202,149,34041,240};
        if (s.b != (uint8_t)149) failures++;
    }


    {
        int8_t a = 58;
        int8_t b = -96;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2263667529UL;
        uint32_t b = 4120879432UL;
        uint32_t r = a | b;
        if (r != 4160746313UL) failures++;
    }


    {
        uint8_t v = 235;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)209) + (uint16_t)56426;
        if (r != 56635) failures++;
    }


    {
        uint16_t r = add2(215,144) + add2(144,238) + add2(215,238);
        if (r != 1194) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)83) / (int16_t)((int8_t)-117);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 151;
        v &= ~(uint8_t)4;
        if (v != 147) failures++;
    }


    {
        uint16_t x = 100;
        x = x + 220;
        if (x != 320) failures++;
    }


    {
        g16 = 47723;
        if (read_g16() != 47723) failures++;
    }


    {
        uint8_t v = 173;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(194,47,167,93,131,95);
        if (r != 727) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)245) + (uint16_t)7419;
        if (r != 7664) failures++;
    }


    {
        uint8_t a[6] = {165,53,1,249,85,107};
        if (a[3] != 249) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        volatile uint8_t port = 236;
        uint8_t r = port;
        if (r != 236) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -116;
        int8_t b = 125;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 61844 + 60074 + 42628 + 39223 + 14290 + 51395 + 63329 + 38608;
        if (r != 43711) failures++;
    }


    {
        uint8_t m[2][4] = {{92,130,227,57},{12,91,19,40}};
        if (m[0][3] != 57) failures++;
    }


    {
        uint8_t v = 163;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint8_t buf[8] = {188,120,102,247,138,216,222,243};
        uint8_t *p = buf;
        p += 1;
        if (*p != 120) failures++;
    }


    {
        uint16_t r = call6(31,72,241,81,51,105);
        if (r != 581) failures++;
    }


    {
        uint8_t src[10] = {170,196,119,33,64,222,85,205,60,42};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[3] != 33) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)92) + (uint16_t)39323;
        if (r != 39415) failures++;
    }


    {
        uint8_t m[2][4] = {{162,12,46,137},{81,239,28,201}};
        if (m[1][1] != 239) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)36) % (int16_t)((int8_t)-14);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        g16 = 55639;
        if (read_g16() != 55639) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)78) % (int16_t)((int8_t)-120);
        if ((uint16_t)r != (uint16_t)78) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {245,132,25327,120};
        if (s.b != (uint8_t)132) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        volatile uint8_t port = 151;
        uint8_t r = port;
        if (r != 151) failures++;
    }


    {
        uint16_t r = 31127 + 45567 + 20696 + 38837 + 2032 + 11087 + 22273 + 28071;
        if (r != 3082) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)230) + (uint16_t)36048;
        if (r != 36278) failures++;
    }


    {
        uint16_t r = call6(102,93,49,17,201,50);
        if (r != 512) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t a[6] = {19,243,149,115,38,190};
        if (a[0] != 19) failures++;
    }


    {
        uint16_t x = 251;
        x = x + 36;
        if (x != 287) failures++;
    }


    {
        volatile int16_t a = -20742;
        volatile int16_t b = 30163;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 205;
        x = x + 46;
        if (x != 251) failures++;
    }


    {
        volatile int16_t a = -8833;
        volatile int16_t b = -16977;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][4] = {{85,170,226,253},{56,56,178,62},{56,186,209,56},{32,206,96,133}};
        if (m[3][3] != 133) failures++;
    }


    {
        uint8_t v = 243;
        v &= ~(uint8_t)2;
        if (v != 241) failures++;
    }


    {
        uint8_t x = 59;
        x <<= 2;
        if (x != 236) failures++;
    }


    {
        uint8_t v = 87;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t v = 211;
        v |= 1;
        if (v != 211) failures++;
    }


    {
        uint8_t v = 115;
        v ^= 8;
        if (v != 123) failures++;
    }


    {
        uint8_t x = 171;
        x <<= 3;
        if (x != 88) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 253;
        if (buf[1] != 253) failures++;
    }


    {
        volatile uint8_t port = 31;
        uint8_t r = port;
        if (r != 31) failures++;
    }


    {
        uint16_t r = call6(62,185,31,113,138,235);
        if (r != 764) failures++;
    }


    {
        uint8_t m[4][3] = {{42,160,249},{11,11,111},{135,54,191},{66,83,219}};
        if (m[0][1] != 160) failures++;
    }


    {
        uint32_t a = 2273649760UL;
        uint32_t b = 3350200375UL;
        uint32_t r = a & b;
        if (r != 2273312800UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(163,220) != 65479) failures++;
    }


    {
        if (((uint16_t)(71 ^ (38 | (12 | 149)))) != 248) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        uint16_t x = 24424;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)((44 - (111 + 83)) ^ ((30 + 135) & 87))) != 65391) failures++;
    }


    {
        uint32_t a = 877015423UL;
        uint32_t b = 250977633UL;
        uint32_t r = a & b;
        if (r != 71569761UL) failures++;
    }


    {
        volatile uint8_t port = 160;
        uint8_t r = port;
        if (r != 160) failures++;
    }


    {
        volatile uint8_t port = 69;
        uint8_t r = port;
        if (r != 69) failures++;
    }


    {
        uint16_t r = add2(157,162) + add2(162,7) + add2(157,7);
        if (r != 652) failures++;
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
        for (uint8_t j = 0; j < 12; j++) buf[j] = 91;
        if (buf[11] != 91) failures++;
    }


    {
        g16 = 58835;
        if (read_g16() != 58835) failures++;
    }


    {
        uint16_t r = 58649 + 19645 + 45566 + 14765 + 31561 + 2535 + 58061 + 6796;
        if (r != 40970) failures++;
    }


    {
        uint8_t v = 48;
        v |= 2;
        if (v != 50) failures++;
    }


    {
        volatile uint8_t port = 76;
        uint8_t r = port;
        if (r != 76) failures++;
    }


    {
        uint8_t src[7] = {157,15,105,116,102,228,156};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[0] != 157) failures++;
    }


    {
        volatile int16_t a = -9326;
        volatile int16_t b = -1714;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(12,24) + add2(24,241) + add2(12,241);
        if (r != 554) failures++;
    }


    {
        uint8_t x = 166;
        x <<= 4;
        if (x != 96) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[9] = {162,191,127,227,99,243,125,232,73};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 127) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(92,56) + add2(56,242) + add2(92,242);
        if (r != 780) failures++;
    }


    {
        uint8_t src[1] = {226};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 226) failures++;
    }


    {
        uint16_t r = 63056 + 59534 + 48290 + 49703 + 50400 + 27498 + 33648 + 34895;
        if (r != 39344) failures++;
    }


    {
        uint8_t v = 203;
        v ^= 4;
        if (v != 207) failures++;
    }


    {
        int8_t a = 92;
        int8_t b = -14;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -32;
        int8_t b = 82;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 37;
        if (buf[5] != 37) failures++;
    }


    {
        uint16_t x = 133;
        x = x + 99;
        if (x != 232) failures++;
    }


    {
        uint8_t x = 18;
        x <<= 2;
        if (x != 72) failures++;
    }


    {
        uint16_t r = 45163 + 53688 + 29728 + 58902 + 33925 + 51059 + 14413 + 32004;
        if (r != 56738) failures++;
    }


    {
        uint16_t x = 15482;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {41,169,49067,67};
        if (s.a != (uint8_t)41) failures++;
    }


    {
        volatile uint8_t port = 154;
        uint8_t r = port;
        if (r != 154) failures++;
    }


    {
        uint16_t x = 3247;
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
        case 9: result = 159; break;
        case 14: result = 76; break;
        case 2: result = 81; break;
        case 12: result = 242; break;
        default: result = 202; break;
        }
        if (result != 81) failures++;
    }


    {
        volatile uint8_t port = 48;
        uint8_t r = port;
        if (r != 48) failures++;
    }


    {
        uint16_t r = call6(43,166,1,216,135,235);
        if (r != 796) failures++;
    }


    {
        uint8_t x = 153;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 79;
        if (buf[11] != 79) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)100) + (uint16_t)63316;
        if (r != 63416) failures++;
    }


    {
        uint16_t r = 18874 + 61296 + 32908 + 2795 + 52073 + 847 + 27891 + 56462;
        if (r != 56538) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint8_t a[6] = {53,110,108,229,197,237};
        if (a[5] != 237) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -3226;
        volatile int16_t b = -19821;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {6,161,13162,111};
        if (s.d != (uint8_t)111) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 210; break;
        case 5: result = 180; break;
        case 12: result = 26; break;
        default: result = 93; break;
        }
        if (result != 210) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t r = call6(48,14,148,155,61,52);
        if (r != 478) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 83;
        if (buf[3] != 83) failures++;
    }


    {
        uint16_t r = call6(217,92,176,122,110,46);
        if (r != 763) failures++;
    }


    {
        uint8_t m[3][3] = {{73,246,42},{234,155,54},{134,225,153}};
        if (m[1][1] != 155) failures++;
    }


    {
        uint16_t x = 23829;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 38963;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 132;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint8_t a[6] = {181,255,25,253,161,87};
        if (a[0] != 181) failures++;
    }


    {
        uint16_t r = add2(30,97) + add2(97,168) + add2(30,168);
        if (r != 590) failures++;
    }


    {
        uint8_t m[2][2] = {{207,117},{181,5}};
        if (m[0][0] != 207) failures++;
    }


    {
        uint16_t r = 45162 + 52574 + 52645 + 14617 + 5088 + 5020 + 12741 + 30136;
        if (r != 21375) failures++;
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
        uint16_t r = (uint16_t)((uint8_t)28) + (uint16_t)58271;
        if (r != 58299) failures++;
    }


    {
        uint8_t v = 85;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 11) failures++;
    }


    {
        uint8_t v = 158;
        v &= ~(uint8_t)2;
        if (v != 156) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {129,94,11146,171};
        if (s.c != (uint16_t)11146) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(175,245) != 420) failures++;
    }


    {
        g16 = 28613;
        if (read_g16() != 28613) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)203) + (uint16_t)18612;
        if (r != 18815) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint8_t v = 207;
        int r = (v & 8) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(37,255) != 65318) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint8_t v = 103;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)70) % (int16_t)((int8_t)-126);
        if ((uint16_t)r != (uint16_t)70) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)124) / (int16_t)((int8_t)121);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        g16 = 4714;
        if (read_g16() != 4714) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)194) + (uint16_t)26173;
        if (r != 26367) failures++;
    }


    {
        uint16_t x = 52650;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 49167;
        if (read_g16() != 49167) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 116;
        if (buf[11] != 116) failures++;
    }


    {
        uint8_t x = 115;
        x <<= 2;
        if (x != 204) failures++;
    }


    {
        uint8_t x = 97;
        x <<= 4;
        if (x != 16) failures++;
    }


    {
        uint16_t r = 5193 + 36334 + 65305 + 28867 + 9954 + 44246 + 49958 + 34674;
        if (r != 12387) failures++;
    }


    {
        uint8_t v = 135;
        v &= ~(uint8_t)64;
        if (v != 135) failures++;
    }


    {
        g16 = 21988;
        if (read_g16() != 21988) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {226,5,10555,196};
        if (s.c != (uint16_t)10555) failures++;
    }


    {
        uint32_t a = 617940380UL;
        uint32_t b = 2327984065UL;
        uint32_t r = a & b;
        if (r != 12584320UL) failures++;
    }


    {
        uint16_t r = 51411 + 28936 + 6878 + 63282 + 43935 + 21916 + 46590 + 43454;
        if (r != 44258) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        volatile uint8_t port = 25;
        uint8_t r = port;
        if (r != 25) failures++;
    }


    {
        uint8_t x = 97;
        x <<= 4;
        if (x != 16) failures++;
    }


    {
        uint16_t r = call6(27,204,75,180,30,248);
        if (r != 764) failures++;
    }


    {
        uint8_t x = 197;
        x <<= 3;
        if (x != 40) failures++;
    }


    {
        uint16_t x = 52517;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 197;
        v &= ~(uint8_t)2;
        if (v != 197) failures++;
    }


    {
        int8_t a = 78;
        int8_t b = 122;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 224;
        uint8_t r = port;
        if (r != 224) failures++;
    }


    {
        volatile int16_t a = -29628;
        volatile int16_t b = -13392;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)((9 ^ 75) | ((27 + 147) + 39))) != 215) failures++;
    }


    {
        g16 = 25263;
        if (read_g16() != 25263) failures++;
    }


    {
        if (((uint16_t)13) != 13) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint8_t v = 178;
        int r = (v & 128) ? 1 : 0;
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
        g16 = 34022;
        if (read_g16() != 34022) failures++;
    }


    {
        volatile uint8_t port = 59;
        uint8_t r = port;
        if (r != 59) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 10;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        volatile int16_t a = 21937;
        volatile int16_t b = -1890;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-8) / (int16_t)((int8_t)122);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 45;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 19) failures++;
    }


    {
        uint8_t m[3][4] = {{188,243,84,170},{211,44,18,145},{48,158,247,203}};
        if (m[1][2] != 18) failures++;
    }


    {
        uint8_t x = 83;
        x <<= 3;
        if (x != 152) failures++;
    }


    {
        uint8_t x = 172;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint8_t src[12] = {219,219,192,199,25,103,129,185,156,59,51,121};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[5] != 103) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {206,216,59073,138};
        if (s.c != (uint16_t)59073) failures++;
    }


    {
        uint8_t v = 241;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)135) + (uint16_t)25307;
        if (r != 25442) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 61; break;
        case 0: result = 54; break;
        case 11: result = 194; break;
        case 13: result = 186; break;
        case 4: result = 114; break;
        case 10: result = 233; break;
        case 16: result = 90; break;
        default: result = 137; break;
        }
        if (result != 61) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(26,75) != 101) failures++;
    }


    {
        volatile int16_t a = 1175;
        volatile int16_t b = -10841;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 192;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        int8_t a = -35;
        int8_t b = 67;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 61;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 180;
        v &= ~(uint8_t)8;
        if (v != 180) failures++;
    }


    {
        uint16_t r = 22628 + 35701 + 9320 + 51556 + 24669 + 23227 + 25704 + 55389;
        if (r != 51586) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t r = 3092 + 2232 + 30909 + 52457 + 4914 + 26657 + 59315 + 49766;
        if (r != 32734) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)48) % (int16_t)((int8_t)2);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(108,50) != 58) failures++;
    }


    {
        uint8_t a[6] = {218,80,189,104,27,175};
        if (a[5] != 175) failures++;
    }


    {
        uint8_t m[4][2] = {{30,4},{232,104},{121,72},{143,160}};
        if (m[0][0] != 30) failures++;
    }


    {
        int8_t a = -107;
        int8_t b = 39;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 139;
        if (buf[13] != 139) failures++;
    }


    {
        int8_t a = 61;
        int8_t b = 14;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[2][3] = {{201,26,207},{178,113,131}};
        if (m[0][0] != 201) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-109) / (int16_t)((int8_t)22);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        uint8_t src[13] = {120,212,179,36,174,132,84,102,74,205,83,82,115};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[9] != 205) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        int8_t a = 54;
        int8_t b = 52;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(86,149,208,168,32,205);
        if (r != 848) failures++;
    }


    {
        uint32_t a = 1208957151UL;
        uint32_t b = 1738349306UL;
        uint32_t r = a + b;
        if (r != 2947306457UL) failures++;
    }


    {
        uint16_t r = add2(237,182) + add2(182,166) + add2(237,166);
        if (r != 1170) failures++;
    }


    {
        uint8_t v = 109;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 19) failures++;
    }


    {
        uint8_t x = 218;
        x <<= 5;
        if (x != 64) failures++;
    }


    {
        uint16_t r = call6(205,143,59,125,149,50);
        if (r != 731) failures++;
    }


    {
        volatile int16_t a = 20773;
        volatile int16_t b = -30932;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[7] = {113,69,40,237,43,12,13};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[5] != 12) failures++;
    }


    {
        if (((uint16_t)(137 | (72 ^ (121 + 209)))) != 395) failures++;
    }


    {
        uint16_t r = add2(168,150) + add2(150,68) + add2(168,68);
        if (r != 772) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(128,161) != 65503) failures++;
    }


    {
        uint16_t x = 57;
        x = x + 114;
        if (x != 171) failures++;
    }


    {
        uint32_t a = 1273266980UL;
        uint32_t b = 3831221092UL;
        uint32_t r = a ^ b;
        if (r != 2948548672UL) failures++;
    }


    {
        uint32_t a = 3304975059UL;
        uint32_t b = 1934124490UL;
        uint32_t r = a - b;
        if (r != 1370850569UL) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 0: result = 181; break;
        case 1: result = 9; break;
        case 8: result = 40; break;
        default: result = 184; break;
        }
        if (result != 40) failures++;
    }


    {
        uint8_t src[3] = {171,177,180};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 180) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 7: result = 125; break;
        case 4: result = 33; break;
        case 17: result = 127; break;
        case 8: result = 71; break;
        default: result = 253; break;
        }
        if (result != 71) failures++;
    }


    {
        uint16_t r = 11945 + 58137 + 65453 + 48742 + 24565 + 41768 + 4835 + 22277;
        if (r != 15578) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(205,223) != 65518) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-44) / (int16_t)((int8_t)-19);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint8_t src[3] = {197,252,158};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 197) failures++;
    }


    {
        uint8_t v = 182;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {144,14,28,236,32,58,70,174};
        uint8_t *p = buf;
        p += 3;
        if (*p != 236) failures++;
    }


    {
        uint16_t r = call6(203,248,241,70,255,46);
        if (r != 1063) failures++;
    }


    {
        uint8_t m[4][3] = {{46,130,176},{140,241,82},{163,111,156},{122,175,8}};
        if (m[3][1] != 175) failures++;
    }


    {
        int8_t a = -63;
        int8_t b = 20;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {191,52,18,164,121,50,19,20};
        uint8_t *p = buf;
        p += 5;
        if (*p != 50) failures++;
    }


    {
        volatile int16_t a = 9626;
        volatile int16_t b = 19853;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-72) / (int16_t)((int8_t)73);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 116;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 14580;
        if (read_g16() != 14580) failures++;
    }


    {
        volatile uint8_t port = 144;
        uint8_t r = port;
        if (r != 144) failures++;
    }


    {
        uint8_t m[2][4] = {{227,207,73,252},{86,59,244,194}};
        if (m[0][1] != 207) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {39,2,15743,215};
        if (s.b != (uint8_t)2) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 105;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 23) failures++;
    }


    {
        volatile int16_t a = 14614;
        volatile int16_t b = 23608;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(44,24) + add2(24,234) + add2(44,234);
        if (r != 604) failures++;
    }


    {
        uint8_t src[15] = {99,83,129,69,177,2,50,250,223,4,174,240,8,170,161};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[4] != 177) failures++;
    }


    {
        uint16_t r = add2(23,61) + add2(61,55) + add2(23,55);
        if (r != 278) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(67,236,244,14,226,63);
        if (r != 850) failures++;
    }


    {
        uint8_t a[6] = {177,188,36,252,69,13};
        if (a[3] != 252) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 5: result = 84; break;
        case 14: result = 232; break;
        case 18: result = 231; break;
        case 4: result = 83; break;
        case 15: result = 14; break;
        case 12: result = 225; break;
        case 8: result = 88; break;
        case 0: result = 120; break;
        default: result = 53; break;
        }
        if (result != 83) failures++;
    }


    {
        uint16_t r = 50089 + 314 + 59760 + 20090 + 32859 + 5792 + 44314 + 46670;
        if (r != 63280) failures++;
    }


    {
        int8_t a = -20;
        int8_t b = -7;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 110;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t a[6] = {222,121,1,67,233,165};
        if (a[5] != 165) failures++;
    }


    {
        uint16_t x = 109;
        x = x + 109;
        if (x != 218) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(193,57) != 136) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(1,164) != 65373) failures++;
    }


    {
        uint8_t m[4][2] = {{64,3},{195,181},{102,109},{178,155}};
        if (m[0][0] != 64) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 9201;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)47) + (uint16_t)20572;
        if (r != 20619) failures++;
    }


    {
        uint16_t r = 41688 + 2147 + 3151 + 45023 + 10897 + 19332 + 14110 + 15456;
        if (r != 20732) failures++;
    }


    {
        uint16_t x = 73;
        x = x + 110;
        if (x != 183) failures++;
    }


    {
        uint16_t r = call6(31,15,157,174,198,120);
        if (r != 695) failures++;
    }


    {
        int8_t a = 47;
        int8_t b = 98;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {201,43,43311,32};
        if (s.c != (uint16_t)43311) failures++;
    }


    {
        volatile uint8_t port = 86;
        uint8_t r = port;
        if (r != 86) failures++;
    }


    {
        uint8_t a[6] = {3,24,220,79,98,158};
        if (a[2] != 220) failures++;
    }


    {
        uint16_t r = 49077 + 52373 + 29620 + 16905 + 38543 + 7753 + 61373 + 53571;
        if (r != 47071) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(33,181) != 214) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint8_t src[7] = {38,189,100,86,167,46,179};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[5] != 46) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint16_t x = 36858;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 23;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 113;
        x <<= 2;
        if (x != 196) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)229) + (uint16_t)35591;
        if (r != 35820) failures++;
    }


    {
        uint8_t src[8] = {39,240,11,134,140,99,15,211};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[4] != 140) failures++;
    }


    {
        uint16_t r = 2014 + 24083 + 42315 + 55130 + 31921 + 12498 + 31455 + 9454;
        if (r != 12262) failures++;
    }


    {
        uint32_t a = 4235869925UL;
        uint32_t b = 4149859766UL;
        uint32_t r = a | b;
        if (r != 4286316535UL) failures++;
    }


    {
        uint8_t v = 208;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {231,253,49860,26};
        if (s.b != (uint8_t)253) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 9: result = 153; break;
        case 0: result = 27; break;
        case 4: result = 120; break;
        case 6: result = 134; break;
        case 3: result = 213; break;
        case 13: result = 64; break;
        default: result = 230; break;
        }
        if (result != 64) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)189) + (uint16_t)25819;
        if (r != 26008) failures++;
    }


    {
        volatile uint8_t port = 56;
        uint8_t r = port;
        if (r != 56) failures++;
    }


    {
        uint8_t x = 229;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint8_t v = 38;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {192,39,76,134,61,59,167,146};
        uint8_t *p = buf;
        p += 6;
        if (*p != 167) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {128,62,1518,75};
        if (s.d != (uint8_t)75) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 225;
        v &= ~(uint8_t)8;
        if (v != 225) failures++;
    }


    {
        uint16_t x = 64;
        x = x + 37;
        if (x != 101) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(247,138) != 109) failures++;
    }


    {
        uint16_t r = 40645 + 48463 + 24032 + 232 + 52260 + 9991 + 38070 + 14296;
        if (r != 31381) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-106) / (int16_t)((int8_t)99);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t a[6] = {254,146,150,75,164,35};
        if (a[4] != 164) failures++;
    }


    {
        uint8_t a[6] = {203,110,139,226,99,107};
        if (a[3] != 226) failures++;
    }


    {
        uint16_t r = add2(0,221) + add2(221,9) + add2(0,9);
        if (r != 460) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(171 & ((237 & 142) + 105))) != 161) failures++;
    }


    {
        uint16_t r = 2438 + 64309 + 40498 + 45400 + 41623 + 32786 + 1737 + 18416;
        if (r != 50599) failures++;
    }


    {
        uint16_t x = 248;
        x = x + 37;
        if (x != 285) failures++;
    }


    {
        g16 = 11681;
        if (read_g16() != 11681) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)178) + (uint16_t)41450;
        if (r != 41628) failures++;
    }


    {
        uint8_t v = 112;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 8) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {216,1,60056,229};
        if (s.a != (uint8_t)216) failures++;
    }


    {
        uint8_t v = 21;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 12;
        x = x + 53;
        if (x != 65) failures++;
    }


    {
        uint16_t r = call6(194,35,155,42,165,221);
        if (r != 812) failures++;
    }


    {
        uint8_t v = 214;
        v &= ~(uint8_t)128;
        if (v != 86) failures++;
    }


    {
        uint8_t src[14] = {184,204,116,19,234,124,75,166,39,203,133,169,83,244};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[1] != 204) failures++;
    }


    {
        uint8_t x = 92;
        x <<= 4;
        if (x != 192) failures++;
    }


    {
        uint16_t r = call6(78,202,96,46,252,7);
        if (r != 681) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 20;
        if (buf[12] != 20) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 115;
        v |= 2;
        if (v != 115) failures++;
    }


    {
        if (((uint16_t)(((153 | 195) & 245) ^ ((39 & 15) & (34 - 9)))) != 208) failures++;
    }


    {
        uint16_t x = 135;
        x = x + 68;
        if (x != 203) failures++;
    }


    {
        uint8_t m[3][2] = {{117,100},{52,14},{200,178}};
        if (m[2][0] != 200) failures++;
    }


    {
        uint16_t r = 33197 + 7740 + 24304 + 57802 + 13374 + 46253 + 29446 + 35698;
        if (r != 51206) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
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
        if (fn(111,108) != 219) failures++;
    }


    {
        volatile uint8_t port = 81;
        uint8_t r = port;
        if (r != 81) failures++;
    }


    {
        uint16_t x = 61597;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {53,167,104,90,69,25};
        if (a[0] != 53) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[7] = {170,130,114,69,131,46,110};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[6] != 110) failures++;
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
        if (fn(154,151) != 305) failures++;
    }


    {
        uint16_t r = 32034 + 41260 + 14239 + 19342 + 56452 + 3939 + 59004 + 7952;
        if (r != 37614) failures++;
    }


    {
        volatile int16_t a = 14293;
        volatile int16_t b = 27966;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)16) + (uint16_t)7449;
        if (r != 7465) failures++;
    }


    {
        uint8_t v = 139;
        v &= ~(uint8_t)8;
        if (v != 131) failures++;
    }


    {
        g16 = 17931;
        if (read_g16() != 17931) failures++;
    }


    {
        uint8_t a[6] = {244,237,120,198,112,5};
        if (a[1] != 237) failures++;
    }


    {
        uint16_t x = 13468;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[4][4] = {{162,105,203,247},{185,75,193,106},{114,25,124,19},{144,81,211,11}};
        if (m[3][0] != 144) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 217;
        if (buf[8] != 217) failures++;
    }


    {
        uint8_t v = 189;
        v |= 2;
        if (v != 191) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 185;
        if (buf[11] != 185) failures++;
    }


    {
        volatile uint8_t port = 139;
        uint8_t r = port;
        if (r != 139) failures++;
    }


    {
        uint8_t a[6] = {188,188,82,173,89,28};
        if (a[0] != 188) failures++;
    }


    {
        uint8_t buf[8] = {143,19,181,222,74,141,64,104};
        uint8_t *p = buf;
        p += 2;
        if (*p != 181) failures++;
    }


    {
        uint8_t buf[8] = {4,195,106,29,29,208,37,68};
        uint8_t *p = buf;
        p += 2;
        if (*p != 106) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 4: result = 81; break;
        case 6: result = 209; break;
        case 1: result = 72; break;
        case 11: result = 209; break;
        case 12: result = 173; break;
        default: result = 203; break;
        }
        if (result != 173) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(133,246) != 65423) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)1) % (int16_t)((int8_t)124);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t buf[8] = {177,4,195,229,4,18,187,80};
        uint8_t *p = buf;
        p += 1;
        if (*p != 4) failures++;
    }


    {
        uint16_t x = 48241;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {57,163,59898,34};
        if (s.d != (uint8_t)34) failures++;
    }


    {
        uint16_t x = 96;
        x = x + 99;
        if (x != 195) failures++;
    }


    {
        int8_t a = -97;
        int8_t b = -2;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 39626;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 12;
        int8_t b = -126;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {16,242,14,12,233,6,58,23};
        uint8_t *p = buf;
        p += 1;
        if (*p != 242) failures++;
    }


    {
        uint32_t a = 25793170UL;
        uint32_t b = 3116892598UL;
        uint32_t r = a | b;
        if (r != 3116995510UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {78,151,14153,149};
        if (s.a != (uint8_t)78) failures++;
    }


    {
        if (((uint16_t)201) != 201) failures++;
    }


    {
        uint32_t a = 2488356381UL;
        uint32_t b = 132776192UL;
        uint32_t r = a + b;
        if (r != 2621132573UL) failures++;
    }


    {
        uint8_t a[6] = {134,105,57,170,152,130};
        if (a[2] != 57) failures++;
    }


    {
        uint8_t src[8] = {207,7,252,36,151,197,102,102};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[7] != 102) failures++;
    }


    {
        uint8_t a[6] = {233,236,105,145,242,64};
        if (a[2] != 105) failures++;
    }


    {
        uint16_t r = add2(103,234) + add2(234,124) + add2(103,124);
        if (r != 922) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 2) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint16_t r = add2(31,40) + add2(40,205) + add2(31,205);
        if (r != 552) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(62,74) != 136) failures++;
    }


    {
        uint8_t src[13] = {88,45,163,186,228,172,180,19,78,19,147,217,230};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[5] != 172) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {21,193,417,6};
        if (s.a != (uint8_t)21) failures++;
    }


    {
        volatile int16_t a = 24764;
        volatile int16_t b = 30303;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(13,134) != 65415) failures++;
    }


    {
        g16 = 62549;
        if (read_g16() != 62549) failures++;
    }


    {
        uint8_t m[4][4] = {{253,236,128,5},{117,56,8,87},{176,144,190,210},{170,63,175,240}};
        if (m[0][2] != 128) failures++;
    }


    {
        volatile uint8_t port = 77;
        uint8_t r = port;
        if (r != 77) failures++;
    }


    {
        uint16_t x = 39;
        x = x + 197;
        if (x != 236) failures++;
    }


    {
        uint16_t x = 153;
        x = x + 116;
        if (x != 269) failures++;
    }


    {
        volatile uint8_t port = 78;
        uint8_t r = port;
        if (r != 78) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)139) + (uint16_t)6829;
        if (r != 6968) failures++;
    }


    {
        uint8_t a[6] = {155,210,78,169,67,60};
        if (a[4] != 67) failures++;
    }


    {
        uint8_t v = 169;
        int r = (v & 64) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int8_t a = 34;
        int8_t b = -42;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {248,9,85,38,55,37,104,116};
        uint8_t *p = buf;
        p += 0;
        if (*p != 248) failures++;
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
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {109,253,41396,82};
        if (s.c != (uint16_t)41396) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-64) % (int16_t)((int8_t)81);
        if ((uint16_t)r != (uint16_t)65472) failures++;
    }


    {
        volatile int16_t a = 939;
        volatile int16_t b = -4647;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 125;
        x = x + 159;
        if (x != 284) failures++;
    }


    {
        uint8_t m[3][4] = {{194,199,147,55},{129,255,231,182},{67,93,207,147}};
        if (m[1][2] != 231) failures++;
    }


    {
        int8_t a = 98;
        int8_t b = -58;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)50) != 50) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)108) + (uint16_t)26527;
        if (r != 26635) failures++;
    }


    {
        uint8_t v = 250;
        v ^= 4;
        if (v != 254) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint8_t v = 124;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        volatile uint8_t port = 243;
        uint8_t r = port;
        if (r != 243) failures++;
    }


    {
        uint8_t v = 34;
        v ^= 128;
        if (v != 162) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)39) + (uint16_t)62170;
        if (r != 62209) failures++;
    }


    {
        uint8_t m[2][4] = {{28,175,230,108},{84,126,248,10}};
        if (m[1][2] != 248) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t r = call6(80,244,216,214,219,237);
        if (r != 1210) failures++;
    }


    {
        uint16_t r = 61400 + 23518 + 38627 + 46710 + 20291 + 1906 + 6662 + 41917;
        if (r != 44423) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 1) sum += j;
        if (sum != 66) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-78) % (int16_t)((int8_t)-80);
        if ((uint16_t)r != (uint16_t)65458) failures++;
    }


    {
        uint16_t r = add2(157,39) + add2(39,139) + add2(157,139);
        if (r != 670) failures++;
    }


    {
        uint16_t r = call6(143,49,128,142,172,155);
        if (r != 789) failures++;
    }


    {
        uint8_t v = 177;
        v |= 8;
        if (v != 185) failures++;
    }


    {
        uint8_t m[4][4] = {{56,151,165,0},{127,115,228,165},{254,231,74,60},{225,140,230,207}};
        if (m[0][3] != 0) failures++;
    }


    {
        uint16_t r = add2(131,46) + add2(46,88) + add2(131,88);
        if (r != 530) failures++;
    }


    {
        volatile int16_t a = -21365;
        volatile int16_t b = -11224;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)((44 + (134 - 221)) & (48 ^ (211 | 248)))) != 193) failures++;
    }


    {
        uint8_t buf[8] = {228,213,0,19,191,236,62,40};
        uint8_t *p = buf;
        p += 2;
        if (*p != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {37,1,45959,201};
        if (s.a != (uint8_t)37) failures++;
    }


    {
        uint8_t a[6] = {182,14,130,26,18,27};
        if (a[3] != 26) failures++;
    }


    {
        uint8_t src[4] = {241,240,90,185};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 185) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 175;
        if (buf[4] != 175) failures++;
    }


    {
        uint8_t v = 137;
        v &= ~(uint8_t)1;
        if (v != 136) failures++;
    }


    {
        uint8_t src[16] = {4,112,59,84,227,28,134,78,125,210,154,125,251,84,197,17};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[7] != 78) failures++;
    }


    {
        uint16_t r = call6(153,110,55,10,143,125);
        if (r != 596) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 16: result = 229; break;
        case 14: result = 3; break;
        case 4: result = 137; break;
        case 5: result = 62; break;
        case 10: result = 117; break;
        default: result = 32; break;
        }
        if (result != 3) failures++;
    }


    {
        volatile uint8_t port = 191;
        uint8_t r = port;
        if (r != 191) failures++;
    }


    {
        volatile uint8_t port = 64;
        uint8_t r = port;
        if (r != 64) failures++;
    }


    {
        uint8_t src[14] = {31,252,119,243,115,143,95,185,74,92,11,82,144,239};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[13] != 239) failures++;
    }


    {
        g16 = 59036;
        if (read_g16() != 59036) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 8: result = 243; break;
        case 6: result = 125; break;
        case 1: result = 223; break;
        case 16: result = 38; break;
        case 5: result = 181; break;
        case 10: result = 11; break;
        default: result = 153; break;
        }
        if (result != 243) failures++;
    }


    {
        uint32_t a = 2976933453UL;
        uint32_t b = 1919213810UL;
        uint32_t r = a | b;
        if (r != 4084524799UL) failures++;
    }


    {
        uint16_t x = 174;
        x = x + 98;
        if (x != 272) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)243) + (uint16_t)9099;
        if (r != 9342) failures++;
    }


    {
        uint16_t r = call6(175,37,229,175,157,124);
        if (r != 897) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {64,117,44779,35};
        if (s.b != (uint8_t)117) failures++;
    }


    {
        uint16_t x = 112;
        x = x + 153;
        if (x != 265) failures++;
    }


    {
        volatile int16_t a = -12022;
        volatile int16_t b = 4576;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 32;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint8_t src[4] = {115,160,214,86};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 86) failures++;
    }


    {
        uint8_t v = 54;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 29;
        x = x + 30;
        if (x != 59) failures++;
    }


    {
        uint8_t m[4][4] = {{246,138,221,83},{123,97,4,146},{240,9,158,35},{241,228,202,90}};
        if (m[0][0] != 246) failures++;
    }


    {
        if (((uint16_t)(118 + 107)) != 225) failures++;
    }


    {
        uint16_t r = add2(142,112) + add2(112,65) + add2(142,65);
        if (r != 638) failures++;
    }


    {
        uint8_t v = 1;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 31) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint16_t x = 87;
        x = x + 199;
        if (x != 286) failures++;
    }


    {
        uint8_t v = 118;
        v ^= 8;
        if (v != 126) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {235,104,1829,109};
        if (s.d != (uint8_t)109) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 33857;
        if (read_g16() != 33857) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {100,160,48605,113};
        if (s.c != (uint16_t)48605) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 1;
        if (buf[13] != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) / (int16_t)((int8_t)-118);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint16_t r = call6(248,30,123,99,82,49);
        if (r != 631) failures++;
    }


    {
        volatile int16_t a = 17086;
        volatile int16_t b = -7555;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)((202 & (43 ^ 199)) + ((43 & 160) | (180 - 133)))) != 247) failures++;
    }


    {
        volatile int16_t a = -22550;
        volatile int16_t b = -23042;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 83;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 128;
        x <<= 2;
        if (x != 0) failures++;
    }


    {
        uint16_t r = 9616 + 23408 + 60846 + 47858 + 45763 + 15113 + 52417 + 44930;
        if (r != 37807) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 2701;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 704;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(41,142,61,199,203,80);
        if (r != 726) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 17: result = 96; break;
        case 12: result = 221; break;
        case 5: result = 230; break;
        case 18: result = 26; break;
        case 0: result = 185; break;
        default: result = 38; break;
        }
        if (result != 96) failures++;
    }


    {
        int8_t a = 69;
        int8_t b = 74;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][4] = {{99,47,102,139},{188,129,247,145},{247,99,146,11},{136,144,82,102}};
        if (m[0][2] != 102) failures++;
    }


    {
        g16 = 7518;
        if (read_g16() != 7518) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 27479;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {37,131,151,138,191,74,195,87};
        uint8_t *p = buf;
        p += 1;
        if (*p != 131) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {64,206,19328,181};
        if (s.a != (uint8_t)64) failures++;
    }


    {
        uint8_t v = 191;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)160) + (uint16_t)3346;
        if (r != 3506) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)142) + (uint16_t)61499;
        if (r != 61641) failures++;
    }


    {
        uint8_t x = 237;
        x <<= 3;
        if (x != 104) failures++;
    }


    {
        uint8_t m[3][3] = {{152,57,141},{185,104,222},{67,151,79}};
        if (m[1][2] != 222) failures++;
    }


    {
        volatile uint8_t port = 20;
        uint8_t r = port;
        if (r != 20) failures++;
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
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t v = 147;
        v |= 128;
        if (v != 147) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 105;
        if (buf[3] != 105) failures++;
    }


    {
        if (((uint16_t)135) != 135) failures++;
    }


    {
        uint8_t a[6] = {84,184,181,200,233,248};
        if (a[1] != 184) failures++;
    }


    {
        uint32_t a = 2349088204UL;
        uint32_t b = 3965586808UL;
        uint32_t r = a + b;
        if (r != 2019707716UL) failures++;
    }


    {
        uint8_t buf[8] = {195,74,18,27,113,4,50,32};
        uint8_t *p = buf;
        p += 5;
        if (*p != 4) failures++;
    }


    {
        uint8_t buf[8] = {103,106,165,7,153,126,126,231};
        uint8_t *p = buf;
        p += 6;
        if (*p != 126) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 230;
        if (buf[2] != 230) failures++;
    }


    {
        if (((uint16_t)(246 ^ 198)) != 48) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(118,167) != 285) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 208;
        if (buf[5] != 208) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 6: result = 190; break;
        case 10: result = 184; break;
        case 16: result = 91; break;
        case 17: result = 228; break;
        case 1: result = 26; break;
        case 13: result = 229; break;
        default: result = 0; break;
        }
        if (result != 26) failures++;
    }


    {
        uint16_t r = 59535 + 44571 + 48924 + 28799 + 51672 + 9723 + 65312 + 35764;
        if (r != 16620) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {105,184,62514,16};
        if (s.a != (uint8_t)105) failures++;
    }


    {
        uint32_t a = 972895345UL;
        uint32_t b = 2660595680UL;
        uint32_t r = a + b;
        if (r != 3633491025UL) failures++;
    }


    {
        uint16_t r = call6(164,66,226,13,132,46);
        if (r != 647) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,122,43620,148};
        if (s.d != (uint8_t)148) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)235) + (uint16_t)25217;
        if (r != 25452) failures++;
    }


    {
        uint8_t x = 14;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(194,202) != 396) failures++;
    }


    {
        int8_t a = -44;
        int8_t b = -71;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 46825 + 31502 + 46503 + 49801 + 14214 + 55517 + 57627 + 34039;
        if (r != 8348) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 190;
        if (buf[11] != 190) failures++;
    }


    {
        uint8_t a[6] = {142,179,192,166,148,73};
        if (a[2] != 192) failures++;
    }


    {
        uint16_t r = 31237 + 41927 + 36933 + 21463 + 19901 + 35369 + 58402 + 49978;
        if (r != 33066) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {65,90,26125,244};
        if (s.b != (uint8_t)90) failures++;
    }


    {
        uint8_t buf[8] = {75,120,79,72,249,192,51,1};
        uint8_t *p = buf;
        p += 6;
        if (*p != 51) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {95,154,44123,246};
        if (s.a != (uint8_t)95) failures++;
    }


    {
        uint8_t x = 85;
        x <<= 1;
        if (x != 170) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)44) + (uint16_t)13821;
        if (r != 13865) failures++;
    }


    {
        uint16_t x = 26018;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 47285;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {233,68,55,176,129,76,106,199};
        uint8_t *p = buf;
        p += 6;
        if (*p != 106) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(79 ^ 231)) != 168) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)227) + (uint16_t)57661;
        if (r != 57888) failures++;
    }


    {
        g16 = 27539;
        if (read_g16() != 27539) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)46) / (int16_t)((int8_t)-85);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        volatile uint8_t port = 153;
        uint8_t r = port;
        if (r != 153) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        volatile uint8_t port = 147;
        uint8_t r = port;
        if (r != 147) failures++;
    }


    {
        volatile uint8_t port = 145;
        uint8_t r = port;
        if (r != 145) failures++;
    }


    {
        volatile uint8_t port = 75;
        uint8_t r = port;
        if (r != 75) failures++;
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
        if (fn(44,120) != 164) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 10: result = 254; break;
        case 18: result = 8; break;
        case 11: result = 211; break;
        case 14: result = 226; break;
        case 2: result = 105; break;
        case 19: result = 85; break;
        default: result = 190; break;
        }
        if (result != 105) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(106,222) != 328) failures++;
    }


    {
        uint16_t x = 154;
        x = x + 167;
        if (x != 321) failures++;
    }


    {
        uint8_t a[6] = {69,61,27,134,243,240};
        if (a[1] != 61) failures++;
    }


    {
        g16 = 20522;
        if (read_g16() != 20522) failures++;
    }


    {
        uint8_t src[1] = {248};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 248) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {126,127,21365,10};
        if (s.d != (uint8_t)10) failures++;
    }


    {
        uint8_t m[4][3] = {{51,107,25},{171,240,86},{153,46,190},{154,166,97}};
        if (m[0][2] != 25) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 20;
        if (buf[5] != 20) failures++;
    }


    {
        g16 = 13851;
        if (read_g16() != 13851) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 58;
        if (buf[0] != 58) failures++;
    }


    {
        uint8_t v = 164;
        v |= 2;
        if (v != 166) failures++;
    }


    {
        g16 = 61199;
        if (read_g16() != 61199) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 16: result = 17; break;
        case 8: result = 58; break;
        case 11: result = 22; break;
        case 2: result = 168; break;
        case 7: result = 9; break;
        case 1: result = 124; break;
        case 10: result = 142; break;
        case 3: result = 236; break;
        default: result = 165; break;
        }
        if (result != 124) failures++;
    }


    {
        uint8_t v = 158;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 206;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t m[2][3] = {{127,225,8},{116,207,232}};
        if (m[1][0] != 116) failures++;
    }


    {
        uint8_t v = 226;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 3826;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 223;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 9) failures++;
    }


    {
        uint16_t x = 35647;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 17: result = 23; break;
        case 16: result = 30; break;
        case 13: result = 102; break;
        case 1: result = 22; break;
        case 18: result = 225; break;
        case 5: result = 131; break;
        case 2: result = 24; break;
        default: result = 243; break;
        }
        if (result != 30) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {144,244,22604,67};
        if (s.c != (uint16_t)22604) failures++;
    }


    {
        uint16_t x = 17;
        x = x + 135;
        if (x != 152) failures++;
    }


    {
        if (((uint16_t)125) != 125) failures++;
    }


    {
        if (((uint16_t)(((188 | 181) + (29 & 211)) + ((181 ^ 252) | (198 + 11)))) != 423) failures++;
    }


    {
        uint8_t a[6] = {221,252,157,199,23,22};
        if (a[1] != 252) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {107,147,48676,57};
        if (s.b != (uint8_t)147) failures++;
    }


    {
        uint8_t v = 102;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t buf[8] = {188,196,239,49,191,210,7,53};
        uint8_t *p = buf;
        p += 5;
        if (*p != 210) failures++;
    }


    {
        volatile int16_t a = 5391;
        volatile int16_t b = -19675;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-102) % (int16_t)((int8_t)-98);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {137,117,42194,21};
        if (s.b != (uint8_t)117) failures++;
    }


    {
        uint8_t v = 244;
        v |= 64;
        if (v != 244) failures++;
    }


    {
        uint8_t x = 171;
        x <<= 0;
        if (x != 171) failures++;
    }


    {
        uint16_t r = add2(12,72) + add2(72,10) + add2(12,10);
        if (r != 188) failures++;
    }


    {
        if (((uint16_t)(((63 & 199) | (48 - 48)) - 87)) != 65456) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(122,93) != 215) failures++;
    }


    {
        uint16_t r = call6(68,204,130,185,211,218);
        if (r != 1016) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {216,214,19694,90};
        if (s.a != (uint8_t)216) failures++;
    }


    {
        uint16_t x = 42072;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 17;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 7) failures++;
    }


    {
        uint8_t src[4] = {65,214,255,185};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 185) failures++;
    }


    {
        uint16_t x = 57;
        x = x + 74;
        if (x != 131) failures++;
    }


    {
        uint8_t buf[8] = {241,156,98,51,43,207,141,120};
        uint8_t *p = buf;
        p += 2;
        if (*p != 98) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)19) % (int16_t)((int8_t)45);
        if ((uint16_t)r != (uint16_t)19) failures++;
    }


    {
        int8_t a = -113;
        int8_t b = -32;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(4,248,144,132,175,240);
        if (r != 943) failures++;
    }


    {
        uint8_t buf[8] = {219,115,60,210,160,5,119,85};
        uint8_t *p = buf;
        p += 2;
        if (*p != 60) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)96) + (uint16_t)53805;
        if (r != 53901) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-104) / (int16_t)((int8_t)46);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint8_t m[3][4] = {{100,217,128,156},{146,230,97,127},{233,25,214,92}};
        if (m[0][2] != 128) failures++;
    }


    {
        uint16_t r = add2(158,61) + add2(61,81) + add2(158,81);
        if (r != 600) failures++;
    }

    return failures;
}