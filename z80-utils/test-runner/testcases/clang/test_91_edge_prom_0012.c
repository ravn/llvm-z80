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
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 2;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(122,201) != 65457) failures++;
    }


    {
        uint16_t r = add2(165,108) + add2(108,84) + add2(165,84);
        if (r != 714) failures++;
    }


    {
        uint16_t r = call6(203,184,226,243,253,56);
        if (r != 1165) failures++;
    }


    {
        g16 = 59583;
        if (read_g16() != 59583) failures++;
    }


    {
        uint16_t x = 36;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(40,136,72,147,58,117);
        if (r != 570) failures++;
    }


    {
        volatile int16_t a = 8947;
        volatile int16_t b = -25223;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(206,112) + add2(112,30) + add2(206,30);
        if (r != 696) failures++;
    }


    {
        uint16_t x = 19754;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][3] = {{103,67,242},{53,223,210},{206,225,103}};
        if (m[1][2] != 210) failures++;
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
        uint8_t m[2][2] = {{40,29},{168,158}};
        if (m[1][1] != 158) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-62) % (int16_t)((int8_t)35);
        if ((uint16_t)r != (uint16_t)65509) failures++;
    }


    {
        uint16_t r = 28288 + 12983 + 41381 + 14132 + 1719 + 27204 + 47835 + 19469;
        if (r != 61939) failures++;
    }


    {
        uint16_t x = 62463;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 21;
        int8_t b = -120;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(206,100,161,112,29,171);
        if (r != 779) failures++;
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
        g16 = 62535;
        if (read_g16() != 62535) failures++;
    }


    {
        uint8_t v = 175;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 34755;
        if (read_g16() != 34755) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)123) / (int16_t)((int8_t)-15);
        if ((uint16_t)r != (uint16_t)65528) failures++;
    }


    {
        uint8_t a[6] = {14,92,83,89,61,80};
        if (a[4] != 61) failures++;
    }


    {
        uint8_t src[8] = {133,18,184,95,147,236,69,69};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[4] != 147) failures++;
    }


    {
        uint16_t x = 37366;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t src[5] = {79,183,228,5,83};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 228) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(217,90) != 307) failures++;
    }


    {
        uint32_t a = 587922048UL;
        uint32_t b = 2008665221UL;
        uint32_t r = a - b;
        if (r != 2874224123UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 98;
        x <<= 0;
        if (x != 98) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-43) % (int16_t)((int8_t)86);
        if ((uint16_t)r != (uint16_t)65493) failures++;
    }


    {
        uint8_t x = 248;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 14: result = 241; break;
        case 19: result = 229; break;
        case 7: result = 75; break;
        case 2: result = 237; break;
        default: result = 18; break;
        }
        if (result != 229) failures++;
    }


    {
        uint8_t src[9] = {88,143,183,53,149,169,4,82,83};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[4] != 149) failures++;
    }


    {
        uint8_t v = 238;
        v &= ~(uint8_t)2;
        if (v != 236) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {177,152,19358,177};
        if (s.a != (uint8_t)177) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 144;
        if (buf[5] != 144) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-45) % (int16_t)((int8_t)5);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        if (((uint16_t)(((185 | 0) ^ (108 ^ 209)) - 96)) != 65444) failures++;
    }


    {
        uint8_t a[6] = {131,0,247,138,34,60};
        if (a[2] != 247) failures++;
    }


    {
        uint16_t r = 60678 + 16010 + 36355 + 63883 + 12235 + 51815 + 34925 + 6134;
        if (r != 19891) failures++;
    }


    {
        volatile uint8_t port = 40;
        uint8_t r = port;
        if (r != 40) failures++;
    }


    {
        uint16_t r = call6(197,174,74,155,93,183);
        if (r != 876) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 5: result = 238; break;
        case 14: result = 121; break;
        case 9: result = 84; break;
        default: result = 250; break;
        }
        if (result != 121) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(139,173) != 312) failures++;
    }


    {
        uint16_t r = call6(192,137,72,145,211,203);
        if (r != 960) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {162,218,9815,61};
        if (s.c != (uint16_t)9815) failures++;
    }


    {
        volatile uint8_t port = 38;
        uint8_t r = port;
        if (r != 38) failures++;
    }


    {
        int8_t a = -64;
        int8_t b = 48;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 47618;
        if (read_g16() != 47618) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[3] = {194,215,142};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 142) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        volatile int16_t a = -13321;
        volatile int16_t b = -9597;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(84,209) + add2(209,222) + add2(84,222);
        if (r != 1030) failures++;
    }


    {
        uint16_t x = 2555;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {204,186,106,209,110,192,169,1};
        uint8_t *p = buf;
        p += 4;
        if (*p != 110) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 14: result = 50; break;
        case 6: result = 222; break;
        case 1: result = 51; break;
        case 2: result = 148; break;
        case 16: result = 178; break;
        case 12: result = 125; break;
        case 11: result = 189; break;
        case 5: result = 43; break;
        default: result = 201; break;
        }
        if (result != 125) failures++;
    }


    {
        if (((uint16_t)((219 ^ (179 + 244)) ^ (102 & 201))) != 316) failures++;
    }


    {
        uint16_t x = 116;
        x = x + 129;
        if (x != 245) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 11: result = 15; break;
        case 0: result = 188; break;
        case 19: result = 17; break;
        case 15: result = 211; break;
        case 2: result = 64; break;
        case 4: result = 148; break;
        default: result = 81; break;
        }
        if (result != 64) failures++;
    }


    {
        int8_t a = 26;
        int8_t b = -51;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 45679 + 57030 + 61978 + 35792 + 54015 + 38721 + 26116 + 57821;
        if (r != 49472) failures++;
    }


    {
        uint16_t r = call6(160,71,171,40,128,214);
        if (r != 784) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
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
        if (fn(53,158) != 211) failures++;
    }


    {
        uint8_t x = 207;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint16_t r = call6(167,108,160,22,128,35);
        if (r != 620) failures++;
    }


    {
        g16 = 50075;
        if (read_g16() != 50075) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 1: result = 2; break;
        case 6: result = 11; break;
        case 11: result = 138; break;
        case 8: result = 186; break;
        case 15: result = 184; break;
        default: result = 162; break;
        }
        if (result != 138) failures++;
    }


    {
        uint16_t x = 158;
        x = x + 12;
        if (x != 170) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 6: result = 54; break;
        case 10: result = 198; break;
        case 18: result = 253; break;
        case 8: result = 26; break;
        case 7: result = 180; break;
        case 1: result = 148; break;
        default: result = 71; break;
        }
        if (result != 26) failures++;
    }


    {
        uint16_t x = 238;
        x = x + 129;
        if (x != 367) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {135,114,65193,19};
        if (s.b != (uint8_t)114) failures++;
    }


    {
        uint16_t r = call6(133,66,215,0,32,1);
        if (r != 447) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-69) % (int16_t)((int8_t)124);
        if ((uint16_t)r != (uint16_t)65467) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-122) % (int16_t)((int8_t)-31);
        if ((uint16_t)r != (uint16_t)65507) failures++;
    }


    {
        uint8_t src[6] = {187,69,146,254,239,133};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[0] != 187) failures++;
    }


    {
        volatile uint8_t port = 202;
        uint8_t r = port;
        if (r != 202) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(250,221) != 29) failures++;
    }


    {
        int8_t a = 27;
        int8_t b = -78;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)5) + (uint16_t)63596;
        if (r != 63601) failures++;
    }


    {
        uint32_t a = 1511594316UL;
        uint32_t b = 686716417UL;
        uint32_t r = a & b;
        if (r != 134746112UL) failures++;
    }


    {
        volatile int16_t a = 23038;
        volatile int16_t b = 13087;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)49) + (uint16_t)55651;
        if (r != 55700) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 13: result = 149; break;
        case 5: result = 144; break;
        case 8: result = 87; break;
        case 9: result = 33; break;
        default: result = 50; break;
        }
        if (result != 33) failures++;
    }


    {
        uint8_t src[2] = {148,179};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 179) failures++;
    }


    {
        uint8_t v = 106;
        v |= 16;
        if (v != 122) failures++;
    }


    {
        uint16_t r = add2(92,78) + add2(78,108) + add2(92,108);
        if (r != 556) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t a[6] = {105,6,14,102,189,167};
        if (a[4] != 189) failures++;
    }


    {
        uint8_t buf[8] = {145,98,18,184,25,249,145,232};
        uint8_t *p = buf;
        p += 3;
        if (*p != 184) failures++;
    }


    {
        uint8_t a[6] = {218,224,167,13,71,224};
        if (a[1] != 224) failures++;
    }


    {
        uint8_t v = 96;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {60,146,59028,99};
        if (s.c != (uint16_t)59028) failures++;
    }


    {
        volatile int16_t a = -20532;
        volatile int16_t b = -23297;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[2][3] = {{254,17,206},{111,162,88}};
        if (m[1][1] != 162) failures++;
    }


    {
        uint16_t r = 24969 + 46473 + 6071 + 5710 + 55681 + 37284 + 9683 + 16368;
        if (r != 5631) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)93) / (int16_t)((int8_t)97);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(90,53) != 143) failures++;
    }


    {
        uint16_t x = 59811;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 1830120990UL;
        uint32_t b = 3564172627UL;
        uint32_t r = a | b;
        if (r != 4252368735UL) failures++;
    }


    {
        uint16_t r = call6(230,151,171,0,30,162);
        if (r != 744) failures++;
    }


    {
        uint16_t x = 82;
        x = x + 19;
        if (x != 101) failures++;
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
        for (uint8_t j = 0; j < 1; j++) buf[j] = 172;
        if (buf[0] != 172) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(71,170) != 65437) failures++;
    }


    {
        uint16_t r = 14122 + 34937 + 8261 + 10034 + 40920 + 10830 + 24335 + 61410;
        if (r != 8241) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)198) + (uint16_t)50819;
        if (r != 51017) failures++;
    }


    {
        volatile uint8_t port = 146;
        uint8_t r = port;
        if (r != 146) failures++;
    }


    {
        int8_t a = 103;
        int8_t b = 3;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {240,252,56,4,139,50};
        if (a[4] != 139) failures++;
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
        if (((uint16_t)(97 - ((217 + 238) + (14 ^ 130)))) != 65038) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)3) / (int16_t)((int8_t)-122);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 64337 + 42591 + 30864 + 23237 + 7295 + 30615 + 40017 + 7969;
        if (r != 50317) failures++;
    }


    {
        uint32_t a = 223585064UL;
        uint32_t b = 3171860933UL;
        uint32_t r = a & b;
        if (r != 218267904UL) failures++;
    }


    {
        uint8_t buf[8] = {10,88,138,121,26,88,177,29};
        uint8_t *p = buf;
        p += 5;
        if (*p != 88) failures++;
    }


    {
        uint8_t x = 10;
        x <<= 4;
        if (x != 160) failures++;
    }


    {
        g16 = 35501;
        if (read_g16() != 35501) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)47) + (uint16_t)33823;
        if (r != 33870) failures++;
    }


    {
        uint16_t x = 13840;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -13036;
        volatile int16_t b = -31448;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(((70 + 24) & (135 + 130)) + ((99 | 237) + (142 - 176)))) != 213) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 191;
        if (buf[10] != 191) failures++;
    }


    {
        uint8_t x = 52;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t v = 69;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 6887;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[12] = {84,109,76,188,95,9,223,4,125,86,145,62};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[2] != 76) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {136,199,44,211,117,60};
        if (a[1] != 199) failures++;
    }


    {
        uint16_t r = add2(105,171) + add2(171,161) + add2(105,161);
        if (r != 874) failures++;
    }


    {
        uint32_t a = 3502402425UL;
        uint32_t b = 2797614896UL;
        uint32_t r = a & b;
        if (r != 2160075568UL) failures++;
    }


    {
        uint8_t v = 179;
        v |= 1;
        if (v != 179) failures++;
    }


    {
        uint8_t src[16] = {104,251,115,21,52,172,80,202,7,64,38,5,87,218,38,104};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[7] != 202) failures++;
    }


    {
        uint8_t src[12] = {111,43,173,64,79,139,97,144,152,160,150,253};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[7] != 144) failures++;
    }


    {
        uint8_t buf[8] = {140,210,104,66,209,21,124,110};
        uint8_t *p = buf;
        p += 6;
        if (*p != 124) failures++;
    }


    {
        uint16_t r = add2(228,113) + add2(113,11) + add2(228,11);
        if (r != 704) failures++;
    }


    {
        uint16_t r = call6(163,71,65,96,158,159);
        if (r != 712) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint16_t x = 30;
        x = x + 110;
        if (x != 140) failures++;
    }


    {
        uint16_t r = call6(241,88,249,164,70,113);
        if (r != 925) failures++;
    }


    {
        int8_t a = -39;
        int8_t b = -65;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 29275 + 21023 + 62415 + 14055 + 55636 + 22643 + 643 + 35320;
        if (r != 44402) failures++;
    }


    {
        uint8_t a[6] = {110,54,61,102,42,233};
        if (a[0] != 110) failures++;
    }


    {
        g16 = 11309;
        if (read_g16() != 11309) failures++;
    }


    {
        uint8_t src[7] = {197,49,186,111,123,190,216};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[1] != 49) failures++;
    }


    {
        uint8_t v = 35;
        v |= 2;
        if (v != 35) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        volatile uint8_t port = 6;
        uint8_t r = port;
        if (r != 6) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {249,14,16218,4};
        if (s.b != (uint8_t)14) failures++;
    }


    {
        uint8_t buf[8] = {176,155,16,97,93,37,16,242};
        uint8_t *p = buf;
        p += 5;
        if (*p != 37) failures++;
    }


    {
        uint16_t x = 13803;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-70) % (int16_t)((int8_t)-57);
        if ((uint16_t)r != (uint16_t)65523) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 16: result = 100; break;
        case 12: result = 207; break;
        case 13: result = 232; break;
        case 7: result = 4; break;
        case 11: result = 226; break;
        case 1: result = 154; break;
        case 19: result = 244; break;
        case 15: result = 1; break;
        default: result = 170; break;
        }
        if (result != 226) failures++;
    }


    {
        uint8_t buf[8] = {53,20,153,13,151,55,163,28};
        uint8_t *p = buf;
        p += 5;
        if (*p != 55) failures++;
    }


    {
        int8_t a = -72;
        int8_t b = 84;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {67,78,65,43,214,125,67,137};
        uint8_t *p = buf;
        p += 7;
        if (*p != 137) failures++;
    }


    {
        uint32_t a = 2315112931UL;
        uint32_t b = 1068313059UL;
        uint32_t r = a ^ b;
        if (r != 3058759680UL) failures++;
    }


    {
        uint8_t src[10] = {160,141,120,243,163,199,128,193,44,72};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[4] != 163) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {207,243,59102,124};
        if (s.d != (uint8_t)124) failures++;
    }


    {
        uint16_t x = 172;
        x = x + 74;
        if (x != 246) failures++;
    }


    {
        if (((uint16_t)((224 ^ (15 - 48)) & 52)) != 52) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(158,73) != 85) failures++;
    }


    {
        uint8_t a[6] = {173,194,136,66,107,183};
        if (a[3] != 66) failures++;
    }


    {
        uint16_t r = 17664 + 60270 + 38895 + 18877 + 60970 + 19223 + 28111 + 58937;
        if (r != 40803) failures++;
    }


    {
        volatile uint8_t port = 207;
        uint8_t r = port;
        if (r != 207) failures++;
    }


    {
        uint32_t a = 2765188759UL;
        uint32_t b = 1703806851UL;
        uint32_t r = a ^ b;
        if (r != 3244255508UL) failures++;
    }


    {
        uint16_t r = call6(45,160,183,88,145,31);
        if (r != 652) failures++;
    }


    {
        g16 = 42209;
        if (read_g16() != 42209) failures++;
    }


    {
        volatile int16_t a = -8313;
        volatile int16_t b = 6106;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(199,242,225,36,71,225);
        if (r != 998) failures++;
    }


    {
        volatile uint8_t port = 88;
        uint8_t r = port;
        if (r != 88) failures++;
    }


    {
        int8_t a = -55;
        int8_t b = -92;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 41322 + 58290 + 18131 + 6570 + 34840 + 41245 + 53068 + 16098;
        if (r != 7420) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 23;
        if (buf[10] != 23) failures++;
    }


    {
        int8_t a = -35;
        int8_t b = -91;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t x = 146;
        x = x + 250;
        if (x != 396) failures++;
    }


    {
        uint8_t v = 182;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = add2(71,87) + add2(87,146) + add2(71,146);
        if (r != 608) failures++;
    }


    {
        uint16_t r = add2(158,17) + add2(17,5) + add2(158,5);
        if (r != 360) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(146,7) != 139) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 3;
        if (buf[1] != 3) failures++;
    }


    {
        volatile int16_t a = -29875;
        volatile int16_t b = 12074;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t v = 123;
        v ^= 128;
        if (v != 251) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(72,88) != 65520) failures++;
    }


    {
        uint16_t r = call6(164,45,49,121,182,70);
        if (r != 631) failures++;
    }


    {
        uint16_t r = 22622 + 37600 + 50536 + 47660 + 11119 + 22025 + 30117 + 18228;
        if (r != 43299) failures++;
    }


    {
        uint16_t r = call6(156,4,94,54,19,208);
        if (r != 535) failures++;
    }


    {
        uint16_t r = add2(202,131) + add2(131,39) + add2(202,39);
        if (r != 744) failures++;
    }


    {
        uint8_t a[6] = {112,8,242,45,180,222};
        if (a[5] != 222) failures++;
    }


    {
        uint32_t a = 2821994527UL;
        uint32_t b = 3618672469UL;
        uint32_t r = a | b;
        if (r != 4290035551UL) failures++;
    }


    {
        uint8_t a[6] = {223,170,183,115,126,108};
        if (a[0] != 223) failures++;
    }


    {
        uint16_t r = call6(99,29,22,186,127,46);
        if (r != 509) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {56,242,44533,164};
        if (s.d != (uint8_t)164) failures++;
    }


    {
        uint8_t v = 162;
        v ^= 64;
        if (v != 226) failures++;
    }


    {
        volatile int16_t a = 11159;
        volatile int16_t b = -6190;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 101;
        uint8_t r = port;
        if (r != 101) failures++;
    }


    {
        uint16_t r = add2(169,102) + add2(102,236) + add2(169,236);
        if (r != 1014) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-33) % (int16_t)((int8_t)-115);
        if ((uint16_t)r != (uint16_t)65503) failures++;
    }


    {
        uint16_t r = add2(36,220) + add2(220,95) + add2(36,95);
        if (r != 702) failures++;
    }


    {
        uint8_t x = 238;
        x <<= 0;
        if (x != 238) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 209;
        v &= ~(uint8_t)64;
        if (v != 145) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 5: result = 179; break;
        case 13: result = 75; break;
        case 15: result = 149; break;
        default: result = 177; break;
        }
        if (result != 149) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {138,41,52707,158};
        if (s.a != (uint8_t)138) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 25;
        if (buf[7] != 25) failures++;
    }


    {
        uint16_t r = 29141 + 38891 + 25474 + 62276 + 11844 + 40589 + 24897 + 55393;
        if (r != 26361) failures++;
    }


    {
        if (((uint16_t)(((166 + 132) | (196 - 109)) & ((120 | 112) ^ (243 ^ 113)))) != 122) failures++;
    }


    {
        if (((uint16_t)(((73 | 131) & (108 - 116)) ^ (117 - 252))) != 65457) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 19: result = 128; break;
        case 7: result = 64; break;
        case 14: result = 62; break;
        case 5: result = 124; break;
        case 13: result = 195; break;
        case 17: result = 62; break;
        case 9: result = 31; break;
        default: result = 77; break;
        }
        if (result != 62) failures++;
    }


    {
        g16 = 57509;
        if (read_g16() != 57509) failures++;
    }


    {
        uint8_t x = 183;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint16_t r = 12602 + 4620 + 65497 + 15915 + 19883 + 8882 + 8800 + 12692;
        if (r != 17819) failures++;
    }


    {
        uint16_t x = 58475;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {183,214,21795,140};
        if (s.c != (uint16_t)21795) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)43) % (int16_t)((int8_t)78);
        if ((uint16_t)r != (uint16_t)43) failures++;
    }


    {
        uint8_t m[2][3] = {{35,29,187},{50,26,167}};
        if (m[0][1] != 29) failures++;
    }


    {
        uint8_t v = 144;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint8_t x = 91;
        x <<= 0;
        if (x != 91) failures++;
    }


    {
        uint32_t a = 2054579549UL;
        uint32_t b = 2433649162UL;
        uint32_t r = a ^ b;
        if (r != 3950571351UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(53,248) != 301) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        if (((uint16_t)4) != 4) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t m[3][2] = {{246,143},{100,178},{254,174}};
        if (m[1][1] != 178) failures++;
    }


    {
        uint8_t v = 17;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 111) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        volatile int16_t a = -17146;
        volatile int16_t b = -29555;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -18515;
        volatile int16_t b = -31141;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -26;
        int8_t b = -69;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {3,19,242,33,181,156};
        if (a[5] != 156) failures++;
    }


    {
        uint8_t m[2][2] = {{179,81},{72,80}};
        if (m[0][0] != 179) failures++;
    }


    {
        uint16_t x = 117;
        x = x + 65;
        if (x != 182) failures++;
    }


    {
        uint32_t a = 198309785UL;
        uint32_t b = 3128071921UL;
        uint32_t r = a ^ b;
        if (r != 2980274536UL) failures++;
    }


    {
        uint8_t a[6] = {74,36,235,235,68,66};
        if (a[4] != 68) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)17) % (int16_t)((int8_t)121);
        if ((uint16_t)r != (uint16_t)17) failures++;
    }


    {
        uint16_t x = 106;
        x = x + 216;
        if (x != 322) failures++;
    }


    {
        uint8_t x = 50;
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
        uint32_t a = 183997470UL;
        uint32_t b = 3564590720UL;
        uint32_t r = a & b;
        if (r != 7803904UL) failures++;
    }


    {
        int8_t a = 21;
        int8_t b = -84;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 1971363438UL;
        uint32_t b = 4266433740UL;
        uint32_t r = a | b;
        if (r != 4291600110UL) failures++;
    }


    {
        uint8_t buf[8] = {162,57,90,165,233,246,90,100};
        uint8_t *p = buf;
        p += 2;
        if (*p != 90) failures++;
    }


    {
        uint8_t v = 10;
        v ^= 64;
        if (v != 74) failures++;
    }


    {
        uint8_t m[4][4] = {{157,226,108,232},{55,251,27,2},{131,196,148,71},{5,252,21,170}};
        if (m[2][1] != 196) failures++;
    }


    {
        uint8_t x = 137;
        x <<= 3;
        if (x != 72) failures++;
    }


    {
        uint16_t r = 59568 + 58662 + 45013 + 5237 + 39892 + 46311 + 36785 + 1844;
        if (r != 31168) failures++;
    }


    {
        volatile uint8_t port = 186;
        uint8_t r = port;
        if (r != 186) failures++;
    }


    {
        uint16_t x = 60833;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 116;
        x = x + 246;
        if (x != 362) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {193,80,55211,31};
        if (s.b != (uint8_t)80) failures++;
    }


    {
        uint8_t v = 96;
        v &= ~(uint8_t)64;
        if (v != 32) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)208) + (uint16_t)13489;
        if (r != 13697) failures++;
    }


    {
        uint16_t x = 61868;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)151) + (uint16_t)61701;
        if (r != 61852) failures++;
    }


    {
        uint8_t x = 144;
        x <<= 3;
        if (x != 128) failures++;
    }


    {
        volatile int16_t a = 22627;
        volatile int16_t b = -3415;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {65,71,4,41,63,90};
        if (a[0] != 65) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 180; break;
        case 7: result = 196; break;
        case 0: result = 58; break;
        default: result = 3; break;
        }
        if (result != 180) failures++;
    }


    {
        uint8_t buf[8] = {117,169,0,14,215,211,137,44};
        uint8_t *p = buf;
        p += 1;
        if (*p != 169) failures++;
    }


    {
        uint8_t m[4][2] = {{70,49},{147,223},{151,55},{181,163}};
        if (m[2][1] != 55) failures++;
    }


    {
        uint16_t r = add2(201,216) + add2(216,232) + add2(201,232);
        if (r != 1298) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)127) / (int16_t)((int8_t)-46);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 214;
        if (buf[2] != 214) failures++;
    }


    {
        uint8_t a[6] = {241,226,20,47,254,91};
        if (a[3] != 47) failures++;
    }


    {
        uint8_t x = 54;
        x <<= 0;
        if (x != 54) failures++;
    }


    {
        uint16_t r = 3142 + 32368 + 23170 + 30567 + 10239 + 35862 + 4973 + 64849;
        if (r != 8562) failures++;
    }


    {
        volatile uint8_t port = 162;
        uint8_t r = port;
        if (r != 162) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(45,172) != 217) failures++;
    }


    {
        uint8_t v = 88;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{122,178,147},{148,7,193},{83,238,240},{32,13,195}};
        if (m[3][1] != 13) failures++;
    }


    {
        uint8_t buf[8] = {91,199,162,79,223,46,93,253};
        uint8_t *p = buf;
        p += 6;
        if (*p != 93) failures++;
    }


    {
        uint16_t r = call6(152,131,154,82,123,63);
        if (r != 705) failures++;
    }


    {
        uint8_t src[5] = {99,151,237,0,132};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 0) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 197;
        if (buf[4] != 197) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)12) / (int16_t)((int8_t)-114);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 60766 + 30591 + 22795 + 14828 + 53389 + 39783 + 17478 + 42583;
        if (r != 20069) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)93) / (int16_t)((int8_t)-7);
        if ((uint16_t)r != (uint16_t)65523) failures++;
    }


    {
        uint8_t v = 162;
        v &= ~(uint8_t)8;
        if (v != 162) failures++;
    }


    {
        uint16_t r = call6(158,186,98,254,234,210);
        if (r != 1140) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 6: result = 14; break;
        case 14: result = 220; break;
        case 2: result = 222; break;
        case 0: result = 200; break;
        case 9: result = 99; break;
        default: result = 38; break;
        }
        if (result != 222) failures++;
    }


    {
        uint16_t r = call6(174,199,58,85,122,127);
        if (r != 765) failures++;
    }


    {
        uint16_t r = 35218 + 24730 + 59374 + 7558 + 7424 + 16667 + 3945 + 43872;
        if (r != 2180) failures++;
    }


    {
        uint8_t v = 57;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t r = 21685 + 5103 + 48230 + 46958 + 2579 + 40078 + 65178 + 39693;
        if (r != 7360) failures++;
    }


    {
        uint16_t x = 55052;
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
        case 9: result = 93; break;
        case 5: result = 170; break;
        case 0: result = 123; break;
        case 8: result = 85; break;
        case 13: result = 250; break;
        case 18: result = 178; break;
        case 1: result = 183; break;
        case 15: result = 122; break;
        default: result = 40; break;
        }
        if (result != 122) failures++;
    }


    {
        uint32_t a = 1467791309UL;
        uint32_t b = 575765756UL;
        uint32_t r = a ^ b;
        if (r != 1965935409UL) failures++;
    }


    {
        volatile uint8_t port = 164;
        uint8_t r = port;
        if (r != 164) failures++;
    }


    {
        uint8_t src[15] = {184,83,39,48,9,193,8,236,229,156,22,201,82,193,171};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[3] != 48) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {159,14,29258,25};
        if (s.d != (uint8_t)25) failures++;
    }


    {
        g16 = 55917;
        if (read_g16() != 55917) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {178,105,39360,71};
        if (s.c != (uint16_t)39360) failures++;
    }


    {
        uint8_t v = 119;
        v &= ~(uint8_t)8;
        if (v != 119) failures++;
    }


    {
        uint8_t src[6] = {105,214,4,51,45,115};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[3] != 51) failures++;
    }


    {
        uint16_t r = 22425 + 23130 + 65079 + 26856 + 32315 + 22045 + 59605 + 10181;
        if (r != 65028) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint8_t v = 38;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 90) failures++;
    }


    {
        int8_t a = 49;
        int8_t b = 2;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[7] = {251,24,114,17,172,136,243};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[1] != 24) failures++;
    }


    {
        uint8_t v = 127;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
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
        uint16_t x = 184;
        x = x + 101;
        if (x != 285) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint32_t a = 3974315426UL;
        uint32_t b = 85015950UL;
        uint32_t r = a & b;
        if (r != 67190146UL) failures++;
    }


    {
        uint8_t src[15] = {38,88,253,247,168,57,110,31,213,236,105,180,27,2,6};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[9] != 236) failures++;
    }


    {
        uint32_t a = 677196892UL;
        uint32_t b = 2335787934UL;
        uint32_t r = a | b;
        if (r != 2877128670UL) failures++;
    }


    {
        int8_t a = -18;
        int8_t b = -65;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {39,250,126,138,50,101};
        if (a[4] != 50) failures++;
    }


    {
        uint8_t m[4][3] = {{30,169,170},{252,42,243},{214,18,121},{35,29,110}};
        if (m[1][1] != 42) failures++;
    }


    {
        uint16_t r = add2(253,51) + add2(51,142) + add2(253,142);
        if (r != 892) failures++;
    }


    {
        int8_t a = 32;
        int8_t b = 84;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(177,196) != 373) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 142;
        x = x + 134;
        if (x != 276) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t buf[8] = {143,115,228,198,57,185,109,68};
        uint8_t *p = buf;
        p += 7;
        if (*p != 68) failures++;
    }


    {
        uint16_t r = 55033 + 46394 + 19416 + 16153 + 50645 + 25397 + 28456 + 31891;
        if (r != 11241) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)27) / (int16_t)((int8_t)106);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        g16 = 42066;
        if (read_g16() != 42066) failures++;
    }


    {
        uint16_t x = 54905;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 130;
        uint8_t r = port;
        if (r != 130) failures++;
    }


    {
        uint8_t m[4][3] = {{25,192,87},{139,208,136},{164,159,208},{136,188,211}};
        if (m[2][0] != 164) failures++;
    }


    {
        uint32_t a = 1958058346UL;
        uint32_t b = 1454124105UL;
        uint32_t r = a | b;
        if (r != 1992146283UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)153) + (uint16_t)9864;
        if (r != 10017) failures++;
    }


    {
        uint8_t a[6] = {78,51,21,114,81,138};
        if (a[2] != 21) failures++;
    }


    {
        uint8_t v = 139;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 16: result = 230; break;
        case 18: result = 70; break;
        case 3: result = 230; break;
        case 19: result = 56; break;
        case 17: result = 63; break;
        default: result = 127; break;
        }
        if (result != 230) failures++;
    }


    {
        if (((uint16_t)(86 ^ ((223 + 117) + (101 | 107)))) != 405) failures++;
    }


    {
        uint8_t x = 244;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint8_t buf[8] = {69,116,193,77,121,246,98,224};
        uint8_t *p = buf;
        p += 0;
        if (*p != 69) failures++;
    }


    {
        volatile uint8_t port = 253;
        uint8_t r = port;
        if (r != 253) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 7: result = 62; break;
        case 12: result = 223; break;
        case 10: result = 174; break;
        default: result = 126; break;
        }
        if (result != 126) failures++;
    }


    {
        uint8_t m[4][4] = {{157,192,124,139},{191,148,9,196},{151,228,79,186},{125,44,243,183}};
        if (m[0][3] != 139) failures++;
    }


    {
        uint16_t x = 32;
        x = x + 238;
        if (x != 270) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 183;
        if (buf[5] != 183) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(248,25) != 273) failures++;
    }


    {
        volatile int16_t a = -11690;
        volatile int16_t b = -3185;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 61;
        x <<= 0;
        if (x != 61) failures++;
    }


    {
        uint8_t x = 192;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t a[6] = {26,23,204,172,39,224};
        if (a[5] != 224) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)118) + (uint16_t)42159;
        if (r != 42277) failures++;
    }


    {
        uint16_t r = call6(176,250,167,27,70,48);
        if (r != 738) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {163,191,13116,6};
        if (s.d != (uint8_t)6) failures++;
    }


    {
        uint8_t a[6] = {79,35,201,158,132,3};
        if (a[3] != 158) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {96,255,63555,81};
        if (s.a != (uint8_t)96) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t m[3][2] = {{66,168},{115,7},{56,40}};
        if (m[1][0] != 115) failures++;
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
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(160,10) != 170) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(113,107) != 220) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 3: result = 33; break;
        case 0: result = 96; break;
        case 4: result = 137; break;
        case 18: result = 148; break;
        default: result = 143; break;
        }
        if (result != 137) failures++;
    }


    {
        g16 = 25765;
        if (read_g16() != 25765) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 46427 + 38847 + 18482 + 21335 + 173 + 23694 + 20103 + 44551;
        if (r != 17004) failures++;
    }


    {
        uint8_t src[12] = {150,249,218,153,238,13,89,92,72,195,126,104};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[8] != 72) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 86;
        int8_t b = 86;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)111) + (uint16_t)38114;
        if (r != 38225) failures++;
    }


    {
        uint8_t v = 75;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 21) failures++;
    }


    {
        uint16_t x = 38877;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(((183 + 181) & (5 + 184)) ^ ((17 + 161) - (39 & 117)))) != 161) failures++;
    }


    {
        uint8_t v = 178;
        v |= 8;
        if (v != 186) failures++;
    }


    {
        uint8_t a[6] = {26,38,152,231,8,179};
        if (a[4] != 8) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(97,81) != 178) failures++;
    }


    {
        uint32_t a = 2414909953UL;
        uint32_t b = 3257545061UL;
        uint32_t r = a + b;
        if (r != 1377487718UL) failures++;
    }


    {
        uint8_t a[6] = {191,105,59,198,240,110};
        if (a[3] != 198) failures++;
    }


    {
        uint16_t r = add2(68,85) + add2(85,231) + add2(68,231);
        if (r != 768) failures++;
    }


    {
        uint16_t x = 13726;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 62;
        x <<= 3;
        if (x != 240) failures++;
    }


    {
        uint8_t buf[8] = {25,13,237,94,88,183,86,16};
        uint8_t *p = buf;
        p += 1;
        if (*p != 13) failures++;
    }


    {
        uint16_t r = 47181 + 55245 + 14473 + 21498 + 34639 + 35469 + 12002 + 35156;
        if (r != 59055) failures++;
    }


    {
        volatile uint8_t port = 74;
        uint8_t r = port;
        if (r != 74) failures++;
    }


    {
        uint8_t src[16] = {164,162,31,94,157,160,173,165,108,183,126,243,46,10,74,101};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[8] != 108) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 13460 + 58377 + 31101 + 16185 + 59517 + 8484 + 33 + 40297;
        if (r != 30846) failures++;
    }


    {
        uint8_t v = 6;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 52;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(144,231) != 65449) failures++;
    }


    {
        int8_t a = 38;
        int8_t b = -16;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {89,42,201,236,35,182,91,234};
        uint8_t *p = buf;
        p += 5;
        if (*p != 182) failures++;
    }


    {
        uint16_t x = 45244;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(104,60,241,237,163,225);
        if (r != 1030) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 12: result = 0; break;
        case 2: result = 122; break;
        case 13: result = 5; break;
        case 7: result = 159; break;
        case 6: result = 229; break;
        default: result = 121; break;
        }
        if (result != 122) failures++;
    }


    {
        uint8_t buf[8] = {202,17,227,34,236,132,177,117};
        uint8_t *p = buf;
        p += 5;
        if (*p != 132) failures++;
    }


    {
        int8_t a = -60;
        int8_t b = 108;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 29;
        x <<= 1;
        if (x != 58) failures++;
    }


    {
        uint16_t r = 25462 + 89 + 39524 + 48107 + 39983 + 44695 + 639 + 3734;
        if (r != 5625) failures++;
    }


    {
        uint8_t m[2][2] = {{4,48},{241,40}};
        if (m[1][1] != 40) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-30) / (int16_t)((int8_t)-18);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t m[4][3] = {{9,1,161},{121,238,150},{45,142,243},{99,188,33}};
        if (m[0][1] != 1) failures++;
    }


    {
        volatile uint8_t port = 84;
        uint8_t r = port;
        if (r != 84) failures++;
    }


    {
        volatile int16_t a = -18536;
        volatile int16_t b = 22809;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 184;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][4] = {{135,40,178,248},{45,15,206,125}};
        if (m[0][3] != 248) failures++;
    }


    {
        g16 = 62625;
        if (read_g16() != 62625) failures++;
    }


    {
        g16 = 40176;
        if (read_g16() != 40176) failures++;
    }


    {
        uint16_t r = add2(150,12) + add2(12,74) + add2(150,74);
        if (r != 472) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(143,84) != 227) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 28;
        if (buf[12] != 28) failures++;
    }


    {
        uint8_t v = 146;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {188,200,44309,45};
        if (s.a != (uint8_t)188) failures++;
    }


    {
        uint8_t a[6] = {107,72,252,16,112,209};
        if (a[1] != 72) failures++;
    }


    {
        uint8_t v = 19;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t v = 163;
        v &= ~(uint8_t)4;
        if (v != 163) failures++;
    }


    {
        uint8_t v = 104;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t x = 229;
        x = x + 24;
        if (x != 253) failures++;
    }


    {
        int8_t a = -14;
        int8_t b = -113;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -61;
        int8_t b = 71;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 2688671231UL;
        uint32_t b = 2600895383UL;
        uint32_t r = a + b;
        if (r != 994599318UL) failures++;
    }


    {
        uint8_t v = 253;
        v |= 64;
        if (v != 253) failures++;
    }


    {
        uint16_t r = 22824 + 1104 + 47900 + 63460 + 6768 + 39662 + 62733 + 3042;
        if (r != 50885) failures++;
    }


    {
        uint16_t x = 213;
        x = x + 89;
        if (x != 302) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)235) + (uint16_t)17499;
        if (r != 17734) failures++;
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
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint16_t r = 5759 + 13533 + 17484 + 11526 + 41512 + 58498 + 51984 + 40854;
        if (r != 44542) failures++;
    }


    {
        int8_t a = 122;
        int8_t b = 99;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 182;
        v ^= 2;
        if (v != 180) failures++;
    }


    {
        volatile int16_t a = -21030;
        volatile int16_t b = -12668;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {140,184,12961,15};
        if (s.c != (uint16_t)12961) failures++;
    }


    {
        uint32_t a = 4171309852UL;
        uint32_t b = 3643821410UL;
        uint32_t r = a & b;
        if (r != 3625976064UL) failures++;
    }


    {
        uint16_t x = 8278;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 248;
        v |= 8;
        if (v != 248) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(244,230) != 14) failures++;
    }


    {
        uint8_t buf[8] = {160,94,28,80,120,201,44,74};
        uint8_t *p = buf;
        p += 7;
        if (*p != 74) failures++;
    }


    {
        uint32_t a = 706307204UL;
        uint32_t b = 1247016881UL;
        uint32_t r = a + b;
        if (r != 1953324085UL) failures++;
    }


    {
        uint16_t r = add2(68,248) + add2(248,124) + add2(68,124);
        if (r != 880) failures++;
    }


    {
        uint16_t r = add2(87,150) + add2(150,57) + add2(87,57);
        if (r != 588) failures++;
    }


    {
        uint8_t v = 33;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 186;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        int8_t a = -108;
        int8_t b = -44;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(251,126) + add2(126,96) + add2(251,96);
        if (r != 946) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {220,201,31457,160};
        if (s.a != (uint8_t)220) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(189,166) != 355) failures++;
    }


    {
        int8_t a = -71;
        int8_t b = 11;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 95;
        uint8_t r = port;
        if (r != 95) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 93; break;
        case 10: result = 207; break;
        case 1: result = 14; break;
        case 19: result = 50; break;
        case 16: result = 81; break;
        case 13: result = 226; break;
        case 18: result = 83; break;
        case 3: result = 70; break;
        default: result = 171; break;
        }
        if (result != 93) failures++;
    }


    {
        uint8_t x = 24;
        x <<= 1;
        if (x != 48) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        g16 = 23827;
        if (read_g16() != 23827) failures++;
    }


    {
        uint8_t x = 85;
        x <<= 4;
        if (x != 80) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint16_t x = 12;
        x = x + 81;
        if (x != 93) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 8: result = 218; break;
        case 16: result = 172; break;
        case 10: result = 231; break;
        case 2: result = 1; break;
        case 4: result = 247; break;
        default: result = 240; break;
        }
        if (result != 240) failures++;
    }


    {
        uint16_t r = add2(75,219) + add2(219,23) + add2(75,23);
        if (r != 634) failures++;
    }


    {
        volatile uint8_t port = 39;
        uint8_t r = port;
        if (r != 39) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(47,87) != 65496) failures++;
    }


    {
        uint16_t x = 251;
        x = x + 230;
        if (x != 481) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 205;
        if (buf[3] != 205) failures++;
    }


    {
        uint8_t buf[8] = {199,80,46,255,118,117,220,115};
        uint8_t *p = buf;
        p += 4;
        if (*p != 118) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -119;
        int8_t b = 38;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {212,187,38,129,236,216};
        if (a[2] != 38) failures++;
    }


    {
        uint16_t r = 51906 + 40870 + 62305 + 23703 + 17577 + 58673 + 543 + 34436;
        if (r != 27869) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-21) % (int16_t)((int8_t)68);
        if ((uint16_t)r != (uint16_t)65515) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(70,240) != 65366) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[13] = {93,230,168,244,215,231,177,106,47,27,66,49,0};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[10] != 66) failures++;
    }


    {
        uint8_t m[3][3] = {{211,247,99},{33,17,217},{104,185,53}};
        if (m[1][1] != 17) failures++;
    }


    {
        uint8_t src[3] = {93,204,33};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 93) failures++;
    }


    {
        uint8_t v = 210;
        v |= 64;
        if (v != 210) failures++;
    }


    {
        uint16_t x = 31;
        x = x + 67;
        if (x != 98) failures++;
    }


    {
        uint8_t a[6] = {75,117,172,140,163,42};
        if (a[3] != 140) failures++;
    }


    {
        uint8_t x = 94;
        x <<= 0;
        if (x != 94) failures++;
    }


    {
        uint8_t a[6] = {218,50,229,74,40,186};
        if (a[4] != 40) failures++;
    }


    {
        uint32_t a = 500940039UL;
        uint32_t b = 1994040892UL;
        uint32_t r = a & b;
        if (r != 349873156UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(119,61) != 180) failures++;
    }


    {
        uint32_t a = 1126282400UL;
        uint32_t b = 3687458798UL;
        uint32_t r = a & b;
        if (r != 1124082848UL) failures++;
    }


    {
        if (((uint16_t)(((85 + 236) | (204 | 222)) + (149 + (232 ^ 230)))) != 642) failures++;
    }


    {
        uint8_t a[6] = {9,169,68,100,143,87};
        if (a[0] != 9) failures++;
    }


    {
        uint16_t r = add2(15,72) + add2(72,202) + add2(15,202);
        if (r != 578) failures++;
    }


    {
        uint16_t x = 31262;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -104;
        int8_t b = 6;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 105;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint8_t v = 183;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][3] = {{200,172,163},{201,76,133}};
        if (m[0][2] != 163) failures++;
    }


    {
        uint8_t v = 162;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {117,18,117,96,152,151};
        if (a[3] != 96) failures++;
    }


    {
        uint32_t a = 1781917059UL;
        uint32_t b = 3019128670UL;
        uint32_t r = a - b;
        if (r != 3057755685UL) failures++;
    }


    {
        g16 = 2614;
        if (read_g16() != 2614) failures++;
    }


    {
        g16 = 11456;
        if (read_g16() != 11456) failures++;
    }


    {
        int8_t a = 118;
        int8_t b = -50;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 63;
        if (buf[5] != 63) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)125) + (uint16_t)38326;
        if (r != 38451) failures++;
    }


    {
        uint16_t r = 4711 + 50841 + 11357 + 57699 + 39086 + 6237 + 24619 + 7286;
        if (r != 5228) failures++;
    }


    {
        uint16_t x = 27818;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)209) + (uint16_t)32090;
        if (r != 32299) failures++;
    }


    {
        uint16_t r = add2(172,118) + add2(118,105) + add2(172,105);
        if (r != 790) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(18,231) != 65323) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {121,137,37633,89};
        if (s.a != (uint8_t)121) failures++;
    }


    {
        uint8_t a[6] = {27,32,239,101,175,45};
        if (a[4] != 175) failures++;
    }


    {
        volatile int16_t a = 12136;
        volatile int16_t b = -162;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2487398366UL;
        uint32_t b = 2734671353UL;
        uint32_t r = a ^ b;
        if (r != 918383143UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {64,10,15800,236};
        if (s.b != (uint8_t)10) failures++;
    }


    {
        uint16_t r = 7671 + 7441 + 43226 + 56297 + 54164 + 7981 + 61358 + 8935;
        if (r != 50465) failures++;
    }


    {
        if (((uint16_t)(((138 & 216) & (190 + 35)) + (15 - 197))) != 65482) failures++;
    }


    {
        uint16_t r = 59341 + 55377 + 62623 + 3244 + 48893 + 47465 + 5696 + 21373;
        if (r != 41868) failures++;
    }


    {
        uint16_t x = 37663;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 45;
        uint8_t r = port;
        if (r != 45) failures++;
    }


    {
        uint16_t r = call6(67,201,79,122,128,165);
        if (r != 762) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {80,28,2561,29};
        if (s.b != (uint8_t)28) failures++;
    }


    {
        uint32_t a = 301793690UL;
        uint32_t b = 1251718515UL;
        uint32_t r = a & b;
        if (r != 10027282UL) failures++;
    }


    {
        volatile uint8_t port = 168;
        uint8_t r = port;
        if (r != 168) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-121) % (int16_t)((int8_t)100);
        if ((uint16_t)r != (uint16_t)65515) failures++;
    }


    {
        uint8_t v = 92;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 36) failures++;
    }


    {
        uint8_t m[4][3] = {{55,14,15},{31,19,122},{27,250,47},{4,25,177}};
        if (m[3][0] != 4) failures++;
    }


    {
        volatile uint8_t port = 127;
        uint8_t r = port;
        if (r != 127) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint8_t v = 117;
        v &= ~(uint8_t)4;
        if (v != 113) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 42;
        if (buf[15] != 42) failures++;
    }


    {
        uint16_t r = call6(161,153,66,48,167,69);
        if (r != 664) failures++;
    }


    {
        volatile int16_t a = 12145;
        volatile int16_t b = 25479;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 148;
        uint8_t r = port;
        if (r != 148) failures++;
    }


    {
        uint16_t x = 120;
        x = x + 155;
        if (x != 275) failures++;
    }


    {
        uint8_t v = 24;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint32_t a = 3158270538UL;
        uint32_t b = 2963084078UL;
        uint32_t r = a ^ b;
        if (r != 211972452UL) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 168;
        if (buf[2] != 168) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)198) + (uint16_t)17896;
        if (r != 18094) failures++;
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
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 18: result = 248; break;
        case 6: result = 66; break;
        case 10: result = 79; break;
        case 1: result = 243; break;
        case 15: result = 189; break;
        case 16: result = 245; break;
        case 0: result = 173; break;
        case 5: result = 104; break;
        default: result = 0; break;
        }
        if (result != 248) failures++;
    }


    {
        volatile int16_t a = -193;
        volatile int16_t b = 1870;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 192;
        uint8_t r = port;
        if (r != 192) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 8: result = 210; break;
        case 7: result = 38; break;
        case 6: result = 117; break;
        case 16: result = 150; break;
        case 5: result = 93; break;
        default: result = 155; break;
        }
        if (result != 210) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {200,164,54540,74};
        if (s.c != (uint16_t)54540) failures++;
    }


    {
        uint8_t v = 44;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 2096475389UL;
        uint32_t b = 3036209770UL;
        uint32_t r = a + b;
        if (r != 837717863UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {248,196,5519,139};
        if (s.b != (uint8_t)196) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)72) % (int16_t)((int8_t)-125);
        if ((uint16_t)r != (uint16_t)72) failures++;
    }


    {
        if (((uint16_t)222) != 222) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {178,123,61146,20};
        if (s.b != (uint8_t)123) failures++;
    }


    {
        uint8_t v = 165;
        v |= 16;
        if (v != 181) failures++;
    }


    {
        g16 = 4783;
        if (read_g16() != 4783) failures++;
    }


    {
        g16 = 1283;
        if (read_g16() != 1283) failures++;
    }


    {
        if (((uint16_t)((194 & 251) | 13)) != 207) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-128) / (int16_t)((int8_t)108);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 1) sum += j;
        if (sum != 105) failures++;
    }


    {
        uint8_t buf[8] = {151,234,134,193,94,46,228,144};
        uint8_t *p = buf;
        p += 4;
        if (*p != 94) failures++;
    }


    {
        uint8_t x = 214;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 183;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 35;
        x <<= 1;
        if (x != 70) failures++;
    }


    {
        uint8_t x = 19;
        x <<= 1;
        if (x != 38) failures++;
    }


    {
        uint32_t a = 3926327799UL;
        uint32_t b = 832753718UL;
        uint32_t r = a ^ b;
        if (r != 3685077441UL) failures++;
    }


    {
        uint8_t x = 174;
        x <<= 3;
        if (x != 112) failures++;
    }


    {
        uint16_t x = 156;
        x = x + 142;
        if (x != 298) failures++;
    }


    {
        uint8_t src[8] = {209,88,23,28,122,118,68,109};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[5] != 118) failures++;
    }


    {
        uint32_t a = 940236825UL;
        uint32_t b = 1041221760UL;
        uint32_t r = a | b;
        if (r != 1041229977UL) failures++;
    }


    {
        uint8_t a[6] = {173,201,78,237,171,189};
        if (a[3] != 237) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)79) / (int16_t)((int8_t)-110);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)78) % (int16_t)((int8_t)-44);
        if ((uint16_t)r != (uint16_t)34) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        volatile int16_t a = 4501;
        volatile int16_t b = -9399;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) / (int16_t)((int8_t)-68);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        if (((uint16_t)(((160 | 9) ^ 101) ^ 105)) != 165) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 57621;
        if (read_g16() != 57621) failures++;
    }


    {
        volatile int16_t a = -6092;
        volatile int16_t b = 3838;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)92) + (uint16_t)3326;
        if (r != 3418) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)233) + (uint16_t)42493;
        if (r != 42726) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 232;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int8_t a = 90;
        int8_t b = 74;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 201;
        uint8_t r = port;
        if (r != 201) failures++;
    }


    {
        volatile uint8_t port = 145;
        uint8_t r = port;
        if (r != 145) failures++;
    }


    {
        volatile uint8_t port = 179;
        uint8_t r = port;
        if (r != 179) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        uint16_t r = 53320 + 38462 + 8775 + 27255 + 32575 + 27567 + 40943 + 37855;
        if (r != 4608) failures++;
    }

    return failures;
}