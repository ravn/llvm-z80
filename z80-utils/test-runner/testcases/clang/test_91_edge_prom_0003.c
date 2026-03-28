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
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(220,37) != 183) failures++;
    }


    {
        uint8_t v = 205;
        v |= 32;
        if (v != 237) failures++;
    }


    {
        volatile uint8_t port = 11;
        uint8_t r = port;
        if (r != 11) failures++;
    }


    {
        uint16_t r = call6(154,114,197,250,179,147);
        if (r != 1041) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-89) % (int16_t)((int8_t)-31);
        if ((uint16_t)r != (uint16_t)65509) failures++;
    }


    {
        if (((uint16_t)105) != 105) failures++;
    }


    {
        uint16_t r = call6(98,81,1,227,61,57);
        if (r != 525) failures++;
    }


    {
        uint8_t x = 150;
        x <<= 4;
        if (x != 96) failures++;
    }


    {
        g16 = 26467;
        if (read_g16() != 26467) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 247;
        if (buf[9] != 247) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint8_t v = 82;
        v |= 4;
        if (v != 86) failures++;
    }


    {
        uint16_t r = 19424 + 64338 + 3148 + 63573 + 44886 + 29350 + 16299 + 43383;
        if (r != 22257) failures++;
    }


    {
        uint8_t m[3][4] = {{89,6,87,195},{135,89,168,230},{94,234,41,140}};
        if (m[1][0] != 135) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 154;
        if (buf[11] != 154) failures++;
    }


    {
        uint8_t x = 63;
        x <<= 4;
        if (x != 240) failures++;
    }


    {
        uint16_t x = 16837;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -91;
        int8_t b = 63;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(252,26) != 226) failures++;
    }


    {
        volatile uint8_t port = 165;
        uint8_t r = port;
        if (r != 165) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(1,98) != 99) failures++;
    }


    {
        volatile uint8_t port = 56;
        uint8_t r = port;
        if (r != 56) failures++;
    }


    {
        uint16_t x = 233;
        x = x + 180;
        if (x != 413) failures++;
    }


    {
        if (((uint16_t)(((87 + 208) & (192 - 23)) + ((1 & 162) & (218 | 59)))) != 33) failures++;
    }


    {
        uint16_t r = call6(142,127,158,61,242,241);
        if (r != 971) failures++;
    }


    {
        g16 = 41635;
        if (read_g16() != 41635) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 242;
        if (buf[5] != 242) failures++;
    }


    {
        uint16_t r = call6(158,173,31,219,18,107);
        if (r != 706) failures++;
    }


    {
        volatile uint8_t port = 31;
        uint8_t r = port;
        if (r != 31) failures++;
    }


    {
        if (((uint16_t)(((77 | 61) + (236 + 225)) ^ (137 - (134 ^ 229)))) != 620) failures++;
    }


    {
        uint16_t r = call6(223,81,54,126,56,111);
        if (r != 651) failures++;
    }


    {
        if (((uint16_t)(((70 + 184) | (149 | 195)) + ((91 - 239) ^ 168))) != 195) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(71,112) != 183) failures++;
    }


    {
        uint8_t src[3] = {185,226,7};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 226) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(185,216) != 65505) failures++;
    }


    {
        uint16_t r = 53698 + 35503 + 40602 + 64576 + 64213 + 18006 + 11963 + 40419;
        if (r != 1300) failures++;
    }


    {
        volatile uint8_t port = 87;
        uint8_t r = port;
        if (r != 87) failures++;
    }


    {
        uint16_t r = call6(157,89,185,145,47,249);
        if (r != 872) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(72,46) != 118) failures++;
    }


    {
        g16 = 31278;
        if (read_g16() != 31278) failures++;
    }


    {
        uint8_t buf[8] = {242,39,6,239,194,43,200,83};
        uint8_t *p = buf;
        p += 6;
        if (*p != 200) failures++;
    }


    {
        uint16_t r = call6(224,28,139,6,183,19);
        if (r != 599) failures++;
    }


    {
        volatile uint8_t port = 133;
        uint8_t r = port;
        if (r != 133) failures++;
    }


    {
        uint8_t v = 125;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(237,103,47,250,149,150);
        if (r != 936) failures++;
    }


    {
        uint8_t v = 189;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 159;
        uint8_t r = port;
        if (r != 159) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 80;
        if (buf[3] != 80) failures++;
    }


    {
        uint8_t x = 6;
        x <<= 4;
        if (x != 96) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 95; break;
        case 19: result = 72; break;
        case 10: result = 121; break;
        case 8: result = 11; break;
        case 2: result = 205; break;
        case 18: result = 135; break;
        case 13: result = 136; break;
        default: result = 210; break;
        }
        if (result != 95) failures++;
    }


    {
        uint16_t r = call6(171,166,63,134,21,201);
        if (r != 756) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)16) + (uint16_t)27128;
        if (r != 27144) failures++;
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
        if (fn(199,163) != 362) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t src[8] = {29,173,87,240,82,189,224,177};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[0] != 29) failures++;
    }


    {
        uint16_t r = call6(71,64,142,173,120,181);
        if (r != 751) failures++;
    }


    {
        volatile int16_t a = -6461;
        volatile int16_t b = 3790;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {29,63,185,104,183,1};
        if (a[4] != 183) failures++;
    }


    {
        if (((uint16_t)(((90 ^ 249) + (231 ^ 112)) | ((218 & 232) & (230 + 133)))) != 378) failures++;
    }


    {
        uint8_t buf[8] = {114,245,169,180,133,11,21,71};
        uint8_t *p = buf;
        p += 7;
        if (*p != 71) failures++;
    }


    {
        uint16_t r = add2(15,27) + add2(27,221) + add2(15,221);
        if (r != 526) failures++;
    }


    {
        if (((uint16_t)65) != 65) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {215,15,33073,245};
        if (s.a != (uint8_t)215) failures++;
    }


    {
        uint16_t r = call6(17,194,167,214,82,81);
        if (r != 755) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {125,163,2245,188};
        if (s.b != (uint8_t)163) failures++;
    }


    {
        uint8_t v = 80;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 124;
        x = x + 94;
        if (x != 218) failures++;
    }


    {
        g16 = 23657;
        if (read_g16() != 23657) failures++;
    }


    {
        uint8_t v = 73;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 248;
        x = x + 50;
        if (x != 298) failures++;
    }


    {
        if (((uint16_t)((172 - (3 & 6)) | ((230 & 106) | 148))) != 254) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        int8_t a = 11;
        int8_t b = -113;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint16_t r = add2(68,255) + add2(255,80) + add2(68,80);
        if (r != 806) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(86,222,138,201,37,79);
        if (r != 763) failures++;
    }


    {
        uint16_t x = 87;
        x = x + 37;
        if (x != 124) failures++;
    }


    {
        int8_t a = 73;
        int8_t b = 61;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 7: result = 50; break;
        case 0: result = 51; break;
        case 6: result = 153; break;
        case 17: result = 28; break;
        case 15: result = 70; break;
        case 14: result = 48; break;
        case 1: result = 6; break;
        case 16: result = 234; break;
        default: result = 192; break;
        }
        if (result != 28) failures++;
    }


    {
        uint8_t v = 236;
        v |= 8;
        if (v != 236) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {235,31,16660,162};
        if (s.c != (uint16_t)16660) failures++;
    }


    {
        uint16_t x = 36226;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 252;
        x = x + 117;
        if (x != 369) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {10,99,23189,14};
        if (s.c != (uint16_t)23189) failures++;
    }


    {
        volatile int16_t a = 27675;
        volatile int16_t b = -21728;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 11495 + 47380 + 24586 + 16026 + 30442 + 41148 + 51274 + 64764;
        if (r != 24971) failures++;
    }


    {
        uint16_t r = add2(4,240) + add2(240,150) + add2(4,150);
        if (r != 788) failures++;
    }


    {
        g16 = 32923;
        if (read_g16() != 32923) failures++;
    }


    {
        uint8_t x = 156;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t v = 63;
        v &= ~(uint8_t)8;
        if (v != 55) failures++;
    }


    {
        uint16_t x = 48;
        x = x + 37;
        if (x != 85) failures++;
    }


    {
        uint8_t v = 238;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(197,74,26,91,65,205);
        if (r != 658) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)215) + (uint16_t)1817;
        if (r != 2032) failures++;
    }


    {
        uint8_t a[6] = {144,234,154,201,56,60};
        if (a[2] != 154) failures++;
    }


    {
        uint8_t a[6] = {37,1,109,181,18,142};
        if (a[2] != 109) failures++;
    }


    {
        uint8_t v = 170;
        v &= ~(uint8_t)8;
        if (v != 162) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {38,220,46665,249};
        if (s.b != (uint8_t)220) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)80) + (uint16_t)57356;
        if (r != 57436) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-80) / (int16_t)((int8_t)62);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t v = 252;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)161) + (uint16_t)63846;
        if (r != 64007) failures++;
    }


    {
        uint16_t r = call6(109,130,154,190,199,108);
        if (r != 890) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 3: result = 122; break;
        case 18: result = 89; break;
        case 6: result = 63; break;
        case 11: result = 155; break;
        default: result = 43; break;
        }
        if (result != 89) failures++;
    }


    {
        uint16_t x = 154;
        x = x + 1;
        if (x != 155) failures++;
    }


    {
        volatile uint8_t port = 166;
        uint8_t r = port;
        if (r != 166) failures++;
    }


    {
        uint8_t m[3][2] = {{17,30},{224,71},{224,129}};
        if (m[2][0] != 224) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-98) / (int16_t)((int8_t)-7);
        if ((uint16_t)r != (uint16_t)14) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        uint8_t v = 59;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
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
        uint8_t v = 160;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint16_t r = 40292 + 156 + 18647 + 45261 + 39122 + 7882 + 8867 + 36992;
        if (r != 611) failures++;
    }


    {
        uint32_t a = 3207420915UL;
        uint32_t b = 1598454054UL;
        uint32_t r = a + b;
        if (r != 510907673UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(100,4) != 96) failures++;
    }


    {
        g16 = 51097;
        if (read_g16() != 51097) failures++;
    }


    {
        uint16_t r = add2(8,169) + add2(169,150) + add2(8,150);
        if (r != 654) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint16_t r = 7497 + 49997 + 36289 + 46573 + 26786 + 57204 + 64443 + 22236;
        if (r != 48881) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(3,45) != 48) failures++;
    }


    {
        volatile uint8_t port = 243;
        uint8_t r = port;
        if (r != 243) failures++;
    }


    {
        uint8_t v = 204;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 98;
        if (buf[13] != 98) failures++;
    }


    {
        uint16_t x = 21349;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {75,158,5010,145};
        if (s.b != (uint8_t)158) failures++;
    }


    {
        uint16_t x = 41;
        x = x + 20;
        if (x != 61) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 107;
        x = x + 162;
        if (x != 269) failures++;
    }


    {
        uint8_t x = 118;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t m[2][3] = {{13,188,177},{231,10,187}};
        if (m[1][2] != 187) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 14: result = 155; break;
        case 18: result = 250; break;
        case 7: result = 41; break;
        case 13: result = 80; break;
        case 8: result = 3; break;
        default: result = 251; break;
        }
        if (result != 41) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t src[15] = {198,207,74,102,221,185,48,126,82,67,249,220,146,254,248};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[1] != 207) failures++;
    }


    {
        uint16_t x = 253;
        x = x + 227;
        if (x != 480) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(82,226) != 65392) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(186,106) != 292) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 43;
        if (buf[13] != 43) failures++;
    }


    {
        uint16_t x = 55457;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t r = add2(186,48) + add2(48,103) + add2(186,103);
        if (r != 674) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)109) / (int16_t)((int8_t)29);
        if ((uint16_t)r != (uint16_t)3) failures++;
    }


    {
        int8_t a = 110;
        int8_t b = 16;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(92,160) != 65468) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 3: result = 206; break;
        case 14: result = 67; break;
        case 1: result = 101; break;
        case 5: result = 254; break;
        default: result = 75; break;
        }
        if (result != 254) failures++;
    }


    {
        uint16_t x = 120;
        x = x + 39;
        if (x != 159) failures++;
    }


    {
        uint8_t a[6] = {31,85,77,2,105,251};
        if (a[3] != 2) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        volatile int16_t a = 3173;
        volatile int16_t b = 2753;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{12,67,94},{89,118,41},{117,217,61},{14,57,18}};
        if (m[0][0] != 12) failures++;
    }


    {
        volatile int16_t a = -13791;
        volatile int16_t b = -31705;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(80,211) != 291) failures++;
    }


    {
        uint16_t r = 9788 + 4116 + 45815 + 56485 + 22296 + 62760 + 38724 + 62700;
        if (r != 40540) failures++;
    }


    {
        if (((uint16_t)(208 + ((125 ^ 158) + 63))) != 498) failures++;
    }


    {
        int8_t a = 3;
        int8_t b = 66;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {147,7,70,93,20,17,225,117};
        uint8_t *p = buf;
        p += 6;
        if (*p != 225) failures++;
    }


    {
        g16 = 48164;
        if (read_g16() != 48164) failures++;
    }


    {
        g16 = 48955;
        if (read_g16() != 48955) failures++;
    }


    {
        uint8_t m[2][3] = {{40,100,20},{252,72,21}};
        if (m[1][2] != 21) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(112,28) != 84) failures++;
    }


    {
        int8_t a = -37;
        int8_t b = 14;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 100;
        x = x + 15;
        if (x != 115) failures++;
    }


    {
        volatile uint8_t port = 207;
        uint8_t r = port;
        if (r != 207) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)83) + (uint16_t)36775;
        if (r != 36858) failures++;
    }


    {
        if (((uint16_t)114) != 114) failures++;
    }


    {
        uint8_t x = 162;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        int8_t a = -22;
        int8_t b = -71;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(140,194) + add2(194,114) + add2(140,114);
        if (r != 896) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(120,1) != 119) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(182,86) != 268) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(242,249) != 491) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)61) + (uint16_t)24454;
        if (r != 24515) failures++;
    }


    {
        uint16_t x = 138;
        x = x + 69;
        if (x != 207) failures++;
    }


    {
        uint8_t x = 62;
        x <<= 1;
        if (x != 124) failures++;
    }


    {
        uint8_t buf[8] = {31,241,187,190,56,245,182,93};
        uint8_t *p = buf;
        p += 0;
        if (*p != 31) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {215,190,21953,152};
        if (s.a != (uint8_t)215) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint16_t r = call6(225,188,229,65,136,12);
        if (r != 855) failures++;
    }


    {
        g16 = 60943;
        if (read_g16() != 60943) failures++;
    }


    {
        uint8_t a[6] = {28,155,176,73,212,11};
        if (a[2] != 176) failures++;
    }


    {
        int8_t a = -88;
        int8_t b = 115;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -91;
        int8_t b = 31;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 109; break;
        case 11: result = 152; break;
        case 10: result = 96; break;
        default: result = 252; break;
        }
        if (result != 252) failures++;
    }


    {
        uint8_t buf[8] = {133,87,11,39,116,198,138,103};
        uint8_t *p = buf;
        p += 0;
        if (*p != 133) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) % (int16_t)((int8_t)-54);
        if ((uint16_t)r != (uint16_t)65522) failures++;
    }


    {
        uint16_t r = add2(124,165) + add2(165,129) + add2(124,129);
        if (r != 836) failures++;
    }


    {
        volatile int16_t a = -23525;
        volatile int16_t b = 15017;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {141,47,249,187,109,211,182,28};
        uint8_t *p = buf;
        p += 3;
        if (*p != 187) failures++;
    }


    {
        uint16_t r = 60844 + 49857 + 39655 + 26107 + 52944 + 50366 + 26819 + 30946;
        if (r != 9858) failures++;
    }


    {
        uint16_t r = 29981 + 22126 + 6174 + 26781 + 35209 + 8972 + 5674 + 45512;
        if (r != 49357) failures++;
    }


    {
        uint8_t v = 129;
        v |= 8;
        if (v != 137) failures++;
    }


    {
        int8_t a = 114;
        int8_t b = 127;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {193,184,116,38,116,31};
        if (a[0] != 193) failures++;
    }


    {
        volatile int16_t a = 16430;
        volatile int16_t b = -3138;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 8: result = 54; break;
        case 0: result = 200; break;
        case 1: result = 84; break;
        default: result = 251; break;
        }
        if (result != 54) failures++;
    }


    {
        if (((uint16_t)((208 & (173 | 31)) + 88)) != 232) failures++;
    }


    {
        if (((uint16_t)(((182 - 117) | (177 - 177)) | ((142 & 181) ^ (195 & 6)))) != 199) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)10) + (uint16_t)19133;
        if (r != 19143) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 18: result = 57; break;
        case 8: result = 171; break;
        case 12: result = 106; break;
        case 17: result = 159; break;
        case 15: result = 85; break;
        default: result = 179; break;
        }
        if (result != 57) failures++;
    }


    {
        uint32_t a = 4058787181UL;
        uint32_t b = 3451143796UL;
        uint32_t r = a - b;
        if (r != 607643385UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 138;
        if (buf[7] != 138) failures++;
    }


    {
        uint16_t r = 44602 + 39235 + 58734 + 2768 + 47844 + 46438 + 580 + 20873;
        if (r != 64466) failures++;
    }


    {
        uint16_t r = add2(3,134) + add2(134,193) + add2(3,193);
        if (r != 660) failures++;
    }


    {
        int8_t a = 38;
        int8_t b = 106;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 58427 + 14683 + 54394 + 41130 + 6647 + 25667 + 61146 + 38353;
        if (r != 38303) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {212,113,26467,13};
        if (s.d != (uint8_t)13) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)152) + (uint16_t)65063;
        if (r != 65215) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {212,46,114,236,15,219};
        if (a[1] != 46) failures++;
    }


    {
        g16 = 9803;
        if (read_g16() != 9803) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint8_t a[6] = {43,0,36,165,40,254};
        if (a[5] != 254) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(135,142) != 277) failures++;
    }


    {
        uint16_t x = 33911;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)199) + (uint16_t)21965;
        if (r != 22164) failures++;
    }


    {
        uint16_t r = add2(153,56) + add2(56,209) + add2(153,209);
        if (r != 836) failures++;
    }


    {
        uint8_t m[3][2] = {{132,129},{48,151},{109,123}};
        if (m[0][0] != 132) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 13: result = 243; break;
        case 5: result = 202; break;
        case 3: result = 55; break;
        case 0: result = 81; break;
        default: result = 168; break;
        }
        if (result != 202) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 10: result = 114; break;
        case 3: result = 44; break;
        case 5: result = 76; break;
        case 7: result = 147; break;
        case 1: result = 124; break;
        default: result = 213; break;
        }
        if (result != 44) failures++;
    }


    {
        uint8_t m[4][4] = {{40,178,243,79},{3,22,139,97},{157,141,176,6},{67,215,110,183}};
        if (m[0][0] != 40) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {43,45,26562,109};
        if (s.d != (uint8_t)109) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-62) / (int16_t)((int8_t)127);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 177;
        x = x + 86;
        if (x != 263) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)195) + (uint16_t)45847;
        if (r != 46042) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(113,168,4,20,211,70);
        if (r != 586) failures++;
    }


    {
        g16 = 28531;
        if (read_g16() != 28531) failures++;
    }


    {
        uint16_t x = 6;
        x = x + 59;
        if (x != 65) failures++;
    }


    {
        uint8_t a[6] = {143,36,89,175,162,93};
        if (a[2] != 89) failures++;
    }


    {
        uint8_t v = 245;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[4][2] = {{141,39},{167,235},{103,86},{53,135}};
        if (m[1][0] != 167) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)156) + (uint16_t)42605;
        if (r != 42761) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(95,133) != 65498) failures++;
    }


    {
        uint8_t v = 195;
        v |= 64;
        if (v != 195) failures++;
    }


    {
        uint16_t r = 54792 + 60450 + 7530 + 20906 + 56359 + 32170 + 53716 + 26457;
        if (r != 50236) failures++;
    }


    {
        g16 = 33228;
        if (read_g16() != 33228) failures++;
    }


    {
        if (((uint16_t)(((239 & 73) + 179) & 160)) != 160) failures++;
    }


    {
        uint8_t buf[8] = {200,209,33,192,210,189,35,229};
        uint8_t *p = buf;
        p += 4;
        if (*p != 210) failures++;
    }


    {
        uint8_t src[9] = {31,13,151,145,158,185,210,105,124};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 151) failures++;
    }


    {
        uint8_t x = 10;
        x <<= 0;
        if (x != 10) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 251;
        if (buf[8] != 251) failures++;
    }


    {
        uint16_t r = call6(252,6,222,127,49,93);
        if (r != 749) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(104,242,239,187,50,123);
        if (r != 945) failures++;
    }


    {
        volatile int16_t a = -27460;
        volatile int16_t b = -6740;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 14435 + 35101 + 28343 + 61203 + 30043 + 20045 + 44370 + 6275;
        if (r != 43207) failures++;
    }


    {
        uint8_t v = 116;
        v &= ~(uint8_t)32;
        if (v != 84) failures++;
    }


    {
        uint16_t r = call6(109,192,238,57,26,196);
        if (r != 818) failures++;
    }


    {
        uint8_t v = 190;
        v ^= 2;
        if (v != 188) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 2;
        if (buf[12] != 2) failures++;
    }


    {
        volatile uint8_t port = 73;
        uint8_t r = port;
        if (r != 73) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(152,42) != 110) failures++;
    }


    {
        uint8_t v = 14;
        v &= ~(uint8_t)32;
        if (v != 14) failures++;
    }


    {
        uint16_t x = 117;
        x = x + 0;
        if (x != 117) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)61) + (uint16_t)49561;
        if (r != 49622) failures++;
    }


    {
        uint8_t m[2][4] = {{143,205,83,184},{64,35,48,229}};
        if (m[0][1] != 205) failures++;
    }


    {
        uint32_t a = 3319396304UL;
        uint32_t b = 3008523561UL;
        uint32_t r = a ^ b;
        if (r != 1988860665UL) failures++;
    }


    {
        uint32_t a = 375773546UL;
        uint32_t b = 3815759343UL;
        uint32_t r = a | b;
        if (r != 4151303663UL) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 15: result = 150; break;
        case 0: result = 38; break;
        case 18: result = 181; break;
        default: result = 113; break;
        }
        if (result != 150) failures++;
    }


    {
        uint16_t x = 240;
        x = x + 208;
        if (x != 448) failures++;
    }


    {
        volatile int16_t a = 24953;
        volatile int16_t b = 4615;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 12: result = 194; break;
        case 4: result = 130; break;
        case 17: result = 96; break;
        case 5: result = 87; break;
        case 9: result = 214; break;
        case 6: result = 82; break;
        default: result = 109; break;
        }
        if (result != 194) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        volatile int16_t a = 16748;
        volatile int16_t b = -9268;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(177,234) + add2(234,168) + add2(177,168);
        if (r != 1158) failures++;
    }


    {
        if (((uint16_t)((55 + 209) ^ (171 - (7 | 121)))) != 292) failures++;
    }


    {
        uint16_t x = 163;
        x = x + 90;
        if (x != 253) failures++;
    }


    {
        uint16_t r = add2(184,44) + add2(44,144) + add2(184,144);
        if (r != 744) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 10: result = 110; break;
        case 11: result = 102; break;
        case 15: result = 169; break;
        case 16: result = 244; break;
        case 14: result = 183; break;
        case 12: result = 238; break;
        default: result = 119; break;
        }
        if (result != 238) failures++;
    }


    {
        uint8_t m[4][2] = {{209,122},{238,184},{3,204},{122,252}};
        if (m[3][1] != 252) failures++;
    }


    {
        uint8_t v = 114;
        v ^= 128;
        if (v != 242) failures++;
    }


    {
        uint8_t m[2][4] = {{83,94,52,87},{102,124,88,26}};
        if (m[1][3] != 26) failures++;
    }


    {
        uint8_t src[6] = {63,17,141,221,162,178};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 178) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 11: result = 132; break;
        case 9: result = 39; break;
        case 7: result = 50; break;
        case 18: result = 26; break;
        case 17: result = 248; break;
        case 0: result = 195; break;
        case 1: result = 196; break;
        default: result = 24; break;
        }
        if (result != 132) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)19) / (int16_t)((int8_t)-121);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 117;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][3] = {{128,222,42},{235,231,80}};
        if (m[1][0] != 235) failures++;
    }


    {
        volatile int16_t a = -20424;
        volatile int16_t b = 25015;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 19656;
        if (read_g16() != 19656) failures++;
    }


    {
        uint8_t x = 235;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        if (((uint16_t)((184 | (247 & 118)) ^ ((131 & 193) ^ (14 | 168)))) != 209) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)125) + (uint16_t)63871;
        if (r != 63996) failures++;
    }


    {
        uint8_t v = 65;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)184) != 184) failures++;
    }


    {
        uint16_t r = 34506 + 61757 + 5101 + 64018 + 41982 + 57399 + 49062 + 8521;
        if (r != 60202) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(124,170) != 65490) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)164) + (uint16_t)40700;
        if (r != 40864) failures++;
    }


    {
        volatile int16_t a = -31588;
        volatile int16_t b = 9683;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        g16 = 6973;
        if (read_g16() != 6973) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 34;
        if (buf[6] != 34) failures++;
    }


    {
        uint16_t x = 210;
        x = x + 181;
        if (x != 391) failures++;
    }


    {
        if (((uint16_t)((187 + (172 ^ 145)) ^ ((189 & 12) ^ 186))) != 78) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)218) + (uint16_t)5598;
        if (r != 5816) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {59,218,159,67,213,236,86,155};
        uint8_t *p = buf;
        p += 1;
        if (*p != 218) failures++;
    }


    {
        volatile int16_t a = 29120;
        volatile int16_t b = -13384;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 25;
        if (buf[9] != 25) failures++;
    }


    {
        uint32_t a = 2804024137UL;
        uint32_t b = 2160559462UL;
        uint32_t r = a & b;
        if (r != 2147615040UL) failures++;
    }


    {
        uint16_t r = call6(151,4,87,23,233,153);
        if (r != 651) failures++;
    }


    {
        uint32_t a = 1719537382UL;
        uint32_t b = 3330154763UL;
        uint32_t r = a - b;
        if (r != 2684349915UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(80,244) != 65372) failures++;
    }


    {
        uint8_t m[4][3] = {{49,42,180},{222,200,16},{134,159,7},{227,191,33}};
        if (m[0][0] != 49) failures++;
    }


    {
        uint8_t buf[8] = {144,26,237,251,196,31,210,34};
        uint8_t *p = buf;
        p += 3;
        if (*p != 251) failures++;
    }


    {
        uint16_t r = add2(225,177) + add2(177,150) + add2(225,150);
        if (r != 1104) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)62) % (int16_t)((int8_t)-41);
        if ((uint16_t)r != (uint16_t)21) failures++;
    }


    {
        volatile uint8_t port = 58;
        uint8_t r = port;
        if (r != 58) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-52) / (int16_t)((int8_t)37);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t v = 61;
        v ^= 8;
        if (v != 53) failures++;
    }


    {
        uint8_t src[3] = {200,239,181};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 239) failures++;
    }


    {
        uint16_t r = 30915 + 22070 + 36074 + 24285 + 202 + 560 + 30059 + 40552;
        if (r != 53645) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 2: result = 49; break;
        case 18: result = 130; break;
        case 8: result = 37; break;
        case 4: result = 40; break;
        case 17: result = 215; break;
        default: result = 147; break;
        }
        if (result != 40) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 22;
        if (buf[11] != 22) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t m[3][4] = {{221,216,22,67},{173,100,3,76},{228,85,132,166}};
        if (m[2][1] != 85) failures++;
    }


    {
        uint8_t v = 236;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(90,141) != 65485) failures++;
    }


    {
        int8_t a = 63;
        int8_t b = 8;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)89) + (uint16_t)24884;
        if (r != 24973) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint8_t a[6] = {30,62,176,182,215,146};
        if (a[5] != 146) failures++;
    }


    {
        uint8_t src[5] = {51,244,182,240,221};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[1] != 244) failures++;
    }


    {
        uint8_t v = 97;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 2: result = 97; break;
        case 12: result = 135; break;
        case 5: result = 208; break;
        case 3: result = 198; break;
        default: result = 177; break;
        }
        if (result != 208) failures++;
    }


    {
        int8_t a = -31;
        int8_t b = -11;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 98;
        uint8_t r = port;
        if (r != 98) failures++;
    }


    {
        uint8_t src[12] = {229,22,184,151,161,209,122,190,206,141,66,7};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[0] != 229) failures++;
    }


    {
        int8_t a = 19;
        int8_t b = -70;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(198,187,183,4,233,75);
        if (r != 880) failures++;
    }


    {
        uint16_t x = 204;
        x = x + 203;
        if (x != 407) failures++;
    }


    {
        uint8_t buf[8] = {162,70,201,121,51,96,56,140};
        uint8_t *p = buf;
        p += 2;
        if (*p != 201) failures++;
    }


    {
        volatile int16_t a = 9171;
        volatile int16_t b = -19663;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[2][2] = {{162,215},{180,209}};
        if (m[1][1] != 209) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        g16 = 64549;
        if (read_g16() != 64549) failures++;
    }


    {
        uint8_t a[6] = {27,42,82,169,100,245};
        if (a[0] != 27) failures++;
    }


    {
        uint8_t buf[8] = {131,43,60,150,211,59,117,63};
        uint8_t *p = buf;
        p += 7;
        if (*p != 63) failures++;
    }


    {
        uint8_t src[1] = {206};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 206) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint8_t x = 72;
        x <<= 0;
        if (x != 72) failures++;
    }


    {
        uint16_t r = 5578 + 19898 + 11303 + 52145 + 691 + 21987 + 1374 + 42574;
        if (r != 24478) failures++;
    }


    {
        uint8_t v = 46;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 18) failures++;
    }


    {
        uint8_t buf[8] = {92,106,168,157,24,96,65,152};
        uint8_t *p = buf;
        p += 1;
        if (*p != 106) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint16_t x = 68;
        x = x + 177;
        if (x != 245) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {10,184,25428,144};
        if (s.d != (uint8_t)144) failures++;
    }


    {
        uint8_t m[2][2] = {{145,86},{122,103}};
        if (m[1][0] != 122) failures++;
    }


    {
        uint8_t v = 33;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(214,96) + add2(96,245) + add2(214,245);
        if (r != 1110) failures++;
    }


    {
        uint8_t src[2] = {192,199};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 192) failures++;
    }


    {
        volatile int16_t a = -18490;
        volatile int16_t b = -31220;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        volatile int16_t a = -21623;
        volatile int16_t b = 8861;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 62;
        v ^= 4;
        if (v != 58) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint16_t x = 58;
        x = x + 118;
        if (x != 176) failures++;
    }


    {
        uint8_t m[2][2] = {{78,23},{71,242}};
        if (m[1][1] != 242) failures++;
    }


    {
        volatile uint8_t port = 97;
        uint8_t r = port;
        if (r != 97) failures++;
    }


    {
        volatile int16_t a = -32335;
        volatile int16_t b = 6658;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 187;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-122) / (int16_t)((int8_t)-109);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {67,216,65321,170};
        if (s.b != (uint8_t)216) failures++;
    }


    {
        uint8_t v = 157;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        volatile int16_t a = 18488;
        volatile int16_t b = -31580;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {134,172,29,111,120,57};
        if (a[5] != 57) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 1) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint16_t r = call6(66,0,135,198,3,101);
        if (r != 503) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 4: result = 114; break;
        case 5: result = 152; break;
        case 15: result = 189; break;
        case 1: result = 82; break;
        case 13: result = 191; break;
        case 10: result = 150; break;
        case 16: result = 124; break;
        case 14: result = 44; break;
        default: result = 10; break;
        }
        if (result != 189) failures++;
    }


    {
        volatile uint8_t port = 27;
        uint8_t r = port;
        if (r != 27) failures++;
    }


    {
        uint16_t r = add2(27,91) + add2(91,66) + add2(27,66);
        if (r != 368) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {191,164,18217,223};
        if (s.c != (uint16_t)18217) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(65,181) != 65420) failures++;
    }


    {
        uint16_t r = 31749 + 35849 + 47230 + 36198 + 57411 + 36100 + 44887 + 16754;
        if (r != 44034) failures++;
    }


    {
        volatile int16_t a = -13973;
        volatile int16_t b = 21966;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 168;
        x = x + 67;
        if (x != 235) failures++;
    }


    {
        g16 = 25391;
        if (read_g16() != 25391) failures++;
    }


    {
        uint16_t r = add2(228,243) + add2(243,65) + add2(228,65);
        if (r != 1072) failures++;
    }


    {
        if (((uint16_t)((188 & (41 ^ 191)) ^ 24)) != 140) failures++;
    }


    {
        uint16_t r = call6(44,23,235,155,80,64);
        if (r != 601) failures++;
    }


    {
        uint8_t a[6] = {4,175,197,72,201,187};
        if (a[3] != 72) failures++;
    }


    {
        volatile uint8_t port = 14;
        uint8_t r = port;
        if (r != 14) failures++;
    }


    {
        uint8_t v = 189;
        v |= 4;
        if (v != 189) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-34) / (int16_t)((int8_t)125);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = add2(87,185) + add2(185,177) + add2(87,177);
        if (r != 898) failures++;
    }


    {
        uint8_t v = 27;
        v |= 64;
        if (v != 91) failures++;
    }


    {
        uint8_t buf[8] = {165,39,54,210,202,165,50,44};
        uint8_t *p = buf;
        p += 1;
        if (*p != 39) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(133,8) != 141) failures++;
    }


    {
        uint8_t x = 213;
        x <<= 5;
        if (x != 160) failures++;
    }


    {
        if (((uint16_t)(((129 | 117) - (184 ^ 169)) | 224)) != 228) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)77) + (uint16_t)14163;
        if (r != 14240) failures++;
    }


    {
        uint16_t r = 14833 + 16431 + 39925 + 7798 + 46109 + 55674 + 45483 + 37159;
        if (r != 1268) failures++;
    }


    {
        volatile uint8_t port = 161;
        uint8_t r = port;
        if (r != 161) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(1,187) != 188) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)105) / (int16_t)((int8_t)2);
        if ((uint16_t)r != (uint16_t)52) failures++;
    }


    {
        uint16_t x = 14702;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 119;
        v |= 32;
        if (v != 119) failures++;
    }


    {
        uint8_t a[6] = {11,163,152,146,58,143};
        if (a[0] != 11) failures++;
    }


    {
        uint8_t src[9] = {243,143,159,111,173,121,26,49,230};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[1] != 143) failures++;
    }


    {
        uint16_t r = add2(51,127) + add2(127,104) + add2(51,104);
        if (r != 564) failures++;
    }


    {
        uint8_t a[6] = {2,211,249,244,233,13};
        if (a[1] != 211) failures++;
    }


    {
        uint8_t v = 41;
        v ^= 128;
        if (v != 169) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 100;
        if (buf[12] != 100) failures++;
    }


    {
        uint16_t r = 54945 + 22620 + 11577 + 939 + 54308 + 53582 + 51770 + 53573;
        if (r != 41170) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {131,178,58640,219};
        if (s.a != (uint8_t)131) failures++;
    }


    {
        uint16_t r = 31823 + 3228 + 64362 + 14304 + 42223 + 62124 + 21723 + 65106;
        if (r != 42749) failures++;
    }


    {
        uint16_t x = 20869;
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
        uint16_t x = 75;
        x = x + 192;
        if (x != 267) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint8_t src[4] = {193,175,239,56};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 193) failures++;
    }


    {
        uint8_t m[3][2] = {{102,106},{136,78},{175,184}};
        if (m[1][1] != 78) failures++;
    }


    {
        uint16_t x = 37;
        x = x + 55;
        if (x != 92) failures++;
    }


    {
        uint16_t r = add2(176,224) + add2(224,175) + add2(176,175);
        if (r != 1150) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {8,251,34518,221};
        if (s.d != (uint8_t)221) failures++;
    }


    {
        uint32_t a = 2543035685UL;
        uint32_t b = 3732569805UL;
        uint32_t r = a + b;
        if (r != 1980638194UL) failures++;
    }


    {
        volatile uint8_t port = 74;
        uint8_t r = port;
        if (r != 74) failures++;
    }


    {
        volatile uint8_t port = 213;
        uint8_t r = port;
        if (r != 213) failures++;
    }


    {
        uint8_t v = 187;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 114;
        uint8_t r = port;
        if (r != 114) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 21;
        if (buf[0] != 21) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {136,113,37675,64};
        if (s.a != (uint8_t)136) failures++;
    }


    {
        g16 = 32010;
        if (read_g16() != 32010) failures++;
    }


    {
        uint8_t m[4][4] = {{228,101,7,143},{46,181,51,191},{101,234,98,83},{160,122,163,17}};
        if (m[2][0] != 101) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 245;
        if (buf[1] != 245) failures++;
    }


    {
        uint8_t src[9] = {242,22,4,154,174,104,204,114,4};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[8] != 4) failures++;
    }


    {
        uint16_t r = call6(26,139,218,208,97,142);
        if (r != 830) failures++;
    }


    {
        g16 = 24884;
        if (read_g16() != 24884) failures++;
    }


    {
        uint8_t buf[8] = {115,243,200,82,62,18,225,6};
        uint8_t *p = buf;
        p += 3;
        if (*p != 82) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)60) / (int16_t)((int8_t)-109);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 25792;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(49,12) + add2(12,165) + add2(49,165);
        if (r != 452) failures++;
    }


    {
        uint16_t x = 167;
        x = x + 65;
        if (x != 232) failures++;
    }


    {
        uint16_t x = 5819;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 104;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint32_t a = 1378249435UL;
        uint32_t b = 605711118UL;
        uint32_t r = a | b;
        if (r != 1983802335UL) failures++;
    }


    {
        uint16_t r = call6(39,89,67,246,51,216);
        if (r != 708) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t x = 183;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t r = call6(253,18,110,150,131,191);
        if (r != 853) failures++;
    }


    {
        g16 = 25196;
        if (read_g16() != 25196) failures++;
    }


    {
        int8_t a = -46;
        int8_t b = -128;
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
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 167;
        if (buf[6] != 167) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 59;
        if (buf[8] != 59) failures++;
    }


    {
        uint16_t x = 60140;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(137,73) != 64) failures++;
    }


    {
        uint8_t v = 143;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {245,233,59489,208};
        if (s.a != (uint8_t)245) failures++;
    }


    {
        if (((uint16_t)((134 | (235 | 5)) | ((2 & 161) & (64 & 127)))) != 239) failures++;
    }


    {
        volatile uint8_t port = 116;
        uint8_t r = port;
        if (r != 116) failures++;
    }


    {
        uint8_t src[13] = {94,225,240,119,82,59,207,40,19,163,216,81,94};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[0] != 94) failures++;
    }


    {
        uint8_t v = 207;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        volatile uint8_t port = 5;
        uint8_t r = port;
        if (r != 5) failures++;
    }


    {
        if (((uint16_t)(135 | 8)) != 143) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)215) + (uint16_t)5234;
        if (r != 5449) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {204,145,17986,63};
        if (s.b != (uint8_t)145) failures++;
    }


    {
        uint16_t x = 171;
        x = x + 4;
        if (x != 175) failures++;
    }


    {
        uint8_t buf[8] = {81,163,228,25,95,174,211,80};
        uint8_t *p = buf;
        p += 0;
        if (*p != 81) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)31) % (int16_t)((int8_t)46);
        if ((uint16_t)r != (uint16_t)31) failures++;
    }


    {
        uint8_t m[2][3] = {{124,203,157},{64,13,168}};
        if (m[0][0] != 124) failures++;
    }


    {
        volatile uint8_t port = 227;
        uint8_t r = port;
        if (r != 227) failures++;
    }


    {
        uint8_t src[8] = {118,196,234,36,108,9,149,121};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[6] != 149) failures++;
    }


    {
        uint8_t v = 32;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 16) failures++;
    }


    {
        volatile uint8_t port = 104;
        uint8_t r = port;
        if (r != 104) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-75) / (int16_t)((int8_t)-91);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 53;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t v = 143;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 9) failures++;
    }


    {
        uint8_t m[3][3] = {{142,223,9},{85,179,225},{83,31,253}};
        if (m[0][0] != 142) failures++;
    }


    {
        uint16_t x = 23;
        x = x + 55;
        if (x != 78) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)58) + (uint16_t)51530;
        if (r != 51588) failures++;
    }


    {
        if (((uint16_t)(((236 - 114) + (78 + 69)) - (188 ^ (169 | 22)))) != 266) failures++;
    }


    {
        volatile uint8_t port = 243;
        uint8_t r = port;
        if (r != 243) failures++;
    }


    {
        if (((uint16_t)56) != 56) failures++;
    }


    {
        uint16_t x = 164;
        x = x + 35;
        if (x != 199) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)107) / (int16_t)((int8_t)-31);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(11,113) != 65434) failures++;
    }


    {
        uint8_t x = 124;
        x <<= 3;
        if (x != 224) failures++;
    }


    {
        g16 = 56614;
        if (read_g16() != 56614) failures++;
    }


    {
        uint8_t src[11] = {119,118,94,11,200,135,198,173,176,152,212};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[9] != 152) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(247,169) != 78) failures++;
    }


    {
        uint8_t buf[8] = {183,251,64,220,139,155,35,23};
        uint8_t *p = buf;
        p += 6;
        if (*p != 35) failures++;
    }


    {
        uint8_t m[3][2] = {{49,15},{248,96},{61,235}};
        if (m[2][1] != 235) failures++;
    }


    {
        uint8_t a[6] = {239,30,229,134,208,129};
        if (a[5] != 129) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 4: result = 66; break;
        case 3: result = 104; break;
        case 0: result = 247; break;
        case 12: result = 2; break;
        case 13: result = 58; break;
        case 6: result = 122; break;
        case 1: result = 143; break;
        default: result = 42; break;
        }
        if (result != 66) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {88,40,33963,19};
        if (s.b != (uint8_t)40) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 1) sum += j;
        if (sum != 0) failures++;
    }


    {
        int8_t a = -34;
        int8_t b = -98;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {188,163,251,147,188,198};
        if (a[4] != 188) failures++;
    }


    {
        uint16_t r = 19958 + 6245 + 56553 + 64843 + 27095 + 17335 + 60786 + 41653;
        if (r != 32324) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(99,231,119,192,63,136);
        if (r != 840) failures++;
    }


    {
        uint16_t r = add2(179,136) + add2(136,153) + add2(179,153);
        if (r != 936) failures++;
    }


    {
        uint16_t r = add2(2,35) + add2(35,218) + add2(2,218);
        if (r != 510) failures++;
    }


    {
        uint8_t v = 207;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 56400;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(242,53,106,191,159,26);
        if (r != 777) failures++;
    }


    {
        int8_t a = 92;
        int8_t b = -9;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 45;
        int8_t b = 59;
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
        int16_t r = (int16_t)((int8_t)37) / (int16_t)((int8_t)-29);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t r = call6(173,152,31,97,239,44);
        if (r != 736) failures++;
    }


    {
        uint8_t v = 243;
        v |= 128;
        if (v != 243) failures++;
    }


    {
        uint8_t buf[8] = {134,128,208,149,234,250,56,203};
        uint8_t *p = buf;
        p += 3;
        if (*p != 149) failures++;
    }


    {
        uint8_t src[4] = {2,0,122,88};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 88) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {54,4,59339,160};
        if (s.d != (uint8_t)160) failures++;
    }


    {
        volatile uint8_t port = 160;
        uint8_t r = port;
        if (r != 160) failures++;
    }


    {
        uint16_t r = call6(69,90,42,219,221,153);
        if (r != 794) failures++;
    }


    {
        if (((uint16_t)164) != 164) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {52,166,32223,45};
        if (s.d != (uint8_t)45) failures++;
    }


    {
        uint8_t buf[8] = {57,132,44,176,153,143,114,224};
        uint8_t *p = buf;
        p += 4;
        if (*p != 153) failures++;
    }


    {
        uint8_t src[1] = {252};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 252) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(103,137) != 240) failures++;
    }


    {
        uint32_t a = 2543529327UL;
        uint32_t b = 3790728542UL;
        uint32_t r = a & b;
        if (r != 2173775182UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint8_t x = 195;
        x <<= 2;
        if (x != 12) failures++;
    }


    {
        uint8_t v = 143;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint32_t a = 210362680UL;
        uint32_t b = 825894725UL;
        uint32_t r = a ^ b;
        if (r != 1035191933UL) failures++;
    }


    {
        uint8_t m[3][4] = {{9,252,94,62},{145,78,103,161},{37,30,178,111}};
        if (m[0][0] != 9) failures++;
    }


    {
        uint8_t src[5] = {71,240,94,109,7};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[1] != 240) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 52; break;
        case 7: result = 111; break;
        case 2: result = 46; break;
        default: result = 49; break;
        }
        if (result != 49) failures++;
    }


    {
        uint16_t r = call6(180,83,242,115,58,91);
        if (r != 769) failures++;
    }


    {
        uint8_t v = 152;
        v |= 16;
        if (v != 152) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)249) + (uint16_t)41730;
        if (r != 41979) failures++;
    }


    {
        uint8_t a[6] = {139,146,55,176,106,51};
        if (a[2] != 55) failures++;
    }


    {
        uint16_t x = 38377;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 78;
        uint8_t r = port;
        if (r != 78) failures++;
    }


    {
        g16 = 23196;
        if (read_g16() != 23196) failures++;
    }


    {
        uint8_t m[2][4] = {{87,203,221,239},{104,124,35,132}};
        if (m[0][1] != 203) failures++;
    }


    {
        uint8_t x = 49;
        x <<= 2;
        if (x != 196) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {97,240,5568,56};
        if (s.c != (uint16_t)5568) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 8: result = 62; break;
        case 9: result = 229; break;
        case 5: result = 201; break;
        case 7: result = 252; break;
        case 3: result = 180; break;
        case 2: result = 51; break;
        case 17: result = 74; break;
        case 16: result = 78; break;
        default: result = 56; break;
        }
        if (result != 74) failures++;
    }


    {
        uint8_t buf[8] = {177,80,85,132,244,171,102,31};
        uint8_t *p = buf;
        p += 2;
        if (*p != 85) failures++;
    }


    {
        uint8_t buf[8] = {29,219,128,73,131,148,209,72};
        uint8_t *p = buf;
        p += 5;
        if (*p != 148) failures++;
    }


    {
        uint8_t a[6] = {106,254,183,1,225,55};
        if (a[1] != 254) failures++;
    }


    {
        uint8_t a[6] = {228,176,9,239,160,8};
        if (a[0] != 228) failures++;
    }


    {
        uint16_t r = add2(234,12) + add2(12,189) + add2(234,189);
        if (r != 870) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {110,87,40,12,237,138,155,59};
        uint8_t *p = buf;
        p += 7;
        if (*p != 59) failures++;
    }


    {
        uint8_t m[3][2] = {{43,182},{12,222},{49,222}};
        if (m[0][1] != 182) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)((200 | (148 - 248)) + 231)) != 195) failures++;
    }


    {
        uint8_t v = 135;
        v &= ~(uint8_t)64;
        if (v != 135) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 174;
        if (buf[13] != 174) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint16_t r = 12798 + 23628 + 59074 + 5795 + 30696 + 19533 + 21285 + 41486;
        if (r != 17687) failures++;
    }


    {
        uint8_t src[7] = {153,245,23,39,242,211,188};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[5] != 211) failures++;
    }


    {
        volatile uint8_t port = 196;
        uint8_t r = port;
        if (r != 196) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)42) + (uint16_t)30349;
        if (r != 30391) failures++;
    }


    {
        uint8_t x = 103;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t x = 57203;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {226,220,156,171,225,84,197,191};
        uint8_t *p = buf;
        p += 6;
        if (*p != 197) failures++;
    }


    {
        uint16_t x = 24841;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[4][3] = {{163,26,170},{91,19,157},{153,62,54},{102,46,62}};
        if (m[2][0] != 153) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {238,124,21722,124};
        if (s.b != (uint8_t)124) failures++;
    }


    {
        uint16_t r = add2(242,184) + add2(184,94) + add2(242,94);
        if (r != 1040) failures++;
    }


    {
        uint16_t r = call6(172,202,245,246,132,247);
        if (r != 1244) failures++;
    }


    {
        uint8_t m[2][3] = {{14,46,6},{247,98,63}};
        if (m[1][2] != 63) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 186;
        if (buf[15] != 186) failures++;
    }


    {
        uint8_t x = 196;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t x = 1653;
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
        case 10: result = 120; break;
        case 6: result = 123; break;
        case 11: result = 108; break;
        case 16: result = 226; break;
        case 13: result = 11; break;
        case 3: result = 3; break;
        case 9: result = 102; break;
        default: result = 78; break;
        }
        if (result != 78) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)88) + (uint16_t)45176;
        if (r != 45264) failures++;
    }


    {
        uint8_t src[14] = {87,174,166,42,25,248,240,148,27,168,64,186,20,25};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[5] != 248) failures++;
    }


    {
        uint8_t v = 2;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 62) failures++;
    }


    {
        uint8_t m[2][2] = {{79,236},{182,114}};
        if (m[1][0] != 182) failures++;
    }


    {
        uint16_t x = 46763;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)192) + (uint16_t)38865;
        if (r != 39057) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t a[6] = {72,195,178,80,181,182};
        if (a[4] != 181) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(157,3) != 160) failures++;
    }


    {
        volatile uint8_t port = 206;
        uint8_t r = port;
        if (r != 206) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {219,98,59842,11};
        if (s.b != (uint8_t)98) failures++;
    }


    {
        uint16_t r = add2(45,140) + add2(140,77) + add2(45,77);
        if (r != 524) failures++;
    }


    {
        volatile uint8_t port = 96;
        uint8_t r = port;
        if (r != 96) failures++;
    }


    {
        uint16_t r = add2(46,141) + add2(141,108) + add2(46,108);
        if (r != 590) failures++;
    }


    {
        int8_t a = 63;
        int8_t b = 59;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 49;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        int8_t a = -32;
        int8_t b = 9;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {154,203,107,24,184,39};
        if (a[4] != 184) failures++;
    }


    {
        uint8_t v = 202;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(228,198,99,210,150,197);
        if (r != 1082) failures++;
    }


    {
        uint8_t a[6] = {199,125,149,65,83,49};
        if (a[2] != 149) failures++;
    }


    {
        volatile int16_t a = 25523;
        volatile int16_t b = -22320;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(234,98) != 332) failures++;
    }

    return failures;
}