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
        int16_t r = (int16_t)((int8_t)74) / (int16_t)((int8_t)-92);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 18: result = 123; break;
        case 14: result = 212; break;
        case 9: result = 103; break;
        case 19: result = 164; break;
        case 6: result = 46; break;
        case 12: result = 145; break;
        case 2: result = 212; break;
        case 17: result = 97; break;
        default: result = 117; break;
        }
        if (result != 123) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(154 ^ ((20 - 211) & 156))) != 154) failures++;
    }


    {
        uint16_t r = add2(11,79) + add2(79,130) + add2(11,130);
        if (r != 440) failures++;
    }


    {
        uint8_t x = 186;
        x <<= 1;
        if (x != 116) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 3: result = 171; break;
        case 5: result = 125; break;
        case 11: result = 227; break;
        case 18: result = 115; break;
        case 17: result = 74; break;
        case 4: result = 178; break;
        case 0: result = 186; break;
        case 8: result = 207; break;
        default: result = 208; break;
        }
        if (result != 125) failures++;
    }


    {
        if (((uint16_t)((183 | 93) ^ ((243 - 184) & (178 + 152)))) != 245) failures++;
    }


    {
        uint8_t x = 77;
        x <<= 2;
        if (x != 52) failures++;
    }


    {
        uint8_t m[2][4] = {{95,218,82,40},{15,248,172,191}};
        if (m[1][1] != 248) failures++;
    }


    {
        uint8_t x = 175;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        volatile uint8_t port = 125;
        uint8_t r = port;
        if (r != 125) failures++;
    }


    {
        int8_t a = -14;
        int8_t b = 14;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(222,192,117,131,101,139);
        if (r != 902) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-19) / (int16_t)((int8_t)-97);
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
        uint16_t x = 33352;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(169,22) + add2(22,51) + add2(169,51);
        if (r != 484) failures++;
    }


    {
        volatile uint8_t port = 167;
        uint8_t r = port;
        if (r != 167) failures++;
    }


    {
        g16 = 969;
        if (read_g16() != 969) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {45,49,54651,222};
        if (s.b != (uint8_t)49) failures++;
    }


    {
        if (((uint16_t)(((17 - 135) & 219) ^ 134)) != 12) failures++;
    }


    {
        int8_t a = 15;
        int8_t b = -107;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][2] = {{180,4},{12,145},{13,176},{48,180}};
        if (m[0][0] != 180) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {184,202,14972,39};
        if (s.d != (uint8_t)39) failures++;
    }


    {
        uint16_t r = call6(208,80,59,230,112,237);
        if (r != 926) failures++;
    }


    {
        uint16_t x = 210;
        x = x + 33;
        if (x != 243) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        g16 = 2651;
        if (read_g16() != 2651) failures++;
    }


    {
        g16 = 47718;
        if (read_g16() != 47718) failures++;
    }


    {
        int8_t a = -13;
        int8_t b = 77;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 103;
        x <<= 0;
        if (x != 103) failures++;
    }


    {
        uint16_t r = 22267 + 33710 + 56511 + 50436 + 51190 + 57724 + 32839 + 3379;
        if (r != 45912) failures++;
    }


    {
        g16 = 19947;
        if (read_g16() != 19947) failures++;
    }


    {
        uint16_t x = 49381;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 24;
        x = x + 110;
        if (x != 134) failures++;
    }


    {
        volatile int16_t a = -27875;
        volatile int16_t b = 9817;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint8_t v = 95;
        v ^= 128;
        if (v != 223) failures++;
    }


    {
        uint8_t buf[8] = {215,50,145,161,204,86,197,129};
        uint8_t *p = buf;
        p += 2;
        if (*p != 145) failures++;
    }


    {
        volatile uint8_t port = 245;
        uint8_t r = port;
        if (r != 245) failures++;
    }


    {
        uint32_t a = 2731869359UL;
        uint32_t b = 514406316UL;
        uint32_t r = a | b;
        if (r != 3204265903UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 50083;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-1) % (int16_t)((int8_t)40);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        if (((uint16_t)(((97 & 252) ^ (120 - 31)) ^ ((179 | 231) & (7 & 98)))) != 59) failures++;
    }


    {
        uint8_t a[6] = {0,188,109,143,151,227};
        if (a[5] != 227) failures++;
    }


    {
        uint8_t m[3][4] = {{223,254,177,234},{5,38,61,55},{32,4,38,16}};
        if (m[2][1] != 4) failures++;
    }


    {
        uint16_t r = 32345 + 18599 + 20703 + 21474 + 7481 + 48145 + 15437 + 8750;
        if (r != 41862) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 22;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(37,165,171,8,169,48);
        if (r != 598) failures++;
    }


    {
        uint16_t r = call6(154,160,179,142,181,58);
        if (r != 874) failures++;
    }


    {
        uint32_t a = 3981679171UL;
        uint32_t b = 2378070891UL;
        uint32_t r = a | b;
        if (r != 3992976235UL) failures++;
    }


    {
        uint8_t x = 68;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        uint8_t v = 254;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = 24640 + 37415 + 36001 + 15953 + 25130 + 46460 + 57740 + 41586;
        if (r != 22781) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {37,246,22297,225};
        if (s.d != (uint8_t)225) failures++;
    }


    {
        uint8_t v = 42;
        v |= 1;
        if (v != 43) failures++;
    }


    {
        uint8_t v = 70;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 50796;
        if (read_g16() != 50796) failures++;
    }


    {
        uint16_t x = 97;
        x = x + 177;
        if (x != 274) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 18: result = 53; break;
        case 8: result = 220; break;
        case 12: result = 99; break;
        default: result = 150; break;
        }
        if (result != 99) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 163;
        if (buf[5] != 163) failures++;
    }


    {
        uint32_t a = 566686704UL;
        uint32_t b = 620223411UL;
        uint32_t r = a & b;
        if (r != 549901232UL) failures++;
    }


    {
        uint8_t buf[8] = {199,191,56,187,11,73,203,160};
        uint8_t *p = buf;
        p += 5;
        if (*p != 73) failures++;
    }


    {
        int8_t a = 65;
        int8_t b = 118;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 0;
        x = x + 177;
        if (x != 177) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -17366;
        volatile int16_t b = -9884;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 131;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint16_t x = 109;
        x = x + 112;
        if (x != 221) failures++;
    }


    {
        uint8_t x = 23;
        x <<= 1;
        if (x != 46) failures++;
    }


    {
        uint8_t v = 207;
        v ^= 2;
        if (v != 205) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 19;
        if (buf[3] != 19) failures++;
    }


    {
        uint8_t buf[8] = {42,205,194,205,126,35,74,19};
        uint8_t *p = buf;
        p += 7;
        if (*p != 19) failures++;
    }


    {
        uint16_t x = 35653;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {95,180,187,219,31,210,35,14};
        uint8_t *p = buf;
        p += 0;
        if (*p != 95) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 12: result = 164; break;
        case 13: result = 192; break;
        case 9: result = 71; break;
        case 2: result = 125; break;
        default: result = 77; break;
        }
        if (result != 77) failures++;
    }


    {
        uint8_t a[6] = {218,15,226,12,159,100};
        if (a[0] != 218) failures++;
    }


    {
        g16 = 31717;
        if (read_g16() != 31717) failures++;
    }


    {
        int8_t a = 12;
        int8_t b = 85;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 103;
        v |= 16;
        if (v != 119) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(179,199) != 378) failures++;
    }


    {
        uint8_t v = 59;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 9: result = 238; break;
        case 13: result = 60; break;
        case 2: result = 15; break;
        case 12: result = 233; break;
        default: result = 32; break;
        }
        if (result != 32) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint32_t a = 626866885UL;
        uint32_t b = 2578485977UL;
        uint32_t r = a ^ b;
        if (r != 3169691676UL) failures++;
    }


    {
        int8_t a = -102;
        int8_t b = -95;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-36) % (int16_t)((int8_t)43);
        if ((uint16_t)r != (uint16_t)65500) failures++;
    }


    {
        uint16_t r = call6(137,127,174,142,154,7);
        if (r != 741) failures++;
    }


    {
        uint8_t a[6] = {96,73,219,110,104,209};
        if (a[0] != 96) failures++;
    }


    {
        uint8_t src[6] = {72,125,189,114,241,232};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 232) failures++;
    }


    {
        g16 = 52988;
        if (read_g16() != 52988) failures++;
    }


    {
        uint8_t m[3][2] = {{120,230},{49,172},{24,186}};
        if (m[0][0] != 120) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)207) + (uint16_t)499;
        if (r != 706) failures++;
    }


    {
        uint8_t a[6] = {217,47,14,249,45,95};
        if (a[3] != 249) failures++;
    }


    {
        if (((uint16_t)(121 | 37)) != 125) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 185;
        if (buf[7] != 185) failures++;
    }


    {
        uint16_t r = add2(42,68) + add2(68,182) + add2(42,182);
        if (r != 584) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 3: result = 13; break;
        case 2: result = 183; break;
        case 16: result = 241; break;
        case 14: result = 224; break;
        case 12: result = 139; break;
        case 4: result = 34; break;
        case 18: result = 146; break;
        case 10: result = 140; break;
        default: result = 31; break;
        }
        if (result != 224) failures++;
    }


    {
        uint8_t x = 174;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint8_t buf[8] = {190,225,12,60,183,238,227,200};
        uint8_t *p = buf;
        p += 0;
        if (*p != 190) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(48,163) != 211) failures++;
    }


    {
        if (((uint16_t)130) != 130) failures++;
    }


    {
        uint16_t x = 188;
        x = x + 163;
        if (x != 351) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)43) / (int16_t)((int8_t)58);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        g16 = 23078;
        if (read_g16() != 23078) failures++;
    }


    {
        int8_t a = 40;
        int8_t b = -92;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)50) != 50) failures++;
    }


    {
        uint8_t a[6] = {190,65,181,42,255,231};
        if (a[2] != 181) failures++;
    }


    {
        uint16_t r = call6(144,147,26,175,47,148);
        if (r != 687) failures++;
    }


    {
        uint16_t r = add2(185,199) + add2(199,94) + add2(185,94);
        if (r != 956) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-36) / (int16_t)((int8_t)90);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = call6(63,240,83,220,55,255);
        if (r != 916) failures++;
    }


    {
        int8_t a = 8;
        int8_t b = -9;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-117) / (int16_t)((int8_t)123);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 221;
        uint8_t r = port;
        if (r != 221) failures++;
    }


    {
        uint8_t x = 127;
        x <<= 1;
        if (x != 254) failures++;
    }


    {
        uint8_t x = 255;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint32_t a = 4113021825UL;
        uint32_t b = 4090962533UL;
        uint32_t r = a | b;
        if (r != 4160207845UL) failures++;
    }


    {
        uint8_t x = 131;
        x <<= 0;
        if (x != 131) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(17,138) != 65415) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(134,126,133,125,223,115);
        if (r != 856) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {138,91,106,165,165,68,172,94};
        uint8_t *p = buf;
        p += 0;
        if (*p != 138) failures++;
    }


    {
        uint16_t x = 19017;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 50607 + 5857 + 20492 + 29702 + 26484 + 57784 + 30028 + 29506;
        if (r != 53852) failures++;
    }


    {
        uint16_t r = add2(88,56) + add2(56,30) + add2(88,30);
        if (r != 348) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 46;
        if (buf[7] != 46) failures++;
    }


    {
        uint8_t src[4] = {27,161,36,77};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 77) failures++;
    }


    {
        g16 = 60384;
        if (read_g16() != 60384) failures++;
    }


    {
        volatile uint8_t port = 41;
        uint8_t r = port;
        if (r != 41) failures++;
    }


    {
        uint8_t src[5] = {71,227,88,153,78};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[4] != 78) failures++;
    }


    {
        uint16_t r = call6(53,57,174,22,195,56);
        if (r != 557) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {107,86,27605,32};
        if (s.c != (uint16_t)27605) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        uint16_t r = add2(29,115) + add2(115,199) + add2(29,199);
        if (r != 686) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)101) % (int16_t)((int8_t)22);
        if ((uint16_t)r != (uint16_t)13) failures++;
    }


    {
        uint8_t m[2][4] = {{150,215,32,247},{143,5,191,135}};
        if (m[1][1] != 5) failures++;
    }


    {
        uint8_t m[3][2] = {{238,14},{55,203},{172,74}};
        if (m[2][1] != 74) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(248,48) != 200) failures++;
    }


    {
        uint8_t src[13] = {243,9,102,7,143,19,133,4,65,38,73,72,18};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[12] != 18) failures++;
    }


    {
        uint16_t r = call6(98,126,172,4,70,154);
        if (r != 624) failures++;
    }


    {
        uint32_t a = 4253942697UL;
        uint32_t b = 974297526UL;
        uint32_t r = a - b;
        if (r != 3279645171UL) failures++;
    }


    {
        uint8_t a[6] = {31,62,202,64,158,106};
        if (a[1] != 62) failures++;
    }


    {
        uint8_t v = 162;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        volatile int16_t a = 13481;
        volatile int16_t b = -19121;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {168,207,96,188,101,218};
        if (a[0] != 168) failures++;
    }


    {
        uint8_t src[9] = {191,139,165,59,112,181,103,215,122};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[1] != 139) failures++;
    }


    {
        uint16_t r = call6(85,24,32,143,224,99);
        if (r != 607) failures++;
    }


    {
        uint16_t r = call6(160,57,135,170,102,16);
        if (r != 640) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        if (((uint16_t)(106 ^ ((59 ^ 12) + (207 & 192)))) != 157) failures++;
    }


    {
        g16 = 8071;
        if (read_g16() != 8071) failures++;
    }


    {
        uint8_t a[6] = {231,47,123,210,192,127};
        if (a[0] != 231) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 16: result = 155; break;
        case 6: result = 23; break;
        case 7: result = 242; break;
        case 10: result = 94; break;
        case 12: result = 39; break;
        case 15: result = 23; break;
        case 9: result = 247; break;
        default: result = 103; break;
        }
        if (result != 23) failures++;
    }


    {
        volatile int16_t a = 10906;
        volatile int16_t b = -2335;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 208;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        if (((uint16_t)109) != 109) failures++;
    }


    {
        int8_t a = 51;
        int8_t b = -58;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)149) != 149) failures++;
    }


    {
        uint8_t x = 113;
        x <<= 0;
        if (x != 113) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)246) + (uint16_t)3174;
        if (r != 3420) failures++;
    }


    {
        uint8_t src[9] = {165,224,253,43,119,102,164,82,20};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[7] != 82) failures++;
    }


    {
        uint8_t a[6] = {99,61,216,43,28,14};
        if (a[5] != 14) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint16_t x = 32990;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint16_t r = 54350 + 49272 + 21365 + 41145 + 9897 + 25713 + 5261 + 49268;
        if (r != 59663) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {44,213,48127,202};
        if (s.b != (uint8_t)213) failures++;
    }


    {
        uint8_t src[10] = {96,85,77,76,187,98,172,48,114,39};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[1] != 85) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint16_t r = call6(0,205,14,27,48,28);
        if (r != 322) failures++;
    }


    {
        uint32_t a = 83948072UL;
        uint32_t b = 4155673574UL;
        uint32_t r = a + b;
        if (r != 4239621646UL) failures++;
    }


    {
        uint8_t x = 40;
        x <<= 4;
        if (x != 128) failures++;
    }


    {
        uint8_t buf[8] = {223,55,85,216,104,82,172,61};
        uint8_t *p = buf;
        p += 5;
        if (*p != 82) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 8: result = 222; break;
        case 0: result = 31; break;
        case 11: result = 156; break;
        case 7: result = 206; break;
        case 13: result = 101; break;
        case 4: result = 115; break;
        case 15: result = 185; break;
        default: result = 205; break;
        }
        if (result != 31) failures++;
    }


    {
        uint8_t v = 59;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t x = 171;
        x <<= 1;
        if (x != 86) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 11: result = 134; break;
        case 3: result = 101; break;
        case 17: result = 15; break;
        case 14: result = 210; break;
        case 6: result = 233; break;
        case 12: result = 21; break;
        default: result = 128; break;
        }
        if (result != 210) failures++;
    }


    {
        if (((uint16_t)78) != 78) failures++;
    }


    {
        volatile int16_t a = 8900;
        volatile int16_t b = 9161;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {62,104,139,231};
        if (s.a != (uint8_t)62) failures++;
    }


    {
        uint16_t r = add2(184,21) + add2(21,129) + add2(184,129);
        if (r != 668) failures++;
    }


    {
        uint8_t v = 37;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)145) + (uint16_t)28965;
        if (r != 29110) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint8_t buf[8] = {139,210,160,0,14,214,246,93};
        uint8_t *p = buf;
        p += 7;
        if (*p != 93) failures++;
    }


    {
        volatile uint8_t port = 156;
        uint8_t r = port;
        if (r != 156) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = 30903 + 63363 + 40369 + 27308 + 53455 + 43888 + 53643 + 15844;
        if (r != 1093) failures++;
    }


    {
        int8_t a = 8;
        int8_t b = -72;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 27049 + 36259 + 29412 + 9322 + 20353 + 43144 + 3983 + 53747;
        if (r != 26661) failures++;
    }


    {
        uint16_t x = 137;
        x = x + 90;
        if (x != 227) failures++;
    }


    {
        uint32_t a = 1472867810UL;
        uint32_t b = 2158973325UL;
        uint32_t r = a | b;
        if (r != 3622796783UL) failures++;
    }


    {
        volatile uint8_t port = 97;
        uint8_t r = port;
        if (r != 97) failures++;
    }


    {
        uint16_t r = add2(62,158) + add2(158,220) + add2(62,220);
        if (r != 880) failures++;
    }


    {
        uint8_t src[13] = {169,80,69,119,173,223,51,65,173,88,63,164,181};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[2] != 69) failures++;
    }


    {
        uint8_t v = 84;
        int r = (v & 32) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t r = 53573 + 41393 + 42785 + 13650 + 37746 + 8280 + 64518 + 30094;
        if (r != 29895) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 201; break;
        case 2: result = 39; break;
        case 4: result = 127; break;
        case 3: result = 186; break;
        default: result = 196; break;
        }
        if (result != 196) failures++;
    }


    {
        volatile int16_t a = 29314;
        volatile int16_t b = 22320;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 145;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 47) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)183) + (uint16_t)58546;
        if (r != 58729) failures++;
    }


    {
        uint8_t v = 66;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t src[1] = {138};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 138) failures++;
    }


    {
        uint16_t r = 16973 + 54853 + 28779 + 20941 + 26904 + 43761 + 63389 + 16540;
        if (r != 9996) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        volatile int16_t a = 4581;
        volatile int16_t b = 10834;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 243;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)159) != 159) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 5: result = 34; break;
        case 12: result = 243; break;
        case 10: result = 254; break;
        case 19: result = 196; break;
        case 2: result = 231; break;
        default: result = 114; break;
        }
        if (result != 34) failures++;
    }


    {
        uint16_t r = call6(104,225,29,36,88,108);
        if (r != 590) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(62,37) != 25) failures++;
    }


    {
        uint8_t v = 93;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t v = 194;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 232;
        x <<= 0;
        if (x != 232) failures++;
    }


    {
        uint8_t a[6] = {178,98,50,145,245,157};
        if (a[1] != 98) failures++;
    }


    {
        uint8_t v = 105;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 3243055009UL;
        uint32_t b = 3984514843UL;
        uint32_t r = a ^ b;
        if (r != 741591226UL) failures++;
    }


    {
        uint8_t buf[8] = {210,77,249,109,206,50,43,233};
        uint8_t *p = buf;
        p += 1;
        if (*p != 77) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)87) % (int16_t)((int8_t)63);
        if ((uint16_t)r != (uint16_t)24) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 0: result = 245; break;
        case 6: result = 77; break;
        case 11: result = 90; break;
        case 2: result = 56; break;
        case 10: result = 141; break;
        case 7: result = 190; break;
        case 9: result = 90; break;
        case 19: result = 229; break;
        default: result = 218; break;
        }
        if (result != 245) failures++;
    }


    {
        uint32_t a = 2781709738UL;
        uint32_t b = 3059721714UL;
        uint32_t r = a + b;
        if (r != 1546464156UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 1) sum += j;
        if (sum != 0) failures++;
    }


    {
        int8_t a = -37;
        int8_t b = 125;
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
        uint16_t r = (uint16_t)((uint8_t)34) + (uint16_t)4536;
        if (r != 4570) failures++;
    }


    {
        uint8_t x = 117;
        x <<= 1;
        if (x != 234) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {27,17,7311,118};
        if (s.d != (uint8_t)118) failures++;
    }


    {
        uint16_t x = 13;
        x = x + 241;
        if (x != 254) failures++;
    }


    {
        uint16_t x = 0;
        x = x + 168;
        if (x != 168) failures++;
    }


    {
        uint8_t v = 123;
        v ^= 16;
        if (v != 107) failures++;
    }


    {
        uint16_t r = call6(71,245,137,224,193,223);
        if (r != 1093) failures++;
    }


    {
        volatile int16_t a = -25401;
        volatile int16_t b = 17590;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        g16 = 50648;
        if (read_g16() != 50648) failures++;
    }


    {
        uint16_t r = add2(223,128) + add2(128,120) + add2(223,120);
        if (r != 942) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)155) + (uint16_t)3750;
        if (r != 3905) failures++;
    }


    {
        volatile int16_t a = 5491;
        volatile int16_t b = 25620;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 18;
        uint8_t r = port;
        if (r != 18) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 178;
        if (buf[9] != 178) failures++;
    }


    {
        uint8_t m[3][4] = {{22,223,153,220},{101,45,67,151},{138,243,115,121}};
        if (m[0][0] != 22) failures++;
    }


    {
        volatile uint8_t port = 150;
        uint8_t r = port;
        if (r != 150) failures++;
    }


    {
        volatile int16_t a = 310;
        volatile int16_t b = -32654;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 46;
        int8_t b = 45;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-19) / (int16_t)((int8_t)-120);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(38,201) != 65373) failures++;
    }


    {
        if (((uint16_t)((194 + (91 | 103)) ^ ((231 & 111) + 31))) != 455) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 26;
        if (buf[3] != 26) failures++;
    }


    {
        uint8_t a[6] = {145,137,142,237,152,21};
        if (a[5] != 21) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(151,89) != 240) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)223) + (uint16_t)50966;
        if (r != 51189) failures++;
    }


    {
        volatile uint8_t port = 164;
        uint8_t r = port;
        if (r != 164) failures++;
    }


    {
        volatile int16_t a = 24076;
        volatile int16_t b = 16733;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 3414619159UL;
        uint32_t b = 1623170940UL;
        uint32_t r = a ^ b;
        if (r != 2872666987UL) failures++;
    }


    {
        volatile uint8_t port = 143;
        uint8_t r = port;
        if (r != 143) failures++;
    }


    {
        uint8_t v = 61;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        g16 = 29112;
        if (read_g16() != 29112) failures++;
    }


    {
        uint8_t src[16] = {121,16,65,8,220,47,17,100,89,255,162,29,117,26,10,116};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[6] != 17) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)73) + (uint16_t)63479;
        if (r != 63552) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)224) + (uint16_t)41652;
        if (r != 41876) failures++;
    }


    {
        uint8_t a[6] = {50,200,54,111,208,66};
        if (a[3] != 111) failures++;
    }


    {
        uint8_t v = 82;
        v ^= 128;
        if (v != 210) failures++;
    }


    {
        uint32_t a = 41692530UL;
        uint32_t b = 1729736800UL;
        uint32_t r = a + b;
        if (r != 1771429330UL) failures++;
    }


    {
        if (((uint16_t)164) != 164) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 41;
        if (buf[12] != 41) failures++;
    }


    {
        if (((uint16_t)19) != 19) failures++;
    }


    {
        volatile uint8_t port = 19;
        uint8_t r = port;
        if (r != 19) failures++;
    }


    {
        uint8_t buf[8] = {92,178,76,130,40,202,63,32};
        uint8_t *p = buf;
        p += 4;
        if (*p != 40) failures++;
    }


    {
        uint8_t v = 214;
        v |= 2;
        if (v != 214) failures++;
    }


    {
        uint8_t src[2] = {52,78};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 78) failures++;
    }


    {
        uint16_t x = 41786;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)187) + (uint16_t)22723;
        if (r != 22910) failures++;
    }


    {
        uint16_t x = 33;
        x = x + 29;
        if (x != 62) failures++;
    }


    {
        uint16_t r = call6(126,150,226,11,214,89);
        if (r != 816) failures++;
    }


    {
        uint16_t r = call6(112,159,39,90,45,239);
        if (r != 684) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(122,183) != 65475) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 6: result = 153; break;
        case 4: result = 207; break;
        case 11: result = 150; break;
        case 19: result = 212; break;
        case 10: result = 118; break;
        case 15: result = 203; break;
        case 5: result = 193; break;
        default: result = 41; break;
        }
        if (result != 203) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-81) / (int16_t)((int8_t)-90);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {123,13,7481,162};
        if (s.d != (uint8_t)162) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 6: result = 149; break;
        case 7: result = 187; break;
        case 8: result = 60; break;
        case 18: result = 16; break;
        default: result = 3; break;
        }
        if (result != 187) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(191,239) != 65488) failures++;
    }


    {
        uint16_t r = 47813 + 3587 + 17253 + 23170 + 10119 + 46865 + 19616 + 32642;
        if (r != 4457) failures++;
    }


    {
        uint8_t v = 202;
        v |= 4;
        if (v != 206) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-115) / (int16_t)((int8_t)121);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t x = 138;
        x = x + 39;
        if (x != 177) failures++;
    }


    {
        if (((uint16_t)68) != 68) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        volatile uint8_t port = 54;
        uint8_t r = port;
        if (r != 54) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {164,92,57790,140};
        if (s.d != (uint8_t)140) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 22;
        if (buf[5] != 22) failures++;
    }


    {
        if (((uint16_t)207) != 207) failures++;
    }


    {
        uint8_t src[3] = {239,52,170};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 170) failures++;
    }


    {
        uint16_t r = 36234 + 50343 + 63526 + 31341 + 35870 + 36887 + 41512 + 9454;
        if (r != 43023) failures++;
    }


    {
        uint16_t r = add2(206,111) + add2(111,132) + add2(206,132);
        if (r != 898) failures++;
    }


    {
        volatile uint8_t port = 133;
        uint8_t r = port;
        if (r != 133) failures++;
    }


    {
        uint8_t src[3] = {71,212,91};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 91) failures++;
    }


    {
        g16 = 37184;
        if (read_g16() != 37184) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(254,11) != 265) failures++;
    }


    {
        uint8_t m[2][3] = {{44,81,166},{0,166,23}};
        if (m[1][1] != 166) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 9: result = 29; break;
        case 16: result = 211; break;
        case 18: result = 85; break;
        case 11: result = 248; break;
        case 6: result = 161; break;
        case 8: result = 175; break;
        case 15: result = 153; break;
        default: result = 12; break;
        }
        if (result != 153) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 1) sum += j;
        if (sum != 105) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)180) + (uint16_t)22080;
        if (r != 22260) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        volatile uint8_t port = 69;
        uint8_t r = port;
        if (r != 69) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)20) / (int16_t)((int8_t)59);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = add2(3,176) + add2(176,190) + add2(3,190);
        if (r != 738) failures++;
    }


    {
        volatile int16_t a = 12430;
        volatile int16_t b = 15927;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 129;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t a[6] = {118,213,4,193,128,186};
        if (a[0] != 118) failures++;
    }


    {
        uint8_t a[6] = {9,92,47,19,181,231};
        if (a[1] != 92) failures++;
    }


    {
        uint8_t v = 226;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        volatile uint8_t port = 64;
        uint8_t r = port;
        if (r != 64) failures++;
    }


    {
        uint16_t x = 161;
        x = x + 116;
        if (x != 277) failures++;
    }


    {
        uint16_t x = 20429;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 135;
        v ^= 2;
        if (v != 133) failures++;
    }


    {
        uint8_t v = 170;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {181,73,18855,204};
        if (s.c != (uint16_t)18855) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)36) + (uint16_t)65175;
        if (r != 65211) failures++;
    }


    {
        if (((uint16_t)(211 ^ ((217 + 73) - (198 + 231)))) != 65446) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(211,188) != 399) failures++;
    }


    {
        uint8_t x = 241;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        volatile int16_t a = -12684;
        volatile int16_t b = -27781;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)65) % (int16_t)((int8_t)-88);
        if ((uint16_t)r != (uint16_t)65) failures++;
    }


    {
        uint8_t src[3] = {72,11,78};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 72) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)186) + (uint16_t)29120;
        if (r != 29306) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint16_t x = 27920;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(171,212) != 65495) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {30,82,3898,152};
        if (s.a != (uint8_t)30) failures++;
    }


    {
        uint8_t buf[8] = {24,182,193,176,155,77,11,109};
        uint8_t *p = buf;
        p += 5;
        if (*p != 77) failures++;
    }


    {
        volatile uint8_t port = 173;
        uint8_t r = port;
        if (r != 173) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint16_t r = call6(223,65,153,85,102,50);
        if (r != 678) failures++;
    }


    {
        volatile uint8_t port = 145;
        uint8_t r = port;
        if (r != 145) failures++;
    }


    {
        volatile uint8_t port = 150;
        uint8_t r = port;
        if (r != 150) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)239) + (uint16_t)40761;
        if (r != 41000) failures++;
    }


    {
        uint8_t v = 197;
        v ^= 4;
        if (v != 193) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)127) + (uint16_t)1648;
        if (r != 1775) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 91;
        if (buf[12] != 91) failures++;
    }


    {
        int8_t a = -64;
        int8_t b = -83;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 118;
        int8_t b = 47;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[12] = {124,177,66,16,245,181,73,196,209,226,114,5};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[9] != 226) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)131) + (uint16_t)10110;
        if (r != 10241) failures++;
    }


    {
        volatile uint8_t port = 146;
        uint8_t r = port;
        if (r != 146) failures++;
    }


    {
        uint16_t x = 18157;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 2858033370UL;
        uint32_t b = 646069546UL;
        uint32_t r = a & b;
        if (r != 570564618UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 97;
        uint8_t r = port;
        if (r != 97) failures++;
    }


    {
        uint16_t x = 42698;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 57;
        x = x + 177;
        if (x != 234) failures++;
    }


    {
        uint8_t a[6] = {117,7,9,62,206,57};
        if (a[1] != 7) failures++;
    }


    {
        volatile int16_t a = 17297;
        volatile int16_t b = 13127;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(49,45,253,37,78,91);
        if (r != 553) failures++;
    }


    {
        uint8_t x = 87;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        uint8_t x = 21;
        x <<= 4;
        if (x != 80) failures++;
    }


    {
        uint8_t src[14] = {1,206,237,138,132,65,152,56,140,44,156,78,116,40};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[4] != 132) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {217,192,16005,92};
        if (s.a != (uint8_t)217) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 8: result = 211; break;
        case 3: result = 86; break;
        case 12: result = 6; break;
        case 13: result = 36; break;
        default: result = 203; break;
        }
        if (result != 211) failures++;
    }


    {
        uint8_t v = 59;
        v |= 128;
        if (v != 187) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {33,42,40010,76};
        if (s.c != (uint16_t)40010) failures++;
    }


    {
        uint8_t src[6] = {104,253,99,162,60,56};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[4] != 60) failures++;
    }


    {
        uint16_t r = 17542 + 51265 + 12410 + 21705 + 20656 + 32907 + 41072 + 30108;
        if (r != 31057) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint16_t r = call6(194,251,198,47,86,56);
        if (r != 832) failures++;
    }


    {
        uint16_t x = 42196;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][4] = {{105,86,112,92},{199,222,92,140},{245,43,72,245}};
        if (m[1][3] != 140) failures++;
    }


    {
        uint32_t a = 735054922UL;
        uint32_t b = 3687775098UL;
        uint32_t r = a - b;
        if (r != 1342247120UL) failures++;
    }


    {
        volatile int16_t a = 9104;
        volatile int16_t b = 26023;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(76,91) + add2(91,58) + add2(76,58);
        if (r != 450) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 1: result = 195; break;
        case 3: result = 42; break;
        case 12: result = 25; break;
        default: result = 130; break;
        }
        if (result != 130) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)147) + (uint16_t)34629;
        if (r != 34776) failures++;
    }


    {
        uint16_t x = 52261;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[2][2] = {{9,76},{120,214}};
        if (m[0][0] != 9) failures++;
    }


    {
        uint8_t buf[8] = {114,14,68,18,76,57,232,251};
        uint8_t *p = buf;
        p += 1;
        if (*p != 14) failures++;
    }


    {
        int8_t a = 12;
        int8_t b = 117;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)169) + (uint16_t)51690;
        if (r != 51859) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)206) + (uint16_t)9246;
        if (r != 9452) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)177) + (uint16_t)63489;
        if (r != 63666) failures++;
    }


    {
        uint16_t r = add2(74,101) + add2(101,116) + add2(74,116);
        if (r != 582) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 1: result = 215; break;
        case 11: result = 239; break;
        case 12: result = 236; break;
        case 9: result = 171; break;
        case 15: result = 22; break;
        case 10: result = 158; break;
        case 14: result = 219; break;
        default: result = 200; break;
        }
        if (result != 158) failures++;
    }


    {
        int8_t a = 8;
        int8_t b = 26;
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
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t r = call6(240,117,216,204,120,235);
        if (r != 1132) failures++;
    }


    {
        uint8_t v = 143;
        v ^= 64;
        if (v != 207) failures++;
    }


    {
        uint8_t a[6] = {232,179,7,65,236,50};
        if (a[2] != 7) failures++;
    }


    {
        uint8_t m[4][4] = {{128,6,21,234},{56,173,248,123},{21,241,25,91},{6,28,147,131}};
        if (m[3][2] != 147) failures++;
    }


    {
        uint8_t src[11] = {22,230,177,110,201,203,142,117,137,59,12};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[6] != 142) failures++;
    }


    {
        uint8_t v = 32;
        int r = (v & 8) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {208,70,63558,227};
        if (s.c != (uint16_t)63558) failures++;
    }


    {
        uint16_t r = call6(110,243,30,67,4,73);
        if (r != 527) failures++;
    }


    {
        volatile int16_t a = -27057;
        volatile int16_t b = 2015;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)126) + (uint16_t)17863;
        if (r != 17989) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)245) + (uint16_t)60318;
        if (r != 60563) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 61;
        x = x + 115;
        if (x != 176) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 60;
        if (buf[10] != 60) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {54,188,26341,102};
        if (s.b != (uint8_t)188) failures++;
    }


    {
        uint8_t v = 133;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 98;
        x <<= 4;
        if (x != 32) failures++;
    }


    {
        uint16_t x = 38135;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 4093681513UL;
        uint32_t b = 3621737770UL;
        uint32_t r = a | b;
        if (r != 4158644075UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {165,60,21081,78};
        if (s.b != (uint8_t)60) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)82) / (int16_t)((int8_t)-98);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 232;
        v &= ~(uint8_t)2;
        if (v != 232) failures++;
    }


    {
        uint8_t a[6] = {214,35,203,147,18,62};
        if (a[5] != 62) failures++;
    }


    {
        uint8_t v = 24;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {204,223,55187,241};
        if (s.b != (uint8_t)223) failures++;
    }


    {
        uint16_t x = 4036;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 9453;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 207;
        if (buf[0] != 207) failures++;
    }


    {
        uint16_t r = 33505 + 40743 + 5139 + 37778 + 23798 + 11768 + 24686 + 13184;
        if (r != 59529) failures++;
    }


    {
        uint16_t r = add2(75,131) + add2(131,131) + add2(75,131);
        if (r != 674) failures++;
    }


    {
        uint16_t r = 5020 + 3664 + 31125 + 30015 + 47148 + 28795 + 14273 + 33939;
        if (r != 62907) failures++;
    }


    {
        uint16_t x = 181;
        x = x + 195;
        if (x != 376) failures++;
    }


    {
        uint8_t a[6] = {30,119,219,178,88,69};
        if (a[0] != 30) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 151;
        if (buf[4] != 151) failures++;
    }


    {
        uint8_t buf[8] = {253,88,1,185,155,15,185,109};
        uint8_t *p = buf;
        p += 6;
        if (*p != 185) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t x = 248;
        x = x + 64;
        if (x != 312) failures++;
    }


    {
        uint8_t src[1] = {106};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 106) failures++;
    }


    {
        uint8_t v = 8;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 120) failures++;
    }


    {
        uint8_t a[6] = {23,193,8,94,29,63};
        if (a[3] != 94) failures++;
    }


    {
        uint16_t x = 253;
        x = x + 164;
        if (x != 417) failures++;
    }


    {
        uint8_t v = 70;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t m[3][4] = {{228,125,32,44},{40,15,41,242},{126,213,3,6}};
        if (m[2][0] != 126) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)7) / (int16_t)((int8_t)64);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile int16_t a = 25900;
        volatile int16_t b = -7160;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        g16 = 29278;
        if (read_g16() != 29278) failures++;
    }


    {
        uint8_t v = 129;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 111;
        uint8_t r = port;
        if (r != 111) failures++;
    }


    {
        uint8_t x = 218;
        x <<= 2;
        if (x != 104) failures++;
    }


    {
        volatile int16_t a = 28487;
        volatile int16_t b = -25192;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -112;
        int8_t b = -71;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 47285;
        if (read_g16() != 47285) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 35457 + 16901 + 562 + 54963 + 47919 + 8432 + 32917 + 30939;
        if (r != 31482) failures++;
    }


    {
        uint16_t x = 10367;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(176,13,249,139,166,12);
        if (r != 755) failures++;
    }


    {
        volatile int16_t a = -25479;
        volatile int16_t b = 27101;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        g16 = 53338;
        if (read_g16() != 53338) failures++;
    }


    {
        uint16_t r = 15401 + 24900 + 6386 + 11371 + 46789 + 64857 + 47046 + 38666;
        if (r != 58808) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        g16 = 47973;
        if (read_g16() != 47973) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)93) + (uint16_t)51889;
        if (r != 51982) failures++;
    }


    {
        uint8_t v = 89;
        int r = (v & 2) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 1) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint8_t m[2][2] = {{18,9},{219,130}};
        if (m[0][1] != 9) failures++;
    }


    {
        uint32_t a = 568104171UL;
        uint32_t b = 1208144191UL;
        uint32_t r = a | b;
        if (r != 1776211455UL) failures++;
    }


    {
        uint16_t x = 70;
        x = x + 69;
        if (x != 139) failures++;
    }


    {
        uint16_t r = 59544 + 45187 + 37610 + 65287 + 60141 + 44785 + 24619 + 4528;
        if (r != 14021) failures++;
    }


    {
        int8_t a = 86;
        int8_t b = -10;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 9;
        x <<= 0;
        if (x != 9) failures++;
    }


    {
        uint8_t src[7] = {153,155,240,186,184,115,128};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[0] != 153) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)226) + (uint16_t)38147;
        if (r != 38373) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 12: result = 137; break;
        case 17: result = 189; break;
        case 19: result = 117; break;
        default: result = 182; break;
        }
        if (result != 117) failures++;
    }


    {
        uint8_t x = 82;
        x <<= 0;
        if (x != 82) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 188;
        if (buf[1] != 188) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)251) + (uint16_t)3490;
        if (r != 3741) failures++;
    }


    {
        volatile uint8_t port = 129;
        uint8_t r = port;
        if (r != 129) failures++;
    }


    {
        uint8_t src[4] = {58,10,219,197};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[3] != 197) failures++;
    }


    {
        volatile uint8_t port = 146;
        uint8_t r = port;
        if (r != 146) failures++;
    }


    {
        volatile uint8_t port = 143;
        uint8_t r = port;
        if (r != 143) failures++;
    }


    {
        uint8_t src[1] = {33};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 33) failures++;
    }


    {
        uint16_t x = 191;
        x = x + 224;
        if (x != 415) failures++;
    }


    {
        volatile int16_t a = 32031;
        volatile int16_t b = -31082;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(16,194) != 210) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)193) + (uint16_t)49407;
        if (r != 49600) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint16_t r = call6(10,218,73,74,236,152);
        if (r != 763) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-112) % (int16_t)((int8_t)41);
        if ((uint16_t)r != (uint16_t)65506) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(165,207) != 372) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {58,89,64464,158};
        if (s.a != (uint8_t)58) failures++;
    }


    {
        uint8_t v = 222;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 55;
        x = x + 7;
        if (x != 62) failures++;
    }


    {
        uint32_t a = 976221226UL;
        uint32_t b = 4017025449UL;
        uint32_t r = a + b;
        if (r != 698279379UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(108 | (240 | (32 + 177)))) != 253) failures++;
    }


    {
        uint16_t x = 4;
        x = x + 251;
        if (x != 255) failures++;
    }


    {
        uint16_t r = add2(158,151) + add2(151,25) + add2(158,25);
        if (r != 668) failures++;
    }


    {
        uint8_t m[2][4] = {{55,179,140,206},{197,63,73,198}};
        if (m[1][3] != 198) failures++;
    }


    {
        uint8_t v = 89;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        volatile int16_t a = -5556;
        volatile int16_t b = -16878;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 25678 + 30146 + 42608 + 46405 + 36155 + 29500 + 52546 + 12972;
        if (r != 13866) failures++;
    }


    {
        uint8_t m[3][2] = {{87,228},{1,157},{179,6}};
        if (m[2][0] != 179) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)183) + (uint16_t)33661;
        if (r != 33844) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 1) sum += j;
        if (sum != 21) failures++;
    }


    {
        uint8_t buf[8] = {79,172,93,107,149,66,134,150};
        uint8_t *p = buf;
        p += 3;
        if (*p != 107) failures++;
    }


    {
        uint16_t x = 86;
        x = x + 137;
        if (x != 223) failures++;
    }


    {
        uint16_t r = 19745 + 46133 + 31480 + 44643 + 60974 + 52304 + 34088 + 4785;
        if (r != 32008) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 165;
        if (buf[2] != 165) failures++;
    }


    {
        uint16_t r = call6(194,107,249,213,147,147);
        if (r != 1057) failures++;
    }


    {
        uint16_t r = add2(11,130) + add2(130,4) + add2(11,4);
        if (r != 290) failures++;
    }


    {
        if (((uint16_t)(((121 - 38) - (94 - 225)) | ((39 ^ 166) ^ 158))) != 223) failures++;
    }


    {
        uint16_t r = 28450 + 41630 + 13638 + 16643 + 19639 + 9709 + 17669 + 18879;
        if (r != 35185) failures++;
    }


    {
        if (((uint16_t)(((170 | 11) - 130) + (99 ^ (138 & 254)))) != 274) failures++;
    }


    {
        uint32_t a = 4159910425UL;
        uint32_t b = 571593433UL;
        uint32_t r = a & b;
        if (r != 571544089UL) failures++;
    }


    {
        uint16_t r = 54906 + 25006 + 56505 + 39969 + 59692 + 28049 + 27898 + 36718;
        if (r != 1063) failures++;
    }


    {
        uint8_t src[3] = {15,187,62};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[2] != 62) failures++;
    }


    {
        volatile uint8_t port = 228;
        uint8_t r = port;
        if (r != 228) failures++;
    }


    {
        g16 = 13380;
        if (read_g16() != 13380) failures++;
    }


    {
        uint8_t buf[8] = {178,227,73,153,46,144,168,187};
        uint8_t *p = buf;
        p += 6;
        if (*p != 168) failures++;
    }


    {
        uint8_t a[6] = {225,22,204,28,161,111};
        if (a[1] != 22) failures++;
    }


    {
        volatile int16_t a = 4902;
        volatile int16_t b = 32478;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 219;
        uint8_t r = port;
        if (r != 219) failures++;
    }


    {
        uint16_t x = 2527;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][2] = {{108,164},{115,196},{221,54}};
        if (m[0][1] != 164) failures++;
    }


    {
        if (((uint16_t)(79 - ((186 + 229) & (24 + 189)))) != 65466) failures++;
    }


    {
        g16 = 48895;
        if (read_g16() != 48895) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(16,210) != 226) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)24) % (int16_t)((int8_t)39);
        if ((uint16_t)r != (uint16_t)24) failures++;
    }


    {
        uint8_t buf[8] = {191,99,56,30,210,218,173,188};
        uint8_t *p = buf;
        p += 4;
        if (*p != 210) failures++;
    }


    {
        volatile uint8_t port = 173;
        uint8_t r = port;
        if (r != 173) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t a[6] = {99,127,2,203,79,199};
        if (a[1] != 127) failures++;
    }


    {
        uint16_t x = 3004;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(((245 - 229) - (179 ^ 156)) ^ ((8 - 27) - (133 ^ 3)))) != 134) failures++;
    }


    {
        uint32_t a = 457692961UL;
        uint32_t b = 447454769UL;
        uint32_t r = a ^ b;
        if (r != 32262416UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)102) / (int16_t)((int8_t)89);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)88) % (int16_t)((int8_t)-100);
        if ((uint16_t)r != (uint16_t)88) failures++;
    }


    {
        uint8_t src[1] = {29};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 29) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {104,82,23433,36};
        if (s.c != (uint16_t)23433) failures++;
    }


    {
        g16 = 50225;
        if (read_g16() != 50225) failures++;
    }


    {
        uint32_t a = 1427449653UL;
        uint32_t b = 1673478884UL;
        uint32_t r = a + b;
        if (r != 3100928537UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-27) / (int16_t)((int8_t)78);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 26979 + 44762 + 44034 + 4296 + 23363 + 58611 + 4592 + 63914;
        if (r != 8407) failures++;
    }


    {
        uint8_t a[6] = {106,244,143,172,151,70};
        if (a[1] != 244) failures++;
    }


    {
        uint8_t x = 223;
        x <<= 1;
        if (x != 190) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {245,175,54535,183};
        if (s.c != (uint16_t)54535) failures++;
    }


    {
        volatile uint8_t port = 151;
        uint8_t r = port;
        if (r != 151) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)34) + (uint16_t)56759;
        if (r != 56793) failures++;
    }


    {
        uint8_t v = 205;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 19) failures++;
    }


    {
        uint8_t v = 26;
        v ^= 16;
        if (v != 10) failures++;
    }


    {
        volatile int16_t a = 11879;
        volatile int16_t b = -21772;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 63;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 65) failures++;
    }


    {
        uint8_t v = 145;
        int r = (v & 128) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint32_t a = 923303030UL;
        uint32_t b = 2181869204UL;
        uint32_t r = a - b;
        if (r != 3036401122UL) failures++;
    }


    {
        uint32_t a = 1301680116UL;
        uint32_t b = 997189778UL;
        uint32_t r = a ^ b;
        if (r != 1996089190UL) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 15: result = 28; break;
        case 5: result = 218; break;
        case 19: result = 145; break;
        case 10: result = 55; break;
        case 14: result = 173; break;
        case 7: result = 137; break;
        default: result = 246; break;
        }
        if (result != 55) failures++;
    }


    {
        uint32_t a = 892873858UL;
        uint32_t b = 417594526UL;
        uint32_t r = a | b;
        if (r != 1039924382UL) failures++;
    }


    {
        uint16_t r = 65084 + 8114 + 39223 + 46099 + 37353 + 15856 + 51270 + 16759;
        if (r != 17614) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)50) % (int16_t)((int8_t)89);
        if ((uint16_t)r != (uint16_t)50) failures++;
    }


    {
        uint8_t buf[8] = {58,71,79,158,200,137,213,109};
        uint8_t *p = buf;
        p += 3;
        if (*p != 158) failures++;
    }


    {
        uint16_t r = call6(174,210,86,196,103,180);
        if (r != 949) failures++;
    }


    {
        uint8_t a[6] = {237,240,98,121,8,151};
        if (a[3] != 121) failures++;
    }


    {
        volatile int16_t a = 7555;
        volatile int16_t b = -25808;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 57;
        if (buf[13] != 57) failures++;
    }


    {
        if (((uint16_t)122) != 122) failures++;
    }


    {
        volatile uint8_t port = 104;
        uint8_t r = port;
        if (r != 104) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {180,229,56131,56};
        if (s.a != (uint8_t)180) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 192;
        if (buf[9] != 192) failures++;
    }


    {
        uint16_t x = 5855;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -12415;
        volatile int16_t b = -10531;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(136 + (148 & (102 & 8)))) != 136) failures++;
    }


    {
        int8_t a = 30;
        int8_t b = -32;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][3] = {{83,252,135},{251,229,239},{140,159,161},{99,222,58}};
        if (m[2][1] != 159) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 239;
        x <<= 2;
        if (x != 188) failures++;
    }


    {
        uint8_t x = 166;
        x <<= 3;
        if (x != 48) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 50430;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 187;
        x = x + 57;
        if (x != 244) failures++;
    }


    {
        int8_t a = 72;
        int8_t b = -83;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(217,190) != 407) failures++;
    }


    {
        uint8_t v = 6;
        v &= ~(uint8_t)16;
        if (v != 6) failures++;
    }


    {
        uint16_t x = 54445;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[7] = {87,59,44,117,236,227,27};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[1] != 59) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-118) / (int16_t)((int8_t)80);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t m[3][3] = {{159,37,95},{2,121,128},{72,239,97}};
        if (m[2][2] != 97) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 154;
        if (buf[0] != 154) failures++;
    }


    {
        uint16_t x = 20;
        x = x + 60;
        if (x != 80) failures++;
    }


    {
        uint8_t src[4] = {6,25,95,255};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[1] != 25) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 37;
        if (buf[4] != 37) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 3: result = 88; break;
        case 8: result = 235; break;
        case 17: result = 146; break;
        case 4: result = 47; break;
        case 19: result = 69; break;
        case 16: result = 31; break;
        default: result = 94; break;
        }
        if (result != 88) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 97;
        if (buf[1] != 97) failures++;
    }


    {
        uint16_t r = add2(120,112) + add2(112,40) + add2(120,40);
        if (r != 544) failures++;
    }


    {
        uint16_t r = 36842 + 6600 + 4646 + 9114 + 29247 + 27118 + 43880 + 14445;
        if (r != 40820) failures++;
    }


    {
        int8_t a = -89;
        int8_t b = 11;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 191;
        v &= ~(uint8_t)8;
        if (v != 183) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 36;
        if (buf[2] != 36) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {204,249,54125,158};
        if (s.b != (uint8_t)249) failures++;
    }


    {
        uint8_t a[6] = {248,47,212,3,116,174};
        if (a[0] != 248) failures++;
    }


    {
        volatile int16_t a = 32539;
        volatile int16_t b = 1228;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 16670;
        volatile int16_t b = 23929;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 3) sum += j;
        if (sum != 63) failures++;
    }

    return failures;
}