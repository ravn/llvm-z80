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
        if (fn(93,252) != 65377) failures++;
    }


    {
        uint32_t a = 2650864204UL;
        uint32_t b = 131529221UL;
        uint32_t r = a | b;
        if (r != 2681666125UL) failures++;
    }


    {
        uint8_t x = 255;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 13: result = 161; break;
        case 15: result = 237; break;
        case 9: result = 211; break;
        default: result = 32; break;
        }
        if (result != 211) failures++;
    }


    {
        if (((uint16_t)(((92 | 72) + (151 ^ 181)) | ((208 + 243) | (251 - 154)))) != 511) failures++;
    }


    {
        uint8_t v = 199;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint32_t a = 1884031649UL;
        uint32_t b = 188978871UL;
        uint32_t r = a ^ b;
        if (r != 2064620566UL) failures++;
    }


    {
        uint8_t v = 205;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = 17337;
        volatile int16_t b = -29522;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 64;
        uint8_t r = port;
        if (r != 64) failures++;
    }


    {
        uint8_t a[6] = {73,80,199,238,244,229};
        if (a[5] != 229) failures++;
    }


    {
        uint8_t m[3][3] = {{49,27,150},{122,121,111},{167,225,196}};
        if (m[1][2] != 111) failures++;
    }


    {
        if (((uint16_t)(((191 & 42) + (193 & 23)) - ((30 & 169) | (132 - 144)))) != 47) failures++;
    }


    {
        uint16_t r = call6(83,11,66,155,32,101);
        if (r != 448) failures++;
    }


    {
        uint16_t r = 3682 + 52731 + 35946 + 16412 + 36243 + 31045 + 13979 + 20268;
        if (r != 13698) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 2) sum += j;
        if (sum != 42) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(77,132) + add2(132,131) + add2(77,131);
        if (r != 680) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 2: result = 78; break;
        case 0: result = 44; break;
        case 13: result = 55; break;
        case 8: result = 123; break;
        case 17: result = 251; break;
        case 10: result = 146; break;
        case 1: result = 126; break;
        case 18: result = 70; break;
        default: result = 130; break;
        }
        if (result != 44) failures++;
    }


    {
        uint16_t r = 23122 + 4656 + 23972 + 46227 + 12141 + 14738 + 12833 + 17479;
        if (r != 24096) failures++;
    }


    {
        uint16_t r = add2(191,80) + add2(80,84) + add2(191,84);
        if (r != 710) failures++;
    }


    {
        g16 = 2961;
        if (read_g16() != 2961) failures++;
    }


    {
        uint8_t src[13] = {169,211,71,109,56,113,230,73,216,188,50,227,239};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[1] != 211) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 99;
        if (buf[6] != 99) failures++;
    }


    {
        uint16_t r = 5833 + 50227 + 57314 + 31026 + 55720 + 49796 + 31892 + 50460;
        if (r != 4588) failures++;
    }


    {
        uint8_t v = 205;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-1) / (int16_t)((int8_t)-89);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)184) + (uint16_t)1370;
        if (r != 1554) failures++;
    }


    {
        uint8_t a[6] = {180,33,13,221,185,92};
        if (a[1] != 33) failures++;
    }


    {
        uint8_t v = 215;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 50;
        v ^= 128;
        if (v != 178) failures++;
    }


    {
        int8_t a = -79;
        int8_t b = -124;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 34682;
        if (read_g16() != 34682) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)4) / (int16_t)((int8_t)20);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 2054 + 40227 + 49084 + 40381 + 12807 + 3548 + 29343 + 48237;
        if (r != 29073) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 10: result = 173; break;
        case 19: result = 16; break;
        case 2: result = 109; break;
        case 16: result = 52; break;
        case 14: result = 125; break;
        case 15: result = 72; break;
        case 6: result = 61; break;
        default: result = 41; break;
        }
        if (result != 16) failures++;
    }


    {
        if (((uint16_t)(((145 ^ 188) | 201) - 222)) != 15) failures++;
    }


    {
        uint8_t v = 246;
        v &= ~(uint8_t)128;
        if (v != 118) failures++;
    }


    {
        uint8_t buf[8] = {241,200,231,184,206,216,149,165};
        uint8_t *p = buf;
        p += 5;
        if (*p != 216) failures++;
    }


    {
        if (((uint16_t)(((71 - 246) - (189 ^ 66)) & (165 + (145 & 157)))) != 18) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(222,18) != 204) failures++;
    }


    {
        uint32_t a = 2122959214UL;
        uint32_t b = 2509873764UL;
        uint32_t r = a & b;
        if (r != 344555620UL) failures++;
    }


    {
        uint8_t m[3][3] = {{66,62,40},{108,19,52},{97,24,53}};
        if (m[1][1] != 19) failures++;
    }


    {
        uint16_t x = 39;
        x = x + 7;
        if (x != 46) failures++;
    }


    {
        uint8_t x = 189;
        x <<= 2;
        if (x != 244) failures++;
    }


    {
        uint8_t v = 64;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-96) / (int16_t)((int8_t)99);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = add2(37,0) + add2(0,228) + add2(37,228);
        if (r != 530) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {145,51,42736,157};
        if (s.c != (uint16_t)42736) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {255,86,37813,245};
        if (s.a != (uint8_t)255) failures++;
    }


    {
        volatile uint8_t port = 13;
        uint8_t r = port;
        if (r != 13) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 158;
        if (buf[8] != 158) failures++;
    }


    {
        uint8_t m[2][2] = {{2,128},{50,195}};
        if (m[0][1] != 128) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)113) + (uint16_t)16605;
        if (r != 16718) failures++;
    }


    {
        uint32_t a = 1245097844UL;
        uint32_t b = 438687411UL;
        uint32_t r = a ^ b;
        if (r != 1343453639UL) failures++;
    }


    {
        uint32_t a = 1865818609UL;
        uint32_t b = 3687495360UL;
        uint32_t r = a ^ b;
        if (r != 3036452657UL) failures++;
    }


    {
        uint8_t m[3][4] = {{144,228,229,7},{146,69,12,34},{112,218,126,128}};
        if (m[1][1] != 69) failures++;
    }


    {
        uint16_t x = 83;
        x = x + 123;
        if (x != 206) failures++;
    }


    {
        uint8_t v = 131;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 61) failures++;
    }


    {
        uint8_t v = 143;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t buf[8] = {178,136,206,90,110,148,96,35};
        uint8_t *p = buf;
        p += 0;
        if (*p != 178) failures++;
    }


    {
        uint8_t v = 168;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(26,6,64,123,234,164);
        if (r != 617) failures++;
    }


    {
        int8_t a = 8;
        int8_t b = 74;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-38) % (int16_t)((int8_t)114);
        if ((uint16_t)r != (uint16_t)65498) failures++;
    }


    {
        uint16_t x = 88;
        x = x + 51;
        if (x != 139) failures++;
    }


    {
        uint16_t r = 59039 + 23103 + 443 + 55592 + 53507 + 44520 + 38204 + 19085;
        if (r != 31349) failures++;
    }


    {
        uint8_t v = 4;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        g16 = 23601;
        if (read_g16() != 23601) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {214,106,60118,39};
        if (s.d != (uint8_t)39) failures++;
    }


    {
        if (((uint16_t)83) != 83) failures++;
    }


    {
        uint8_t buf[8] = {184,83,16,195,200,37,35,115};
        uint8_t *p = buf;
        p += 7;
        if (*p != 115) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(229,0) != 229) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-23) % (int16_t)((int8_t)31);
        if ((uint16_t)r != (uint16_t)65513) failures++;
    }


    {
        g16 = 44072;
        if (read_g16() != 44072) failures++;
    }


    {
        volatile int16_t a = 25111;
        volatile int16_t b = -8305;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint8_t v = 66;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 30) failures++;
    }


    {
        uint16_t x = 39552;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)90) + (uint16_t)35401;
        if (r != 35491) failures++;
    }


    {
        uint8_t buf[8] = {155,53,177,174,88,201,50,81};
        uint8_t *p = buf;
        p += 3;
        if (*p != 174) failures++;
    }


    {
        g16 = 6441;
        if (read_g16() != 6441) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(148,39) != 109) failures++;
    }


    {
        uint8_t a[6] = {143,49,192,161,117,30};
        if (a[0] != 143) failures++;
    }


    {
        if (((uint16_t)(52 - ((240 - 255) ^ (129 + 88)))) != 268) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-41) / (int16_t)((int8_t)102);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {249,168,45084,0};
        if (s.a != (uint8_t)249) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        int8_t a = 86;
        int8_t b = -117;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)1) + (uint16_t)33883;
        if (r != 33884) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)66) + (uint16_t)16317;
        if (r != 16383) failures++;
    }


    {
        uint16_t r = call6(11,199,130,182,121,110);
        if (r != 753) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {255,55,295,206};
        if (s.a != (uint8_t)255) failures++;
    }


    {
        g16 = 48822;
        if (read_g16() != 48822) failures++;
    }


    {
        uint16_t r = 30872 + 39446 + 25003 + 46084 + 23726 + 30926 + 6574 + 51629;
        if (r != 57652) failures++;
    }


    {
        uint16_t x = 62693;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 250;
        if (buf[8] != 250) failures++;
    }


    {
        uint8_t a[6] = {253,200,114,101,38,230};
        if (a[0] != 253) failures++;
    }


    {
        uint8_t a[6] = {46,137,162,168,133,19};
        if (a[3] != 168) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(77,40,29,174,201,231);
        if (r != 752) failures++;
    }


    {
        uint16_t r = add2(161,41) + add2(41,213) + add2(161,213);
        if (r != 830) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(191,177) != 14) failures++;
    }


    {
        uint16_t r = 780 + 28437 + 61681 + 38411 + 34254 + 34326 + 51494 + 30995;
        if (r != 18234) failures++;
    }


    {
        volatile uint8_t port = 211;
        uint8_t r = port;
        if (r != 211) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 184;
        x = x + 12;
        if (x != 196) failures++;
    }


    {
        uint8_t x = 156;
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
        uint32_t a = 1465053507UL;
        uint32_t b = 2977882380UL;
        uint32_t r = a + b;
        if (r != 147968591UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-51) / (int16_t)((int8_t)16);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)18) % (int16_t)((int8_t)-123);
        if ((uint16_t)r != (uint16_t)18) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)204) + (uint16_t)29847;
        if (r != 30051) failures++;
    }


    {
        uint16_t r = call6(63,142,19,81,144,59);
        if (r != 508) failures++;
    }


    {
        volatile int16_t a = 32663;
        volatile int16_t b = -30035;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {79,18,35,33,202,147,144,97};
        uint8_t *p = buf;
        p += 7;
        if (*p != 97) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 10: result = 94; break;
        case 4: result = 123; break;
        case 18: result = 115; break;
        case 11: result = 64; break;
        case 19: result = 236; break;
        case 15: result = 71; break;
        case 14: result = 182; break;
        default: result = 34; break;
        }
        if (result != 71) failures++;
    }


    {
        uint16_t x = 34155;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[16] = {12,198,128,94,111,33,204,181,107,188,66,190,79,160,155,88};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[7] != 181) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(153,204) != 65485) failures++;
    }


    {
        uint8_t m[3][2] = {{225,175},{201,87},{16,35}};
        if (m[2][1] != 35) failures++;
    }


    {
        int8_t a = -38;
        int8_t b = 11;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[15] = {3,110,241,71,29,227,160,161,193,60,200,96,212,48,183};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[1] != 110) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 124;
        if (buf[4] != 124) failures++;
    }


    {
        uint8_t src[4] = {27,250,146,109};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[2] != 146) failures++;
    }


    {
        if (((uint16_t)(((177 & 75) & (226 & 118)) & ((192 ^ 60) | 28))) != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint32_t a = 3352449932UL;
        uint32_t b = 963662234UL;
        uint32_t r = a ^ b;
        if (r != 4272030230UL) failures++;
    }


    {
        uint8_t src[6] = {251,2,103,84,180,227};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[1] != 2) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 129;
        if (buf[14] != 129) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)233) + (uint16_t)50586;
        if (r != 50819) failures++;
    }


    {
        volatile int16_t a = 25048;
        volatile int16_t b = 23967;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(110,145) != 65501) failures++;
    }


    {
        uint8_t x = 180;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-51) % (int16_t)((int8_t)-92);
        if ((uint16_t)r != (uint16_t)65485) failures++;
    }


    {
        g16 = 59159;
        if (read_g16() != 59159) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 198;
        if (buf[7] != 198) failures++;
    }


    {
        if (((uint16_t)((140 & 117) | 156)) != 156) failures++;
    }


    {
        uint8_t src[2] = {151,38};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 151) failures++;
    }


    {
        uint16_t r = call6(19,103,44,89,155,194);
        if (r != 604) failures++;
    }


    {
        uint16_t x = 197;
        x = x + 242;
        if (x != 439) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 221;
        if (buf[2] != 221) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)91) + (uint16_t)57583;
        if (r != 57674) failures++;
    }


    {
        uint8_t v = 218;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[4][4] = {{189,211,134,133},{44,160,21,238},{97,16,168,253},{47,84,30,66}};
        if (m[0][2] != 134) failures++;
    }


    {
        uint16_t x = 40;
        x = x + 5;
        if (x != 45) failures++;
    }


    {
        uint8_t v = 136;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 56) failures++;
    }


    {
        volatile uint8_t port = 203;
        uint8_t r = port;
        if (r != 203) failures++;
    }


    {
        uint8_t a[6] = {17,59,172,81,75,33};
        if (a[2] != 172) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(96,139) != 65493) failures++;
    }


    {
        uint8_t m[3][2] = {{49,195},{187,196},{231,56}};
        if (m[0][0] != 49) failures++;
    }


    {
        uint8_t a[6] = {103,224,211,128,20,154};
        if (a[3] != 128) failures++;
    }


    {
        uint8_t v = 240;
        v &= ~(uint8_t)32;
        if (v != 208) failures++;
    }


    {
        uint8_t src[9] = {200,113,27,126,104,87,236,233,107};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[4] != 104) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(236,147) != 383) failures++;
    }


    {
        volatile int16_t a = -16357;
        volatile int16_t b = -1482;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(166,179) + add2(179,49) + add2(166,49);
        if (r != 788) failures++;
    }


    {
        uint32_t a = 4016814198UL;
        uint32_t b = 3555813208UL;
        uint32_t r = a + b;
        if (r != 3277660110UL) failures++;
    }


    {
        uint8_t v = 0;
        v |= 16;
        if (v != 16) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 0: result = 238; break;
        case 4: result = 208; break;
        case 1: result = 168; break;
        case 17: result = 249; break;
        case 3: result = 157; break;
        case 14: result = 24; break;
        case 7: result = 228; break;
        default: result = 189; break;
        }
        if (result != 249) failures++;
    }


    {
        uint8_t src[1] = {143};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 143) failures++;
    }


    {
        uint16_t r = add2(229,163) + add2(163,138) + add2(229,138);
        if (r != 1060) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-63) / (int16_t)((int8_t)3);
        if ((uint16_t)r != (uint16_t)65515) failures++;
    }


    {
        uint16_t r = call6(185,33,227,5,238,228);
        if (r != 916) failures++;
    }


    {
        uint16_t x = 22221;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint16_t r = call6(150,228,141,60,175,199);
        if (r != 953) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(12,208) != 65340) failures++;
    }


    {
        uint8_t v = 238;
        v |= 32;
        if (v != 238) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)93) % (int16_t)((int8_t)68);
        if ((uint16_t)r != (uint16_t)25) failures++;
    }


    {
        volatile int16_t a = -22203;
        volatile int16_t b = 28389;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)22) / (int16_t)((int8_t)30);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 21697 + 36370 + 53171 + 370 + 55718 + 43647 + 11581 + 7543;
        if (r != 33489) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 17: result = 36; break;
        case 10: result = 178; break;
        case 7: result = 83; break;
        case 13: result = 241; break;
        default: result = 204; break;
        }
        if (result != 241) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)190) + (uint16_t)65211;
        if (r != 65401) failures++;
    }


    {
        if (((uint16_t)(((72 + 78) + (66 - 12)) | (63 | (82 + 36)))) != 255) failures++;
    }


    {
        g16 = 56389;
        if (read_g16() != 56389) failures++;
    }


    {
        int8_t a = 14;
        int8_t b = -61;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[11] = {163,58,165,5,157,189,58,89,100,240,250};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[6] != 58) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-71) / (int16_t)((int8_t)79);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = call6(6,183,71,52,29,79);
        if (r != 420) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)143) + (uint16_t)10825;
        if (r != 10968) failures++;
    }


    {
        uint8_t m[2][3] = {{9,129,3},{152,235,195}};
        if (m[0][0] != 9) failures++;
    }


    {
        volatile uint8_t port = 183;
        uint8_t r = port;
        if (r != 183) failures++;
    }


    {
        uint8_t v = 244;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)115) + (uint16_t)55340;
        if (r != 55455) failures++;
    }


    {
        uint16_t r = call6(83,193,26,157,250,64);
        if (r != 773) failures++;
    }


    {
        volatile uint8_t port = 15;
        uint8_t r = port;
        if (r != 15) failures++;
    }


    {
        uint16_t x = 140;
        x = x + 16;
        if (x != 156) failures++;
    }


    {
        volatile int16_t a = 30686;
        volatile int16_t b = 6032;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {167,0,44282,82};
        if (s.b != (uint8_t)0) failures++;
    }


    {
        uint16_t r = add2(148,86) + add2(86,234) + add2(148,234);
        if (r != 936) failures++;
    }


    {
        uint16_t r = 39997 + 38092 + 8774 + 30239 + 31695 + 54024 + 4655 + 8981;
        if (r != 19849) failures++;
    }


    {
        uint32_t a = 1280585716UL;
        uint32_t b = 668296224UL;
        uint32_t r = a - b;
        if (r != 612289492UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {232,64,25884,242};
        if (s.b != (uint8_t)64) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)230) + (uint16_t)51330;
        if (r != 51560) failures++;
    }


    {
        uint8_t x = 41;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t a[6] = {80,12,178,92,160,183};
        if (a[3] != 92) failures++;
    }


    {
        uint8_t x = 84;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        uint16_t x = 40197;
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
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 243;
        if (buf[13] != 243) failures++;
    }


    {
        uint16_t x = 208;
        x = x + 89;
        if (x != 297) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint16_t r = call6(204,100,182,95,117,149);
        if (r != 847) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(206,22) != 228) failures++;
    }


    {
        uint8_t buf[8] = {45,236,162,73,34,99,126,104};
        uint8_t *p = buf;
        p += 0;
        if (*p != 45) failures++;
    }


    {
        if (((uint16_t)(10 & ((211 & 37) - (242 + 139)))) != 0) failures++;
    }


    {
        uint8_t buf[8] = {161,58,232,250,250,255,163,216};
        uint8_t *p = buf;
        p += 2;
        if (*p != 232) failures++;
    }


    {
        uint8_t buf[8] = {141,186,110,66,194,220,61,200};
        uint8_t *p = buf;
        p += 3;
        if (*p != 66) failures++;
    }


    {
        uint8_t v = 164;
        v ^= 128;
        if (v != 36) failures++;
    }


    {
        uint16_t r = add2(35,251) + add2(251,35) + add2(35,35);
        if (r != 642) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)95) + (uint16_t)3366;
        if (r != 3461) failures++;
    }


    {
        if (((uint16_t)((101 - 35) ^ 230)) != 164) failures++;
    }


    {
        uint8_t src[16] = {72,57,182,116,47,177,56,192,31,195,214,169,40,8,242,189};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[15] != 189) failures++;
    }


    {
        uint8_t m[4][3] = {{51,5,80},{138,9,3},{175,114,252},{203,250,7}};
        if (m[2][0] != 175) failures++;
    }


    {
        if (((uint16_t)((203 ^ (243 ^ 246)) | ((28 | 254) & 187))) != 254) failures++;
    }


    {
        uint8_t a[6] = {156,40,250,15,111,121};
        if (a[1] != 40) failures++;
    }


    {
        if (((uint16_t)(122 | ((189 & 214) | (143 | 154)))) != 255) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[4][2] = {{93,23},{117,179},{173,18},{55,10}};
        if (m[2][0] != 173) failures++;
    }


    {
        uint16_t x = 15;
        x = x + 102;
        if (x != 117) failures++;
    }


    {
        uint8_t a[6] = {176,139,112,32,140,160};
        if (a[0] != 176) failures++;
    }


    {
        g16 = 39557;
        if (read_g16() != 39557) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-62) % (int16_t)((int8_t)-90);
        if ((uint16_t)r != (uint16_t)65474) failures++;
    }


    {
        volatile int16_t a = -9679;
        volatile int16_t b = 14505;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {20,0,221,58,207,183,25,177};
        uint8_t *p = buf;
        p += 7;
        if (*p != 177) failures++;
    }


    {
        uint16_t r = 670 + 37285 + 8098 + 54344 + 51936 + 63118 + 27110 + 45044;
        if (r != 25461) failures++;
    }


    {
        uint8_t m[4][4] = {{100,189,2,52},{92,200,47,65},{4,25,113,124},{80,201,93,132}};
        if (m[0][0] != 100) failures++;
    }


    {
        uint16_t x = 137;
        x = x + 141;
        if (x != 278) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(41,68) != 109) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 17;
        if (buf[0] != 17) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 61;
        if (buf[1] != 61) failures++;
    }


    {
        uint8_t m[3][4] = {{154,14,9,69},{44,240,129,16},{237,109,70,194}};
        if (m[1][2] != 129) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)22) / (int16_t)((int8_t)20);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t src[12] = {66,124,139,193,152,134,227,60,72,204,204,247};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[8] != 72) failures++;
    }


    {
        uint8_t src[7] = {85,234,155,10,243,186,143};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[3] != 10) failures++;
    }


    {
        if (((uint16_t)((238 + (59 | 103)) & 146)) != 0) failures++;
    }


    {
        volatile uint8_t port = 12;
        uint8_t r = port;
        if (r != 12) failures++;
    }


    {
        volatile uint8_t port = 24;
        uint8_t r = port;
        if (r != 24) failures++;
    }


    {
        g16 = 13274;
        if (read_g16() != 13274) failures++;
    }


    {
        uint16_t r = 21640 + 56796 + 33687 + 24322 + 41948 + 1576 + 21723 + 16284;
        if (r != 21368) failures++;
    }


    {
        uint16_t r = call6(190,183,60,249,125,254);
        if (r != 1061) failures++;
    }


    {
        uint8_t m[2][3] = {{80,176,12},{242,50,175}};
        if (m[0][0] != 80) failures++;
    }


    {
        int8_t a = 56;
        int8_t b = 126;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -12236;
        volatile int16_t b = 9765;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][4] = {{90,119,157,105},{141,121,49,55},{70,135,252,49},{147,38,154,41}};
        if (m[3][3] != 41) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {130,106,438,55};
        if (s.d != (uint8_t)55) failures++;
    }


    {
        uint16_t r = add2(184,87) + add2(87,237) + add2(184,237);
        if (r != 1016) failures++;
    }


    {
        int8_t a = 39;
        int8_t b = 114;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 13090;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 57;
        x = x + 128;
        if (x != 185) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 3976978710UL;
        uint32_t b = 2890368963UL;
        uint32_t r = a & b;
        if (r != 2885910786UL) failures++;
    }


    {
        uint16_t x = 27929;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 29142;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 219;
        uint8_t r = port;
        if (r != 219) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[2][3] = {{126,133,138},{217,10,52}};
        if (m[1][0] != 217) failures++;
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
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint16_t r = call6(70,141,102,46,188,55);
        if (r != 602) failures++;
    }


    {
        uint16_t r = call6(94,75,91,186,220,192);
        if (r != 858) failures++;
    }


    {
        uint32_t a = 3886561649UL;
        uint32_t b = 2559410225UL;
        uint32_t r = a ^ b;
        if (r != 2133178688UL) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 7: result = 11; break;
        case 0: result = 217; break;
        case 16: result = 18; break;
        case 15: result = 200; break;
        default: result = 187; break;
        }
        if (result != 217) failures++;
    }


    {
        int8_t a = 66;
        int8_t b = 49;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(((165 & 66) - (94 - 13)) | 109)) != 65519) failures++;
    }


    {
        uint16_t r = 2963 + 10908 + 62101 + 62633 + 2171 + 9776 + 29265 + 34024;
        if (r != 17233) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)22) + (uint16_t)18203;
        if (r != 18225) failures++;
    }


    {
        uint16_t x = 65;
        x = x + 83;
        if (x != 148) failures++;
    }


    {
        uint16_t r = 54905 + 54673 + 28603 + 55387 + 59775 + 16339 + 14482 + 7206;
        if (r != 29226) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 100;
        if (buf[6] != 100) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 63518;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(72,145) != 217) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {184,187,44782,93};
        if (s.a != (uint8_t)184) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint16_t x = 80;
        x = x + 250;
        if (x != 330) failures++;
    }


    {
        uint16_t x = 3381;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 10869;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(250,63) != 313) failures++;
    }


    {
        volatile uint8_t port = 172;
        uint8_t r = port;
        if (r != 172) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t r = add2(162,125) + add2(125,76) + add2(162,76);
        if (r != 726) failures++;
    }


    {
        if (((uint16_t)(((200 + 73) ^ (50 | 136)) ^ 8)) != 419) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 11: result = 82; break;
        case 12: result = 10; break;
        case 4: result = 173; break;
        case 19: result = 255; break;
        case 1: result = 9; break;
        case 6: result = 33; break;
        case 15: result = 245; break;
        default: result = 175; break;
        }
        if (result != 82) failures++;
    }


    {
        uint8_t x = 105;
        x <<= 1;
        if (x != 210) failures++;
    }


    {
        int8_t a = -24;
        int8_t b = 0;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 201;
        if (buf[10] != 201) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(246,184) != 62) failures++;
    }


    {
        uint16_t r = call6(249,60,183,182,120,53);
        if (r != 847) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)58) % (int16_t)((int8_t)-54);
        if ((uint16_t)r != (uint16_t)4) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t buf[8] = {163,1,168,233,245,8,79,6};
        uint8_t *p = buf;
        p += 3;
        if (*p != 233) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint32_t a = 2734842372UL;
        uint32_t b = 3785812788UL;
        uint32_t r = a | b;
        if (r != 3819367220UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)21) / (int16_t)((int8_t)-98);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 26830 + 6194 + 34933 + 41479 + 51956 + 54513 + 40700 + 1342;
        if (r != 61339) failures++;
    }


    {
        uint8_t m[3][3] = {{211,198,105},{29,30,53},{186,18,246}};
        if (m[2][0] != 186) failures++;
    }


    {
        uint8_t src[9] = {139,125,226,2,191,205,15,185,240};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[3] != 2) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 141;
        if (buf[11] != 141) failures++;
    }


    {
        uint8_t v = 97;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 15) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {154,125,58848,102};
        if (s.b != (uint8_t)125) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(245,108) != 353) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(((4 + 39) ^ (252 | 78)) - ((169 ^ 44) + (196 & 201)))) != 65424) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[4][2] = {{215,175},{90,37},{122,224},{19,17}};
        if (m[3][1] != 17) failures++;
    }


    {
        uint16_t x = 238;
        x = x + 183;
        if (x != 421) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 0: result = 212; break;
        case 14: result = 40; break;
        case 7: result = 58; break;
        case 16: result = 106; break;
        case 4: result = 77; break;
        default: result = 141; break;
        }
        if (result != 141) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {16,126,19978,32};
        if (s.c != (uint16_t)19978) failures++;
    }


    {
        volatile uint8_t port = 22;
        uint8_t r = port;
        if (r != 22) failures++;
    }


    {
        uint8_t v = 102;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = 63864 + 33702 + 63654 + 39158 + 61821 + 37030 + 57221 + 61550;
        if (r != 24784) failures++;
    }


    {
        int8_t a = -123;
        int8_t b = -10;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)61) != 61) failures++;
    }


    {
        uint8_t v = 112;
        v |= 8;
        if (v != 120) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint16_t x = 57;
        x = x + 115;
        if (x != 172) failures++;
    }


    {
        uint8_t m[3][3] = {{218,134,62},{104,37,15},{200,193,34}};
        if (m[1][0] != 104) failures++;
    }


    {
        uint8_t buf[8] = {180,64,76,176,85,159,144,225};
        uint8_t *p = buf;
        p += 3;
        if (*p != 176) failures++;
    }


    {
        uint8_t src[13] = {142,120,159,80,251,191,42,250,81,21,208,61,22};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[4] != 251) failures++;
    }


    {
        uint16_t x = 79;
        x = x + 107;
        if (x != 186) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {19,100,9612,209};
        if (s.b != (uint8_t)100) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(35,53) != 88) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 119;
        if (buf[15] != 119) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 63;
        if (buf[13] != 63) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 167;
        if (buf[4] != 167) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t buf[8] = {186,131,188,118,236,247,195,39};
        uint8_t *p = buf;
        p += 0;
        if (*p != 186) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-16) % (int16_t)((int8_t)56);
        if ((uint16_t)r != (uint16_t)65520) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(98,63) != 161) failures++;
    }


    {
        uint32_t a = 561010438UL;
        uint32_t b = 2250436072UL;
        uint32_t r = a - b;
        if (r != 2605541662UL) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 166;
        if (buf[5] != 166) failures++;
    }


    {
        uint16_t x = 51480;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(3,199) != 65340) failures++;
    }


    {
        uint8_t src[10] = {117,118,48,32,253,160,69,176,109,24};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[6] != 69) failures++;
    }


    {
        uint8_t m[2][4] = {{184,124,33,10},{40,127,132,105}};
        if (m[1][1] != 127) failures++;
    }


    {
        uint16_t x = 19314;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 59420 + 58440 + 16248 + 10150 + 46743 + 40 + 1490 + 38921;
        if (r != 34844) failures++;
    }


    {
        uint8_t src[1] = {131};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 131) failures++;
    }


    {
        uint8_t src[1] = {187};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 187) failures++;
    }


    {
        if (((uint16_t)218) != 218) failures++;
    }


    {
        uint8_t buf[8] = {105,132,85,73,58,141,239,79};
        uint8_t *p = buf;
        p += 7;
        if (*p != 79) failures++;
    }


    {
        uint8_t v = 106;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)107) % (int16_t)((int8_t)65);
        if ((uint16_t)r != (uint16_t)42) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(31,80) != 111) failures++;
    }


    {
        uint16_t r = call6(19,122,115,159,72,235);
        if (r != 722) failures++;
    }


    {
        if (((uint16_t)(((3 + 150) & (105 | 151)) ^ ((138 + 225) - 50))) != 416) failures++;
    }


    {
        uint8_t buf[8] = {76,86,89,12,94,38,187,119};
        uint8_t *p = buf;
        p += 3;
        if (*p != 12) failures++;
    }


    {
        volatile uint8_t port = 141;
        uint8_t r = port;
        if (r != 141) failures++;
    }


    {
        uint8_t v = 108;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 20) failures++;
    }


    {
        uint8_t a[6] = {90,3,249,167,30,151};
        if (a[4] != 30) failures++;
    }


    {
        uint8_t x = 84;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint32_t a = 2009173095UL;
        uint32_t b = 2746372971UL;
        uint32_t r = a & b;
        if (r != 595591267UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)133) + (uint16_t)35321;
        if (r != 35454) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 1) sum += j;
        if (sum != 105) failures++;
    }


    {
        uint8_t v = 251;
        int r = (v & 128) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 2: result = 247; break;
        case 6: result = 136; break;
        case 16: result = 249; break;
        case 9: result = 43; break;
        case 4: result = 124; break;
        case 13: result = 1; break;
        case 1: result = 158; break;
        default: result = 124; break;
        }
        if (result != 249) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -30541;
        volatile int16_t b = 21951;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        g16 = 2814;
        if (read_g16() != 2814) failures++;
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
        for (uint8_t j = 0; j < 1; j++) buf[j] = 130;
        if (buf[0] != 130) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)15) % (int16_t)((int8_t)-7);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(39,164) != 65411) failures++;
    }


    {
        uint8_t x = 148;
        x <<= 6;
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
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(225,15) != 240) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)203) + (uint16_t)5083;
        if (r != 5286) failures++;
    }


    {
        uint8_t m[3][4] = {{0,203,218,203},{156,160,79,1},{0,142,26,244}};
        if (m[1][2] != 79) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {182,248,9267,250};
        if (s.d != (uint8_t)250) failures++;
    }


    {
        int8_t a = 37;
        int8_t b = -4;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 162;
        uint8_t r = port;
        if (r != 162) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)152) + (uint16_t)5383;
        if (r != 5535) failures++;
    }


    {
        uint8_t src[14] = {94,166,43,23,240,146,14,145,161,180,128,61,251,108};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[9] != 180) failures++;
    }


    {
        uint8_t v = 153;
        v |= 4;
        if (v != 157) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 7: result = 124; break;
        case 8: result = 44; break;
        case 6: result = 106; break;
        case 13: result = 171; break;
        default: result = 167; break;
        }
        if (result != 44) failures++;
    }


    {
        volatile int16_t a = 3852;
        volatile int16_t b = 20132;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {216,116,170,226,114,128};
        if (a[0] != 216) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)83) + (uint16_t)44842;
        if (r != 44925) failures++;
    }


    {
        uint16_t x = 65094;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 50000 + 30177 + 20410 + 26465 + 36509 + 26905 + 59292 + 43796;
        if (r != 31410) failures++;
    }


    {
        uint16_t r = call6(183,236,237,238,23,137);
        if (r != 1054) failures++;
    }


    {
        uint8_t x = 212;
        x <<= 1;
        if (x != 168) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 17: result = 213; break;
        case 6: result = 21; break;
        case 15: result = 146; break;
        case 9: result = 27; break;
        case 13: result = 44; break;
        default: result = 132; break;
        }
        if (result != 132) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)53) / (int16_t)((int8_t)-92);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {181,213,34,70,161,2,3,69};
        uint8_t *p = buf;
        p += 0;
        if (*p != 181) failures++;
    }


    {
        uint8_t src[8] = {191,241,51,170,1,130,88,62};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[4] != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-26) % (int16_t)((int8_t)39);
        if ((uint16_t)r != (uint16_t)65510) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)197) + (uint16_t)44583;
        if (r != 44780) failures++;
    }


    {
        uint16_t r = call6(78,75,96,16,236,167);
        if (r != 668) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(245,189) != 56) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t m[4][2] = {{130,150},{85,142},{174,162},{204,198}};
        if (m[3][1] != 198) failures++;
    }


    {
        uint32_t a = 304142895UL;
        uint32_t b = 779019533UL;
        uint32_t r = a ^ b;
        if (r != 1011760930UL) failures++;
    }


    {
        uint8_t v = 182;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 66;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 186;
        uint8_t r = port;
        if (r != 186) failures++;
    }


    {
        uint8_t src[14] = {59,242,162,162,93,69,179,145,56,44,23,65,117,172};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[5] != 69) failures++;
    }


    {
        uint8_t v = 248;
        v |= 2;
        if (v != 250) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)33) / (int16_t)((int8_t)24);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 38;
        uint8_t r = port;
        if (r != 38) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 1: result = 0; break;
        case 0: result = 166; break;
        case 18: result = 217; break;
        case 5: result = 96; break;
        default: result = 202; break;
        }
        if (result != 217) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 3: result = 230; break;
        case 1: result = 167; break;
        case 12: result = 104; break;
        case 15: result = 172; break;
        case 10: result = 118; break;
        case 18: result = 112; break;
        case 14: result = 240; break;
        default: result = 25; break;
        }
        if (result != 104) failures++;
    }


    {
        g16 = 42130;
        if (read_g16() != 42130) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)145) + (uint16_t)41133;
        if (r != 41278) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 16;
        if (buf[0] != 16) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 32;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 16) failures++;
    }


    {
        uint8_t a[6] = {226,29,193,99,47,54};
        if (a[1] != 29) failures++;
    }


    {
        volatile uint8_t port = 210;
        uint8_t r = port;
        if (r != 210) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 255;
        if (buf[15] != 255) failures++;
    }


    {
        uint8_t v = 56;
        v |= 2;
        if (v != 58) failures++;
    }


    {
        uint8_t buf[8] = {56,194,144,11,197,223,185,119};
        uint8_t *p = buf;
        p += 2;
        if (*p != 144) failures++;
    }


    {
        volatile int16_t a = 17647;
        volatile int16_t b = -21734;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)115) / (int16_t)((int8_t)-114);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t x = 145;
        x <<= 1;
        if (x != 34) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 237;
        if (buf[9] != 237) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {57,21,51224,173};
        if (s.a != (uint8_t)57) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 17: result = 34; break;
        case 12: result = 89; break;
        case 5: result = 127; break;
        case 18: result = 208; break;
        case 9: result = 219; break;
        case 2: result = 26; break;
        case 15: result = 119; break;
        case 13: result = 181; break;
        default: result = 160; break;
        }
        if (result != 119) failures++;
    }


    {
        uint8_t v = 229;
        v |= 2;
        if (v != 231) failures++;
    }


    {
        if (((uint16_t)(81 & ((43 ^ 205) | (171 & 130)))) != 64) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)126) % (int16_t)((int8_t)-125);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 59;
        int8_t b = 98;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 1;
        uint8_t r = port;
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {118,44,78,129,157,69};
        if (a[3] != 129) failures++;
    }


    {
        volatile uint8_t port = 27;
        uint8_t r = port;
        if (r != 27) failures++;
    }


    {
        uint16_t r = add2(202,192) + add2(192,222) + add2(202,222);
        if (r != 1232) failures++;
    }


    {
        uint8_t m[4][3] = {{67,201,20},{71,30,73},{39,69,93},{199,73,159}};
        if (m[0][2] != 20) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 19: result = 151; break;
        case 2: result = 94; break;
        case 9: result = 130; break;
        case 8: result = 169; break;
        case 6: result = 54; break;
        case 0: result = 89; break;
        case 13: result = 48; break;
        case 5: result = 41; break;
        default: result = 205; break;
        }
        if (result != 130) failures++;
    }


    {
        g16 = 34691;
        if (read_g16() != 34691) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint8_t v = 204;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[3][3] = {{22,18,82},{109,93,220},{36,95,157}};
        if (m[2][1] != 95) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 52;
        if (buf[14] != 52) failures++;
    }


    {
        uint8_t v = 169;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 23) failures++;
    }


    {
        uint8_t v = 130;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 478;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 20;
        x <<= 2;
        if (x != 80) failures++;
    }


    {
        volatile int16_t a = -21695;
        volatile int16_t b = -6242;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 242;
        if (buf[8] != 242) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 192;
        if (buf[9] != 192) failures++;
    }


    {
        uint8_t src[7] = {99,115,222,9,237,229,186};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[2] != 222) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint8_t buf[8] = {196,203,158,75,206,71,173,225};
        uint8_t *p = buf;
        p += 5;
        if (*p != 71) failures++;
    }


    {
        volatile uint8_t port = 199;
        uint8_t r = port;
        if (r != 199) failures++;
    }


    {
        uint16_t r = add2(0,146) + add2(146,22) + add2(0,22);
        if (r != 336) failures++;
    }


    {
        uint16_t r = call6(75,207,197,80,32,9);
        if (r != 600) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-65) / (int16_t)((int8_t)-19);
        if ((uint16_t)r != (uint16_t)3) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 12: result = 32; break;
        case 19: result = 47; break;
        case 0: result = 33; break;
        case 6: result = 77; break;
        case 17: result = 47; break;
        case 8: result = 178; break;
        case 14: result = 239; break;
        default: result = 181; break;
        }
        if (result != 32) failures++;
    }


    {
        uint16_t r = call6(36,199,76,171,171,193);
        if (r != 846) failures++;
    }


    {
        uint8_t a[6] = {131,51,134,15,149,235};
        if (a[1] != 51) failures++;
    }


    {
        uint8_t a[6] = {237,154,201,138,229,161};
        if (a[2] != 201) failures++;
    }


    {
        uint16_t x = 48595;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {120,15,36359,41};
        if (s.c != (uint16_t)36359) failures++;
    }


    {
        g16 = 60072;
        if (read_g16() != 60072) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 115;
        v |= 2;
        if (v != 115) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 26;
        if (buf[14] != 26) failures++;
    }


    {
        uint16_t r = add2(137,79) + add2(79,122) + add2(137,122);
        if (r != 676) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)171) + (uint16_t)45495;
        if (r != 45666) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {31,167,54765,33};
        if (s.a != (uint8_t)31) failures++;
    }


    {
        if (((uint16_t)(112 + 236)) != 348) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 62;
        if (buf[2] != 62) failures++;
    }


    {
        uint8_t a[6] = {146,237,237,48,27,253};
        if (a[5] != 253) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[11] = {4,128,255,102,24,52,132,70,154,152,192};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[10] != 192) failures++;
    }


    {
        uint8_t src[4] = {1,210,50,138};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[2] != 50) failures++;
    }


    {
        uint16_t r = 28204 + 56535 + 45423 + 25038 + 47425 + 1931 + 15370 + 62207;
        if (r != 19989) failures++;
    }


    {
        uint8_t buf[8] = {144,15,65,32,80,183,155,133};
        uint8_t *p = buf;
        p += 6;
        if (*p != 155) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)253) + (uint16_t)41788;
        if (r != 42041) failures++;
    }


    {
        uint8_t a[6] = {136,110,43,45,51,184};
        if (a[4] != 51) failures++;
    }


    {
        uint8_t m[4][3] = {{128,128,163},{206,224,121},{130,2,14},{120,247,231}};
        if (m[2][2] != 14) failures++;
    }


    {
        uint16_t r = add2(66,4) + add2(4,61) + add2(66,61);
        if (r != 262) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 69;
        if (buf[9] != 69) failures++;
    }


    {
        uint32_t a = 2441311027UL;
        uint32_t b = 1670420600UL;
        uint32_t r = a | b;
        if (r != 4086561659UL) failures++;
    }


    {
        uint8_t buf[8] = {247,142,185,221,67,253,235,149};
        uint8_t *p = buf;
        p += 4;
        if (*p != 67) failures++;
    }


    {
        uint16_t r = add2(84,109) + add2(109,214) + add2(84,214);
        if (r != 814) failures++;
    }


    {
        uint8_t buf[8] = {13,2,243,174,47,189,59,161};
        uint8_t *p = buf;
        p += 6;
        if (*p != 59) failures++;
    }


    {
        uint8_t src[1] = {99};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 99) failures++;
    }


    {
        uint16_t x = 46426;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)16) % (int16_t)((int8_t)-64);
        if ((uint16_t)r != (uint16_t)16) failures++;
    }


    {
        uint16_t r = call6(101,232,89,68,9,0);
        if (r != 499) failures++;
    }


    {
        uint8_t x = 34;
        x <<= 3;
        if (x != 16) failures++;
    }


    {
        uint16_t r = call6(99,136,105,1,7,187);
        if (r != 535) failures++;
    }


    {
        uint8_t a[6] = {244,176,33,123,147,130};
        if (a[5] != 130) failures++;
    }


    {
        uint8_t v = 241;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {213,141,7864,210};
        if (s.c != (uint16_t)7864) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(31,156) != 187) failures++;
    }


    {
        uint16_t r = 16269 + 31084 + 618 + 35994 + 62599 + 58224 + 6682 + 58334;
        if (r != 7660) failures++;
    }


    {
        uint16_t x = 57646;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -59;
        int8_t b = -95;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {237,160,31581,227};
        if (s.a != (uint8_t)237) failures++;
    }


    {
        uint16_t r = 59942 + 29333 + 58641 + 34708 + 42234 + 45165 + 58453 + 62434;
        if (r != 63230) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 2: result = 48; break;
        case 8: result = 129; break;
        case 4: result = 1; break;
        case 17: result = 166; break;
        case 1: result = 136; break;
        default: result = 249; break;
        }
        if (result != 166) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 6: result = 70; break;
        case 0: result = 171; break;
        case 10: result = 85; break;
        default: result = 81; break;
        }
        if (result != 171) failures++;
    }


    {
        volatile int16_t a = 21098;
        volatile int16_t b = 27983;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 43325;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 45149;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {3,232,56986,154};
        if (s.a != (uint8_t)3) failures++;
    }


    {
        int8_t a = 121;
        int8_t b = 23;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {191,255,3,48,32,137,55,129};
        uint8_t *p = buf;
        p += 6;
        if (*p != 55) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 126;
        if (buf[7] != 126) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-74) % (int16_t)((int8_t)94);
        if ((uint16_t)r != (uint16_t)65462) failures++;
    }


    {
        uint32_t a = 2425170707UL;
        uint32_t b = 34900988UL;
        uint32_t r = a + b;
        if (r != 2460071695UL) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 16: result = 18; break;
        case 13: result = 51; break;
        case 0: result = 208; break;
        case 4: result = 254; break;
        case 12: result = 254; break;
        case 9: result = 234; break;
        case 3: result = 91; break;
        case 7: result = 30; break;
        default: result = 30; break;
        }
        if (result != 254) failures++;
    }


    {
        if (((uint16_t)(((192 ^ 75) & (51 ^ 130)) + 112)) != 241) failures++;
    }


    {
        uint16_t r = add2(84,146) + add2(146,133) + add2(84,133);
        if (r != 726) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 1: result = 103; break;
        case 6: result = 22; break;
        case 18: result = 214; break;
        case 9: result = 46; break;
        case 11: result = 87; break;
        case 13: result = 104; break;
        default: result = 198; break;
        }
        if (result != 87) failures++;
    }


    {
        volatile uint8_t port = 107;
        uint8_t r = port;
        if (r != 107) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 13: result = 243; break;
        case 2: result = 175; break;
        case 5: result = 252; break;
        case 18: result = 179; break;
        case 11: result = 228; break;
        case 8: result = 199; break;
        case 15: result = 62; break;
        case 12: result = 102; break;
        default: result = 205; break;
        }
        if (result != 205) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(67,148,166,3,68,138);
        if (r != 590) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 66;
        if (buf[1] != 66) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {25,48,21121,148};
        if (s.a != (uint8_t)25) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)201) + (uint16_t)63574;
        if (r != 63775) failures++;
    }


    {
        if (((uint16_t)((155 ^ 73) + ((11 | 218) & 186))) != 364) failures++;
    }


    {
        uint8_t v = 131;
        v ^= 2;
        if (v != 129) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {41,180,36326,11};
        if (s.a != (uint8_t)41) failures++;
    }


    {
        g16 = 57176;
        if (read_g16() != 57176) failures++;
    }


    {
        uint32_t a = 1141906534UL;
        uint32_t b = 2835125041UL;
        uint32_t r = a ^ b;
        if (r != 3974926167UL) failures++;
    }


    {
        g16 = 32246;
        if (read_g16() != 32246) failures++;
    }


    {
        uint16_t r = call6(18,140,26,3,95,7);
        if (r != 289) failures++;
    }


    {
        uint16_t r = 62823 + 21732 + 33587 + 23878 + 9963 + 29530 + 35918 + 29052;
        if (r != 49875) failures++;
    }


    {
        uint16_t r = 63959 + 11592 + 10875 + 42869 + 29377 + 54758 + 14804 + 31221;
        if (r != 62847) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint16_t r = 23212 + 55499 + 62660 + 42776 + 51994 + 48534 + 26351 + 34351;
        if (r != 17697) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 49;
        if (x != 247) failures++;
    }


    {
        uint8_t a[6] = {164,8,86,94,76,169};
        if (a[2] != 86) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(207,95) != 112) failures++;
    }


    {
        uint8_t src[12] = {12,64,98,235,17,231,203,38,128,153,179,39};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[9] != 153) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 8: result = 87; break;
        case 18: result = 60; break;
        case 19: result = 112; break;
        default: result = 6; break;
        }
        if (result != 60) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {206,59,39999,180};
        if (s.d != (uint8_t)180) failures++;
    }


    {
        uint16_t x = 223;
        x = x + 130;
        if (x != 353) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 98;
        if (buf[12] != 98) failures++;
    }


    {
        uint8_t x = 129;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        volatile int16_t a = 16903;
        volatile int16_t b = 30271;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(242,157) != 399) failures++;
    }


    {
        volatile uint8_t port = 122;
        uint8_t r = port;
        if (r != 122) failures++;
    }


    {
        uint16_t r = add2(215,122) + add2(122,147) + add2(215,147);
        if (r != 968) failures++;
    }


    {
        volatile uint8_t port = 68;
        uint8_t r = port;
        if (r != 68) failures++;
    }


    {
        uint8_t src[11] = {36,219,251,149,17,159,216,150,202,48,248};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[0] != 36) failures++;
    }


    {
        uint16_t x = 192;
        x = x + 166;
        if (x != 358) failures++;
    }


    {
        uint8_t a[6] = {181,123,143,51,212,85};
        if (a[2] != 143) failures++;
    }


    {
        uint8_t m[3][4] = {{209,52,255,243},{49,76,244,229},{220,251,25,252}};
        if (m[1][2] != 244) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) % (int16_t)((int8_t)11);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        uint8_t x = 231;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        if (((uint16_t)(244 | 153)) != 253) failures++;
    }


    {
        uint8_t src[5] = {172,101,116,84,191};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 172) failures++;
    }


    {
        uint8_t v = 41;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint16_t r = call6(234,112,135,132,214,86);
        if (r != 913) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 210;
        if (x != 408) failures++;
    }


    {
        uint8_t v = 172;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 3792306030UL;
        uint32_t b = 829658170UL;
        uint32_t r = a + b;
        if (r != 326996904UL) failures++;
    }


    {
        uint8_t a[6] = {226,139,56,78,138,253};
        if (a[2] != 56) failures++;
    }


    {
        uint16_t r = add2(182,157) + add2(157,246) + add2(182,246);
        if (r != 1170) failures++;
    }


    {
        uint8_t v = 148;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 44) failures++;
    }


    {
        uint16_t x = 58771;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(80,98) != 178) failures++;
    }


    {
        uint8_t v = 79;
        v ^= 1;
        if (v != 78) failures++;
    }


    {
        uint16_t x = 31214;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {113,105,51489,13};
        if (s.c != (uint16_t)51489) failures++;
    }


    {
        uint8_t m[3][2] = {{222,28},{159,151},{5,169}};
        if (m[2][0] != 5) failures++;
    }


    {
        uint8_t buf[8] = {247,50,58,99,72,133,78,62};
        uint8_t *p = buf;
        p += 7;
        if (*p != 62) failures++;
    }


    {
        if (((uint16_t)(207 ^ ((161 | 192) + (116 | 164)))) != 282) failures++;
    }


    {
        if (((uint16_t)(((63 & 22) + (175 | 63)) + 73)) != 286) failures++;
    }


    {
        uint32_t a = 2245334725UL;
        uint32_t b = 3191976364UL;
        uint32_t r = a ^ b;
        if (r != 999597929UL) failures++;
    }


    {
        uint8_t a[6] = {168,127,141,242,130,56};
        if (a[4] != 130) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)37) + (uint16_t)64280;
        if (r != 64317) failures++;
    }


    {
        uint16_t x = 44471;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[16] = {52,165,178,23,18,63,182,98,136,34,150,215,54,74,15,20};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[0] != 52) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)42) / (int16_t)((int8_t)-72);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)110) / (int16_t)((int8_t)121);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t src[1] = {54};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 54) failures++;
    }


    {
        uint16_t x = 20;
        x = x + 239;
        if (x != 259) failures++;
    }


    {
        uint8_t src[6] = {209,134,173,219,32,171};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[3] != 219) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {202,45,14324,187};
        if (s.b != (uint8_t)45) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 202;
        if (buf[11] != 202) failures++;
    }


    {
        uint16_t r = add2(240,109) + add2(109,244) + add2(240,244);
        if (r != 1186) failures++;
    }

    return failures;
}