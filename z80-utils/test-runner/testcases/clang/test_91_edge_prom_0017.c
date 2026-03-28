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
        uint32_t a = 647944923UL;
        uint32_t b = 3021445273UL;
        uint32_t r = a ^ b;
        if (r != 2458468930UL) failures++;
    }


    {
        uint16_t r = 14511 + 57268 + 4080 + 49420 + 4373 + 60798 + 26461 + 36915;
        if (r != 57218) failures++;
    }


    {
        g16 = 50531;
        if (read_g16() != 50531) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {247,91,30565,103};
        if (s.c != (uint16_t)30565) failures++;
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
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        if (((uint16_t)226) != 226) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {78,164,39036,170};
        if (s.c != (uint16_t)39036) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t a[6] = {198,44,3,99,145,120};
        if (a[1] != 44) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {101,6,11608,14};
        if (s.b != (uint8_t)6) failures++;
    }


    {
        uint8_t v = 124;
        v ^= 8;
        if (v != 116) failures++;
    }


    {
        uint32_t a = 4238095592UL;
        uint32_t b = 1227194369UL;
        uint32_t r = a & b;
        if (r != 1208221696UL) failures++;
    }


    {
        if (((uint16_t)((111 + (30 + 194)) | 77)) != 335) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 0: result = 6; break;
        case 16: result = 252; break;
        case 8: result = 136; break;
        case 5: result = 151; break;
        default: result = 73; break;
        }
        if (result != 136) failures++;
    }


    {
        uint8_t a[6] = {39,138,7,159,142,24};
        if (a[0] != 39) failures++;
    }


    {
        if (((uint16_t)(((171 ^ 255) | (255 - 175)) ^ ((7 + 208) & (44 ^ 2)))) != 82) failures++;
    }


    {
        uint32_t a = 1259487354UL;
        uint32_t b = 4041092395UL;
        uint32_t r = a ^ b;
        if (r != 3150736721UL) failures++;
    }


    {
        uint32_t a = 3044863932UL;
        uint32_t b = 3263386461UL;
        uint32_t r = a - b;
        if (r != 4076444767UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(197,179) != 18) failures++;
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
        for (uint8_t j = 0; j < 11; j++) buf[j] = 23;
        if (buf[10] != 23) failures++;
    }


    {
        uint8_t buf[8] = {221,231,35,70,48,147,240,118};
        uint8_t *p = buf;
        p += 3;
        if (*p != 70) failures++;
    }


    {
        uint16_t x = 46;
        x = x + 7;
        if (x != 53) failures++;
    }


    {
        uint32_t a = 2273529966UL;
        uint32_t b = 3404839280UL;
        uint32_t r = a ^ b;
        if (r != 1299376414UL) failures++;
    }


    {
        uint8_t m[3][3] = {{155,68,238},{110,192,188},{167,44,26}};
        if (m[1][1] != 192) failures++;
    }


    {
        uint16_t x = 20820;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(180,125,75,41,79,232);
        if (r != 732) failures++;
    }


    {
        uint16_t r = call6(200,41,30,205,2,252);
        if (r != 730) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)66) / (int16_t)((int8_t)119);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {44,159,152,72,97,22};
        if (a[3] != 72) failures++;
    }


    {
        uint16_t x = 231;
        x = x + 67;
        if (x != 298) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(238,108) != 346) failures++;
    }


    {
        uint8_t v = 236;
        v &= ~(uint8_t)16;
        if (v != 236) failures++;
    }


    {
        if (((uint16_t)((36 ^ (116 ^ 159)) & ((217 + 130) | 104))) != 75) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        int8_t a = 114;
        int8_t b = -51;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = -32440;
        volatile int16_t b = 29004;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[3][2] = {{44,107},{97,216},{96,185}};
        if (m[1][0] != 97) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint16_t x = 171;
        x = x + 224;
        if (x != 395) failures++;
    }


    {
        uint16_t r = add2(5,35) + add2(35,87) + add2(5,87);
        if (r != 254) failures++;
    }


    {
        uint16_t x = 28050;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)35) + (uint16_t)26010;
        if (r != 26045) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)54) % (int16_t)((int8_t)-6);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {68,201,218,9,59,59,121,121};
        uint8_t *p = buf;
        p += 1;
        if (*p != 201) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-37) / (int16_t)((int8_t)50);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 32303 + 24313 + 29334 + 865 + 39328 + 49226 + 37133 + 54086;
        if (r != 4444) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 43;
        if (buf[14] != 43) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)95) + (uint16_t)56057;
        if (r != 56152) failures++;
    }


    {
        uint16_t r = 55917 + 12964 + 14714 + 26866 + 45990 + 10511 + 519 + 53440;
        if (r != 24313) failures++;
    }


    {
        uint16_t r = call6(142,94,100,222,52,148);
        if (r != 758) failures++;
    }


    {
        uint8_t a[6] = {193,115,91,168,245,170};
        if (a[5] != 170) failures++;
    }


    {
        uint8_t src[5] = {53,213,48,50,85};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 50) failures++;
    }


    {
        int8_t a = 86;
        int8_t b = 55;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 62;
        int r = (v & 8) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {66,6,178,67,120,151,109,63};
        uint8_t *p = buf;
        p += 5;
        if (*p != 151) failures++;
    }


    {
        uint16_t x = 5891;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -21;
        int8_t b = -32;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = -24360;
        volatile int16_t b = -32475;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)52) != 52) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint8_t buf[8] = {9,225,47,174,195,22,65,231};
        uint8_t *p = buf;
        p += 7;
        if (*p != 231) failures++;
    }


    {
        int8_t a = -54;
        int8_t b = -64;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 173;
        v ^= 4;
        if (v != 169) failures++;
    }


    {
        uint16_t x = 37287;
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
        uint32_t a = 1491904040UL;
        uint32_t b = 3114441598UL;
        uint32_t r = a + b;
        if (r != 311378342UL) failures++;
    }


    {
        uint32_t a = 449881412UL;
        uint32_t b = 1290794707UL;
        uint32_t r = a | b;
        if (r != 1593833431UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 48867 + 35593 + 49601 + 28430 + 10656 + 18525 + 55742 + 45286;
        if (r != 30556) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)6) % (int16_t)((int8_t)-75);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(136,147) != 283) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint16_t r = call6(10,167,152,157,234,62);
        if (r != 782) failures++;
    }


    {
        uint8_t a[6] = {172,161,225,179,245,109};
        if (a[5] != 109) failures++;
    }


    {
        uint8_t v = 29;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t src[11] = {157,14,227,64,99,146,110,159,117,169,64};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[3] != 64) failures++;
    }


    {
        uint8_t v = 135;
        v &= ~(uint8_t)32;
        if (v != 135) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-63) % (int16_t)((int8_t)-71);
        if ((uint16_t)r != (uint16_t)65473) failures++;
    }


    {
        g16 = 20125;
        if (read_g16() != 20125) failures++;
    }


    {
        uint32_t a = 3914216886UL;
        uint32_t b = 235492778UL;
        uint32_t r = a + b;
        if (r != 4149709664UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 1) sum += j;
        if (sum != 28) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint8_t v = 251;
        v ^= 2;
        if (v != 249) failures++;
    }


    {
        uint8_t v = 166;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 26) failures++;
    }


    {
        int8_t a = 72;
        int8_t b = 109;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)122) + (uint16_t)17066;
        if (r != 17188) failures++;
    }


    {
        uint8_t x = 225;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint16_t r = call6(166,113,150,112,52,251);
        if (r != 844) failures++;
    }


    {
        uint8_t v = 25;
        int r = (v & 8) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t v = 174;
        v &= ~(uint8_t)128;
        if (v != 46) failures++;
    }


    {
        uint8_t src[2] = {238,62};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 62) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint8_t a[6] = {136,54,142,237,76,39};
        if (a[4] != 76) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {153,144,949,143};
        if (s.c != (uint16_t)949) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)141) + (uint16_t)21582;
        if (r != 21723) failures++;
    }


    {
        uint8_t src[14] = {11,242,101,92,76,169,123,128,104,71,234,6,220,156};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[10] != 234) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        volatile uint8_t port = 12;
        uint8_t r = port;
        if (r != 12) failures++;
    }


    {
        volatile uint8_t port = 155;
        uint8_t r = port;
        if (r != 155) failures++;
    }


    {
        uint16_t r = add2(62,49) + add2(49,23) + add2(62,23);
        if (r != 268) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t buf[8] = {77,72,130,243,237,29,182,77};
        uint8_t *p = buf;
        p += 5;
        if (*p != 29) failures++;
    }


    {
        uint16_t x = 188;
        x = x + 212;
        if (x != 400) failures++;
    }


    {
        uint8_t v = 149;
        v ^= 16;
        if (v != 133) failures++;
    }


    {
        uint8_t x = 49;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        volatile int16_t a = -8555;
        volatile int16_t b = -28070;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 79;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t x = 54782;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 48836 + 6779 + 29594 + 43010 + 10475 + 51658 + 42521 + 54894;
        if (r != 25623) failures++;
    }


    {
        uint8_t v = 192;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)176) + (uint16_t)26141;
        if (r != 26317) failures++;
    }


    {
        g16 = 26898;
        if (read_g16() != 26898) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint8_t v = 149;
        int r = (v & 64) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = 29054 + 45022 + 50253 + 39637 + 40626 + 46223 + 29522 + 21909;
        if (r != 40102) failures++;
    }


    {
        uint8_t m[2][3] = {{118,253,241},{226,23,68}};
        if (m[1][1] != 23) failures++;
    }


    {
        volatile int16_t a = 18933;
        volatile int16_t b = 30721;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(225,158) != 67) failures++;
    }


    {
        uint8_t m[4][3] = {{51,214,203},{218,9,232},{150,114,36},{242,192,17}};
        if (m[2][0] != 150) failures++;
    }


    {
        volatile uint8_t port = 52;
        uint8_t r = port;
        if (r != 52) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 17: result = 145; break;
        case 19: result = 141; break;
        case 7: result = 127; break;
        case 11: result = 124; break;
        case 18: result = 208; break;
        default: result = 52; break;
        }
        if (result != 52) failures++;
    }


    {
        volatile int16_t a = -31757;
        volatile int16_t b = -10219;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(151,11) + add2(11,238) + add2(151,238);
        if (r != 800) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {63,3,14922,109};
        if (s.c != (uint16_t)14922) failures++;
    }


    {
        uint16_t r = call6(63,73,120,68,247,233);
        if (r != 804) failures++;
    }


    {
        uint32_t a = 1628641007UL;
        uint32_t b = 3562721548UL;
        uint32_t r = a & b;
        if (r != 1074925580UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {62,14,17545,94};
        if (s.c != (uint16_t)17545) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t src[11] = {24,146,21,127,246,140,230,181,202,57,36};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[9] != 57) failures++;
    }


    {
        uint16_t r = call6(139,190,133,148,154,203);
        if (r != 967) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)84) % (int16_t)((int8_t)-74);
        if ((uint16_t)r != (uint16_t)10) failures++;
    }


    {
        int8_t a = 79;
        int8_t b = 7;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {212,224,19898,14};
        if (s.a != (uint8_t)212) failures++;
    }


    {
        uint16_t x = 95;
        x = x + 76;
        if (x != 171) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)77) + (uint16_t)13511;
        if (r != 13588) failures++;
    }


    {
        uint8_t a[6] = {102,198,43,22,132,14};
        if (a[5] != 14) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 16: result = 230; break;
        case 8: result = 17; break;
        case 18: result = 133; break;
        default: result = 250; break;
        }
        if (result != 133) failures++;
    }


    {
        volatile int16_t a = 1195;
        volatile int16_t b = -12634;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 57;
        x <<= 2;
        if (x != 228) failures++;
    }


    {
        uint16_t r = 61956 + 61949 + 37621 + 14405 + 30173 + 13333 + 61928 + 44775;
        if (r != 63996) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {8,110,15504,236};
        if (s.a != (uint8_t)8) failures++;
    }


    {
        uint32_t a = 2545242853UL;
        uint32_t b = 2018216524UL;
        uint32_t r = a & b;
        if (r != 268501572UL) failures++;
    }


    {
        uint8_t m[2][4] = {{5,182,34,107},{37,131,246,159}};
        if (m[0][3] != 107) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        volatile uint8_t port = 213;
        uint8_t r = port;
        if (r != 213) failures++;
    }


    {
        uint16_t r = add2(212,210) + add2(210,187) + add2(212,187);
        if (r != 1218) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {159,255,48,119,90,177,116,183};
        uint8_t *p = buf;
        p += 6;
        if (*p != 116) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)39) + (uint16_t)43399;
        if (r != 43438) failures++;
    }


    {
        uint16_t r = add2(52,6) + add2(6,137) + add2(52,137);
        if (r != 390) failures++;
    }


    {
        volatile uint8_t port = 126;
        uint8_t r = port;
        if (r != 126) failures++;
    }


    {
        uint8_t a[6] = {31,89,15,239,80,85};
        if (a[0] != 31) failures++;
    }


    {
        uint16_t r = add2(161,95) + add2(95,241) + add2(161,241);
        if (r != 994) failures++;
    }


    {
        if (((uint16_t)165) != 165) failures++;
    }


    {
        uint8_t buf[8] = {213,70,226,119,99,233,141,170};
        uint8_t *p = buf;
        p += 7;
        if (*p != 170) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 68;
        if (buf[11] != 68) failures++;
    }


    {
        uint32_t a = 1076926905UL;
        uint32_t b = 1573469763UL;
        uint32_t r = a + b;
        if (r != 2650396668UL) failures++;
    }


    {
        uint8_t src[14] = {175,238,125,55,192,120,233,133,35,246,169,133,200,200};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[2] != 125) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 1: result = 238; break;
        case 0: result = 155; break;
        case 2: result = 90; break;
        default: result = 230; break;
        }
        if (result != 230) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(128,43) != 85) failures++;
    }


    {
        uint8_t src[12] = {199,179,93,144,164,208,38,5,137,238,1,51};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[0] != 199) failures++;
    }


    {
        uint16_t r = call6(23,90,253,251,37,238);
        if (r != 892) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)38) / (int16_t)((int8_t)-3);
        if ((uint16_t)r != (uint16_t)65524) failures++;
    }


    {
        uint16_t r = call6(67,133,121,222,180,15);
        if (r != 738) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)150) + (uint16_t)45211;
        if (r != 45361) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 249;
        if (buf[14] != 249) failures++;
    }


    {
        uint8_t m[4][3] = {{174,203,26},{51,251,31},{55,176,23},{56,13,227}};
        if (m[1][0] != 51) failures++;
    }


    {
        volatile int16_t a = 29803;
        volatile int16_t b = 23690;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint8_t v = 8;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 24) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint16_t r = call6(207,63,155,110,183,8);
        if (r != 726) failures++;
    }


    {
        uint16_t x = 36525;
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
        uint8_t v = 29;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t x = 199;
        x = x + 223;
        if (x != 422) failures++;
    }


    {
        uint8_t x = 70;
        x <<= 2;
        if (x != 24) failures++;
    }


    {
        volatile uint8_t port = 11;
        uint8_t r = port;
        if (r != 11) failures++;
    }


    {
        uint16_t x = 29;
        x = x + 106;
        if (x != 135) failures++;
    }


    {
        if (((uint16_t)(((195 - 236) + (43 | 83)) | (47 & (112 & 110)))) != 114) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(160,176) != 336) failures++;
    }


    {
        uint16_t x = 11047;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 62902;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 209;
        uint8_t r = port;
        if (r != 209) failures++;
    }


    {
        volatile int16_t a = -10983;
        volatile int16_t b = -28416;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 3802381101UL;
        uint32_t b = 2815892083UL;
        uint32_t r = a + b;
        if (r != 2323305888UL) failures++;
    }


    {
        if (((uint16_t)(237 + 159)) != 396) failures++;
    }


    {
        uint8_t v = 29;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        int8_t a = -82;
        int8_t b = 105;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        if (((uint16_t)(((213 + 17) & 253) | ((206 ^ 127) - (58 + 133)))) != 65526) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 203;
        if (buf[2] != 203) failures++;
    }


    {
        uint8_t x = 118;
        x <<= 4;
        if (x != 96) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {248,212,20635,49};
        if (s.c != (uint16_t)20635) failures++;
    }


    {
        uint8_t buf[8] = {234,171,14,83,239,53,222,255};
        uint8_t *p = buf;
        p += 1;
        if (*p != 171) failures++;
    }


    {
        if (((uint16_t)82) != 82) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 16: result = 50; break;
        case 17: result = 134; break;
        case 7: result = 131; break;
        case 12: result = 182; break;
        case 15: result = 212; break;
        case 4: result = 144; break;
        default: result = 40; break;
        }
        if (result != 40) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t v = 43;
        v ^= 1;
        if (v != 42) failures++;
    }


    {
        uint16_t x = 25508;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(128,217,250,99,61,138);
        if (r != 893) failures++;
    }


    {
        g16 = 61084;
        if (read_g16() != 61084) failures++;
    }


    {
        volatile uint8_t port = 124;
        uint8_t r = port;
        if (r != 124) failures++;
    }


    {
        int8_t a = 6;
        int8_t b = -111;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {101,10,183,242,94,100,144,132};
        uint8_t *p = buf;
        p += 1;
        if (*p != 10) failures++;
    }


    {
        uint8_t src[15] = {81,213,108,237,231,103,142,196,64,200,4,24,94,195,103};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[10] != 4) failures++;
    }


    {
        uint8_t m[2][3] = {{54,42,90},{21,196,240}};
        if (m[1][0] != 21) failures++;
    }


    {
        uint8_t v = 204;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 20) failures++;
    }


    {
        uint16_t r = add2(115,128) + add2(128,45) + add2(115,45);
        if (r != 576) failures++;
    }


    {
        int8_t a = 104;
        int8_t b = 60;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 44415;
        if (read_g16() != 44415) failures++;
    }


    {
        uint8_t v = 170;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        uint16_t r = call6(23,245,193,234,245,92);
        if (r != 1032) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint16_t r = add2(137,239) + add2(239,71) + add2(137,71);
        if (r != 894) failures++;
    }


    {
        uint8_t m[2][2] = {{13,103},{240,151}};
        if (m[0][1] != 103) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 17: result = 208; break;
        case 16: result = 135; break;
        case 2: result = 211; break;
        case 19: result = 162; break;
        case 18: result = 136; break;
        case 14: result = 205; break;
        default: result = 74; break;
        }
        if (result != 162) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(152,248) != 400) failures++;
    }


    {
        uint8_t x = 202;
        x <<= 0;
        if (x != 202) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        volatile uint8_t port = 249;
        uint8_t r = port;
        if (r != 249) failures++;
    }


    {
        g16 = 19810;
        if (read_g16() != 19810) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)86) + (uint16_t)64017;
        if (r != 64103) failures++;
    }


    {
        uint16_t x = 148;
        x = x + 76;
        if (x != 224) failures++;
    }


    {
        uint16_t x = 254;
        x = x + 65;
        if (x != 319) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {96,239,64068,127};
        if (s.a != (uint8_t)96) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) / (int16_t)((int8_t)122);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        g16 = 27271;
        if (read_g16() != 27271) failures++;
    }


    {
        uint8_t src[7] = {229,250,162,57,253,38,226};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[3] != 57) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 230; break;
        case 16: result = 136; break;
        case 0: result = 117; break;
        default: result = 236; break;
        }
        if (result != 236) failures++;
    }


    {
        volatile int16_t a = -802;
        volatile int16_t b = 2202;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {113,143,254,127,123,224,139,128};
        uint8_t *p = buf;
        p += 0;
        if (*p != 113) failures++;
    }


    {
        g16 = 60265;
        if (read_g16() != 60265) failures++;
    }


    {
        if (((uint16_t)(227 + (43 + (236 ^ 40)))) != 466) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 200;
        if (buf[13] != 200) failures++;
    }


    {
        uint32_t a = 1376325038UL;
        uint32_t b = 61592443UL;
        uint32_t r = a & b;
        if (r != 34144554UL) failures++;
    }


    {
        uint8_t v = 193;
        v ^= 8;
        if (v != 201) failures++;
    }


    {
        volatile uint8_t port = 99;
        uint8_t r = port;
        if (r != 99) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = call6(191,203,117,206,36,64);
        if (r != 817) failures++;
    }


    {
        uint16_t x = 40;
        x = x + 87;
        if (x != 127) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(254 & ((228 | 56) + (9 & 188)))) != 4) failures++;
    }


    {
        uint16_t r = 31602 + 26914 + 54119 + 13618 + 45236 + 20645 + 26239 + 2901;
        if (r != 24666) failures++;
    }


    {
        uint16_t r = 40760 + 65215 + 54800 + 34274 + 36356 + 28018 + 63826 + 14354;
        if (r != 9923) failures++;
    }


    {
        uint8_t v = 157;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 35) failures++;
    }


    {
        uint8_t src[13] = {186,40,76,116,194,240,31,78,88,167,195,60,150};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[10] != 195) failures++;
    }


    {
        uint32_t a = 3309932444UL;
        uint32_t b = 741945915UL;
        uint32_t r = a - b;
        if (r != 2567986529UL) failures++;
    }


    {
        g16 = 41237;
        if (read_g16() != 41237) failures++;
    }


    {
        uint8_t v = 28;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {115,12,215,107,102,17};
        if (a[1] != 12) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[3][2] = {{149,126},{103,60},{79,215}};
        if (m[2][0] != 79) failures++;
    }


    {
        uint8_t m[4][3] = {{206,112,207},{79,227,234},{32,181,128},{206,67,126}};
        if (m[2][1] != 181) failures++;
    }


    {
        volatile int16_t a = -32048;
        volatile int16_t b = -14927;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {74,148,32313,226};
        if (s.c != (uint16_t)32313) failures++;
    }


    {
        uint8_t v = 123;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 171;
        uint8_t r = port;
        if (r != 171) failures++;
    }


    {
        uint8_t src[16] = {10,113,189,86,199,149,23,187,234,152,231,186,84,205,83,199};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[0] != 10) failures++;
    }


    {
        uint8_t x = 149;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint16_t r = add2(245,48) + add2(48,24) + add2(245,24);
        if (r != 634) failures++;
    }


    {
        uint16_t x = 239;
        x = x + 156;
        if (x != 395) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 1) sum += j;
        if (sum != 15) failures++;
    }


    {
        if (((uint16_t)(145 + 232)) != 377) failures++;
    }


    {
        uint8_t src[1] = {210};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 210) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint32_t a = 3718588445UL;
        uint32_t b = 2324169217UL;
        uint32_t r = a + b;
        if (r != 1747790366UL) failures++;
    }


    {
        volatile uint8_t port = 238;
        uint8_t r = port;
        if (r != 238) failures++;
    }


    {
        uint16_t x = 55644;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 46316 + 52668 + 56674 + 25570 + 60446 + 25024 + 59766 + 47579;
        if (r != 46363) failures++;
    }


    {
        uint16_t x = 26218;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(191,173) + add2(173,45) + add2(191,45);
        if (r != 818) failures++;
    }


    {
        volatile int16_t a = 27689;
        volatile int16_t b = -31939;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)72) + (uint16_t)30293;
        if (r != 30365) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 165;
        if (buf[2] != 165) failures++;
    }


    {
        uint16_t x = 58392;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 180;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t src[14] = {193,147,124,22,135,186,155,72,187,226,244,208,140,72};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[0] != 193) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)68) + (uint16_t)50810;
        if (r != 50878) failures++;
    }


    {
        uint16_t r = add2(207,5) + add2(5,213) + add2(207,213);
        if (r != 850) failures++;
    }


    {
        uint16_t x = 4599;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 31442;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[5] = {176,250,104,92,150};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 92) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 127;
        int8_t b = 67;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 243;
        x <<= 0;
        if (x != 243) failures++;
    }


    {
        uint16_t x = 177;
        x = x + 250;
        if (x != 427) failures++;
    }


    {
        uint16_t r = 8285 + 40329 + 9875 + 14296 + 18447 + 43987 + 54124 + 13252;
        if (r != 5987) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 140;
        x <<= 2;
        if (x != 48) failures++;
    }


    {
        uint8_t a[6] = {125,194,104,102,10,199};
        if (a[0] != 125) failures++;
    }


    {
        if (((uint16_t)141) != 141) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-101) / (int16_t)((int8_t)5);
        if ((uint16_t)r != (uint16_t)65516) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {141,246,42752,205};
        if (s.b != (uint8_t)246) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {95,96,54630,145};
        if (s.c != (uint16_t)54630) failures++;
    }


    {
        uint8_t m[2][4] = {{250,251,131,211},{165,26,128,33}};
        if (m[1][0] != 165) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 28368 + 42803 + 57932 + 60075 + 25344 + 2820 + 54085 + 6837;
        if (r != 16120) failures++;
    }


    {
        volatile int16_t a = -21719;
        volatile int16_t b = 161;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(171,225) != 396) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)2) + (uint16_t)451;
        if (r != 453) failures++;
    }


    {
        if (((uint16_t)(206 + 32)) != 238) failures++;
    }


    {
        uint8_t m[2][4] = {{131,213,133,250},{80,19,187,10}};
        if (m[0][1] != 213) failures++;
    }


    {
        uint8_t x = 165;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)83) / (int16_t)((int8_t)102);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {217,199,162,75,140,177};
        if (a[2] != 162) failures++;
    }


    {
        uint8_t src[11] = {168,67,27,11,249,0,172,217,144,23,178};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[6] != 172) failures++;
    }


    {
        g16 = 11561;
        if (read_g16() != 11561) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)49) + (uint16_t)32152;
        if (r != 32201) failures++;
    }


    {
        uint16_t r = call6(115,171,230,188,82,74);
        if (r != 860) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {212,163,43283,195};
        if (s.a != (uint8_t)212) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 42339;
        if (read_g16() != 42339) failures++;
    }


    {
        uint16_t r = add2(109,78) + add2(78,167) + add2(109,167);
        if (r != 708) failures++;
    }


    {
        uint8_t x = 116;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint8_t v = 5;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {224,230,26903,60};
        if (s.c != (uint16_t)26903) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {9,136,25252,38};
        if (s.c != (uint16_t)25252) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {112,236,11980,166};
        if (s.c != (uint16_t)11980) failures++;
    }


    {
        int8_t a = -67;
        int8_t b = -121;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 1822455282UL;
        uint32_t b = 3136189273UL;
        uint32_t r = a | b;
        if (r != 4277041147UL) failures++;
    }


    {
        uint16_t r = call6(251,237,41,255,1,182);
        if (r != 967) failures++;
    }


    {
        uint16_t r = 21669 + 577 + 7673 + 39121 + 37641 + 11289 + 18828 + 41113;
        if (r != 46839) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)14) / (int16_t)((int8_t)-70);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t a[6] = {238,24,51,117,204,206};
        if (a[5] != 206) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)114) / (int16_t)((int8_t)28);
        if ((uint16_t)r != (uint16_t)4) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)132) + (uint16_t)18057;
        if (r != 18189) failures++;
    }


    {
        if (((uint16_t)(((60 ^ 92) ^ 140) | 123)) != 255) failures++;
    }


    {
        uint8_t v = 39;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 4083;
        if (read_g16() != 4083) failures++;
    }


    {
        int8_t a = -116;
        int8_t b = 92;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 251;
        x = x + 239;
        if (x != 490) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 225;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t m[3][2] = {{63,32},{195,195},{100,222}};
        if (m[0][0] != 63) failures++;
    }


    {
        volatile uint8_t port = 75;
        uint8_t r = port;
        if (r != 75) failures++;
    }


    {
        uint8_t v = 155;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-82) % (int16_t)((int8_t)121);
        if ((uint16_t)r != (uint16_t)65454) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 52;
        if (buf[4] != 52) failures++;
    }


    {
        uint16_t r = add2(36,126) + add2(126,44) + add2(36,44);
        if (r != 412) failures++;
    }


    {
        uint16_t r = call6(166,32,143,201,24,75);
        if (r != 641) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        if (((uint16_t)(((1 - 46) - 229) + 229)) != 65491) failures++;
    }


    {
        int8_t a = -6;
        int8_t b = 36;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 15: result = 5; break;
        case 3: result = 28; break;
        case 9: result = 77; break;
        case 2: result = 149; break;
        case 13: result = 231; break;
        case 0: result = 104; break;
        case 5: result = 215; break;
        default: result = 114; break;
        }
        if (result != 5) failures++;
    }


    {
        if (((uint16_t)((93 + (116 + 204)) & ((182 - 89) - (224 ^ 113)))) != 396) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 15: result = 247; break;
        case 18: result = 187; break;
        case 19: result = 45; break;
        case 0: result = 37; break;
        default: result = 23; break;
        }
        if (result != 45) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 108;
        if (buf[8] != 108) failures++;
    }


    {
        uint8_t src[9] = {243,125,158,182,167,166,204,167,186};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[6] != 204) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        if (((uint16_t)142) != 142) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {22,119,56909,187};
        if (s.c != (uint16_t)56909) failures++;
    }


    {
        uint16_t r = add2(6,203) + add2(203,87) + add2(6,87);
        if (r != 592) failures++;
    }


    {
        uint16_t x = 57607;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 52137 + 64561 + 14585 + 18392 + 58981 + 57827 + 32081 + 39226;
        if (r != 10110) failures++;
    }


    {
        g16 = 24342;
        if (read_g16() != 24342) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(239,234) != 5) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)99) + (uint16_t)23360;
        if (r != 23459) failures++;
    }


    {
        g16 = 55010;
        if (read_g16() != 55010) failures++;
    }


    {
        uint8_t a[6] = {78,99,215,18,49,110};
        if (a[5] != 110) failures++;
    }


    {
        uint8_t a[6] = {204,21,132,113,228,84};
        if (a[1] != 21) failures++;
    }


    {
        uint8_t buf[8] = {156,213,51,82,72,4,143,177};
        uint8_t *p = buf;
        p += 2;
        if (*p != 51) failures++;
    }


    {
        volatile uint8_t port = 56;
        uint8_t r = port;
        if (r != 56) failures++;
    }


    {
        volatile uint8_t port = 110;
        uint8_t r = port;
        if (r != 110) failures++;
    }


    {
        uint8_t a[6] = {83,247,29,53,67,50};
        if (a[0] != 83) failures++;
    }


    {
        uint8_t v = 92;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {4,241,226,8,198,29};
        if (a[3] != 8) failures++;
    }


    {
        uint16_t r = 22468 + 65059 + 64021 + 49310 + 63331 + 44236 + 37018 + 21469;
        if (r != 39232) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        volatile uint8_t port = 79;
        uint8_t r = port;
        if (r != 79) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {117,126,31277,178};
        if (s.a != (uint8_t)117) failures++;
    }


    {
        uint16_t r = 30610 + 38433 + 59122 + 40470 + 12925 + 47734 + 47500 + 45413;
        if (r != 60063) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)27) % (int16_t)((int8_t)-123);
        if ((uint16_t)r != (uint16_t)27) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 213;
        if (buf[11] != 213) failures++;
    }


    {
        uint8_t x = 200;
        x <<= 1;
        if (x != 144) failures++;
    }


    {
        uint16_t r = add2(141,28) + add2(28,32) + add2(141,32);
        if (r != 402) failures++;
    }


    {
        g16 = 2715;
        if (read_g16() != 2715) failures++;
    }


    {
        uint8_t buf[8] = {29,157,188,130,169,227,195,37};
        uint8_t *p = buf;
        p += 1;
        if (*p != 157) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)59) % (int16_t)((int8_t)78);
        if ((uint16_t)r != (uint16_t)59) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 1) sum += j;
        if (sum != 171) failures++;
    }


    {
        uint8_t v = 87;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][2] = {{34,32},{93,40}};
        if (m[1][0] != 93) failures++;
    }


    {
        uint16_t r = call6(127,67,208,110,113,197);
        if (r != 822) failures++;
    }


    {
        volatile uint8_t port = 194;
        uint8_t r = port;
        if (r != 194) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)244) + (uint16_t)43094;
        if (r != 43338) failures++;
    }


    {
        if (((uint16_t)(((112 - 142) - (223 - 87)) ^ ((238 ^ 23) & 205))) != 65427) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        g16 = 22907;
        if (read_g16() != 22907) failures++;
    }


    {
        volatile int16_t a = -7235;
        volatile int16_t b = 18478;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{79,211,13},{76,198,149},{100,137,24},{191,242,219}};
        if (m[3][2] != 219) failures++;
    }


    {
        uint16_t x = 1896;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 40777;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 130; break;
        case 4: result = 39; break;
        case 17: result = 82; break;
        case 6: result = 135; break;
        case 9: result = 158; break;
        default: result = 66; break;
        }
        if (result != 130) failures++;
    }


    {
        volatile uint8_t port = 225;
        uint8_t r = port;
        if (r != 225) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 4) sum += j;
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
        uint16_t r = call6(233,91,88,144,200,82);
        if (r != 838) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 119;
        if (buf[7] != 119) failures++;
    }


    {
        uint8_t v = 196;
        v &= ~(uint8_t)16;
        if (v != 196) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {216,112,39082,77};
        if (s.b != (uint8_t)112) failures++;
    }


    {
        uint32_t a = 2019195163UL;
        uint32_t b = 2894677581UL;
        uint32_t r = a - b;
        if (r != 3419484878UL) failures++;
    }


    {
        volatile uint8_t port = 81;
        uint8_t r = port;
        if (r != 81) failures++;
    }


    {
        int8_t a = 39;
        int8_t b = 35;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {77,142,117,22,94,32};
        if (a[5] != 32) failures++;
    }


    {
        volatile int16_t a = 25076;
        volatile int16_t b = -29922;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 158;
        x = x + 189;
        if (x != 347) failures++;
    }


    {
        volatile int16_t a = -4302;
        volatile int16_t b = 30272;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -116;
        int8_t b = -108;
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
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(240,7) != 247) failures++;
    }


    {
        uint16_t r = call6(155,183,251,168,212,130);
        if (r != 1099) failures++;
    }


    {
        uint8_t buf[8] = {56,5,213,68,35,47,96,252};
        uint8_t *p = buf;
        p += 5;
        if (*p != 47) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 13: result = 227; break;
        case 15: result = 232; break;
        case 5: result = 215; break;
        case 8: result = 228; break;
        case 14: result = 90; break;
        case 3: result = 119; break;
        default: result = 5; break;
        }
        if (result != 228) failures++;
    }


    {
        uint16_t r = add2(254,30) + add2(30,245) + add2(254,245);
        if (r != 1058) failures++;
    }


    {
        uint8_t m[2][2] = {{113,98},{237,94}};
        if (m[0][1] != 98) failures++;
    }


    {
        uint8_t v = 13;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{8,117,152},{210,47,186},{145,174,154},{101,111,53}};
        if (m[0][0] != 8) failures++;
    }


    {
        uint8_t src[10] = {102,178,70,16,80,10,150,222,229,42};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[4] != 80) failures++;
    }


    {
        uint8_t x = 87;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t r = call6(203,250,218,221,224,221);
        if (r != 1337) failures++;
    }


    {
        volatile int16_t a = -24493;
        volatile int16_t b = 12680;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(214,25,70,109,250,142);
        if (r != 810) failures++;
    }


    {
        volatile uint8_t port = 34;
        uint8_t r = port;
        if (r != 34) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)38) + (uint16_t)58437;
        if (r != 58475) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)106) + (uint16_t)49678;
        if (r != 49784) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(60,65) != 125) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)80) + (uint16_t)21781;
        if (r != 21861) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 2;
        if (buf[8] != 2) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint16_t x = 24632;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)67) + (uint16_t)14105;
        if (r != 14172) failures++;
    }


    {
        uint8_t v = 164;
        v |= 16;
        if (v != 180) failures++;
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
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint8_t v = 178;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 17: result = 137; break;
        case 18: result = 206; break;
        case 7: result = 245; break;
        case 8: result = 144; break;
        case 2: result = 63; break;
        case 12: result = 96; break;
        case 16: result = 135; break;
        case 6: result = 255; break;
        default: result = 120; break;
        }
        if (result != 96) failures++;
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {126,180,56427,7};
        if (s.c != (uint16_t)56427) failures++;
    }


    {
        uint8_t x = 128;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t v = 175;
        v |= 64;
        if (v != 239) failures++;
    }


    {
        uint8_t src[5] = {245,154,117,226,112};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 117) failures++;
    }


    {
        uint16_t x = 218;
        x = x + 145;
        if (x != 363) failures++;
    }


    {
        int8_t a = 68;
        int8_t b = 18;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][4] = {{75,6,70,15},{109,102,19,229},{237,157,142,191},{115,149,53,194}};
        if (m[0][3] != 15) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)94) + (uint16_t)5806;
        if (r != 5900) failures++;
    }


    {
        if (((uint16_t)152) != 152) failures++;
    }


    {
        uint16_t x = 58030;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 91;
        x <<= 1;
        if (x != 182) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint16_t x = 42386;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[7] = {102,148,150,208,35,21,39};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[0] != 102) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {107,148,20275,33};
        if (s.a != (uint8_t)107) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {61,158,59359,121};
        if (s.a != (uint8_t)61) failures++;
    }


    {
        uint16_t r = add2(92,76) + add2(76,86) + add2(92,86);
        if (r != 508) failures++;
    }


    {
        uint8_t v = 40;
        v ^= 32;
        if (v != 8) failures++;
    }


    {
        uint8_t v = 177;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 15) failures++;
    }


    {
        uint8_t v = 177;
        v |= 64;
        if (v != 241) failures++;
    }


    {
        uint16_t x = 19872;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)247) != 247) failures++;
    }


    {
        uint8_t x = 173;
        x <<= 0;
        if (x != 173) failures++;
    }


    {
        uint16_t x = 202;
        x = x + 124;
        if (x != 326) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 5: result = 247; break;
        case 6: result = 175; break;
        case 9: result = 207; break;
        default: result = 190; break;
        }
        if (result != 175) failures++;
    }


    {
        uint8_t m[3][3] = {{126,42,157},{47,178,85},{89,30,33}};
        if (m[1][0] != 47) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 61;
        x <<= 2;
        if (x != 244) failures++;
    }


    {
        uint16_t r = 47463 + 35433 + 25939 + 11221 + 64436 + 60949 + 16833 + 44871;
        if (r != 45001) failures++;
    }


    {
        uint8_t v = 163;
        v &= ~(uint8_t)2;
        if (v != 161) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 14: result = 58; break;
        case 7: result = 12; break;
        case 13: result = 244; break;
        case 10: result = 60; break;
        default: result = 164; break;
        }
        if (result != 244) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(85,72) != 157) failures++;
    }


    {
        uint8_t v = 136;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 56) failures++;
    }


    {
        uint8_t m[2][4] = {{115,1,250,231},{28,75,85,254}};
        if (m[0][0] != 115) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {60,198,65460,26};
        if (s.d != (uint8_t)26) failures++;
    }


    {
        volatile uint8_t port = 9;
        uint8_t r = port;
        if (r != 9) failures++;
    }


    {
        volatile uint8_t port = 11;
        uint8_t r = port;
        if (r != 11) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(136,42) != 178) failures++;
    }


    {
        uint32_t a = 1477621479UL;
        uint32_t b = 1327330494UL;
        uint32_t r = a | b;
        if (r != 1595930367UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(157,24) != 181) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {219,119,52906,136};
        if (s.d != (uint8_t)136) failures++;
    }


    {
        uint16_t r = call6(14,96,207,84,53,127);
        if (r != 581) failures++;
    }


    {
        if (((uint16_t)150) != 150) failures++;
    }


    {
        g16 = 60574;
        if (read_g16() != 60574) failures++;
    }


    {
        uint16_t r = call6(157,225,252,109,149,219);
        if (r != 1111) failures++;
    }


    {
        uint8_t buf[8] = {243,227,46,182,149,238,13,47};
        uint8_t *p = buf;
        p += 2;
        if (*p != 46) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)80) + (uint16_t)58059;
        if (r != 58139) failures++;
    }


    {
        uint8_t v = 220;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t m[4][4] = {{12,93,18,82},{58,161,210,30},{79,56,79,23},{191,216,246,114}};
        if (m[0][1] != 93) failures++;
    }


    {
        uint8_t m[3][2] = {{101,64},{83,122},{27,28}};
        if (m[1][1] != 122) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-6) / (int16_t)((int8_t)118);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile int16_t a = -17025;
        volatile int16_t b = -25295;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(221,27,15,250,232,24);
        if (r != 769) failures++;
    }


    {
        uint32_t a = 1249010329UL;
        uint32_t b = 3386584102UL;
        uint32_t r = a | b;
        if (r != 3422252735UL) failures++;
    }


    {
        int8_t a = 10;
        int8_t b = -117;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 136;
        x <<= 3;
        if (x != 64) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)76) / (int16_t)((int8_t)-75);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(89,182) != 271) failures++;
    }


    {
        uint8_t v = 223;
        v ^= 2;
        if (v != 221) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)218) + (uint16_t)61447;
        if (r != 61665) failures++;
    }


    {
        uint8_t src[8] = {5,1,169,60,49,222,9,156};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[3] != 60) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)181) + (uint16_t)53293;
        if (r != 53474) failures++;
    }


    {
        uint8_t v = 6;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 26) failures++;
    }


    {
        uint16_t r = call6(87,206,9,79,27,124);
        if (r != 532) failures++;
    }


    {
        if (((uint16_t)5) != 5) failures++;
    }


    {
        uint16_t x = 67;
        x = x + 251;
        if (x != 318) failures++;
    }


    {
        uint16_t x = 52461;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 33781 + 55796 + 32418 + 23211 + 37890 + 46683 + 3994 + 26287;
        if (r != 63452) failures++;
    }


    {
        uint16_t x = 53676;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 23586 + 42369 + 29492 + 29464 + 25462 + 7595 + 58614 + 46548;
        if (r != 986) failures++;
    }


    {
        uint8_t m[3][4] = {{106,77,198,125},{158,173,60,5},{219,234,185,7}};
        if (m[1][2] != 60) failures++;
    }


    {
        if (((uint16_t)(((162 + 245) ^ (14 | 56)) - 191)) != 234) failures++;
    }


    {
        uint8_t v = 69;
        v |= 8;
        if (v != 77) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(82,202) != 65416) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(143,126) != 17) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(44,39) != 83) failures++;
    }


    {
        uint16_t x = 62221;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[2][2] = {{177,207},{121,24}};
        if (m[1][1] != 24) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {155,240,19787,187};
        if (s.a != (uint8_t)155) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-2) % (int16_t)((int8_t)-125);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint8_t a[6] = {103,212,17,107,2,56};
        if (a[5] != 56) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 250;
        if (buf[10] != 250) failures++;
    }


    {
        volatile int16_t a = -5293;
        volatile int16_t b = -30204;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t buf[8] = {2,196,138,79,226,47,104,124};
        uint8_t *p = buf;
        p += 6;
        if (*p != 104) failures++;
    }


    {
        uint8_t v = 128;
        v |= 128;
        if (v != 128) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-61) % (int16_t)((int8_t)-105);
        if ((uint16_t)r != (uint16_t)65475) failures++;
    }


    {
        g16 = 4873;
        if (read_g16() != 4873) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)58) + (uint16_t)16475;
        if (r != 16533) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
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
        uint8_t a[6] = {169,75,198,213,129,90};
        if (a[5] != 90) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 11: result = 27; break;
        case 9: result = 167; break;
        case 17: result = 61; break;
        default: result = 156; break;
        }
        if (result != 61) failures++;
    }


    {
        g16 = 18559;
        if (read_g16() != 18559) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {17,251,3246,188};
        if (s.b != (uint8_t)251) failures++;
    }


    {
        uint8_t x = 132;
        x <<= 2;
        if (x != 16) failures++;
    }


    {
        uint8_t a[6] = {36,192,113,163,214,97};
        if (a[2] != 113) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)73) % (int16_t)((int8_t)85);
        if ((uint16_t)r != (uint16_t)73) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)6) / (int16_t)((int8_t)19);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 33167 + 62868 + 60845 + 42680 + 52666 + 17214 + 4127 + 9076;
        if (r != 20499) failures++;
    }


    {
        uint8_t src[8] = {174,17,253,26,36,31,195,182};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 17) failures++;
    }


    {
        int8_t a = -42;
        int8_t b = 73;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 11399;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 44106;
        if (read_g16() != 44106) failures++;
    }


    {
        uint8_t v = 212;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 12) failures++;
    }


    {
        volatile uint8_t port = 29;
        uint8_t r = port;
        if (r != 29) failures++;
    }


    {
        uint8_t v = 75;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint32_t a = 2298631913UL;
        uint32_t b = 179171534UL;
        uint32_t r = a & b;
        if (r != 134238408UL) failures++;
    }


    {
        uint16_t x = 52990;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[7] = {190,216,11,54,245,219,211};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[5] != 219) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 1) sum += j;
        if (sum != 78) failures++;
    }


    {
        uint16_t r = call6(69,199,52,27,106,145);
        if (r != 598) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)167) + (uint16_t)19924;
        if (r != 20091) failures++;
    }


    {
        g16 = 730;
        if (read_g16() != 730) failures++;
    }


    {
        uint8_t a[6] = {104,20,143,233,243,192};
        if (a[0] != 104) failures++;
    }


    {
        uint8_t x = 151;
        x <<= 3;
        if (x != 184) failures++;
    }


    {
        uint8_t v = 179;
        v |= 8;
        if (v != 187) failures++;
    }


    {
        uint8_t x = 164;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        uint16_t r = add2(156,237) + add2(237,7) + add2(156,7);
        if (r != 800) failures++;
    }


    {
        uint8_t v = 181;
        v ^= 8;
        if (v != 189) failures++;
    }


    {
        uint16_t x = 52366;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(205 | (234 ^ (68 + 58)))) != 221) failures++;
    }


    {
        uint16_t x = 108;
        x = x + 166;
        if (x != 274) failures++;
    }


    {
        uint16_t x = 50335;
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
        uint8_t src[7] = {93,13,182,161,203,70,41};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[4] != 203) failures++;
    }


    {
        int8_t a = 31;
        int8_t b = 112;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 0: result = 180; break;
        case 12: result = 224; break;
        case 3: result = 229; break;
        case 5: result = 64; break;
        case 19: result = 89; break;
        case 11: result = 67; break;
        case 8: result = 6; break;
        case 7: result = 199; break;
        default: result = 23; break;
        }
        if (result != 199) failures++;
    }


    {
        g16 = 13012;
        if (read_g16() != 13012) failures++;
    }


    {
        uint8_t buf[8] = {221,104,105,2,87,248,144,142};
        uint8_t *p = buf;
        p += 0;
        if (*p != 221) failures++;
    }


    {
        uint32_t a = 2472729330UL;
        uint32_t b = 4054039165UL;
        uint32_t r = a ^ b;
        if (r != 1656840335UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        int8_t a = 68;
        int8_t b = 46;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(76,128) != 204) failures++;
    }


    {
        volatile uint8_t port = 187;
        uint8_t r = port;
        if (r != 187) failures++;
    }


    {
        uint16_t r = add2(175,172) + add2(172,184) + add2(175,184);
        if (r != 1062) failures++;
    }


    {
        if (((uint16_t)(((23 & 157) & (85 | 2)) | 233)) != 253) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(172,117) != 289) failures++;
    }


    {
        uint8_t a[6] = {114,125,204,8,80,34};
        if (a[0] != 114) failures++;
    }


    {
        uint8_t buf[8] = {182,224,111,215,175,182,60,194};
        uint8_t *p = buf;
        p += 7;
        if (*p != 194) failures++;
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
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        volatile int16_t a = -32190;
        volatile int16_t b = -10540;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        g16 = 25782;
        if (read_g16() != 25782) failures++;
    }

    return failures;
}