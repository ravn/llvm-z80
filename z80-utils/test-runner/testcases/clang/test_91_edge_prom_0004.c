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
        uint8_t x = 26;
        x <<= 0;
        if (x != 26) failures++;
    }


    {
        int8_t a = 115;
        int8_t b = -55;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(119,71) + add2(71,74) + add2(119,74);
        if (r != 528) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-38) / (int16_t)((int8_t)-45);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(160,226) != 65470) failures++;
    }


    {
        volatile int16_t a = -21981;
        volatile int16_t b = 93;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 80;
        x <<= 3;
        if (x != 128) failures++;
    }


    {
        volatile uint8_t port = 65;
        uint8_t r = port;
        if (r != 65) failures++;
    }


    {
        uint16_t r = 2864 + 19388 + 61049 + 48004 + 61778 + 32660 + 45467 + 18312;
        if (r != 27378) failures++;
    }


    {
        g16 = 44436;
        if (read_g16() != 44436) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t v = 214;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t src[8] = {207,240,81,221,151,194,163,55};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[5] != 194) failures++;
    }


    {
        uint16_t r = call6(192,83,193,196,177,75);
        if (r != 916) failures++;
    }


    {
        volatile uint8_t port = 31;
        uint8_t r = port;
        if (r != 31) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 51;
        if (buf[4] != 51) failures++;
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
        for (uint8_t j = 0; j < 7; j++) buf[j] = 170;
        if (buf[6] != 170) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 8: result = 168; break;
        case 12: result = 174; break;
        case 11: result = 135; break;
        case 10: result = 67; break;
        default: result = 7; break;
        }
        if (result != 174) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {199,9,62510,24};
        if (s.a != (uint8_t)199) failures++;
    }


    {
        uint8_t a[6] = {230,67,224,177,82,46};
        if (a[3] != 177) failures++;
    }


    {
        g16 = 28062;
        if (read_g16() != 28062) failures++;
    }


    {
        if (((uint16_t)35) != 35) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 10: result = 22; break;
        case 2: result = 20; break;
        case 14: result = 112; break;
        default: result = 63; break;
        }
        if (result != 63) failures++;
    }


    {
        uint16_t x = 71;
        x = x + 43;
        if (x != 114) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(151,34) != 185) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)237) + (uint16_t)20492;
        if (r != 20729) failures++;
    }


    {
        g16 = 50570;
        if (read_g16() != 50570) failures++;
    }


    {
        uint16_t x = 163;
        x = x + 166;
        if (x != 329) failures++;
    }


    {
        int8_t a = -103;
        int8_t b = -125;
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {133,86,20916,102};
        if (s.a != (uint8_t)133) failures++;
    }


    {
        uint16_t x = 56191;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(166,38) + add2(38,36) + add2(166,36);
        if (r != 480) failures++;
    }


    {
        uint16_t r = add2(88,66) + add2(66,126) + add2(88,126);
        if (r != 560) failures++;
    }


    {
        uint16_t r = 61882 + 16399 + 62419 + 45684 + 55151 + 58618 + 62039 + 56850;
        if (r != 25826) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)43) + (uint16_t)12068;
        if (r != 12111) failures++;
    }


    {
        uint8_t m[2][3] = {{148,169,61},{183,26,72}};
        if (m[0][1] != 169) failures++;
    }


    {
        g16 = 43269;
        if (read_g16() != 43269) failures++;
    }


    {
        uint16_t r = call6(179,60,98,57,250,167);
        if (r != 811) failures++;
    }


    {
        volatile int16_t a = -11691;
        volatile int16_t b = -4909;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 12: result = 142; break;
        case 3: result = 47; break;
        case 7: result = 188; break;
        case 11: result = 47; break;
        case 0: result = 21; break;
        case 14: result = 150; break;
        case 19: result = 103; break;
        case 2: result = 89; break;
        default: result = 173; break;
        }
        if (result != 47) failures++;
    }


    {
        uint16_t r = call6(70,74,231,76,142,70);
        if (r != 663) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-126) % (int16_t)((int8_t)64);
        if ((uint16_t)r != (uint16_t)65474) failures++;
    }


    {
        uint16_t x = 24595;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        int8_t a = 47;
        int8_t b = 62;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 572244077UL;
        uint32_t b = 1101020505UL;
        uint32_t r = a ^ b;
        if (r != 1673264436UL) failures++;
    }


    {
        if (((uint16_t)(((68 & 197) ^ (164 ^ 29)) - ((80 | 141) + (82 - 7)))) != 65493) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {74,27,53862,45};
        if (s.a != (uint8_t)74) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t x = 240;
        x <<= 1;
        if (x != 224) failures++;
    }


    {
        int8_t a = -94;
        int8_t b = 56;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 43249;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(2,201) != 203) failures++;
    }


    {
        uint16_t x = 39410;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][4] = {{214,67,17,84},{235,139,9,86},{198,43,213,147}};
        if (m[0][0] != 214) failures++;
    }


    {
        if (((uint16_t)(((181 - 82) + (2 + 58)) ^ ((7 + 180) ^ 70))) != 98) failures++;
    }


    {
        uint16_t x = 37452;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -31597;
        volatile int16_t b = -2901;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 54;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(49,23,240,215,146,250);
        if (r != 923) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t m[3][3] = {{99,154,52},{173,74,248},{77,252,245}};
        if (m[0][0] != 99) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        if (((uint16_t)(84 | (167 ^ (218 & 210)))) != 117) failures++;
    }


    {
        uint8_t v = 1;
        v &= ~(uint8_t)2;
        if (v != 1) failures++;
    }


    {
        int8_t a = 26;
        int8_t b = -70;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint8_t buf[8] = {119,22,149,37,245,111,39,72};
        uint8_t *p = buf;
        p += 2;
        if (*p != 149) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)25) % (int16_t)((int8_t)4);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        volatile uint8_t port = 185;
        uint8_t r = port;
        if (r != 185) failures++;
    }


    {
        uint32_t a = 1617858495UL;
        uint32_t b = 2589054687UL;
        uint32_t r = a + b;
        if (r != 4206913182UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)59) + (uint16_t)14646;
        if (r != 14705) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 223;
        if (buf[9] != 223) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {117,54,229,133,22,24,109,250};
        uint8_t *p = buf;
        p += 0;
        if (*p != 117) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {21,27,32010,40};
        if (s.b != (uint8_t)27) failures++;
    }


    {
        uint8_t a[6] = {25,80,198,100,190,143};
        if (a[4] != 190) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 103;
        if (buf[4] != 103) failures++;
    }


    {
        uint16_t r = add2(228,164) + add2(164,38) + add2(228,38);
        if (r != 860) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-57) / (int16_t)((int8_t)64);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {247,14,35,64,202,211,240,170};
        uint8_t *p = buf;
        p += 1;
        if (*p != 14) failures++;
    }


    {
        uint8_t m[2][2] = {{217,255},{178,209}};
        if (m[0][0] != 217) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(29,58) != 87) failures++;
    }


    {
        uint8_t v = 129;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 61;
        x <<= 5;
        if (x != 160) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {49,98,6041,90};
        if (s.a != (uint8_t)49) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = 6878 + 29911 + 39067 + 25044 + 33221 + 46277 + 14261 + 37235;
        if (r != 35286) failures++;
    }


    {
        uint8_t v = 136;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint8_t v = 172;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = call6(11,89,203,112,115,63);
        if (r != 593) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(193,252) != 65477) failures++;
    }


    {
        uint8_t buf[8] = {211,165,253,217,143,180,137,199};
        uint8_t *p = buf;
        p += 1;
        if (*p != 165) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)13) + (uint16_t)63432;
        if (r != 63445) failures++;
    }


    {
        uint8_t v = 18;
        v &= ~(uint8_t)2;
        if (v != 16) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {226,190,22599,32};
        if (s.c != (uint16_t)22599) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)37) % (int16_t)((int8_t)24);
        if ((uint16_t)r != (uint16_t)13) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 66;
        if (buf[13] != 66) failures++;
    }


    {
        if (((uint16_t)202) != 202) failures++;
    }


    {
        g16 = 11637;
        if (read_g16() != 11637) failures++;
    }


    {
        volatile int16_t a = -17962;
        volatile int16_t b = 12442;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(87,70) + add2(70,67) + add2(87,67);
        if (r != 448) failures++;
    }


    {
        volatile int16_t a = 30661;
        volatile int16_t b = -31057;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        if (((uint16_t)((231 ^ (150 ^ 88)) + 114)) != 155) failures++;
    }


    {
        uint8_t v = 139;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 53) failures++;
    }


    {
        uint16_t x = 43841;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 94;
        uint8_t r = port;
        if (r != 94) failures++;
    }


    {
        volatile int16_t a = 29370;
        volatile int16_t b = 30036;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(107,177) != 284) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 141;
        if (buf[3] != 141) failures++;
    }


    {
        uint16_t r = 23835 + 16260 + 59255 + 56091 + 11191 + 53572 + 5776 + 1099;
        if (r != 30471) failures++;
    }


    {
        uint8_t v = 70;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t x = 65007;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 1) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint16_t x = 199;
        x = x + 206;
        if (x != 405) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t x = 3341;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t x = 207;
        x <<= 0;
        if (x != 207) failures++;
    }


    {
        if (((uint16_t)164) != 164) failures++;
    }


    {
        uint8_t v = 183;
        v |= 128;
        if (v != 183) failures++;
    }


    {
        uint8_t buf[8] = {147,53,23,109,152,234,44,147};
        uint8_t *p = buf;
        p += 2;
        if (*p != 23) failures++;
    }


    {
        if (((uint16_t)((160 - 154) | 136)) != 142) failures++;
    }


    {
        g16 = 57962;
        if (read_g16() != 57962) failures++;
    }


    {
        uint8_t x = 39;
        x <<= 0;
        if (x != 39) failures++;
    }


    {
        uint16_t r = add2(200,99) + add2(99,79) + add2(200,79);
        if (r != 756) failures++;
    }


    {
        uint8_t a[6] = {199,156,77,201,146,243};
        if (a[3] != 201) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(214,25) != 189) failures++;
    }


    {
        volatile uint8_t port = 47;
        uint8_t r = port;
        if (r != 47) failures++;
    }


    {
        uint16_t r = add2(40,135) + add2(135,166) + add2(40,166);
        if (r != 682) failures++;
    }


    {
        uint16_t r = add2(195,93) + add2(93,59) + add2(195,59);
        if (r != 694) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)44) / (int16_t)((int8_t)-2);
        if ((uint16_t)r != (uint16_t)65514) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(226,68) != 158) failures++;
    }


    {
        uint16_t r = call6(8,42,41,22,97,26);
        if (r != 236) failures++;
    }


    {
        uint8_t v = 189;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {3,219,190,193,7,213};
        if (a[4] != 7) failures++;
    }


    {
        uint32_t a = 4109452859UL;
        uint32_t b = 2105305916UL;
        uint32_t r = a | b;
        if (r != 4261244735UL) failures++;
    }


    {
        if (((uint16_t)(((166 - 28) | 15) ^ ((195 ^ 129) - 8))) != 181) failures++;
    }


    {
        uint8_t m[3][3] = {{250,80,9},{245,89,228},{57,113,43}};
        if (m[1][0] != 245) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)251) + (uint16_t)12250;
        if (r != 12501) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {227,243,21897,232};
        if (s.b != (uint8_t)243) failures++;
    }


    {
        g16 = 36325;
        if (read_g16() != 36325) failures++;
    }


    {
        uint16_t r = call6(56,94,5,236,49,184);
        if (r != 624) failures++;
    }


    {
        uint16_t r = add2(33,63) + add2(63,172) + add2(33,172);
        if (r != 536) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)22) / (int16_t)((int8_t)-53);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 51925;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {159,214,38057,111};
        if (s.d != (uint8_t)111) failures++;
    }


    {
        uint16_t r = 4098 + 30014 + 43583 + 10101 + 33308 + 55416 + 33121 + 38154;
        if (r != 51187) failures++;
    }


    {
        uint16_t r = add2(161,233) + add2(233,216) + add2(161,216);
        if (r != 1220) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 3: result = 234; break;
        case 1: result = 21; break;
        case 10: result = 172; break;
        default: result = 246; break;
        }
        if (result != 172) failures++;
    }


    {
        uint8_t m[4][4] = {{255,244,235,111},{175,43,18,75},{161,253,133,11},{157,162,99,125}};
        if (m[3][0] != 157) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 119;
        if (buf[5] != 119) failures++;
    }


    {
        uint16_t x = 184;
        x = x + 26;
        if (x != 210) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 125;
        if (buf[10] != 125) failures++;
    }


    {
        uint8_t m[3][3] = {{65,127,131},{203,107,225},{169,107,153}};
        if (m[2][2] != 153) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t r = 4944 + 7747 + 45356 + 30790 + 49863 + 5799 + 59974 + 15936;
        if (r != 23801) failures++;
    }


    {
        uint16_t r = 27063 + 35297 + 7916 + 3145 + 50211 + 64185 + 57266 + 55866;
        if (r != 38805) failures++;
    }


    {
        uint8_t v = 231;
        int r = (v & 64) ? 1 : 0;
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
        uint8_t m[4][3] = {{229,88,101},{190,111,91},{146,244,63},{148,203,106}};
        if (m[3][1] != 203) failures++;
    }


    {
        volatile uint8_t port = 44;
        uint8_t r = port;
        if (r != 44) failures++;
    }


    {
        uint16_t r = 14966 + 62578 + 46306 + 50788 + 26771 + 43020 + 12482 + 23037;
        if (r != 17804) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)80) + (uint16_t)11746;
        if (r != 11826) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 41308 + 35377 + 64093 + 28019 + 18622 + 37333 + 57579 + 3918;
        if (r != 24105) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 112;
        if (buf[12] != 112) failures++;
    }


    {
        uint16_t r = 25509 + 1361 + 35422 + 18926 + 1384 + 34246 + 24019 + 32035;
        if (r != 41830) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(174,208) != 382) failures++;
    }


    {
        uint32_t a = 925347661UL;
        uint32_t b = 3099519143UL;
        uint32_t r = a | b;
        if (r != 3217027055UL) failures++;
    }


    {
        uint16_t r = add2(26,177) + add2(177,190) + add2(26,190);
        if (r != 786) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = add2(57,207) + add2(207,95) + add2(57,95);
        if (r != 718) failures++;
    }


    {
        int8_t a = -8;
        int8_t b = 4;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)((86 - (41 - 17)) | 76)) != 126) failures++;
    }


    {
        uint16_t r = call6(197,151,170,109,165,181);
        if (r != 973) failures++;
    }


    {
        volatile uint8_t port = 143;
        uint8_t r = port;
        if (r != 143) failures++;
    }


    {
        uint16_t r = 15322 + 59180 + 63992 + 4829 + 63099 + 23330 + 9061 + 57251;
        if (r != 33920) failures++;
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
        uint8_t buf[8] = {156,49,215,31,99,211,181,27};
        uint8_t *p = buf;
        p += 5;
        if (*p != 211) failures++;
    }


    {
        volatile int16_t a = 8466;
        volatile int16_t b = 19921;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 177;
        if (buf[3] != 177) failures++;
    }


    {
        uint16_t x = 201;
        x = x + 0;
        if (x != 201) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {87,74,47819,59};
        if (s.c != (uint16_t)47819) failures++;
    }


    {
        uint8_t x = 81;
        x <<= 2;
        if (x != 68) failures++;
    }


    {
        uint16_t r = 15344 + 54702 + 45866 + 3106 + 43993 + 30377 + 10410 + 20409;
        if (r != 27599) failures++;
    }


    {
        g16 = 50624;
        if (read_g16() != 50624) failures++;
    }


    {
        uint8_t x = 193;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t v = 93;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint32_t a = 1351362526UL;
        uint32_t b = 923019351UL;
        uint32_t r = a | b;
        if (r != 2005675999UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(253,87) != 166) failures++;
    }


    {
        if (((uint16_t)((66 ^ (56 & 69)) ^ ((24 | 166) | (101 | 217)))) != 189) failures++;
    }


    {
        uint16_t r = add2(153,203) + add2(203,92) + add2(153,92);
        if (r != 896) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {190,157,5150,163};
        if (s.c != (uint16_t)5150) failures++;
    }


    {
        uint8_t v = 31;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 33) failures++;
    }


    {
        uint16_t x = 40100;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 46745;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {242,26,236,3,145,124,109,79};
        uint8_t *p = buf;
        p += 7;
        if (*p != 79) failures++;
    }


    {
        uint8_t x = 184;
        x <<= 4;
        if (x != 128) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 2: result = 148; break;
        case 17: result = 198; break;
        case 14: result = 229; break;
        case 1: result = 165; break;
        case 0: result = 168; break;
        case 6: result = 173; break;
        case 7: result = 193; break;
        case 9: result = 36; break;
        default: result = 75; break;
        }
        if (result != 173) failures++;
    }


    {
        g16 = 8671;
        if (read_g16() != 8671) failures++;
    }


    {
        int8_t a = -42;
        int8_t b = 12;
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
        volatile uint8_t port = 115;
        uint8_t r = port;
        if (r != 115) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        volatile int16_t a = 7116;
        volatile int16_t b = -11293;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {68,248,225,27,245,1,25,35};
        uint8_t *p = buf;
        p += 5;
        if (*p != 1) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 2: result = 184; break;
        case 12: result = 33; break;
        case 17: result = 1; break;
        case 15: result = 20; break;
        case 18: result = 177; break;
        case 8: result = 122; break;
        case 11: result = 161; break;
        default: result = 138; break;
        }
        if (result != 177) failures++;
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
        for (uint16_t j = 0; j < 3; j += 1) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 8: result = 242; break;
        case 13: result = 163; break;
        case 15: result = 30; break;
        case 11: result = 145; break;
        case 12: result = 167; break;
        case 2: result = 150; break;
        case 9: result = 205; break;
        case 1: result = 225; break;
        default: result = 70; break;
        }
        if (result != 30) failures++;
    }


    {
        uint8_t buf[8] = {149,192,159,6,150,116,185,238};
        uint8_t *p = buf;
        p += 4;
        if (*p != 150) failures++;
    }


    {
        uint16_t r = 57808 + 22070 + 6945 + 40519 + 57517 + 15518 + 25612 + 31262;
        if (r != 60643) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(127,253) != 380) failures++;
    }


    {
        g16 = 30565;
        if (read_g16() != 30565) failures++;
    }


    {
        int8_t a = -9;
        int8_t b = -98;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 96;
        uint8_t r = port;
        if (r != 96) failures++;
    }


    {
        g16 = 37104;
        if (read_g16() != 37104) failures++;
    }


    {
        uint8_t src[16] = {96,102,169,120,191,168,208,157,56,234,92,216,88,102,174,28};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[9] != 234) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 112;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)119) % (int16_t)((int8_t)-79);
        if ((uint16_t)r != (uint16_t)40) failures++;
    }


    {
        int8_t a = -96;
        int8_t b = -28;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-105) % (int16_t)((int8_t)-38);
        if ((uint16_t)r != (uint16_t)65507) failures++;
    }


    {
        uint16_t r = add2(165,254) + add2(254,187) + add2(165,187);
        if (r != 1212) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 2: result = 110; break;
        case 8: result = 33; break;
        case 11: result = 137; break;
        case 16: result = 68; break;
        case 19: result = 253; break;
        case 17: result = 171; break;
        default: result = 12; break;
        }
        if (result != 12) failures++;
    }


    {
        uint8_t v = 215;
        v |= 128;
        if (v != 215) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)56) % (int16_t)((int8_t)-50);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(192,4) != 188) failures++;
    }


    {
        uint16_t x = 58;
        x = x + 213;
        if (x != 271) failures++;
    }


    {
        int8_t a = 68;
        int8_t b = 29;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 1210171702UL;
        uint32_t b = 689437226UL;
        uint32_t r = a ^ b;
        if (r != 1630944028UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {2,248,226,225,71,52,242,107};
        uint8_t *p = buf;
        p += 4;
        if (*p != 71) failures++;
    }


    {
        uint16_t x = 62049;
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
        volatile int16_t a = -2987;
        volatile int16_t b = 11977;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {74,119,41716,215};
        if (s.d != (uint8_t)215) failures++;
    }


    {
        uint8_t v = 97;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 7656;
        if (read_g16() != 7656) failures++;
    }


    {
        uint8_t m[2][4] = {{152,26,56,47},{195,238,175,232}};
        if (m[0][3] != 47) failures++;
    }


    {
        uint32_t a = 577924395UL;
        uint32_t b = 1285284525UL;
        uint32_t r = a + b;
        if (r != 1863208920UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)66) + (uint16_t)39713;
        if (r != 39779) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(28,21) != 49) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 1) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t v = 175;
        v |= 32;
        if (v != 175) failures++;
    }


    {
        uint16_t r = add2(171,187) + add2(187,250) + add2(171,250);
        if (r != 1216) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 18: result = 128; break;
        case 11: result = 5; break;
        case 6: result = 216; break;
        case 19: result = 122; break;
        default: result = 10; break;
        }
        if (result != 216) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(248,114) != 362) failures++;
    }


    {
        uint16_t r = call6(48,248,169,109,92,148);
        if (r != 814) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {48,118,56751,63};
        if (s.a != (uint8_t)48) failures++;
    }


    {
        uint16_t r = call6(230,112,161,148,5,140);
        if (r != 796) failures++;
    }


    {
        uint16_t r = call6(184,225,103,169,221,58);
        if (r != 960) failures++;
    }


    {
        uint32_t a = 891055138UL;
        uint32_t b = 3905310075UL;
        uint32_t r = a ^ b;
        if (r != 3722060121UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {235,136,35954,129};
        if (s.d != (uint8_t)129) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(176,171) + add2(171,157) + add2(176,157);
        if (r != 1008) failures++;
    }


    {
        uint8_t buf[8] = {109,163,80,158,243,225,209,190};
        uint8_t *p = buf;
        p += 2;
        if (*p != 80) failures++;
    }


    {
        uint8_t a[6] = {176,210,167,20,7,37};
        if (a[4] != 7) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)96) % (int16_t)((int8_t)-76);
        if ((uint16_t)r != (uint16_t)20) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 1) sum += j;
        if (sum != 45) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-57) / (int16_t)((int8_t)-17);
        if ((uint16_t)r != (uint16_t)3) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)92) / (int16_t)((int8_t)96);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 101;
        x = x + 48;
        if (x != 149) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {239,147,36076,6};
        if (s.c != (uint16_t)36076) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 1) sum += j;
        if (sum != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-2) % (int16_t)((int8_t)-69);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint16_t x = 39105;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(92,171) != 65457) failures++;
    }


    {
        uint8_t x = 29;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)162) + (uint16_t)32306;
        if (r != 32468) failures++;
    }


    {
        uint8_t a[6] = {16,157,112,182,246,47};
        if (a[3] != 182) failures++;
    }


    {
        uint16_t x = 34322;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 61914;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t x = 48;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        uint8_t a[6] = {71,113,198,157,19,44};
        if (a[0] != 71) failures++;
    }


    {
        uint32_t a = 962106064UL;
        uint32_t b = 267500733UL;
        uint32_t r = a & b;
        if (r != 156274832UL) failures++;
    }


    {
        uint8_t x = 158;
        x <<= 2;
        if (x != 120) failures++;
    }


    {
        g16 = 21534;
        if (read_g16() != 21534) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)96) + (uint16_t)7083;
        if (r != 7179) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)40) % (int16_t)((int8_t)-43);
        if ((uint16_t)r != (uint16_t)40) failures++;
    }


    {
        uint16_t x = 37729;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(((51 + 195) & (14 | 75)) - 85)) != 65521) failures++;
    }


    {
        uint32_t a = 2887203067UL;
        uint32_t b = 1391882679UL;
        uint32_t r = a + b;
        if (r != 4279085746UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {156,165,58270,77};
        if (s.b != (uint8_t)165) failures++;
    }


    {
        if (((uint16_t)(91 ^ 205)) != 150) failures++;
    }


    {
        uint8_t v = 146;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 46) failures++;
    }


    {
        int8_t a = 88;
        int8_t b = -38;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(161,62,129,189,68,143);
        if (r != 752) failures++;
    }


    {
        if (((uint16_t)252) != 252) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint16_t r = 47837 + 50093 + 42564 + 64169 + 36222 + 59722 + 14309 + 57357;
        if (r != 44593) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t src[3] = {100,31,240};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 240) failures++;
    }


    {
        uint16_t r = add2(12,64) + add2(64,204) + add2(12,204);
        if (r != 560) failures++;
    }


    {
        if (((uint16_t)(3 ^ 72)) != 75) failures++;
    }


    {
        if (((uint16_t)255) != 255) failures++;
    }


    {
        volatile uint8_t port = 216;
        uint8_t r = port;
        if (r != 216) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {198,165,58963,143};
        if (s.a != (uint8_t)198) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 32;
        if (buf[2] != 32) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 0; break;
        case 8: result = 93; break;
        case 5: result = 10; break;
        default: result = 105; break;
        }
        if (result != 105) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 64018;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 206;
        v ^= 1;
        if (v != 207) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {159,243,51687,241};
        if (s.c != (uint16_t)51687) failures++;
    }


    {
        uint8_t v = 114;
        v |= 64;
        if (v != 114) failures++;
    }


    {
        uint16_t x = 42;
        x = x + 3;
        if (x != 45) failures++;
    }


    {
        uint8_t m[3][4] = {{9,182,206,92},{14,178,209,2},{174,241,213,15}};
        if (m[1][1] != 178) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-15) / (int16_t)((int8_t)1);
        if ((uint16_t)r != (uint16_t)65521) failures++;
    }


    {
        uint16_t x = 246;
        x = x + 253;
        if (x != 499) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {83,169,62641,253};
        if (s.d != (uint8_t)253) failures++;
    }


    {
        uint16_t r = 37181 + 43321 + 34952 + 63215 + 5432 + 1308 + 48388 + 34385;
        if (r != 6038) failures++;
    }


    {
        uint8_t v = 195;
        v ^= 4;
        if (v != 199) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)78) + (uint16_t)60891;
        if (r != 60969) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-90) % (int16_t)((int8_t)-16);
        if ((uint16_t)r != (uint16_t)65526) failures++;
    }


    {
        uint8_t src[1] = {250};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 250) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)50) + (uint16_t)32257;
        if (r != 32307) failures++;
    }


    {
        volatile int16_t a = 27169;
        volatile int16_t b = -21075;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-37) / (int16_t)((int8_t)78);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 142;
        uint8_t r = port;
        if (r != 142) failures++;
    }


    {
        uint16_t r = add2(230,239) + add2(239,232) + add2(230,232);
        if (r != 1402) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 29;
        if (buf[6] != 29) failures++;
    }


    {
        uint16_t x = 45484;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 165;
        uint8_t r = port;
        if (r != 165) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t r = add2(159,226) + add2(226,76) + add2(159,76);
        if (r != 922) failures++;
    }


    {
        g16 = 65471;
        if (read_g16() != 65471) failures++;
    }


    {
        uint16_t r = call6(117,29,188,121,106,88);
        if (r != 649) failures++;
    }


    {
        uint8_t buf[8] = {213,177,159,52,28,220,211,25};
        uint8_t *p = buf;
        p += 5;
        if (*p != 220) failures++;
    }


    {
        uint8_t src[16] = {71,17,41,143,10,107,62,251,145,217,106,137,71,70,233,138};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[3] != 143) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 178;
        if (buf[8] != 178) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t m[4][4] = {{46,38,115,120},{240,138,193,132},{25,168,132,151},{133,39,246,146}};
        if (m[0][2] != 115) failures++;
    }


    {
        if (((uint16_t)(((248 | 233) + (159 & 149)) - (95 - (99 + 193)))) != 595) failures++;
    }


    {
        g16 = 2851;
        if (read_g16() != 2851) failures++;
    }


    {
        uint8_t m[2][2] = {{31,42},{68,163}};
        if (m[0][0] != 31) failures++;
    }


    {
        uint8_t buf[8] = {4,101,58,29,87,151,86,147};
        uint8_t *p = buf;
        p += 0;
        if (*p != 4) failures++;
    }


    {
        uint16_t x = 25;
        x = x + 151;
        if (x != 176) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)36) + (uint16_t)10240;
        if (r != 10276) failures++;
    }


    {
        uint16_t r = 18353 + 48372 + 6447 + 42788 + 49148 + 52127 + 39690 + 32297;
        if (r != 27078) failures++;
    }


    {
        uint16_t r = add2(133,95) + add2(95,239) + add2(133,239);
        if (r != 934) failures++;
    }


    {
        uint16_t r = 49483 + 10025 + 48456 + 43500 + 47017 + 46952 + 33686 + 42022;
        if (r != 58997) failures++;
    }


    {
        uint8_t src[16] = {223,8,255,69,79,166,200,110,186,38,197,235,215,208,68,150};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[6] != 200) failures++;
    }


    {
        uint16_t x = 246;
        x = x + 104;
        if (x != 350) failures++;
    }


    {
        uint8_t m[4][2] = {{253,9},{150,122},{60,122},{253,189}};
        if (m[2][0] != 60) failures++;
    }


    {
        uint8_t v = 145;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 7) failures++;
    }


    {
        uint8_t src[10] = {58,211,180,150,250,128,247,13,71,27};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[6] != 247) failures++;
    }


    {
        uint16_t r = call6(181,35,171,237,204,165);
        if (r != 993) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-119) / (int16_t)((int8_t)-47);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(147,51) != 96) failures++;
    }


    {
        uint16_t r = 27975 + 2163 + 64393 + 6857 + 19398 + 46896 + 22788 + 18336;
        if (r != 12198) failures++;
    }


    {
        uint8_t src[1] = {140};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 140) failures++;
    }


    {
        uint16_t r = call6(52,43,167,101,221,11);
        if (r != 595) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(220,149) != 369) failures++;
    }


    {
        uint16_t r = add2(220,129) + add2(129,134) + add2(220,134);
        if (r != 966) failures++;
    }


    {
        uint16_t r = 49856 + 19600 + 33416 + 13183 + 53264 + 20357 + 54400 + 62128;
        if (r != 44060) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-37) % (int16_t)((int8_t)72);
        if ((uint16_t)r != (uint16_t)65499) failures++;
    }


    {
        g16 = 9100;
        if (read_g16() != 9100) failures++;
    }


    {
        uint8_t v = 226;
        v |= 16;
        if (v != 242) failures++;
    }


    {
        volatile int16_t a = -20900;
        volatile int16_t b = -17329;
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
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 160; break;
        case 15: result = 29; break;
        case 18: result = 75; break;
        default: result = 224; break;
        }
        if (result != 160) failures++;
    }


    {
        volatile int16_t a = 28710;
        volatile int16_t b = -8977;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)117) + (uint16_t)42197;
        if (r != 42314) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 129;
        if (buf[12] != 129) failures++;
    }


    {
        uint8_t v = 98;
        v &= ~(uint8_t)8;
        if (v != 98) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(121,110) != 11) failures++;
    }


    {
        volatile int16_t a = -18549;
        volatile int16_t b = -10287;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {187,8,64079,88};
        if (s.c != (uint16_t)64079) failures++;
    }


    {
        uint8_t src[5] = {140,11,125,168,221};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[4] != 221) failures++;
    }


    {
        if (((uint16_t)76) != 76) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {97,148,65293,169};
        if (s.d != (uint8_t)169) failures++;
    }


    {
        uint8_t x = 188;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {94,235,8081,136};
        if (s.d != (uint8_t)136) failures++;
    }


    {
        uint8_t m[4][4] = {{214,136,126,2},{100,30,233,213},{244,12,212,207},{214,61,37,32}};
        if (m[1][1] != 30) failures++;
    }


    {
        uint16_t x = 21253;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-63) / (int16_t)((int8_t)-42);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t v = 252;
        v ^= 2;
        if (v != 254) failures++;
    }


    {
        g16 = 26395;
        if (read_g16() != 26395) failures++;
    }


    {
        uint16_t x = 555;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 151;
        v |= 16;
        if (v != 151) failures++;
    }


    {
        uint16_t x = 7580;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {40,168,33,199,116,121,95,147};
        uint8_t *p = buf;
        p += 0;
        if (*p != 40) failures++;
    }


    {
        uint8_t v = 88;
        v &= ~(uint8_t)8;
        if (v != 80) failures++;
    }


    {
        uint8_t a[6] = {76,164,237,13,46,202};
        if (a[5] != 202) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 17: result = 96; break;
        case 8: result = 49; break;
        case 6: result = 146; break;
        case 16: result = 218; break;
        case 14: result = 5; break;
        default: result = 234; break;
        }
        if (result != 5) failures++;
    }


    {
        uint32_t a = 2936142843UL;
        uint32_t b = 2132209472UL;
        uint32_t r = a ^ b;
        if (r != 3491173563UL) failures++;
    }


    {
        g16 = 62709;
        if (read_g16() != 62709) failures++;
    }


    {
        g16 = 25167;
        if (read_g16() != 25167) failures++;
    }


    {
        uint16_t r = add2(105,32) + add2(32,51) + add2(105,51);
        if (r != 376) failures++;
    }


    {
        uint8_t v = 23;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 1) sum += j;
        if (sum != 36) failures++;
    }


    {
        uint8_t m[2][4] = {{131,18,235,45},{176,51,57,55}};
        if (m[0][2] != 235) failures++;
    }


    {
        if (((uint16_t)(150 - (40 + 152))) != 65494) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)128) + (uint16_t)55323;
        if (r != 55451) failures++;
    }


    {
        if (((uint16_t)(239 + ((99 - 52) ^ (198 | 238)))) != 432) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        int8_t a = -8;
        int8_t b = 10;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 1436 + 41431 + 19390 + 53065 + 10753 + 17207 + 55318 + 11751;
        if (r != 13743) failures++;
    }


    {
        int8_t a = -111;
        int8_t b = 52;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 101;
        uint8_t r = port;
        if (r != 101) failures++;
    }


    {
        uint8_t x = 200;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {148,78,104,114,214,171,102,128};
        uint8_t *p = buf;
        p += 4;
        if (*p != 214) failures++;
    }


    {
        uint8_t v = 56;
        v |= 1;
        if (v != 57) failures++;
    }


    {
        uint16_t x = 86;
        x = x + 179;
        if (x != 265) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(127,54) != 181) failures++;
    }


    {
        uint8_t m[3][4] = {{198,230,186,196},{133,150,54,78},{48,47,2,149}};
        if (m[1][2] != 54) failures++;
    }


    {
        uint16_t r = call6(201,198,25,150,132,10);
        if (r != 716) failures++;
    }


    {
        int8_t a = -71;
        int8_t b = -51;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)89) % (int16_t)((int8_t)-101);
        if ((uint16_t)r != (uint16_t)89) failures++;
    }


    {
        volatile uint8_t port = 131;
        uint8_t r = port;
        if (r != 131) failures++;
    }


    {
        g16 = 65404;
        if (read_g16() != 65404) failures++;
    }


    {
        uint16_t r = 54438 + 4166 + 28022 + 65078 + 187 + 16733 + 16039 + 21449;
        if (r != 9504) failures++;
    }


    {
        uint8_t a[6] = {20,47,57,177,127,170};
        if (a[0] != 20) failures++;
    }


    {
        int8_t a = -32;
        int8_t b = -97;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 203;
        uint8_t r = port;
        if (r != 203) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)174) + (uint16_t)36930;
        if (r != 37104) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint16_t x = 7648;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 59495 + 10621 + 52117 + 14346 + 26528 + 38487 + 19972 + 41069;
        if (r != 491) failures++;
    }


    {
        uint8_t v = 212;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 12) failures++;
    }


    {
        volatile uint8_t port = 95;
        uint8_t r = port;
        if (r != 95) failures++;
    }


    {
        uint16_t x = 253;
        x = x + 148;
        if (x != 401) failures++;
    }


    {
        uint16_t x = 250;
        x = x + 150;
        if (x != 400) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-32) % (int16_t)((int8_t)-4);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 65;
        uint8_t r = port;
        if (r != 65) failures++;
    }


    {
        uint8_t v = 9;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 23) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)26) % (int16_t)((int8_t)123);
        if ((uint16_t)r != (uint16_t)26) failures++;
    }


    {
        uint16_t x = 4713;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(104,134) != 238) failures++;
    }


    {
        uint16_t r = 50613 + 53546 + 32130 + 29954 + 9968 + 26451 + 37747 + 23711;
        if (r != 1976) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 156; break;
        case 5: result = 217; break;
        case 15: result = 74; break;
        case 8: result = 190; break;
        case 13: result = 192; break;
        default: result = 238; break;
        }
        if (result != 156) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 17: result = 107; break;
        case 14: result = 90; break;
        case 15: result = 216; break;
        case 19: result = 117; break;
        case 1: result = 191; break;
        case 12: result = 255; break;
        case 2: result = 79; break;
        default: result = 70; break;
        }
        if (result != 117) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)175) + (uint16_t)8241;
        if (r != 8416) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 81;
        if (buf[3] != 81) failures++;
    }


    {
        uint16_t r = call6(52,201,110,23,192,21);
        if (r != 599) failures++;
    }


    {
        uint8_t v = 136;
        v ^= 128;
        if (v != 8) failures++;
    }


    {
        int8_t a = 71;
        int8_t b = -73;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)86) / (int16_t)((int8_t)118);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        if (((uint16_t)(((207 ^ 209) ^ (36 + 164)) & 34)) != 2) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint16_t r = add2(208,241) + add2(241,243) + add2(208,243);
        if (r != 1384) failures++;
    }


    {
        int8_t a = -63;
        int8_t b = -74;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 25411;
        volatile int16_t b = 27153;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {144,135,22388,100};
        if (s.b != (uint8_t)135) failures++;
    }


    {
        uint8_t buf[8] = {166,157,129,245,160,176,17,112};
        uint8_t *p = buf;
        p += 3;
        if (*p != 245) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {171,134,38233,102};
        if (s.b != (uint8_t)134) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)117) + (uint16_t)21778;
        if (r != 21895) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 183; break;
        case 11: result = 72; break;
        case 8: result = 200; break;
        case 18: result = 9; break;
        case 6: result = 45; break;
        case 17: result = 185; break;
        default: result = 139; break;
        }
        if (result != 183) failures++;
    }


    {
        uint8_t v = 113;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 15) failures++;
    }


    {
        uint16_t x = 62;
        x = x + 84;
        if (x != 146) failures++;
    }


    {
        uint16_t r = add2(89,33) + add2(33,136) + add2(89,136);
        if (r != 516) failures++;
    }


    {
        int8_t a = -36;
        int8_t b = -28;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 49987;
        if (read_g16() != 49987) failures++;
    }


    {
        uint8_t v = 4;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 12) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,123,18576,96};
        if (s.b != (uint8_t)123) failures++;
    }


    {
        uint32_t a = 2553294592UL;
        uint32_t b = 4067762112UL;
        uint32_t r = a & b;
        if (r != 2419068672UL) failures++;
    }


    {
        volatile uint8_t port = 229;
        uint8_t r = port;
        if (r != 229) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 60591731UL;
        uint32_t b = 1945181066UL;
        uint32_t r = a | b;
        if (r != 1946001403UL) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 14: result = 7; break;
        case 8: result = 47; break;
        case 11: result = 182; break;
        case 19: result = 117; break;
        default: result = 74; break;
        }
        if (result != 117) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-107) % (int16_t)((int8_t)71);
        if ((uint16_t)r != (uint16_t)65500) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(105,180) != 65461) failures++;
    }


    {
        uint32_t a = 3484091629UL;
        uint32_t b = 174775631UL;
        uint32_t r = a | b;
        if (r != 3488341487UL) failures++;
    }


    {
        uint16_t x = 18034;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint16_t r = call6(249,63,74,88,202,72);
        if (r != 748) failures++;
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
        for (uint16_t j = 0; j < 4; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint8_t x = 251;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        uint8_t v = 110;
        v |= 32;
        if (v != 110) failures++;
    }


    {
        uint8_t m[3][3] = {{224,176,253},{11,0,125},{225,19,242}};
        if (m[0][2] != 253) failures++;
    }


    {
        uint16_t r = add2(83,23) + add2(23,136) + add2(83,136);
        if (r != 484) failures++;
    }


    {
        volatile uint8_t port = 119;
        uint8_t r = port;
        if (r != 119) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 19: result = 217; break;
        case 6: result = 140; break;
        case 8: result = 86; break;
        case 11: result = 185; break;
        case 7: result = 84; break;
        default: result = 173; break;
        }
        if (result != 140) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {211,19,21521,1};
        if (s.a != (uint8_t)211) failures++;
    }


    {
        if (((uint16_t)(((55 - 4) + (61 & 105)) & ((86 - 20) - 55))) != 8) failures++;
    }


    {
        uint16_t x = 60619;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 26941 + 7106 + 30530 + 9975 + 30454 + 6594 + 11790 + 31249;
        if (r != 23567) failures++;
    }


    {
        if (((uint16_t)((140 & (149 | 17)) ^ 4)) != 128) failures++;
    }


    {
        uint8_t src[2] = {200,72};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 200) failures++;
    }


    {
        uint8_t x = 47;
        x <<= 3;
        if (x != 120) failures++;
    }


    {
        uint16_t r = 48719 + 12939 + 18066 + 16265 + 56497 + 58403 + 40442 + 49713;
        if (r != 38900) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 14;
        if (buf[14] != 14) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(17,149) != 166) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 247;
        if (buf[15] != 247) failures++;
    }


    {
        volatile uint8_t port = 199;
        uint8_t r = port;
        if (r != 199) failures++;
    }


    {
        volatile int16_t a = 24437;
        volatile int16_t b = 8204;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 3582675140UL;
        uint32_t b = 4175570723UL;
        uint32_t r = a & b;
        if (r != 3498180608UL) failures++;
    }


    {
        uint8_t buf[8] = {53,114,127,45,14,51,202,83};
        uint8_t *p = buf;
        p += 2;
        if (*p != 127) failures++;
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
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)97) / (int16_t)((int8_t)116);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint32_t a = 3534223354UL;
        uint32_t b = 3859288870UL;
        uint32_t r = a | b;
        if (r != 4138727422UL) failures++;
    }


    {
        uint32_t a = 2381536323UL;
        uint32_t b = 748212329UL;
        uint32_t r = a + b;
        if (r != 3129748652UL) failures++;
    }


    {
        uint16_t r = add2(240,51) + add2(51,87) + add2(240,87);
        if (r != 756) failures++;
    }


    {
        uint8_t a[6] = {21,113,214,15,89,91};
        if (a[3] != 15) failures++;
    }


    {
        volatile int16_t a = -3629;
        volatile int16_t b = -24170;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 18067;
        volatile int16_t b = -9819;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 168;
        uint8_t r = port;
        if (r != 168) failures++;
    }


    {
        uint8_t x = 38;
        x <<= 4;
        if (x != 96) failures++;
    }


    {
        volatile int16_t a = 14486;
        volatile int16_t b = -27369;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 21;
        if (buf[5] != 21) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 9;
        if (buf[14] != 9) failures++;
    }


    {
        if (((uint16_t)(((59 | 159) - (149 | 121)) | ((124 ^ 175) + (241 + 94)))) != 65506) failures++;
    }


    {
        uint16_t r = call6(181,59,75,57,223,96);
        if (r != 691) failures++;
    }


    {
        int8_t a = -58;
        int8_t b = 31;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -29785;
        volatile int16_t b = 18970;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {205,145,142,102,73,63,142,159};
        uint8_t *p = buf;
        p += 4;
        if (*p != 73) failures++;
    }


    {
        uint8_t v = 182;
        int r = (v & 4) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)93) / (int16_t)((int8_t)-118);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 199;
        uint8_t r = port;
        if (r != 199) failures++;
    }


    {
        uint8_t m[2][3] = {{192,138,117},{189,69,216}};
        if (m[0][2] != 117) failures++;
    }


    {
        uint8_t v = 19;
        v &= ~(uint8_t)64;
        if (v != 19) failures++;
    }


    {
        uint8_t buf[8] = {96,116,74,5,135,219,189,156};
        uint8_t *p = buf;
        p += 7;
        if (*p != 156) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(139,76) != 63) failures++;
    }


    {
        uint8_t v = 104;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-53) % (int16_t)((int8_t)-88);
        if ((uint16_t)r != (uint16_t)65483) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 5: result = 64; break;
        case 3: result = 88; break;
        case 9: result = 252; break;
        case 12: result = 39; break;
        default: result = 79; break;
        }
        if (result != 79) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t x = 120;
        x <<= 4;
        if (x != 128) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(2,206) != 65332) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)141) + (uint16_t)46839;
        if (r != 46980) failures++;
    }


    {
        uint16_t r = call6(120,48,25,216,203,58);
        if (r != 670) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)161) + (uint16_t)23981;
        if (r != 24142) failures++;
    }


    {
        uint16_t x = 232;
        x = x + 70;
        if (x != 302) failures++;
    }


    {
        uint8_t x = 195;
        x <<= 0;
        if (x != 195) failures++;
    }


    {
        uint8_t v = 233;
        v |= 32;
        if (v != 233) failures++;
    }


    {
        uint8_t buf[8] = {185,45,3,226,59,113,172,7};
        uint8_t *p = buf;
        p += 2;
        if (*p != 3) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 12: result = 67; break;
        case 8: result = 246; break;
        case 10: result = 208; break;
        default: result = 37; break;
        }
        if (result != 67) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {41,2,799,178};
        if (s.c != (uint16_t)799) failures++;
    }


    {
        uint8_t v = 69;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(167,144) + add2(144,243) + add2(167,243);
        if (r != 1108) failures++;
    }


    {
        uint8_t m[3][4] = {{14,214,175,57},{222,115,131,11},{152,166,142,98}};
        if (m[2][3] != 98) failures++;
    }


    {
        uint16_t x = 26;
        x = x + 187;
        if (x != 213) failures++;
    }


    {
        uint16_t x = 17443;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {202,251,36715,102};
        if (s.b != (uint8_t)251) failures++;
    }


    {
        uint8_t m[4][4] = {{99,9,73,187},{248,219,169,118},{119,140,236,116},{30,153,79,17}};
        if (m[1][0] != 248) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {220,158,50592,209};
        if (s.d != (uint8_t)209) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)48) % (int16_t)((int8_t)73);
        if ((uint16_t)r != (uint16_t)48) failures++;
    }


    {
        uint8_t v = 48;
        v ^= 8;
        if (v != 56) failures++;
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
        uint16_t r = 28167 + 56282 + 10751 + 45810 + 58123 + 44584 + 30535 + 21083;
        if (r != 33191) failures++;
    }


    {
        uint8_t x = 249;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(187,176) != 363) failures++;
    }


    {
        uint8_t m[3][4] = {{237,43,88,244},{103,83,211,173},{29,139,22,183}};
        if (m[0][0] != 237) failures++;
    }


    {
        uint8_t v = 110;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {202,101,38,49,217,169};
        if (a[4] != 217) failures++;
    }


    {
        uint16_t r = add2(52,23) + add2(23,84) + add2(52,84);
        if (r != 318) failures++;
    }


    {
        uint8_t v = 74;
        v &= ~(uint8_t)64;
        if (v != 10) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(142,9) + add2(9,190) + add2(142,190);
        if (r != 682) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)113) + (uint16_t)60013;
        if (r != 60126) failures++;
    }


    {
        uint8_t v = 42;
        v &= ~(uint8_t)128;
        if (v != 42) failures++;
    }


    {
        uint16_t x = 20846;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(195,160,177,118,94,242);
        if (r != 986) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {89,243,18943,76};
        if (s.c != (uint16_t)18943) failures++;
    }


    {
        uint8_t src[16] = {143,138,118,3,45,67,217,142,39,208,203,124,237,81,44,84};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[5] != 67) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-77) / (int16_t)((int8_t)65);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        volatile uint8_t port = 144;
        uint8_t r = port;
        if (r != 144) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 135;
        if (buf[4] != 135) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 2) sum += j;
        if (sum != 42) failures++;
    }


    {
        if (((uint16_t)(103 | ((34 & 36) ^ (31 | 68)))) != 127) failures++;
    }


    {
        uint16_t x = 75;
        x = x + 152;
        if (x != 227) failures++;
    }


    {
        int8_t a = -113;
        int8_t b = -116;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[3][3] = {{45,243,50},{85,14,28},{93,210,97}};
        if (m[2][0] != 93) failures++;
    }


    {
        uint8_t v = 191;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = add2(51,138) + add2(138,47) + add2(51,47);
        if (r != 472) failures++;
    }


    {
        uint32_t a = 968875708UL;
        uint32_t b = 939714079UL;
        uint32_t r = a + b;
        if (r != 1908589787UL) failures++;
    }


    {
        uint16_t r = call6(124,127,80,159,73,20);
        if (r != 583) failures++;
    }


    {
        uint8_t v = 172;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = 20146;
        volatile int16_t b = 19016;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 56;
        int8_t b = -107;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 162;
        uint8_t r = port;
        if (r != 162) failures++;
    }

    return failures;
}