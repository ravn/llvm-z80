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
        uint32_t a = 663245415UL;
        uint32_t b = 4123252715UL;
        uint32_t r = a | b;
        if (r != 4157331439UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 1) sum += j;
        if (sum != 78) failures++;
    }


    {
        int8_t a = 118;
        int8_t b = -52;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[3][4] = {{120,132,66,73},{109,234,197,111},{173,169,122,80}};
        if (m[0][0] != 120) failures++;
    }


    {
        uint16_t r = add2(77,235) + add2(235,200) + add2(77,200);
        if (r != 1024) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {100,255,34,198,54,168,236,199};
        uint8_t *p = buf;
        p += 7;
        if (*p != 199) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {116,183,570,168};
        if (s.a != (uint8_t)116) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {70,13,18018,177};
        if (s.d != (uint8_t)177) failures++;
    }


    {
        uint8_t v = 22;
        v |= 16;
        if (v != 22) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)188) + (uint16_t)60275;
        if (r != 60463) failures++;
    }


    {
        uint16_t x = 0;
        x = x + 250;
        if (x != 250) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 101;
        if (buf[11] != 101) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(65,29) != 36) failures++;
    }


    {
        uint16_t r = 56846 + 6163 + 27438 + 39272 + 45059 + 25827 + 16537 + 29823;
        if (r != 50357) failures++;
    }


    {
        uint8_t x = 187;
        x <<= 0;
        if (x != 187) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {224,179,64926,141};
        if (s.c != (uint16_t)64926) failures++;
    }


    {
        uint8_t a[6] = {97,113,7,38,65,107};
        if (a[1] != 113) failures++;
    }


    {
        uint16_t r = call6(22,83,72,15,242,107);
        if (r != 541) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(8,143) != 65401) failures++;
    }


    {
        uint32_t a = 167834782UL;
        uint32_t b = 1213231013UL;
        uint32_t r = a ^ b;
        if (r != 1112578875UL) failures++;
    }


    {
        uint16_t x = 145;
        x = x + 80;
        if (x != 225) failures++;
    }


    {
        uint8_t a[6] = {253,107,138,140,0,212};
        if (a[3] != 140) failures++;
    }


    {
        uint16_t x = 1938;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 149;
        x = x + 32;
        if (x != 181) failures++;
    }


    {
        uint8_t src[6] = {116,100,83,6,101,127};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[2] != 83) failures++;
    }


    {
        uint16_t x = 44554;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(55,112) + add2(112,70) + add2(55,70);
        if (r != 474) failures++;
    }


    {
        if (((uint16_t)((254 + (179 - 250)) & 9)) != 1) failures++;
    }


    {
        uint16_t r = add2(32,64) + add2(64,236) + add2(32,236);
        if (r != 664) failures++;
    }


    {
        uint32_t a = 1198250762UL;
        uint32_t b = 3262526083UL;
        uint32_t r = a - b;
        if (r != 2230691975UL) failures++;
    }


    {
        volatile uint8_t port = 110;
        uint8_t r = port;
        if (r != 110) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(91,19) != 110) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-59) % (int16_t)((int8_t)79);
        if ((uint16_t)r != (uint16_t)65477) failures++;
    }


    {
        uint8_t m[2][4] = {{85,69,9,55},{177,25,1,218}};
        if (m[1][0] != 177) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)58) + (uint16_t)18216;
        if (r != 18274) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 159;
        if (buf[15] != 159) failures++;
    }


    {
        uint32_t a = 1724097875UL;
        uint32_t b = 1368622244UL;
        uint32_t r = a | b;
        if (r != 2010359287UL) failures++;
    }


    {
        uint8_t v = 139;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
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
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint16_t x = 4055;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -23418;
        volatile int16_t b = 18377;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint16_t x = 17;
        x = x + 171;
        if (x != 188) failures++;
    }


    {
        uint8_t buf[8] = {4,228,55,194,15,245,174,189};
        uint8_t *p = buf;
        p += 7;
        if (*p != 189) failures++;
    }


    {
        uint8_t v = 203;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        int8_t a = -111;
        int8_t b = -13;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(224,204) != 20) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-107) / (int16_t)((int8_t)7);
        if ((uint16_t)r != (uint16_t)65521) failures++;
    }


    {
        int8_t a = 103;
        int8_t b = 0;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 8: result = 17; break;
        case 16: result = 43; break;
        case 14: result = 80; break;
        default: result = 34; break;
        }
        if (result != 80) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-100) / (int16_t)((int8_t)62);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t a[6] = {189,210,16,22,100,184};
        if (a[4] != 100) failures++;
    }


    {
        uint8_t v = 118;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint32_t a = 1372297170UL;
        uint32_t b = 9451284UL;
        uint32_t r = a | b;
        if (r != 1373353942UL) failures++;
    }


    {
        uint16_t r = 45235 + 8132 + 57353 + 15890 + 57991 + 30972 + 30051 + 42445;
        if (r != 25925) failures++;
    }


    {
        uint8_t m[4][2] = {{223,140},{27,14},{101,180},{84,80}};
        if (m[3][1] != 80) failures++;
    }


    {
        uint16_t x = 21010;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 76;
        uint8_t r = port;
        if (r != 76) failures++;
    }


    {
        uint16_t x = 20585;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 4025;
        volatile int16_t b = -4669;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(197,251) != 448) failures++;
    }


    {
        volatile int16_t a = 27228;
        volatile int16_t b = 16591;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 1) sum += j;
        if (sum != 153) failures++;
    }


    {
        uint16_t r = call6(216,208,222,150,208,186);
        if (r != 1190) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(154,234) != 388) failures++;
    }


    {
        uint16_t r = add2(87,66) + add2(66,250) + add2(87,250);
        if (r != 806) failures++;
    }


    {
        uint8_t buf[8] = {204,203,2,47,189,179,40,21};
        uint8_t *p = buf;
        p += 5;
        if (*p != 179) failures++;
    }


    {
        uint8_t buf[8] = {209,166,180,60,30,224,148,255};
        uint8_t *p = buf;
        p += 6;
        if (*p != 148) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {126,22,19684,98};
        if (s.a != (uint8_t)126) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 6: result = 53; break;
        case 1: result = 50; break;
        case 3: result = 75; break;
        case 14: result = 244; break;
        case 5: result = 212; break;
        case 0: result = 149; break;
        case 15: result = 48; break;
        default: result = 60; break;
        }
        if (result != 48) failures++;
    }


    {
        uint8_t v = 58;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t v = 100;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        int8_t a = -64;
        int8_t b = 62;
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
        g16 = 43486;
        if (read_g16() != 43486) failures++;
    }


    {
        uint8_t m[4][2] = {{123,21},{237,188},{159,149},{105,14}};
        if (m[0][0] != 123) failures++;
    }


    {
        volatile int16_t a = -27364;
        volatile int16_t b = 27307;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)209) + (uint16_t)50890;
        if (r != 51099) failures++;
    }


    {
        int8_t a = 82;
        int8_t b = -69;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 709480837UL;
        uint32_t b = 2438418540UL;
        uint32_t r = a - b;
        if (r != 2566029593UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        g16 = 13775;
        if (read_g16() != 13775) failures++;
    }


    {
        g16 = 48629;
        if (read_g16() != 48629) failures++;
    }


    {
        g16 = 57274;
        if (read_g16() != 57274) failures++;
    }


    {
        uint8_t v = 186;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(119,92) + add2(92,43) + add2(119,43);
        if (r != 508) failures++;
    }


    {
        uint8_t a[6] = {206,220,109,78,164,190};
        if (a[4] != 164) failures++;
    }


    {
        volatile int16_t a = -27157;
        volatile int16_t b = 17282;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 1;
        if (buf[6] != 1) failures++;
    }


    {
        uint16_t r = call6(60,211,211,38,16,104);
        if (r != 640) failures++;
    }


    {
        uint16_t r = add2(205,47) + add2(47,163) + add2(205,163);
        if (r != 830) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {50,187,35451,181};
        if (s.b != (uint8_t)187) failures++;
    }


    {
        if (((uint16_t)(((162 | 165) ^ 254) & 88)) != 88) failures++;
    }


    {
        uint8_t v = 162;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(146,7) != 139) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(192,235) != 65493) failures++;
    }


    {
        uint8_t x = 11;
        x <<= 3;
        if (x != 88) failures++;
    }


    {
        uint8_t m[4][3] = {{120,115,167},{63,191,153},{22,180,206},{150,254,150}};
        if (m[2][1] != 180) failures++;
    }


    {
        uint32_t a = 1057714703UL;
        uint32_t b = 1375721239UL;
        uint32_t r = a | b;
        if (r != 2147481375UL) failures++;
    }


    {
        uint8_t x = 51;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint8_t a[6] = {222,84,42,18,34,33};
        if (a[3] != 18) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        int8_t a = 54;
        int8_t b = 95;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(202,39) != 241) failures++;
    }


    {
        volatile int16_t a = -13711;
        volatile int16_t b = 8988;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[5] = {225,105,26,73,56};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 26) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 33;
        if (buf[10] != 33) failures++;
    }


    {
        uint8_t a[6] = {172,180,201,80,11,29};
        if (a[4] != 11) failures++;
    }


    {
        uint16_t r = 33321 + 2393 + 54657 + 2294 + 31027 + 61538 + 44857 + 47178;
        if (r != 15121) failures++;
    }


    {
        uint16_t r = call6(93,249,69,205,184,43);
        if (r != 843) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)15) + (uint16_t)60880;
        if (r != 60895) failures++;
    }


    {
        uint8_t m[2][2] = {{12,76},{17,160}};
        if (m[0][1] != 76) failures++;
    }


    {
        g16 = 43244;
        if (read_g16() != 43244) failures++;
    }


    {
        uint16_t r = 51703 + 24609 + 56395 + 24914 + 38488 + 19160 + 48911 + 28555;
        if (r != 30591) failures++;
    }


    {
        volatile uint8_t port = 95;
        uint8_t r = port;
        if (r != 95) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 68;
        if (buf[3] != 68) failures++;
    }


    {
        uint8_t buf[8] = {189,115,234,183,245,7,216,64};
        uint8_t *p = buf;
        p += 2;
        if (*p != 234) failures++;
    }


    {
        uint16_t x = 47699;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 57357 + 8453 + 38308 + 6967 + 10812 + 21630 + 33738 + 38405;
        if (r != 19062) failures++;
    }


    {
        g16 = 40480;
        if (read_g16() != 40480) failures++;
    }


    {
        uint8_t a[6] = {3,145,187,181,128,150};
        if (a[2] != 187) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 82;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 138;
        x = x + 202;
        if (x != 340) failures++;
    }


    {
        g16 = 57709;
        if (read_g16() != 57709) failures++;
    }


    {
        int8_t a = -45;
        int8_t b = 15;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)119) + (uint16_t)45927;
        if (r != 46046) failures++;
    }


    {
        uint16_t r = add2(16,211) + add2(211,116) + add2(16,116);
        if (r != 686) failures++;
    }


    {
        uint8_t v = 197;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 27) failures++;
    }


    {
        uint8_t x = 23;
        x <<= 3;
        if (x != 184) failures++;
    }


    {
        volatile int16_t a = -3620;
        volatile int16_t b = -28591;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 41;
        if (buf[1] != 41) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 101;
        if (buf[14] != 101) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(163,70) != 233) failures++;
    }


    {
        uint8_t a[6] = {129,172,110,192,127,40};
        if (a[5] != 40) failures++;
    }


    {
        uint16_t x = 40331;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 31970;
        volatile int16_t b = 3517;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 20;
        if (buf[8] != 20) failures++;
    }


    {
        volatile uint8_t port = 133;
        uint8_t r = port;
        if (r != 133) failures++;
    }


    {
        g16 = 36548;
        if (read_g16() != 36548) failures++;
    }


    {
        uint8_t v = 30;
        v |= 16;
        if (v != 30) failures++;
    }


    {
        int8_t a = 99;
        int8_t b = 67;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-84) / (int16_t)((int8_t)-98);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {176,165,168,136,169,59};
        if (a[0] != 176) failures++;
    }


    {
        uint8_t m[2][2] = {{45,127},{30,110}};
        if (m[0][1] != 127) failures++;
    }


    {
        uint8_t m[4][4] = {{0,156,254,60},{113,162,108,175},{210,94,79,233},{238,61,236,154}};
        if (m[1][3] != 175) failures++;
    }


    {
        g16 = 30361;
        if (read_g16() != 30361) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -6003;
        volatile int16_t b = -5383;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(100,86) + add2(86,45) + add2(100,45);
        if (r != 462) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)118) + (uint16_t)23185;
        if (r != 23303) failures++;
    }


    {
        uint8_t m[4][4] = {{163,58,48,217},{139,206,29,85},{83,60,29,231},{121,16,38,100}};
        if (m[0][3] != 217) failures++;
    }


    {
        uint8_t m[4][2] = {{216,116},{119,123},{245,215},{43,243}};
        if (m[3][1] != 243) failures++;
    }


    {
        uint16_t x = 121;
        x = x + 163;
        if (x != 284) failures++;
    }


    {
        int8_t a = -83;
        int8_t b = 102;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 109;
        v ^= 16;
        if (v != 125) failures++;
    }


    {
        g16 = 18963;
        if (read_g16() != 18963) failures++;
    }


    {
        uint16_t x = 14084;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 9612;
        if (read_g16() != 9612) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)155) + (uint16_t)22910;
        if (r != 23065) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)128) + (uint16_t)44782;
        if (r != 44910) failures++;
    }


    {
        if (((uint16_t)(49 - (119 ^ 2))) != 65468) failures++;
    }


    {
        uint16_t r = add2(214,251) + add2(251,36) + add2(214,36);
        if (r != 1002) failures++;
    }


    {
        uint8_t x = 15;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint32_t a = 3598378103UL;
        uint32_t b = 2263961103UL;
        uint32_t r = a ^ b;
        if (r != 1351330424UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(111,45) != 156) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint32_t a = 3656039669UL;
        uint32_t b = 1008842237UL;
        uint32_t r = a - b;
        if (r != 2647197432UL) failures++;
    }


    {
        uint8_t m[3][3] = {{136,210,252},{239,166,117},{145,3,201}};
        if (m[2][0] != 145) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 29;
        if (buf[4] != 29) failures++;
    }


    {
        volatile uint8_t port = 32;
        uint8_t r = port;
        if (r != 32) failures++;
    }


    {
        if (((uint16_t)98) != 98) failures++;
    }


    {
        uint8_t a[6] = {91,77,84,75,176,82};
        if (a[1] != 77) failures++;
    }


    {
        int8_t a = 103;
        int8_t b = -29;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 163;
        uint8_t r = port;
        if (r != 163) failures++;
    }


    {
        uint16_t x = 14086;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-59) / (int16_t)((int8_t)-9);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint32_t a = 2753817256UL;
        uint32_t b = 1318414054UL;
        uint32_t r = a | b;
        if (r != 4005031662UL) failures++;
    }


    {
        uint16_t x = 212;
        x = x + 178;
        if (x != 390) failures++;
    }


    {
        if (((uint16_t)147) != 147) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)113) % (int16_t)((int8_t)-94);
        if ((uint16_t)r != (uint16_t)19) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {125,61,21986,44};
        if (s.b != (uint8_t)61) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 9;
        if (buf[2] != 9) failures++;
    }


    {
        volatile int16_t a = 32143;
        volatile int16_t b = -6069;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[3][2] = {{27,246},{55,105},{208,28}};
        if (m[1][0] != 55) failures++;
    }


    {
        uint8_t m[2][3] = {{77,134,115},{62,253,243}};
        if (m[0][0] != 77) failures++;
    }


    {
        uint16_t x = 151;
        x = x + 7;
        if (x != 158) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(152,184) != 336) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(121,196) != 65461) failures++;
    }


    {
        uint8_t x = 176;
        x <<= 0;
        if (x != 176) failures++;
    }


    {
        uint16_t x = 78;
        x = x + 132;
        if (x != 210) failures++;
    }


    {
        uint8_t v = 150;
        int r = (v & 4) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t r = 33635 + 9951 + 10245 + 10769 + 24863 + 12939 + 58986 + 31227;
        if (r != 61543) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(169 | ((122 | 159) + 73))) != 489) failures++;
    }


    {
        if (((uint16_t)(192 ^ (72 ^ (97 + 68)))) != 45) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)68) % (int16_t)((int8_t)126);
        if ((uint16_t)r != (uint16_t)68) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {6,16,48974,27};
        if (s.c != (uint16_t)48974) failures++;
    }


    {
        uint8_t src[16] = {183,55,86,9,250,74,120,201,149,31,168,166,189,179,46,35};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[9] != 31) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {56,132,54721,147};
        if (s.a != (uint8_t)56) failures++;
    }


    {
        uint16_t r = call6(88,78,211,186,31,39);
        if (r != 633) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(184,83) != 101) failures++;
    }


    {
        uint32_t a = 449914784UL;
        uint32_t b = 2119404423UL;
        uint32_t r = a | b;
        if (r != 2127802279UL) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 49;
        if (buf[7] != 49) failures++;
    }


    {
        uint8_t src[1] = {153};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 153) failures++;
    }


    {
        volatile int16_t a = 7453;
        volatile int16_t b = -24084;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t x = 27;
        x = x + 198;
        if (x != 225) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)51) % (int16_t)((int8_t)43);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        uint8_t v = 34;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 14) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t a[6] = {33,11,254,140,6,62};
        if (a[2] != 254) failures++;
    }


    {
        uint16_t r = add2(0,126) + add2(126,170) + add2(0,170);
        if (r != 592) failures++;
    }


    {
        uint16_t x = 48510;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 60;
        v &= ~(uint8_t)2;
        if (v != 60) failures++;
    }


    {
        uint8_t src[8] = {187,189,244,17,71,27,96,174};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[7] != 174) failures++;
    }


    {
        uint8_t a[6] = {216,240,125,179,139,72};
        if (a[3] != 179) failures++;
    }


    {
        g16 = 20908;
        if (read_g16() != 20908) failures++;
    }


    {
        uint16_t r = call6(94,254,255,47,58,226);
        if (r != 934) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)78) / (int16_t)((int8_t)126);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 19: result = 30; break;
        case 10: result = 38; break;
        case 11: result = 235; break;
        case 12: result = 131; break;
        default: result = 177; break;
        }
        if (result != 38) failures++;
    }


    {
        uint8_t src[6] = {83,191,136,33,238,233};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 233) failures++;
    }


    {
        g16 = 17272;
        if (read_g16() != 17272) failures++;
    }


    {
        uint16_t x = 6012;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[3] = {59,108,243};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 108) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint16_t x = 7621;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {231,242,53824,62};
        if (s.c != (uint16_t)53824) failures++;
    }


    {
        uint16_t r = add2(156,102) + add2(102,208) + add2(156,208);
        if (r != 932) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 16: result = 118; break;
        case 14: result = 29; break;
        case 10: result = 237; break;
        case 0: result = 68; break;
        case 9: result = 69; break;
        default: result = 189; break;
        }
        if (result != 237) failures++;
    }


    {
        uint8_t m[3][2] = {{142,25},{22,15},{83,194}};
        if (m[1][1] != 15) failures++;
    }


    {
        int8_t a = -19;
        int8_t b = 45;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -30;
        int8_t b = -26;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = call6(214,68,224,255,191,125);
        if (r != 1077) failures++;
    }


    {
        uint8_t buf[8] = {154,1,36,252,112,234,10,108};
        uint8_t *p = buf;
        p += 6;
        if (*p != 10) failures++;
    }


    {
        g16 = 6060;
        if (read_g16() != 6060) failures++;
    }


    {
        uint8_t a[6] = {131,114,191,138,134,99};
        if (a[4] != 134) failures++;
    }


    {
        uint16_t x = 3978;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(69,129) + add2(129,111) + add2(69,111);
        if (r != 618) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 48;
        if (buf[1] != 48) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(148,122) != 270) failures++;
    }


    {
        uint16_t r = 12035 + 48976 + 20874 + 58185 + 14071 + 3802 + 2641 + 27469;
        if (r != 56981) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-43) / (int16_t)((int8_t)43);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t r = call6(166,77,145,197,92,248);
        if (r != 925) failures++;
    }


    {
        if (((uint16_t)(((50 + 198) | (131 ^ 37)) - ((111 + 246) - (169 & 162)))) != 57) failures++;
    }


    {
        uint16_t r = add2(120,201) + add2(201,132) + add2(120,132);
        if (r != 906) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 1) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint8_t buf[8] = {102,225,155,253,0,121,45,132};
        uint8_t *p = buf;
        p += 6;
        if (*p != 45) failures++;
    }


    {
        uint8_t src[4] = {84,55,137,39};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 84) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 73;
        if (buf[2] != 73) failures++;
    }


    {
        uint16_t x = 120;
        x = x + 5;
        if (x != 125) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 17: result = 1; break;
        case 18: result = 34; break;
        case 13: result = 163; break;
        case 12: result = 30; break;
        case 19: result = 82; break;
        default: result = 159; break;
        }
        if (result != 163) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint16_t r = add2(187,128) + add2(128,169) + add2(187,169);
        if (r != 968) failures++;
    }


    {
        uint8_t buf[8] = {219,33,29,185,172,72,32,68};
        uint8_t *p = buf;
        p += 3;
        if (*p != 185) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t r = 39198 + 56342 + 5638 + 58092 + 36052 + 36422 + 28688 + 49804;
        if (r != 48092) failures++;
    }


    {
        uint16_t r = add2(102,42) + add2(42,39) + add2(102,39);
        if (r != 366) failures++;
    }


    {
        volatile uint8_t port = 11;
        uint8_t r = port;
        if (r != 11) failures++;
    }


    {
        uint8_t v = 63;
        v ^= 128;
        if (v != 191) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-7) / (int16_t)((int8_t)-13);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = call6(132,54,40,153,155,180);
        if (r != 714) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)249) + (uint16_t)41982;
        if (r != 42231) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)236) + (uint16_t)56100;
        if (r != 56336) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)73) + (uint16_t)1278;
        if (r != 1351) failures++;
    }


    {
        uint16_t r = add2(172,109) + add2(109,4) + add2(172,4);
        if (r != 570) failures++;
    }


    {
        volatile int16_t a = 30741;
        volatile int16_t b = -16868;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 224;
        int r = (v & 8) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 145;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t m[3][2] = {{81,244},{239,139},{148,37}};
        if (m[0][0] != 81) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-49) % (int16_t)((int8_t)22);
        if ((uint16_t)r != (uint16_t)65531) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(223,156) != 379) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)86) / (int16_t)((int8_t)-110);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 17245 + 9715 + 13566 + 20428 + 45831 + 45070 + 33885 + 51809;
        if (r != 40941) failures++;
    }


    {
        uint16_t r = add2(81,41) + add2(41,4) + add2(81,4);
        if (r != 252) failures++;
    }


    {
        volatile uint8_t port = 225;
        uint8_t r = port;
        if (r != 225) failures++;
    }


    {
        uint16_t x = 33580;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 64202 + 34734 + 54527 + 19565 + 52352 + 30590 + 2040 + 60818;
        if (r != 56684) failures++;
    }


    {
        uint16_t r = 23671 + 52010 + 12500 + 44973 + 54531 + 8620 + 4320 + 44153;
        if (r != 48170) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 40;
        if (buf[2] != 40) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 12: result = 40; break;
        case 13: result = 245; break;
        case 6: result = 245; break;
        case 1: result = 125; break;
        default: result = 82; break;
        }
        if (result != 245) failures++;
    }


    {
        int8_t a = -116;
        int8_t b = 71;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 13;
        int8_t b = -9;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {83,96,8473,122};
        if (s.b != (uint8_t)96) failures++;
    }


    {
        uint16_t r = call6(21,234,148,241,66,190);
        if (r != 900) failures++;
    }


    {
        uint16_t x = 52417;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 14;
        int8_t b = -35;
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
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 23;
        if (buf[7] != 23) failures++;
    }


    {
        uint8_t m[3][2] = {{1,50},{150,61},{162,222}};
        if (m[2][1] != 222) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 144;
        if (buf[9] != 144) failures++;
    }


    {
        g16 = 27733;
        if (read_g16() != 27733) failures++;
    }


    {
        uint16_t r = 74 + 58407 + 53082 + 49031 + 126 + 17095 + 28293 + 44136;
        if (r != 53636) failures++;
    }


    {
        uint8_t buf[8] = {191,69,63,81,150,176,53,150};
        uint8_t *p = buf;
        p += 1;
        if (*p != 69) failures++;
    }


    {
        uint8_t buf[8] = {80,93,44,55,194,42,38,65};
        uint8_t *p = buf;
        p += 2;
        if (*p != 44) failures++;
    }


    {
        uint16_t r = call6(75,129,126,222,248,118);
        if (r != 918) failures++;
    }


    {
        uint8_t v = 196;
        v |= 2;
        if (v != 198) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)52) + (uint16_t)62769;
        if (r != 62821) failures++;
    }


    {
        uint16_t x = 25;
        x = x + 67;
        if (x != 92) failures++;
    }


    {
        int8_t a = -38;
        int8_t b = -44;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2032074420UL;
        uint32_t b = 2492037832UL;
        uint32_t r = a & b;
        if (r != 268991104UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,29,63039,192};
        if (s.a != (uint8_t)205) failures++;
    }


    {
        g16 = 4764;
        if (read_g16() != 4764) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -109;
        int8_t b = 105;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(120,25) + add2(25,26) + add2(120,26);
        if (r != 342) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)136) + (uint16_t)37913;
        if (r != 38049) failures++;
    }


    {
        uint8_t v = 34;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 30) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 154;
        if (buf[5] != 154) failures++;
    }


    {
        uint32_t a = 711808125UL;
        uint32_t b = 2764393169UL;
        uint32_t r = a - b;
        if (r != 2242382252UL) failures++;
    }


    {
        g16 = 60872;
        if (read_g16() != 60872) failures++;
    }


    {
        uint32_t a = 2662706262UL;
        uint32_t b = 3993108721UL;
        uint32_t r = a | b;
        if (r != 4273450231UL) failures++;
    }


    {
        uint32_t a = 1358110986UL;
        uint32_t b = 3542919968UL;
        uint32_t r = a - b;
        if (r != 2110158314UL) failures++;
    }


    {
        uint8_t src[11] = {234,216,238,2,1,109,147,142,77,105,24};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[0] != 234) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 171;
        if (buf[9] != 171) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 16: result = 152; break;
        case 13: result = 154; break;
        case 18: result = 53; break;
        case 12: result = 168; break;
        case 7: result = 5; break;
        case 6: result = 223; break;
        case 1: result = 16; break;
        case 10: result = 200; break;
        default: result = 110; break;
        }
        if (result != 223) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 59;
        if (buf[0] != 59) failures++;
    }


    {
        uint16_t r = call6(132,3,108,160,89,120);
        if (r != 612) failures++;
    }


    {
        uint8_t src[11] = {223,98,196,253,74,205,19,0,149,151,148};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[1] != 98) failures++;
    }


    {
        uint16_t r = add2(117,195) + add2(195,228) + add2(117,228);
        if (r != 1080) failures++;
    }


    {
        uint16_t x = 1924;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {24,88,55266,234};
        if (s.b != (uint8_t)88) failures++;
    }


    {
        uint16_t x = 20738;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(142,114) + add2(114,16) + add2(142,16);
        if (r != 544) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 3;
        int8_t b = 74;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 8395 + 57171 + 40572 + 63883 + 34996 + 47495 + 14495 + 60403;
        if (r != 65266) failures++;
    }


    {
        uint8_t buf[8] = {174,10,67,205,64,163,209,38};
        uint8_t *p = buf;
        p += 4;
        if (*p != 64) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)170) + (uint16_t)49408;
        if (r != 49578) failures++;
    }


    {
        uint32_t a = 3254132802UL;
        uint32_t b = 3986720727UL;
        uint32_t r = a | b;
        if (r != 3992365015UL) failures++;
    }


    {
        uint16_t x = 232;
        x = x + 26;
        if (x != 258) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 15;
        int8_t b = 72;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {161,63,83,44,159,168,43,169};
        uint8_t *p = buf;
        p += 2;
        if (*p != 83) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {145,122,32,177,56,113,105,165};
        uint8_t *p = buf;
        p += 0;
        if (*p != 145) failures++;
    }


    {
        uint8_t x = 149;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 1249;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(205,176) + add2(176,94) + add2(205,94);
        if (r != 950) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t buf[8] = {148,162,219,136,255,27,185,26};
        uint8_t *p = buf;
        p += 1;
        if (*p != 162) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -102;
        int8_t b = 79;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 14;
        if (buf[4] != 14) failures++;
    }


    {
        volatile int16_t a = -8431;
        volatile int16_t b = 21469;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        g16 = 50752;
        if (read_g16() != 50752) failures++;
    }


    {
        uint16_t r = call6(193,97,246,163,176,51);
        if (r != 926) failures++;
    }


    {
        uint8_t x = 108;
        x <<= 0;
        if (x != 108) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {255,100,19483,61};
        if (s.a != (uint8_t)255) failures++;
    }


    {
        uint8_t buf[8] = {128,195,146,86,129,33,119,39};
        uint8_t *p = buf;
        p += 4;
        if (*p != 129) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)5) / (int16_t)((int8_t)-128);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 232;
        v |= 128;
        if (v != 232) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)49) / (int16_t)((int8_t)-86);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-30) % (int16_t)((int8_t)-101);
        if ((uint16_t)r != (uint16_t)65506) failures++;
    }


    {
        uint16_t r = 6989 + 40305 + 54260 + 24626 + 61967 + 15035 + 43507 + 33053;
        if (r != 17598) failures++;
    }


    {
        uint16_t r = 7554 + 29544 + 53223 + 55673 + 51877 + 51687 + 22423 + 11996;
        if (r != 21833) failures++;
    }


    {
        uint8_t v = 41;
        v |= 1;
        if (v != 41) failures++;
    }


    {
        uint8_t v = 209;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t buf[8] = {18,170,144,252,151,211,217,77};
        uint8_t *p = buf;
        p += 6;
        if (*p != 217) failures++;
    }


    {
        uint8_t x = 74;
        x <<= 2;
        if (x != 40) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 175;
        if (buf[10] != 175) failures++;
    }


    {
        uint16_t x = 3;
        x = x + 59;
        if (x != 62) failures++;
    }


    {
        uint8_t v = 3;
        v |= 4;
        if (v != 7) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(125,141) != 65520) failures++;
    }


    {
        volatile uint8_t port = 205;
        uint8_t r = port;
        if (r != 205) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 5: result = 188; break;
        case 3: result = 229; break;
        case 4: result = 54; break;
        case 0: result = 40; break;
        case 14: result = 151; break;
        default: result = 37; break;
        }
        if (result != 54) failures++;
    }


    {
        uint16_t r = add2(73,101) + add2(101,177) + add2(73,177);
        if (r != 702) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(133,88) != 221) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 0: result = 227; break;
        case 2: result = 233; break;
        case 5: result = 63; break;
        case 16: result = 223; break;
        case 18: result = 23; break;
        case 19: result = 106; break;
        default: result = 163; break;
        }
        if (result != 23) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 8: result = 173; break;
        case 0: result = 51; break;
        case 3: result = 173; break;
        case 12: result = 80; break;
        case 14: result = 182; break;
        case 18: result = 93; break;
        case 15: result = 205; break;
        case 2: result = 68; break;
        default: result = 220; break;
        }
        if (result != 68) failures++;
    }


    {
        uint8_t m[2][3] = {{176,83,93},{19,99,61}};
        if (m[0][2] != 93) failures++;
    }


    {
        uint8_t x = 16;
        x <<= 1;
        if (x != 32) failures++;
    }


    {
        uint32_t a = 949640396UL;
        uint32_t b = 2804763595UL;
        uint32_t r = a & b;
        if (r != 537413832UL) failures++;
    }


    {
        uint16_t x = 36174;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 51710;
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
        uint16_t r = (uint16_t)((uint8_t)225) + (uint16_t)40546;
        if (r != 40771) failures++;
    }


    {
        uint8_t v = 162;
        v &= ~(uint8_t)1;
        if (v != 162) failures++;
    }


    {
        uint16_t x = 7073;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 163;
        v |= 64;
        if (v != 227) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {8,165,61515,172};
        if (s.b != (uint8_t)165) failures++;
    }


    {
        uint8_t v = 14;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 50) failures++;
    }


    {
        uint8_t v = 219;
        v &= ~(uint8_t)64;
        if (v != 155) failures++;
    }


    {
        uint16_t x = 30067;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -21;
        int8_t b = 53;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 15: result = 33; break;
        case 1: result = 6; break;
        case 6: result = 82; break;
        case 10: result = 114; break;
        case 11: result = 82; break;
        default: result = 40; break;
        }
        if (result != 82) failures++;
    }


    {
        uint8_t v = 85;
        v |= 64;
        if (v != 85) failures++;
    }


    {
        g16 = 5623;
        if (read_g16() != 5623) failures++;
    }


    {
        uint16_t r = add2(31,65) + add2(65,177) + add2(31,177);
        if (r != 546) failures++;
    }


    {
        int8_t a = -62;
        int8_t b = -72;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[11] = {204,157,195,90,120,131,255,204,155,153,80};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[5] != 131) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(109,155) != 65490) failures++;
    }


    {
        uint8_t v = 202;
        v ^= 64;
        if (v != 138) failures++;
    }


    {
        uint8_t buf[8] = {161,97,65,161,28,216,181,227};
        uint8_t *p = buf;
        p += 5;
        if (*p != 216) failures++;
    }


    {
        volatile uint8_t port = 197;
        uint8_t r = port;
        if (r != 197) failures++;
    }


    {
        uint16_t x = 4;
        x = x + 221;
        if (x != 225) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)18) / (int16_t)((int8_t)-127);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 15: result = 110; break;
        case 3: result = 76; break;
        case 19: result = 76; break;
        case 5: result = 26; break;
        case 8: result = 143; break;
        default: result = 50; break;
        }
        if (result != 143) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 25;
        x = x + 39;
        if (x != 64) failures++;
    }


    {
        uint8_t buf[8] = {98,78,227,213,234,52,81,251};
        uint8_t *p = buf;
        p += 5;
        if (*p != 52) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 111;
        if (buf[13] != 111) failures++;
    }


    {
        uint8_t v = 102;
        v ^= 128;
        if (v != 230) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint8_t a[6] = {252,225,6,92,224,228};
        if (a[4] != 224) failures++;
    }


    {
        int8_t a = 41;
        int8_t b = -69;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 230;
        x = x + 73;
        if (x != 303) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint16_t x = 39587;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)174) + (uint16_t)1548;
        if (r != 1722) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 1) sum += j;
        if (sum != 21) failures++;
    }


    {
        g16 = 10635;
        if (read_g16() != 10635) failures++;
    }


    {
        volatile uint8_t port = 133;
        uint8_t r = port;
        if (r != 133) failures++;
    }


    {
        volatile uint8_t port = 32;
        uint8_t r = port;
        if (r != 32) failures++;
    }


    {
        uint8_t m[4][2] = {{10,152},{36,214},{5,62},{100,232}};
        if (m[3][1] != 232) failures++;
    }


    {
        if (((uint16_t)(67 & ((29 - 138) | (60 - 61)))) != 67) failures++;
    }


    {
        uint16_t x = 56361;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(86,182) + add2(182,61) + add2(86,61);
        if (r != 658) failures++;
    }


    {
        uint16_t x = 59586;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-34) / (int16_t)((int8_t)-97);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {209,78,4310,229};
        if (s.b != (uint8_t)78) failures++;
    }


    {
        uint8_t v = 250;
        v |= 4;
        if (v != 254) failures++;
    }


    {
        uint8_t src[16] = {57,196,86,171,102,14,10,203,8,228,140,197,161,62,23,166};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[10] != 140) failures++;
    }


    {
        g16 = 21716;
        if (read_g16() != 21716) failures++;
    }


    {
        uint32_t a = 398735552UL;
        uint32_t b = 1284875956UL;
        uint32_t r = a + b;
        if (r != 1683611508UL) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 9;
        if (buf[2] != 9) failures++;
    }


    {
        uint8_t v = 93;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t x = 182;
        x <<= 3;
        if (x != 176) failures++;
    }


    {
        volatile uint8_t port = 214;
        uint8_t r = port;
        if (r != 214) failures++;
    }


    {
        uint8_t v = 138;
        v &= ~(uint8_t)32;
        if (v != 138) failures++;
    }


    {
        uint8_t m[3][4] = {{251,71,237,110},{133,24,59,36},{198,8,190,140}};
        if (m[1][3] != 36) failures++;
    }


    {
        uint8_t a[6] = {142,136,217,177,7,153};
        if (a[0] != 142) failures++;
    }


    {
        uint16_t r = add2(41,241) + add2(241,233) + add2(41,233);
        if (r != 1030) failures++;
    }


    {
        int8_t a = -128;
        int8_t b = -39;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 59;
        if (buf[5] != 59) failures++;
    }


    {
        volatile uint8_t port = 51;
        uint8_t r = port;
        if (r != 51) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 15: result = 171; break;
        case 16: result = 41; break;
        case 19: result = 134; break;
        case 9: result = 17; break;
        case 6: result = 44; break;
        case 18: result = 178; break;
        default: result = 22; break;
        }
        if (result != 134) failures++;
    }


    {
        uint8_t src[10] = {147,182,143,148,61,192,127,146,205,246};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[9] != 246) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 227;
        if (buf[2] != 227) failures++;
    }


    {
        uint32_t a = 2085462946UL;
        uint32_t b = 963671717UL;
        uint32_t r = a & b;
        if (r != 943727264UL) failures++;
    }


    {
        uint8_t x = 55;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        uint8_t v = 111;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 17) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t a[6] = {16,115,61,231,21,136};
        if (a[4] != 21) failures++;
    }


    {
        uint8_t v = 16;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 112) failures++;
    }


    {
        g16 = 23673;
        if (read_g16() != 23673) failures++;
    }


    {
        uint32_t a = 1186902046UL;
        uint32_t b = 1647022552UL;
        uint32_t r = a & b;
        if (r != 1110085656UL) failures++;
    }


    {
        int8_t a = -20;
        int8_t b = -104;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {110,175,8804,247};
        if (s.c != (uint16_t)8804) failures++;
    }


    {
        uint16_t x = 168;
        x = x + 2;
        if (x != 170) failures++;
    }


    {
        g16 = 48396;
        if (read_g16() != 48396) failures++;
    }


    {
        uint8_t v = 13;
        v |= 2;
        if (v != 15) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)35) + (uint16_t)33417;
        if (r != 33452) failures++;
    }


    {
        uint8_t v = 207;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = -32541;
        volatile int16_t b = 27600;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 118;
        x <<= 2;
        if (x != 216) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 107;
        if (buf[11] != 107) failures++;
    }


    {
        uint16_t r = 49955 + 47459 + 36679 + 12833 + 31134 + 65208 + 12258 + 21362;
        if (r != 14744) failures++;
    }


    {
        uint8_t a[6] = {58,172,202,203,4,77};
        if (a[3] != 203) failures++;
    }


    {
        uint8_t a[6] = {219,125,87,231,196,78};
        if (a[5] != 78) failures++;
    }


    {
        if (((uint16_t)(((61 & 23) - (78 & 139)) ^ ((229 + 14) | 98))) != 248) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 0: result = 93; break;
        case 13: result = 222; break;
        case 15: result = 68; break;
        case 9: result = 183; break;
        case 18: result = 225; break;
        case 14: result = 46; break;
        case 16: result = 170; break;
        default: result = 20; break;
        }
        if (result != 46) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint16_t r = add2(89,106) + add2(106,24) + add2(89,24);
        if (r != 438) failures++;
    }


    {
        uint8_t v = 246;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(25,169) + add2(169,219) + add2(25,219);
        if (r != 826) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {223,223,7290,49};
        if (s.a != (uint8_t)223) failures++;
    }


    {
        uint8_t buf[8] = {220,159,88,189,207,164,119,45};
        uint8_t *p = buf;
        p += 3;
        if (*p != 189) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-36) / (int16_t)((int8_t)-119);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t m[3][4] = {{168,246,110,238},{57,58,7,162},{131,235,14,47}};
        if (m[0][3] != 238) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {200,93,18394,220};
        if (s.c != (uint16_t)18394) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 1) sum += j;
        if (sum != 66) failures++;
    }


    {
        volatile uint8_t port = 36;
        uint8_t r = port;
        if (r != 36) failures++;
    }


    {
        int8_t a = 23;
        int8_t b = -72;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 3: result = 114; break;
        case 19: result = 230; break;
        case 4: result = 180; break;
        case 0: result = 205; break;
        default: result = 22; break;
        }
        if (result != 205) failures++;
    }


    {
        if (((uint16_t)((51 | 63) + ((0 + 154) ^ (209 | 86)))) != 140) failures++;
    }


    {
        uint16_t r = 27757 + 5006 + 13669 + 39113 + 39818 + 45869 + 59353 + 44301;
        if (r != 12742) failures++;
    }


    {
        if (((uint16_t)((217 ^ (9 - 168)) ^ 213)) != 65389) failures++;
    }


    {
        g16 = 58987;
        if (read_g16() != 58987) failures++;
    }


    {
        uint8_t x = 18;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        volatile uint8_t port = 169;
        uint8_t r = port;
        if (r != 169) failures++;
    }


    {
        uint8_t x = 120;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint8_t m[4][3] = {{151,233,77},{119,45,231},{152,234,159},{191,137,42}};
        if (m[1][1] != 45) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 146;
        if (buf[13] != 146) failures++;
    }


    {
        volatile uint8_t port = 44;
        uint8_t r = port;
        if (r != 44) failures++;
    }


    {
        g16 = 49519;
        if (read_g16() != 49519) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 7: result = 75; break;
        case 5: result = 183; break;
        case 6: result = 98; break;
        case 19: result = 38; break;
        case 10: result = 31; break;
        default: result = 212; break;
        }
        if (result != 212) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 7: result = 6; break;
        case 11: result = 132; break;
        case 13: result = 189; break;
        default: result = 31; break;
        }
        if (result != 6) failures++;
    }


    {
        uint8_t buf[8] = {8,172,167,112,40,232,250,122};
        uint8_t *p = buf;
        p += 1;
        if (*p != 172) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        if (((uint16_t)((89 & (146 - 187)) ^ 212)) != 133) failures++;
    }


    {
        if (((uint16_t)((255 - 242) + 207)) != 220) failures++;
    }


    {
        uint32_t a = 2378383009UL;
        uint32_t b = 2236493074UL;
        uint32_t r = a & b;
        if (r != 2235706368UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(236,22) != 258) failures++;
    }


    {
        uint32_t a = 4245960501UL;
        uint32_t b = 802322989UL;
        uint32_t r = a & b;
        if (r != 756036133UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint8_t m[3][2] = {{248,60},{79,39},{27,100}};
        if (m[1][0] != 79) failures++;
    }


    {
        uint16_t r = 7839 + 23211 + 34829 + 42287 + 34735 + 27479 + 12563 + 49105;
        if (r != 35440) failures++;
    }


    {
        uint8_t m[2][4] = {{124,196,122,241},{107,246,177,57}};
        if (m[1][2] != 177) failures++;
    }


    {
        uint16_t r = add2(6,113) + add2(113,61) + add2(6,61);
        if (r != 360) failures++;
    }


    {
        uint16_t r = 52925 + 11781 + 9571 + 44623 + 50946 + 17123 + 53646 + 42196;
        if (r != 20667) failures++;
    }


    {
        g16 = 14350;
        if (read_g16() != 14350) failures++;
    }


    {
        if (((uint16_t)26) != 26) failures++;
    }


    {
        uint8_t buf[8] = {150,4,211,241,71,164,95,83};
        uint8_t *p = buf;
        p += 3;
        if (*p != 241) failures++;
    }


    {
        int8_t a = 107;
        int8_t b = -7;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 128;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 64) failures++;
    }


    {
        uint16_t x = 210;
        x = x + 162;
        if (x != 372) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)106) % (int16_t)((int8_t)-57);
        if ((uint16_t)r != (uint16_t)49) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 10: result = 59; break;
        case 3: result = 78; break;
        case 7: result = 94; break;
        default: result = 29; break;
        }
        if (result != 29) failures++;
    }


    {
        uint8_t src[3] = {27,42,135};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 27) failures++;
    }


    {
        uint16_t x = 26;
        x = x + 98;
        if (x != 124) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-126) / (int16_t)((int8_t)106);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t r = call6(205,168,180,82,38,119);
        if (r != 792) failures++;
    }


    {
        uint16_t x = 129;
        x = x + 133;
        if (x != 262) failures++;
    }


    {
        uint16_t r = add2(224,95) + add2(95,143) + add2(224,143);
        if (r != 924) failures++;
    }


    {
        uint8_t m[4][4] = {{161,118,4,61},{35,199,194,160},{58,201,175,38},{103,195,109,9}};
        if (m[3][3] != 9) failures++;
    }


    {
        uint8_t v = 217;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)((37 & (127 + 2)) - 244)) != 65293) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)63) / (int16_t)((int8_t)12);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(142,61) != 81) failures++;
    }


    {
        uint8_t v = 225;
        v ^= 2;
        if (v != 227) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(209,232) != 441) failures++;
    }


    {
        uint32_t a = 1817968688UL;
        uint32_t b = 3154068046UL;
        uint32_t r = a - b;
        if (r != 2958867938UL) failures++;
    }


    {
        uint8_t x = 90;
        x <<= 1;
        if (x != 180) failures++;
    }


    {
        uint16_t r = 54951 + 20304 + 26511 + 55553 + 64224 + 43693 + 33628 + 4284;
        if (r != 41004) failures++;
    }


    {
        uint8_t v = 106;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 12396;
        volatile int16_t b = 492;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        g16 = 42534;
        if (read_g16() != 42534) failures++;
    }


    {
        uint8_t m[3][4] = {{158,130,174,16},{62,226,63,96},{209,69,59,122}};
        if (m[2][1] != 69) failures++;
    }


    {
        volatile int16_t a = 355;
        volatile int16_t b = -16518;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t a[6] = {249,153,130,74,119,10};
        if (a[2] != 130) failures++;
    }


    {
        uint8_t x = 131;
        x <<= 1;
        if (x != 6) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)116) / (int16_t)((int8_t)-6);
        if ((uint16_t)r != (uint16_t)65517) failures++;
    }


    {
        uint16_t r = call6(204,197,47,248,103,37);
        if (r != 836) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-58) / (int16_t)((int8_t)-104);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)230) + (uint16_t)42331;
        if (r != 42561) failures++;
    }


    {
        uint8_t a[6] = {202,173,123,44,192,197};
        if (a[5] != 197) failures++;
    }


    {
        uint8_t x = 108;
        x <<= 2;
        if (x != 176) failures++;
    }


    {
        uint32_t a = 1788971951UL;
        uint32_t b = 901958376UL;
        uint32_t r = a | b;
        if (r != 2145635311UL) failures++;
    }


    {
        uint16_t r = add2(200,173) + add2(173,223) + add2(200,223);
        if (r != 1192) failures++;
    }


    {
        int8_t a = 13;
        int8_t b = 100;
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
        uint16_t r = call6(45,177,42,148,10,84);
        if (r != 506) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t r = call6(124,73,226,171,165,176);
        if (r != 935) failures++;
    }


    {
        volatile int16_t a = 7713;
        volatile int16_t b = 28368;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(((5 & 230) - (17 + 2)) ^ ((177 ^ 23) & (71 ^ 237)))) != 65363) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 6: result = 183; break;
        case 15: result = 149; break;
        case 5: result = 152; break;
        case 2: result = 110; break;
        case 11: result = 214; break;
        case 18: result = 144; break;
        default: result = 77; break;
        }
        if (result != 214) failures++;
    }


    {
        if (((uint16_t)(((47 | 40) & 233) & ((39 & 53) & (239 | 168)))) != 33) failures++;
    }


    {
        volatile uint8_t port = 193;
        uint8_t r = port;
        if (r != 193) failures++;
    }


    {
        uint16_t x = 28233;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {169,181,84,235,120,178,45,209};
        uint8_t *p = buf;
        p += 3;
        if (*p != 235) failures++;
    }


    {
        volatile uint8_t port = 135;
        uint8_t r = port;
        if (r != 135) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) % (int16_t)((int8_t)-89);
        if ((uint16_t)r != (uint16_t)65522) failures++;
    }


    {
        uint8_t a[6] = {177,231,193,144,48,174};
        if (a[1] != 231) failures++;
    }


    {
        uint16_t x = 32;
        x = x + 46;
        if (x != 78) failures++;
    }


    {
        uint8_t v = 92;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {189,178,8017,241};
        if (s.b != (uint8_t)178) failures++;
    }


    {
        uint8_t src[6] = {175,141,13,36,102,224};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[2] != 13) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 7: result = 122; break;
        case 1: result = 83; break;
        case 17: result = 139; break;
        case 10: result = 54; break;
        case 4: result = 208; break;
        case 11: result = 177; break;
        default: result = 122; break;
        }
        if (result != 83) failures++;
    }


    {
        g16 = 52732;
        if (read_g16() != 52732) failures++;
    }


    {
        uint8_t m[3][4] = {{51,183,226,93},{115,88,170,56},{20,121,12,134}};
        if (m[1][2] != 170) failures++;
    }


    {
        uint8_t v = 198;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 10) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(78,197) != 275) failures++;
    }


    {
        uint16_t x = 31833;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(46,183,216,125,239,226);
        if (r != 1035) failures++;
    }


    {
        uint32_t a = 2110297717UL;
        uint32_t b = 1024219024UL;
        uint32_t r = a | b;
        if (r != 2110576629UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)116) % (int16_t)((int8_t)94);
        if ((uint16_t)r != (uint16_t)22) failures++;
    }


    {
        uint8_t v = 252;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{205,13,79},{139,95,177},{72,47,111},{40,49,118}};
        if (m[0][0] != 205) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {238,78,27232,207};
        if (s.a != (uint8_t)238) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 191;
        if (buf[6] != 191) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {49,13,53224,103};
        if (s.c != (uint16_t)53224) failures++;
    }


    {
        uint16_t r = 64000 + 63288 + 61449 + 31069 + 6300 + 698 + 16750 + 22153;
        if (r != 3563) failures++;
    }


    {
        uint8_t v = 128;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 16) failures++;
    }

    return failures;
}