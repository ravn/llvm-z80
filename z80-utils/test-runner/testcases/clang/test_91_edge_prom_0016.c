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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {57,255,59517,206};
        if (s.c != (uint16_t)59517) failures++;
    }


    {
        g16 = 42413;
        if (read_g16() != 42413) failures++;
    }


    {
        int8_t a = 0;
        int8_t b = 51;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)((29 - (106 & 137)) | (103 ^ (245 & 51)))) != 87) failures++;
    }


    {
        uint16_t r = 14992 + 50300 + 64850 + 1869 + 7094 + 22090 + 39812 + 22443;
        if (r != 26842) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(188,161) != 27) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        int8_t a = 59;
        int8_t b = 48;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 227;
        x = x + 17;
        if (x != 244) failures++;
    }


    {
        uint16_t r = add2(193,157) + add2(157,185) + add2(193,185);
        if (r != 1070) failures++;
    }


    {
        uint16_t x = 51064;
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
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 79;
        if (buf[5] != 79) failures++;
    }


    {
        uint8_t m[3][2] = {{0,209},{44,200},{191,139}};
        if (m[0][0] != 0) failures++;
    }


    {
        uint8_t x = 161;
        x <<= 0;
        if (x != 161) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 3: result = 128; break;
        case 16: result = 60; break;
        case 2: result = 175; break;
        case 11: result = 209; break;
        case 18: result = 104; break;
        case 5: result = 14; break;
        default: result = 224; break;
        }
        if (result != 209) failures++;
    }


    {
        uint16_t x = 63593;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 61;
        x = x + 246;
        if (x != 307) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 174;
        if (buf[7] != 174) failures++;
    }


    {
        uint8_t v = 248;
        v &= ~(uint8_t)1;
        if (v != 248) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 41;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(6,216) != 65326) failures++;
    }


    {
        uint8_t x = 244;
        x <<= 0;
        if (x != 244) failures++;
    }


    {
        int8_t a = -41;
        int8_t b = 72;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 17674 + 51244 + 13862 + 9398 + 40876 + 18506 + 40305 + 51398;
        if (r != 46655) failures++;
    }


    {
        uint8_t m[2][3] = {{183,206,128},{247,53,222}};
        if (m[1][1] != 53) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint8_t v = 126;
        v |= 64;
        if (v != 126) failures++;
    }


    {
        uint32_t a = 897642038UL;
        uint32_t b = 1427167084UL;
        uint32_t r = a - b;
        if (r != 3765442250UL) failures++;
    }


    {
        if (((uint16_t)75) != 75) failures++;
    }


    {
        int8_t a = -70;
        int8_t b = 19;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {187,18,1673,102};
        if (s.d != (uint8_t)102) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(102,35) != 67) failures++;
    }


    {
        volatile uint8_t port = 198;
        uint8_t r = port;
        if (r != 198) failures++;
    }


    {
        volatile uint8_t port = 134;
        uint8_t r = port;
        if (r != 134) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(58,169) != 227) failures++;
    }


    {
        uint16_t r = call6(39,66,230,161,156,55);
        if (r != 707) failures++;
    }


    {
        uint8_t a[6] = {211,79,125,14,72,194};
        if (a[2] != 125) failures++;
    }


    {
        uint8_t v = 248;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(107,187) != 294) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(148,89) != 59) failures++;
    }


    {
        uint16_t x = 86;
        x = x + 69;
        if (x != 155) failures++;
    }


    {
        uint8_t m[3][2] = {{191,140},{182,248},{25,161}};
        if (m[1][0] != 182) failures++;
    }


    {
        uint16_t r = 17642 + 37448 + 9387 + 33400 + 23034 + 38160 + 28760 + 53795;
        if (r != 45018) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)29) / (int16_t)((int8_t)124);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {52,180,197,35,172,112,96,85};
        uint8_t *p = buf;
        p += 6;
        if (*p != 96) failures++;
    }


    {
        uint8_t a[6] = {165,163,247,223,222,3};
        if (a[0] != 165) failures++;
    }


    {
        uint8_t v = 156;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t v = 246;
        v ^= 128;
        if (v != 118) failures++;
    }


    {
        uint16_t x = 9;
        x = x + 48;
        if (x != 57) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 60;
        if (buf[8] != 60) failures++;
    }


    {
        if (((uint16_t)159) != 159) failures++;
    }


    {
        uint8_t v = 198;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(98,173,48,24,107,78);
        if (r != 528) failures++;
    }


    {
        uint8_t src[15] = {158,231,60,221,121,239,65,91,201,168,153,62,57,99,94};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[6] != 65) failures++;
    }


    {
        uint8_t v = 153;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t x = 73;
        x <<= 4;
        if (x != 144) failures++;
    }


    {
        if (((uint16_t)(((21 | 34) + (251 ^ 59)) + ((139 + 152) - (163 & 245)))) != 377) failures++;
    }


    {
        if (((uint16_t)(195 ^ ((160 ^ 56) | (152 | 103)))) != 60) failures++;
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
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        volatile int16_t a = 17687;
        volatile int16_t b = -27688;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 75;
        uint8_t r = port;
        if (r != 75) failures++;
    }


    {
        uint8_t v = 123;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = call6(108,211,238,248,181,40);
        if (r != 1026) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)116) + (uint16_t)53448;
        if (r != 53564) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)244) + (uint16_t)39397;
        if (r != 39641) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 85;
        if (buf[12] != 85) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 8: result = 42; break;
        case 14: result = 188; break;
        case 2: result = 201; break;
        default: result = 86; break;
        }
        if (result != 201) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)81) / (int16_t)((int8_t)10);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 1: result = 98; break;
        case 11: result = 83; break;
        case 12: result = 183; break;
        case 10: result = 97; break;
        case 2: result = 80; break;
        default: result = 43; break;
        }
        if (result != 183) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)180) + (uint16_t)26929;
        if (r != 27109) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint8_t x = 137;
        x <<= 3;
        if (x != 72) failures++;
    }


    {
        uint16_t x = 60;
        x = x + 148;
        if (x != 208) failures++;
    }


    {
        uint8_t buf[8] = {151,142,235,222,226,10,155,244};
        uint8_t *p = buf;
        p += 6;
        if (*p != 155) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 9;
        uint8_t r = port;
        if (r != 9) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)188) + (uint16_t)38471;
        if (r != 38659) failures++;
    }


    {
        volatile int16_t a = -16715;
        volatile int16_t b = -28634;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][2] = {{229,10},{244,69},{131,101},{193,38}};
        if (m[1][0] != 244) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)19) / (int16_t)((int8_t)-66);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t x = 92;
        x <<= 1;
        if (x != 184) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(243,19,172,203,59,117);
        if (r != 813) failures++;
    }


    {
        uint16_t x = 58761;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 14: result = 13; break;
        case 13: result = 254; break;
        case 16: result = 91; break;
        case 19: result = 88; break;
        case 12: result = 182; break;
        default: result = 225; break;
        }
        if (result != 182) failures++;
    }


    {
        uint16_t r = 17183 + 32951 + 12673 + 49096 + 10697 + 60435 + 41773 + 60427;
        if (r != 23091) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {90,227,9845,0};
        if (s.b != (uint8_t)227) failures++;
    }


    {
        g16 = 29409;
        if (read_g16() != 29409) failures++;
    }


    {
        uint8_t a[6] = {68,81,173,110,75,47};
        if (a[0] != 68) failures++;
    }


    {
        uint8_t m[4][4] = {{106,182,247,9},{235,3,2,48},{42,254,110,120},{236,252,19,220}};
        if (m[0][0] != 106) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)22) + (uint16_t)2585;
        if (r != 2607) failures++;
    }


    {
        uint8_t buf[8] = {96,195,209,194,74,32,149,122};
        uint8_t *p = buf;
        p += 3;
        if (*p != 194) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)23) + (uint16_t)37840;
        if (r != 37863) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 36950;
        if (read_g16() != 36950) failures++;
    }


    {
        if (((uint16_t)(((15 & 15) | 39) | (109 - (52 ^ 8)))) != 63) failures++;
    }


    {
        uint16_t r = call6(17,26,136,222,55,215);
        if (r != 671) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t buf[8] = {6,122,16,224,254,81,150,206};
        uint8_t *p = buf;
        p += 0;
        if (*p != 6) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 1304;
        if (read_g16() != 1304) failures++;
    }


    {
        g16 = 32446;
        if (read_g16() != 32446) failures++;
    }


    {
        int8_t a = -53;
        int8_t b = -42;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 62065;
        if (read_g16() != 62065) failures++;
    }


    {
        volatile int16_t a = -32540;
        volatile int16_t b = 16172;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 2: result = 32; break;
        case 18: result = 57; break;
        case 17: result = 163; break;
        case 8: result = 42; break;
        case 7: result = 218; break;
        case 6: result = 232; break;
        case 15: result = 155; break;
        default: result = 115; break;
        }
        if (result != 232) failures++;
    }


    {
        if (((uint16_t)(157 | 60)) != 189) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)8) / (int16_t)((int8_t)24);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t src[11] = {60,120,46,100,71,65,65,197,197,50,81};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[4] != 71) failures++;
    }


    {
        uint8_t src[2] = {127,187};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 127) failures++;
    }


    {
        uint8_t a[6] = {227,132,171,130,239,113};
        if (a[3] != 130) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {244,102,31797,20};
        if (s.c != (uint16_t)31797) failures++;
    }


    {
        int8_t a = -21;
        int8_t b = 8;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 203;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t v = 166;
        v ^= 64;
        if (v != 230) failures++;
    }


    {
        if (((uint16_t)49) != 49) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 5: result = 70; break;
        case 13: result = 165; break;
        case 17: result = 223; break;
        default: result = 176; break;
        }
        if (result != 176) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)30) % (int16_t)((int8_t)-18);
        if ((uint16_t)r != (uint16_t)12) failures++;
    }


    {
        uint16_t x = 20;
        x = x + 153;
        if (x != 173) failures++;
    }


    {
        uint16_t x = 46162;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[2][4] = {{59,20,78,172},{61,72,159,232}};
        if (m[1][2] != 159) failures++;
    }


    {
        uint8_t x = 21;
        x <<= 0;
        if (x != 21) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-118) / (int16_t)((int8_t)119);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        int8_t a = 116;
        int8_t b = -29;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 134;
        x <<= 2;
        if (x != 24) failures++;
    }


    {
        uint8_t m[3][2] = {{2,140},{56,170},{236,84}};
        if (m[2][1] != 84) failures++;
    }


    {
        uint16_t x = 57109;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 1137977462UL;
        uint32_t b = 1788879312UL;
        uint32_t r = a ^ b;
        if (r != 695470502UL) failures++;
    }


    {
        uint8_t v = 35;
        v ^= 4;
        if (v != 39) failures++;
    }


    {
        uint16_t x = 120;
        x = x + 175;
        if (x != 295) failures++;
    }


    {
        uint8_t src[6] = {141,105,156,184,118,220};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[2] != 156) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 235;
        if (buf[2] != 235) failures++;
    }


    {
        uint8_t x = 170;
        x <<= 3;
        if (x != 80) failures++;
    }


    {
        uint16_t x = 40;
        x = x + 60;
        if (x != 100) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {18,196,15690,92};
        if (s.c != (uint16_t)15690) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {130,43,16024,203};
        if (s.c != (uint16_t)16024) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)221) + (uint16_t)5289;
        if (r != 5510) failures++;
    }


    {
        volatile uint8_t port = 181;
        uint8_t r = port;
        if (r != 181) failures++;
    }


    {
        uint16_t x = 25134;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(46,114) != 160) failures++;
    }


    {
        uint8_t m[3][2] = {{244,169},{159,161},{83,212}};
        if (m[2][0] != 83) failures++;
    }


    {
        uint8_t buf[8] = {133,72,125,76,77,68,48,75};
        uint8_t *p = buf;
        p += 4;
        if (*p != 77) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -10;
        int8_t b = -69;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 125;
        int8_t b = -107;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2138846353UL;
        uint32_t b = 987725741UL;
        uint32_t r = a - b;
        if (r != 1151120612UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {173,202,92,40};
        if (s.b != (uint8_t)202) failures++;
    }


    {
        uint32_t a = 2695103452UL;
        uint32_t b = 3626145908UL;
        uint32_t r = a + b;
        if (r != 2026282064UL) failures++;
    }


    {
        int8_t a = 22;
        int8_t b = -98;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 84;
        x = x + 208;
        if (x != 292) failures++;
    }


    {
        uint8_t src[13] = {222,71,116,39,35,17,66,30,36,69,230,8,63};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[0] != 222) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(75,88) != 163) failures++;
    }


    {
        uint16_t r = call6(12,1,207,158,253,11);
        if (r != 642) failures++;
    }


    {
        uint8_t a[6] = {252,198,99,220,127,156};
        if (a[0] != 252) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 15: result = 141; break;
        case 1: result = 152; break;
        case 6: result = 27; break;
        case 5: result = 25; break;
        case 13: result = 86; break;
        case 14: result = 49; break;
        case 16: result = 42; break;
        default: result = 190; break;
        }
        if (result != 190) failures++;
    }


    {
        volatile uint8_t port = 241;
        uint8_t r = port;
        if (r != 241) failures++;
    }


    {
        volatile uint8_t port = 232;
        uint8_t r = port;
        if (r != 232) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {149,178,19790,143};
        if (s.a != (uint8_t)149) failures++;
    }


    {
        uint8_t m[4][2] = {{131,18},{120,58},{71,71},{35,115}};
        if (m[1][1] != 58) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(129,248) != 377) failures++;
    }


    {
        uint16_t r = 19445 + 26831 + 20021 + 42137 + 40981 + 62169 + 33362 + 35726;
        if (r != 18528) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-28) % (int16_t)((int8_t)15);
        if ((uint16_t)r != (uint16_t)65523) failures++;
    }


    {
        volatile int16_t a = 1403;
        volatile int16_t b = 3229;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)200) + (uint16_t)28351;
        if (r != 28551) failures++;
    }


    {
        uint16_t x = 36;
        x = x + 38;
        if (x != 74) failures++;
    }


    {
        uint8_t v = 121;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 7157;
        if (read_g16() != 7157) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        g16 = 63447;
        if (read_g16() != 63447) failures++;
    }


    {
        uint8_t v = 216;
        v |= 2;
        if (v != 218) failures++;
    }


    {
        volatile int16_t a = 13668;
        volatile int16_t b = -5294;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 111;
        x = x + 46;
        if (x != 157) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 125;
        if (buf[1] != 125) failures++;
    }


    {
        uint16_t r = 33257 + 51940 + 45966 + 46548 + 64529 + 19720 + 25711 + 39163;
        if (r != 64690) failures++;
    }


    {
        uint8_t x = 143;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t r = call6(9,218,235,107,62,93);
        if (r != 724) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)242) + (uint16_t)59223;
        if (r != 59465) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-88) / (int16_t)((int8_t)58);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 18;
        if (buf[8] != 18) failures++;
    }


    {
        uint32_t a = 3765005457UL;
        uint32_t b = 2950217190UL;
        uint32_t r = a + b;
        if (r != 2420255351UL) failures++;
    }


    {
        uint16_t x = 45369;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(67,168) + add2(168,221) + add2(67,221);
        if (r != 912) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t a[6] = {17,189,205,242,5,138};
        if (a[2] != 205) failures++;
    }


    {
        uint32_t a = 3232768344UL;
        uint32_t b = 2010934350UL;
        uint32_t r = a & b;
        if (r != 1083187272UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {1,221,48589,244};
        if (s.b != (uint8_t)221) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {85,7,55275,4};
        if (s.c != (uint16_t)55275) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)202) + (uint16_t)62550;
        if (r != 62752) failures++;
    }


    {
        int8_t a = 91;
        int8_t b = -40;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)232) + (uint16_t)8284;
        if (r != 8516) failures++;
    }


    {
        volatile int16_t a = -2225;
        volatile int16_t b = -10922;
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
        uint16_t x = 197;
        x = x + 139;
        if (x != 336) failures++;
    }


    {
        uint8_t a[6] = {77,27,165,112,113,217};
        if (a[1] != 27) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {43,11,61,94,249,89,29,206};
        uint8_t *p = buf;
        p += 6;
        if (*p != 29) failures++;
    }


    {
        if (((uint16_t)(((44 | 51) ^ 190) | 196)) != 197) failures++;
    }


    {
        g16 = 63938;
        if (read_g16() != 63938) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        if (((uint16_t)(((12 & 128) & (103 + 50)) & 212)) != 0) failures++;
    }


    {
        int8_t a = -49;
        int8_t b = 30;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 14;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {148,129,153,36,145,111};
        if (a[4] != 145) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {0,157,100,226,12,161,118,191};
        uint8_t *p = buf;
        p += 3;
        if (*p != 226) failures++;
    }


    {
        uint16_t r = call6(210,16,251,72,84,58);
        if (r != 691) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(10,181) != 65365) failures++;
    }


    {
        volatile uint8_t port = 238;
        uint8_t r = port;
        if (r != 238) failures++;
    }


    {
        uint8_t src[6] = {145,116,142,215,118,123};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[0] != 145) failures++;
    }


    {
        uint16_t r = add2(15,49) + add2(49,186) + add2(15,186);
        if (r != 500) failures++;
    }


    {
        uint8_t src[5] = {7,29,229,5,14};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 229) failures++;
    }


    {
        volatile uint8_t port = 202;
        uint8_t r = port;
        if (r != 202) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-101) / (int16_t)((int8_t)-82);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        g16 = 44386;
        if (read_g16() != 44386) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)84) / (int16_t)((int8_t)37);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(143,55) != 198) failures++;
    }


    {
        uint8_t m[4][4] = {{58,212,70,36},{204,129,144,144},{44,108,171,9},{209,209,180,29}};
        if (m[3][0] != 209) failures++;
    }


    {
        uint8_t m[3][4] = {{18,208,189,154},{199,31,104,91},{49,19,204,213}};
        if (m[0][2] != 189) failures++;
    }


    {
        g16 = 60259;
        if (read_g16() != 60259) failures++;
    }


    {
        volatile int16_t a = -26301;
        volatile int16_t b = 13426;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 212;
        uint8_t r = port;
        if (r != 212) failures++;
    }


    {
        uint16_t x = 32676;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 5;
        int8_t b = 101;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(18 | ((183 ^ 13) + (36 - 140)))) != 82) failures++;
    }


    {
        uint8_t buf[8] = {76,112,18,1,71,86,90,159};
        uint8_t *p = buf;
        p += 7;
        if (*p != 159) failures++;
    }


    {
        uint16_t x = 19383;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(210,183) + add2(183,152) + add2(210,152);
        if (r != 1090) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint8_t v = 61;
        v ^= 32;
        if (v != 29) failures++;
    }


    {
        uint8_t v = 189;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)(((44 - 59) & (86 & 6)) & ((27 - 163) | (197 ^ 126)))) != 0) failures++;
    }


    {
        if (((uint16_t)(((112 | 143) + (85 - 237)) - ((145 + 96) & (12 & 198)))) != 103) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t a[6] = {86,197,32,191,80,159};
        if (a[0] != 86) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-97) % (int16_t)((int8_t)-127);
        if ((uint16_t)r != (uint16_t)65439) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 4: result = 182; break;
        case 9: result = 188; break;
        case 19: result = 197; break;
        case 6: result = 236; break;
        case 0: result = 156; break;
        default: result = 119; break;
        }
        if (result != 182) failures++;
    }


    {
        uint16_t r = 42179 + 18596 + 23274 + 18104 + 23926 + 7495 + 7610 + 19338;
        if (r != 29450) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)67) + (uint16_t)22166;
        if (r != 22233) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint32_t a = 2379408369UL;
        uint32_t b = 2874759421UL;
        uint32_t r = a - b;
        if (r != 3799616244UL) failures++;
    }


    {
        uint8_t m[4][2] = {{22,159},{87,153},{222,99},{146,140}};
        if (m[0][0] != 22) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)6) + (uint16_t)18123;
        if (r != 18129) failures++;
    }


    {
        uint8_t buf[8] = {133,27,200,157,218,198,78,36};
        uint8_t *p = buf;
        p += 4;
        if (*p != 218) failures++;
    }


    {
        uint8_t x = 120;
        x <<= 0;
        if (x != 120) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 35713 + 49559 + 44876 + 49130 + 56414 + 5134 + 37209 + 44036;
        if (r != 59927) failures++;
    }


    {
        int8_t a = 53;
        int8_t b = -32;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 28483;
        if (read_g16() != 28483) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint16_t x = 15;
        x = x + 97;
        if (x != 112) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint8_t m[4][2] = {{219,47},{92,60},{140,39},{47,108}};
        if (m[3][0] != 47) failures++;
    }


    {
        g16 = 64332;
        if (read_g16() != 64332) failures++;
    }


    {
        uint16_t r = call6(76,177,32,45,88,86);
        if (r != 504) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)181) + (uint16_t)29222;
        if (r != 29403) failures++;
    }


    {
        uint8_t v = 80;
        v ^= 1;
        if (v != 81) failures++;
    }


    {
        uint8_t v = 144;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(91,111,169,151,73,235);
        if (r != 830) failures++;
    }


    {
        g16 = 48160;
        if (read_g16() != 48160) failures++;
    }


    {
        uint16_t x = 188;
        x = x + 21;
        if (x != 209) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 9: result = 174; break;
        case 3: result = 147; break;
        case 7: result = 126; break;
        case 1: result = 166; break;
        case 19: result = 224; break;
        case 15: result = 128; break;
        default: result = 161; break;
        }
        if (result != 166) failures++;
    }


    {
        volatile uint8_t port = 83;
        uint8_t r = port;
        if (r != 83) failures++;
    }


    {
        if (((uint16_t)(222 & ((22 & 43) + (227 & 82)))) != 68) failures++;
    }


    {
        uint8_t src[13] = {118,17,115,51,201,206,75,113,205,65,157,145,135};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[8] != 205) failures++;
    }


    {
        uint8_t a[6] = {189,220,22,245,228,70};
        if (a[1] != 220) failures++;
    }


    {
        uint16_t r = add2(244,70) + add2(70,174) + add2(244,174);
        if (r != 976) failures++;
    }


    {
        uint8_t a[6] = {112,211,66,86,216,193};
        if (a[1] != 211) failures++;
    }


    {
        volatile int16_t a = -17985;
        volatile int16_t b = -19234;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -16;
        int8_t b = 67;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 39424 + 51692 + 46001 + 25988 + 26273 + 46653 + 5096 + 13113;
        if (r != 57632) failures++;
    }


    {
        uint16_t r = call6(93,203,252,206,133,51);
        if (r != 938) failures++;
    }


    {
        uint16_t x = 247;
        x = x + 177;
        if (x != 424) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 127;
        if (buf[6] != 127) failures++;
    }


    {
        uint16_t x = 22762;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 1523711965UL;
        uint32_t b = 2974811065UL;
        uint32_t r = a - b;
        if (r != 2843868196UL) failures++;
    }


    {
        uint8_t buf[8] = {87,34,221,12,219,169,63,97};
        uint8_t *p = buf;
        p += 4;
        if (*p != 219) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-90) / (int16_t)((int8_t)4);
        if ((uint16_t)r != (uint16_t)65514) failures++;
    }


    {
        uint16_t r = add2(77,125) + add2(125,13) + add2(77,13);
        if (r != 430) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 70;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t m[2][2] = {{249,25},{253,127}};
        if (m[1][1] != 127) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(185,143) != 42) failures++;
    }


    {
        uint8_t v = 174;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 51748;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(125,180,117,233,197,190);
        if (r != 1042) failures++;
    }


    {
        uint32_t a = 3144451384UL;
        uint32_t b = 2379067742UL;
        uint32_t r = a - b;
        if (r != 765383642UL) failures++;
    }


    {
        g16 = 27496;
        if (read_g16() != 27496) failures++;
    }


    {
        uint8_t a[6] = {245,142,207,62,69,198};
        if (a[4] != 69) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(161,113) != 48) failures++;
    }


    {
        uint16_t r = add2(4,137) + add2(137,100) + add2(4,100);
        if (r != 482) failures++;
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
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint32_t a = 1510747639UL;
        uint32_t b = 2167533346UL;
        uint32_t r = a - b;
        if (r != 3638181589UL) failures++;
    }


    {
        uint8_t buf[8] = {14,32,214,144,171,41,74,19};
        uint8_t *p = buf;
        p += 7;
        if (*p != 19) failures++;
    }


    {
        g16 = 54126;
        if (read_g16() != 54126) failures++;
    }


    {
        uint16_t x = 240;
        x = x + 234;
        if (x != 474) failures++;
    }


    {
        uint8_t x = 126;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        int8_t a = 114;
        int8_t b = -35;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 10175;
        volatile int16_t b = -28540;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 60;
        int8_t b = -18;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(125,205) != 65456) failures++;
    }


    {
        uint16_t r = 41202 + 27050 + 10584 + 61557 + 31202 + 664 + 21513 + 301;
        if (r != 63001) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 99;
        if (buf[8] != 99) failures++;
    }


    {
        uint16_t x = 4954;
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
        case 1: result = 73; break;
        case 12: result = 13; break;
        case 19: result = 198; break;
        case 15: result = 149; break;
        default: result = 62; break;
        }
        if (result != 62) failures++;
    }


    {
        uint16_t r = 56201 + 16532 + 55334 + 39404 + 7156 + 49447 + 13907 + 59880;
        if (r != 35717) failures++;
    }


    {
        uint8_t m[2][3] = {{151,169,150},{110,10,130}};
        if (m[1][1] != 10) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)69) / (int16_t)((int8_t)-65);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        volatile int16_t a = -18593;
        volatile int16_t b = 24495;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(20,161,236,207,41,116);
        if (r != 781) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)193) + (uint16_t)29926;
        if (r != 30119) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 189;
        if (buf[14] != 189) failures++;
    }


    {
        uint8_t m[2][3] = {{61,85,39},{160,44,216}};
        if (m[1][0] != 160) failures++;
    }


    {
        g16 = 26510;
        if (read_g16() != 26510) failures++;
    }


    {
        uint16_t x = 979;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 131;
        x = x + 188;
        if (x != 319) failures++;
    }


    {
        if (((uint16_t)238) != 238) failures++;
    }


    {
        uint8_t v = 252;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(26,66) != 92) failures++;
    }


    {
        volatile uint8_t port = 105;
        uint8_t r = port;
        if (r != 105) failures++;
    }


    {
        uint16_t x = 58260;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)133) + (uint16_t)12702;
        if (r != 12835) failures++;
    }


    {
        if (((uint16_t)(((67 ^ 181) ^ (30 ^ 26)) + ((118 ^ 176) - 206))) != 234) failures++;
    }


    {
        if (((uint16_t)(160 & ((103 + 106) - 117))) != 0) failures++;
    }


    {
        uint32_t a = 96537454UL;
        uint32_t b = 4025767100UL;
        uint32_t r = a & b;
        if (r != 96469036UL) failures++;
    }


    {
        uint16_t r = 32562 + 55425 + 34929 + 41859 + 60937 + 24086 + 29683 + 20218;
        if (r != 37555) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)100) + (uint16_t)28227;
        if (r != 28327) failures++;
    }


    {
        uint8_t buf[8] = {4,161,63,53,250,183,119,119};
        uint8_t *p = buf;
        p += 4;
        if (*p != 250) failures++;
    }


    {
        uint16_t r = add2(94,218) + add2(218,234) + add2(94,234);
        if (r != 1092) failures++;
    }


    {
        uint32_t a = 2250417804UL;
        uint32_t b = 3436729402UL;
        uint32_t r = a ^ b;
        if (r != 1257960118UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint16_t x = 20528;
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
        uint32_t a = 2291791322UL;
        uint32_t b = 3248564275UL;
        uint32_t r = a | b;
        if (r != 3384409595UL) failures++;
    }


    {
        volatile uint8_t port = 173;
        uint8_t r = port;
        if (r != 173) failures++;
    }


    {
        uint16_t x = 31870;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)9) + (uint16_t)30418;
        if (r != 30427) failures++;
    }


    {
        g16 = 53610;
        if (read_g16() != 53610) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 16: result = 174; break;
        case 5: result = 98; break;
        case 1: result = 214; break;
        default: result = 56; break;
        }
        if (result != 174) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(154,169) != 323) failures++;
    }


    {
        uint8_t v = 44;
        v |= 1;
        if (v != 45) failures++;
    }


    {
        uint8_t a[6] = {227,55,102,111,49,88};
        if (a[3] != 111) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 122;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)40) + (uint16_t)40051;
        if (r != 40091) failures++;
    }


    {
        uint8_t m[4][4] = {{26,221,207,38},{218,250,218,50},{13,142,25,114},{245,105,1,134}};
        if (m[1][3] != 50) failures++;
    }


    {
        uint8_t src[2] = {225,88};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 88) failures++;
    }


    {
        uint8_t m[3][3] = {{84,212,181},{93,217,88},{132,128,107}};
        if (m[0][2] != 181) failures++;
    }


    {
        volatile uint8_t port = 5;
        uint8_t r = port;
        if (r != 5) failures++;
    }


    {
        uint8_t a[6] = {122,213,137,165,177,244};
        if (a[2] != 137) failures++;
    }


    {
        uint8_t v = 128;
        v &= ~(uint8_t)2;
        if (v != 128) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(77,242) != 319) failures++;
    }


    {
        uint32_t a = 3482044759UL;
        uint32_t b = 1096354720UL;
        uint32_t r = a & b;
        if (r != 1091111168UL) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 19: result = 252; break;
        case 15: result = 182; break;
        case 3: result = 227; break;
        case 18: result = 94; break;
        case 0: result = 77; break;
        case 4: result = 212; break;
        default: result = 106; break;
        }
        if (result != 212) failures++;
    }


    {
        uint16_t x = 194;
        x = x + 196;
        if (x != 390) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 210;
        if (buf[4] != 210) failures++;
    }


    {
        uint8_t v = 136;
        int r = (v & 4) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t x = 139;
        x <<= 0;
        if (x != 139) failures++;
    }


    {
        uint8_t m[3][3] = {{244,37,171},{52,84,199},{34,43,148}};
        if (m[1][2] != 199) failures++;
    }


    {
        if (((uint16_t)((199 - (92 | 130)) & (147 | (225 ^ 62)))) != 201) failures++;
    }


    {
        uint8_t x = 18;
        x <<= 5;
        if (x != 64) failures++;
    }


    {
        uint8_t x = 55;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint8_t a[6] = {107,200,131,24,98,10};
        if (a[4] != 98) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 61;
        if (buf[0] != 61) failures++;
    }


    {
        uint8_t v = 12;
        v &= ~(uint8_t)1;
        if (v != 12) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 233;
        uint8_t r = port;
        if (r != 233) failures++;
    }


    {
        uint16_t r = call6(192,133,149,28,111,188);
        if (r != 801) failures++;
    }


    {
        if (((uint16_t)52) != 52) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = 4989 + 18441 + 47464 + 44536 + 47187 + 28097 + 19652 + 4809;
        if (r != 18567) failures++;
    }


    {
        if (((uint16_t)((120 | 192) & ((156 & 90) | (130 ^ 220)))) != 88) failures++;
    }


    {
        uint8_t m[2][4] = {{180,181,162,24},{115,102,131,242}};
        if (m[1][0] != 115) failures++;
    }


    {
        if (((uint16_t)224) != 224) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)226) + (uint16_t)39376;
        if (r != 39602) failures++;
    }


    {
        uint16_t r = call6(218,202,141,73,210,166);
        if (r != 1010) failures++;
    }


    {
        volatile int16_t a = 4729;
        volatile int16_t b = -16906;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t m[2][4] = {{226,218,221,51},{226,138,117,115}};
        if (m[1][3] != 115) failures++;
    }


    {
        uint8_t a[6] = {106,6,57,89,215,193};
        if (a[0] != 106) failures++;
    }


    {
        int8_t a = 109;
        int8_t b = 107;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 18;
        uint8_t r = port;
        if (r != 18) failures++;
    }


    {
        uint8_t v = 210;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 2458206508UL;
        uint32_t b = 1140760323UL;
        uint32_t r = a + b;
        if (r != 3598966831UL) failures++;
    }


    {
        uint16_t r = call6(227,209,156,215,221,174);
        if (r != 1202) failures++;
    }


    {
        uint16_t x = 147;
        x = x + 255;
        if (x != 402) failures++;
    }


    {
        int8_t a = -90;
        int8_t b = 103;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t buf[8] = {203,167,251,11,210,80,2,85};
        uint8_t *p = buf;
        p += 6;
        if (*p != 2) failures++;
    }


    {
        uint8_t src[2] = {79,222};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 79) failures++;
    }


    {
        uint8_t v = 88;
        v ^= 128;
        if (v != 216) failures++;
    }


    {
        volatile int16_t a = -26602;
        volatile int16_t b = 21604;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 1637403593UL;
        uint32_t b = 1666934714UL;
        uint32_t r = a & b;
        if (r != 1628980104UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        g16 = 60379;
        if (read_g16() != 60379) failures++;
    }


    {
        uint16_t r = add2(232,137) + add2(137,153) + add2(232,153);
        if (r != 1044) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 11: result = 114; break;
        case 19: result = 54; break;
        case 1: result = 129; break;
        case 13: result = 6; break;
        case 7: result = 242; break;
        case 0: result = 135; break;
        case 5: result = 218; break;
        case 12: result = 137; break;
        default: result = 212; break;
        }
        if (result != 137) failures++;
    }


    {
        uint8_t buf[8] = {125,210,75,97,5,53,187,148};
        uint8_t *p = buf;
        p += 1;
        if (*p != 210) failures++;
    }


    {
        volatile uint8_t port = 8;
        uint8_t r = port;
        if (r != 8) failures++;
    }


    {
        uint8_t buf[8] = {90,141,188,148,20,48,88,245};
        uint8_t *p = buf;
        p += 7;
        if (*p != 245) failures++;
    }


    {
        volatile uint8_t port = 90;
        uint8_t r = port;
        if (r != 90) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(45,134) != 65447) failures++;
    }


    {
        uint8_t v = 0;
        v ^= 16;
        if (v != 16) failures++;
    }


    {
        g16 = 2991;
        if (read_g16() != 2991) failures++;
    }


    {
        volatile int16_t a = 31288;
        volatile int16_t b = -3493;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {2,99,43625,246};
        if (s.c != (uint16_t)43625) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {43,148,41874,208};
        if (s.c != (uint16_t)41874) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 37967;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(57,182) != 239) failures++;
    }


    {
        uint8_t v = 162;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(188,108,154,0,131,229);
        if (r != 810) failures++;
    }


    {
        uint16_t r = call6(252,75,118,160,247,225);
        if (r != 1077) failures++;
    }


    {
        uint16_t x = 155;
        x = x + 148;
        if (x != 303) failures++;
    }


    {
        uint16_t r = 53977 + 5641 + 22121 + 32031 + 13207 + 39264 + 13337 + 30646;
        if (r != 13616) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        volatile uint8_t port = 2;
        uint8_t r = port;
        if (r != 2) failures++;
    }


    {
        uint16_t x = 46752;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(((239 - 205) & (144 + 183)) ^ ((35 ^ 241) - (46 & 71)))) != 206) failures++;
    }


    {
        uint8_t a[6] = {119,46,148,180,120,27};
        if (a[2] != 148) failures++;
    }


    {
        uint8_t buf[8] = {196,82,143,194,211,137,66,143};
        uint8_t *p = buf;
        p += 0;
        if (*p != 196) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-37) / (int16_t)((int8_t)-4);
        if ((uint16_t)r != (uint16_t)9) failures++;
    }


    {
        uint16_t r = call6(218,11,214,167,143,156);
        if (r != 909) failures++;
    }


    {
        volatile uint8_t port = 251;
        uint8_t r = port;
        if (r != 251) failures++;
    }


    {
        uint8_t x = 71;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 95;
        if (buf[6] != 95) failures++;
    }


    {
        uint8_t buf[8] = {110,181,213,186,224,131,222,205};
        uint8_t *p = buf;
        p += 4;
        if (*p != 224) failures++;
    }


    {
        uint32_t a = 841478193UL;
        uint32_t b = 2935935191UL;
        uint32_t r = a & b;
        if (r != 572968977UL) failures++;
    }


    {
        uint16_t r = 53163 + 19550 + 24279 + 23780 + 37804 + 43858 + 26555 + 74;
        if (r != 32455) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)120) / (int16_t)((int8_t)-88);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(114,210) != 65440) failures++;
    }


    {
        uint16_t x = 217;
        x = x + 28;
        if (x != 245) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(255,201) != 456) failures++;
    }


    {
        uint16_t r = 56799 + 4921 + 53361 + 45770 + 35010 + 32771 + 35529 + 37548;
        if (r != 39565) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(118,73) != 191) failures++;
    }


    {
        uint8_t buf[8] = {217,46,156,244,66,129,195,201};
        uint8_t *p = buf;
        p += 2;
        if (*p != 156) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint16_t r = 28951 + 15791 + 56455 + 5401 + 50541 + 45555 + 53253 + 53311;
        if (r != 47114) failures++;
    }


    {
        uint16_t r = 27570 + 54076 + 5707 + 45965 + 63755 + 19792 + 30303 + 17414;
        if (r != 2438) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 98;
        if (buf[15] != 98) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)107) + (uint16_t)34722;
        if (r != 34829) failures++;
    }


    {
        uint16_t r = 2000 + 31645 + 12013 + 50514 + 14329 + 47980 + 37444 + 32788;
        if (r != 32105) failures++;
    }


    {
        uint8_t src[10] = {3,32,104,173,177,22,179,48,209,237};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[8] != 209) failures++;
    }


    {
        g16 = 30549;
        if (read_g16() != 30549) failures++;
    }


    {
        uint8_t m[3][3] = {{74,167,235},{116,193,42},{61,70,157}};
        if (m[2][0] != 61) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 0;
        if (buf[11] != 0) failures++;
    }


    {
        uint8_t m[2][4] = {{62,30,76,255},{60,238,34,5}};
        if (m[1][3] != 5) failures++;
    }


    {
        int8_t a = -17;
        int8_t b = 20;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)((223 - (210 - 134)) - 107)) != 40) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 249;
        if (buf[3] != 249) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {74,205,50661,99};
        if (s.a != (uint8_t)74) failures++;
    }


    {
        volatile uint8_t port = 228;
        uint8_t r = port;
        if (r != 228) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {230,70,14633,73};
        if (s.c != (uint16_t)14633) failures++;
    }


    {
        uint16_t x = 224;
        x = x + 170;
        if (x != 394) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 47966;
        if (read_g16() != 47966) failures++;
    }


    {
        uint8_t m[4][4] = {{237,91,26,38},{204,160,78,23},{73,193,54,166},{32,183,19,3}};
        if (m[3][2] != 19) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)92) + (uint16_t)52909;
        if (r != 53001) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)204) + (uint16_t)41547;
        if (r != 41751) failures++;
    }


    {
        uint16_t r = call6(154,187,150,36,76,124);
        if (r != 727) failures++;
    }


    {
        uint8_t src[13] = {245,71,64,107,109,16,205,52,135,226,115,127,205};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[2] != 64) failures++;
    }


    {
        uint16_t x = 180;
        x = x + 110;
        if (x != 290) failures++;
    }


    {
        uint8_t m[4][4] = {{48,248,50,147},{37,30,78,152},{42,78,242,22},{243,239,113,239}};
        if (m[2][3] != 22) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 156;
        x = x + 87;
        if (x != 243) failures++;
    }


    {
        uint8_t x = 184;
        x <<= 3;
        if (x != 192) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(231,93) != 324) failures++;
    }


    {
        uint16_t r = 50718 + 43583 + 38550 + 42419 + 35413 + 10497 + 43245 + 41506;
        if (r != 43787) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 9: result = 221; break;
        case 0: result = 255; break;
        case 8: result = 129; break;
        default: result = 50; break;
        }
        if (result != 129) failures++;
    }


    {
        volatile uint8_t port = 176;
        uint8_t r = port;
        if (r != 176) failures++;
    }


    {
        volatile int16_t a = -8029;
        volatile int16_t b = -32296;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 243;
        uint8_t r = port;
        if (r != 243) failures++;
    }


    {
        uint8_t src[7] = {15,245,37,30,180,78,143};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[5] != 78) failures++;
    }


    {
        int8_t a = -116;
        int8_t b = -100;
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {182,75,27413,50};
        if (s.d != (uint8_t)50) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)68) % (int16_t)((int8_t)39);
        if ((uint16_t)r != (uint16_t)29) failures++;
    }


    {
        uint16_t r = 48094 + 33939 + 5389 + 37487 + 7952 + 39712 + 32196 + 64700;
        if (r != 7325) failures++;
    }


    {
        uint8_t v = 232;
        v &= ~(uint8_t)8;
        if (v != 224) failures++;
    }


    {
        uint8_t src[13] = {121,36,105,112,7,200,243,163,224,44,240,119,113};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[2] != 105) failures++;
    }


    {
        uint8_t buf[8] = {250,247,151,211,39,254,185,160};
        uint8_t *p = buf;
        p += 5;
        if (*p != 254) failures++;
    }


    {
        uint8_t src[4] = {24,14,251,38};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 38) failures++;
    }


    {
        uint16_t x = 1322;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-2) / (int16_t)((int8_t)-82);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        g16 = 39119;
        if (read_g16() != 39119) failures++;
    }


    {
        uint32_t a = 2752253870UL;
        uint32_t b = 2818326548UL;
        uint32_t r = a + b;
        if (r != 1275613122UL) failures++;
    }


    {
        uint8_t x = 79;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        int8_t a = -49;
        int8_t b = 123;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {130,93,51,100,7,156};
        if (a[2] != 51) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 248;
        v &= ~(uint8_t)2;
        if (v != 248) failures++;
    }


    {
        uint16_t x = 28344;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)102) + (uint16_t)16392;
        if (r != 16494) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {247,242,44455,168};
        if (s.a != (uint8_t)247) failures++;
    }


    {
        uint8_t v = 67;
        v ^= 4;
        if (v != 71) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(150,194) != 65492) failures++;
    }


    {
        if (((uint16_t)(((59 | 4) | 243) ^ 202)) != 53) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 52;
        if (buf[5] != 52) failures++;
    }


    {
        volatile int16_t a = -31200;
        volatile int16_t b = -871;
        int r = (a <= b);
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
        g16 = 49685;
        if (read_g16() != 49685) failures++;
    }


    {
        uint16_t r = 43086 + 11629 + 30662 + 9495 + 29961 + 53242 + 3021 + 57067;
        if (r != 41555) failures++;
    }


    {
        uint16_t r = call6(122,121,215,210,83,175);
        if (r != 926) failures++;
    }


    {
        if (((uint16_t)(((129 - 176) | (162 | 195)) ^ ((23 & 49) ^ 33))) != 65475) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -1676;
        volatile int16_t b = -11062;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        g16 = 82;
        if (read_g16() != 82) failures++;
    }


    {
        uint8_t x = 206;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 32;
        if (buf[6] != 32) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 89;
        if (buf[2] != 89) failures++;
    }


    {
        uint8_t m[2][2] = {{41,158},{88,124}};
        if (m[0][1] != 158) failures++;
    }


    {
        g16 = 21265;
        if (read_g16() != 21265) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 164;
        if (buf[12] != 164) failures++;
    }


    {
        uint16_t x = 38174;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(13,179) != 65370) failures++;
    }


    {
        uint8_t x = 89;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {8,235,20032,10};
        if (s.a != (uint8_t)8) failures++;
    }


    {
        uint16_t r = call6(224,138,12,21,120,245);
        if (r != 760) failures++;
    }


    {
        uint16_t x = 179;
        x = x + 17;
        if (x != 196) failures++;
    }


    {
        uint8_t x = 112;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        volatile int16_t a = -23408;
        volatile int16_t b = -23944;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 46;
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
        uint16_t r = call6(44,173,153,199,67,103);
        if (r != 739) failures++;
    }


    {
        uint8_t v = 201;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 62068;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(165,97) != 68) failures++;
    }


    {
        uint8_t x = 101;
        x <<= 3;
        if (x != 40) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(231,55) != 176) failures++;
    }


    {
        uint8_t buf[8] = {53,137,77,57,94,181,177,30};
        uint8_t *p = buf;
        p += 5;
        if (*p != 181) failures++;
    }


    {
        uint16_t r = 3848 + 60783 + 42728 + 5704 + 35995 + 65225 + 37637 + 64811;
        if (r != 54587) failures++;
    }


    {
        volatile uint8_t port = 151;
        uint8_t r = port;
        if (r != 151) failures++;
    }


    {
        uint8_t v = 91;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(149,27,245,144,129,118);
        if (r != 812) failures++;
    }


    {
        uint16_t r = add2(51,32) + add2(32,53) + add2(51,53);
        if (r != 272) failures++;
    }


    {
        volatile int16_t a = 12889;
        volatile int16_t b = 20626;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 10091 + 37256 + 16218 + 36649 + 5197 + 64073 + 28131 + 796;
        if (r != 1803) failures++;
    }


    {
        uint16_t r = add2(191,157) + add2(157,164) + add2(191,164);
        if (r != 1024) failures++;
    }


    {
        int8_t a = -92;
        int8_t b = -64;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 45794;
        if (read_g16() != 45794) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t buf[8] = {94,15,188,16,66,216,111,81};
        uint8_t *p = buf;
        p += 5;
        if (*p != 216) failures++;
    }


    {
        volatile uint8_t port = 165;
        uint8_t r = port;
        if (r != 165) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)94) + (uint16_t)57202;
        if (r != 57296) failures++;
    }


    {
        uint8_t src[14] = {188,189,61,118,0,129,17,64,16,63,173,211,236,40};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[6] != 17) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 14: result = 71; break;
        case 3: result = 129; break;
        case 18: result = 134; break;
        case 9: result = 196; break;
        case 19: result = 70; break;
        default: result = 105; break;
        }
        if (result != 70) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-115) % (int16_t)((int8_t)103);
        if ((uint16_t)r != (uint16_t)65524) failures++;
    }


    {
        uint8_t buf[8] = {228,233,64,164,98,226,158,136};
        uint8_t *p = buf;
        p += 5;
        if (*p != 226) failures++;
    }


    {
        int8_t a = 110;
        int8_t b = 90;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 59341 + 56444 + 6616 + 60862 + 44990 + 7654 + 58349 + 44661;
        if (r != 11237) failures++;
    }


    {
        uint16_t r = call6(86,242,110,240,227,177);
        if (r != 1082) failures++;
    }


    {
        uint16_t r = 19653 + 7609 + 13899 + 17799 + 691 + 3737 + 5008 + 940;
        if (r != 3800) failures++;
    }


    {
        uint16_t x = 189;
        x = x + 174;
        if (x != 363) failures++;
    }


    {
        if (((uint16_t)224) != 224) failures++;
    }


    {
        uint8_t buf[8] = {197,94,36,151,4,4,181,51};
        uint8_t *p = buf;
        p += 1;
        if (*p != 94) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(88,253) != 65371) failures++;
    }


    {
        uint8_t a[6] = {154,162,170,156,120,148};
        if (a[4] != 120) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {233,133,25870,115};
        if (s.a != (uint8_t)233) failures++;
    }


    {
        uint16_t x = 204;
        x = x + 28;
        if (x != 232) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {75,53,46976,28};
        if (s.c != (uint16_t)46976) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 57;
        if (buf[7] != 57) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(176,51) != 125) failures++;
    }


    {
        uint16_t r = call6(206,110,163,87,164,213);
        if (r != 943) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint8_t m[4][2] = {{117,238},{69,56},{39,78},{220,207}};
        if (m[2][1] != 78) failures++;
    }


    {
        uint8_t src[3] = {232,72,220};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 220) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)58) / (int16_t)((int8_t)-54);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        g16 = 33067;
        if (read_g16() != 33067) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint32_t a = 3308730077UL;
        uint32_t b = 1448220662UL;
        uint32_t r = a - b;
        if (r != 1860509415UL) failures++;
    }


    {
        uint16_t x = 7120;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint16_t x = 135;
        x = x + 200;
        if (x != 335) failures++;
    }


    {
        uint16_t r = 48485 + 34689 + 13081 + 44955 + 45171 + 63893 + 11139 + 63607;
        if (r != 62876) failures++;
    }


    {
        uint8_t v = 201;
        v ^= 64;
        if (v != 137) failures++;
    }


    {
        volatile uint8_t port = 108;
        uint8_t r = port;
        if (r != 108) failures++;
    }


    {
        uint8_t buf[8] = {106,26,69,29,86,134,81,18};
        uint8_t *p = buf;
        p += 6;
        if (*p != 81) failures++;
    }


    {
        uint8_t a[6] = {200,162,209,162,148,150};
        if (a[3] != 162) failures++;
    }


    {
        uint16_t r = 26678 + 29301 + 50169 + 30448 + 13810 + 11992 + 3030 + 55627;
        if (r != 24447) failures++;
    }


    {
        uint32_t a = 379514565UL;
        uint32_t b = 1821572519UL;
        uint32_t r = a - b;
        if (r != 2852909342UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        uint16_t r = 64956 + 35332 + 5246 + 62365 + 64321 + 32970 + 34632 + 8808;
        if (r != 46486) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint16_t r = call6(40,113,236,20,191,252);
        if (r != 852) failures++;
    }


    {
        uint16_t r = add2(147,41) + add2(41,199) + add2(147,199);
        if (r != 774) failures++;
    }


    {
        uint8_t m[3][2] = {{244,220},{229,63},{33,204}};
        if (m[2][1] != 204) failures++;
    }


    {
        if (((uint16_t)(4 - 191)) != 65349) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-4) % (int16_t)((int8_t)84);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }

    return failures;
}