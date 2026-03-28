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
        uint16_t r = 27167 + 57893 + 37346 + 55423 + 15489 + 39691 + 52035 + 65336;
        if (r != 22700) failures++;
    }


    {
        uint16_t r = add2(47,182) + add2(182,234) + add2(47,234);
        if (r != 926) failures++;
    }


    {
        uint8_t a[6] = {50,154,254,9,62,119};
        if (a[0] != 50) failures++;
    }


    {
        uint16_t x = 206;
        x = x + 56;
        if (x != 262) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t x = 52;
        x <<= 2;
        if (x != 208) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t a[6] = {210,207,198,120,35,107};
        if (a[0] != 210) failures++;
    }


    {
        uint8_t buf[8] = {56,13,110,60,146,29,112,2};
        uint8_t *p = buf;
        p += 4;
        if (*p != 146) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 63;
        if (buf[6] != 63) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)104) % (int16_t)((int8_t)98);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t v = 209;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 15) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {248,180,49345,245};
        if (s.b != (uint8_t)180) failures++;
    }


    {
        uint16_t r = add2(234,142) + add2(142,211) + add2(234,211);
        if (r != 1174) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)6) + (uint16_t)26729;
        if (r != 26735) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-44) / (int16_t)((int8_t)108);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 2: result = 130; break;
        case 9: result = 179; break;
        case 10: result = 21; break;
        case 5: result = 204; break;
        default: result = 10; break;
        }
        if (result != 130) failures++;
    }


    {
        uint8_t v = 133;
        v ^= 32;
        if (v != 165) failures++;
    }


    {
        uint16_t r = call6(123,189,113,91,242,35);
        if (r != 793) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-10) / (int16_t)((int8_t)77);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {141,8,67,216,12,34,128,73};
        uint8_t *p = buf;
        p += 4;
        if (*p != 12) failures++;
    }


    {
        uint8_t m[4][4] = {{235,142,105,234},{94,2,85,73},{150,59,0,180},{217,169,138,171}};
        if (m[0][0] != 235) failures++;
    }


    {
        uint8_t a[6] = {208,156,223,0,242,85};
        if (a[0] != 208) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 208;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-86) % (int16_t)((int8_t)59);
        if ((uint16_t)r != (uint16_t)65509) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {41,64,45389,185};
        if (s.c != (uint16_t)45389) failures++;
    }


    {
        uint8_t src[11] = {25,66,31,79,152,2,101,110,229,103,100};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[6] != 101) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        int8_t a = 106;
        int8_t b = -74;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 152;
        uint8_t r = port;
        if (r != 152) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(6,82) != 88) failures++;
    }


    {
        uint8_t x = 92;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        volatile uint8_t port = 167;
        uint8_t r = port;
        if (r != 167) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-56) / (int16_t)((int8_t)-10);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        g16 = 51415;
        if (read_g16() != 51415) failures++;
    }


    {
        uint8_t buf[8] = {189,216,245,244,229,226,29,14};
        uint8_t *p = buf;
        p += 3;
        if (*p != 244) failures++;
    }


    {
        volatile uint8_t port = 206;
        uint8_t r = port;
        if (r != 206) failures++;
    }


    {
        uint16_t r = 40548 + 63938 + 51668 + 52378 + 19717 + 11329 + 17787 + 8805;
        if (r != 4026) failures++;
    }


    {
        uint8_t v = 186;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 23;
        x <<= 2;
        if (x != 92) failures++;
    }


    {
        uint16_t x = 62154;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[4][2] = {{197,141},{161,223},{70,54},{63,12}};
        if (m[2][0] != 70) failures++;
    }


    {
        int8_t a = -26;
        int8_t b = 17;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 682449342UL;
        uint32_t b = 3864018775UL;
        uint32_t r = a & b;
        if (r != 536877334UL) failures++;
    }


    {
        uint8_t x = 159;
        x <<= 4;
        if (x != 240) failures++;
    }


    {
        uint8_t v = 149;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 3: result = 86; break;
        case 15: result = 123; break;
        case 8: result = 26; break;
        case 2: result = 48; break;
        case 0: result = 116; break;
        default: result = 111; break;
        }
        if (result != 48) failures++;
    }


    {
        uint16_t r = add2(5,41) + add2(41,10) + add2(5,10);
        if (r != 112) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {74,128,40551,180};
        if (s.b != (uint8_t)128) failures++;
    }


    {
        uint16_t r = call6(251,4,147,28,55,174);
        if (r != 659) failures++;
    }


    {
        g16 = 26408;
        if (read_g16() != 26408) failures++;
    }


    {
        uint32_t a = 868842837UL;
        uint32_t b = 2278987097UL;
        uint32_t r = a | b;
        if (r != 3084909917UL) failures++;
    }


    {
        if (((uint16_t)42) != 42) failures++;
    }


    {
        int8_t a = -49;
        int8_t b = 44;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-100) / (int16_t)((int8_t)36);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint8_t v = 101;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t src[4] = {48,240,13,11};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 48) failures++;
    }


    {
        uint8_t buf[8] = {89,24,19,2,45,196,1,109};
        uint8_t *p = buf;
        p += 4;
        if (*p != 45) failures++;
    }


    {
        uint16_t x = 225;
        x = x + 108;
        if (x != 333) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 9;
        if (buf[10] != 9) failures++;
    }


    {
        uint8_t src[2] = {73,11};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 11) failures++;
    }


    {
        uint16_t r = add2(250,133) + add2(133,45) + add2(250,45);
        if (r != 856) failures++;
    }


    {
        uint8_t v = 188;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {161,163,36,87,93,106};
        if (a[4] != 93) failures++;
    }


    {
        uint16_t r = call6(147,39,79,113,90,225);
        if (r != 693) failures++;
    }


    {
        uint8_t v = 71;
        v &= ~(uint8_t)4;
        if (v != 67) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {227,114,57017,243};
        if (s.d != (uint8_t)243) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint16_t r = add2(157,83) + add2(83,231) + add2(157,231);
        if (r != 942) failures++;
    }


    {
        uint16_t r = add2(19,48) + add2(48,96) + add2(19,96);
        if (r != 326) failures++;
    }


    {
        uint8_t m[3][3] = {{212,221,137},{27,240,166},{64,217,113}};
        if (m[0][2] != 137) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 187;
        if (buf[1] != 187) failures++;
    }


    {
        uint8_t src[1] = {59};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 59) failures++;
    }


    {
        volatile uint8_t port = 187;
        uint8_t r = port;
        if (r != 187) failures++;
    }


    {
        uint16_t x = 245;
        x = x + 120;
        if (x != 365) failures++;
    }


    {
        volatile int16_t a = 24451;
        volatile int16_t b = -13071;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 9: result = 161; break;
        case 7: result = 4; break;
        case 5: result = 50; break;
        case 19: result = 92; break;
        default: result = 51; break;
        }
        if (result != 50) failures++;
    }


    {
        uint8_t x = 203;
        x <<= 2;
        if (x != 44) failures++;
    }


    {
        uint8_t a[6] = {15,72,59,48,57,165};
        if (a[1] != 72) failures++;
    }


    {
        uint16_t x = 30155;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 6559;
        volatile int16_t b = -23515;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 31574;
        volatile int16_t b = 16895;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 11: result = 218; break;
        case 19: result = 145; break;
        case 0: result = 211; break;
        case 13: result = 225; break;
        case 18: result = 0; break;
        case 10: result = 39; break;
        default: result = 99; break;
        }
        if (result != 145) failures++;
    }


    {
        uint8_t x = 62;
        x <<= 0;
        if (x != 62) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(33,168) != 65401) failures++;
    }


    {
        if (((uint16_t)(((0 + 161) & 131) - ((98 + 70) ^ (191 | 129)))) != 106) failures++;
    }


    {
        uint8_t m[3][2] = {{229,13},{25,27},{25,28}};
        if (m[0][1] != 13) failures++;
    }


    {
        uint8_t a[6] = {249,188,122,56,125,89};
        if (a[1] != 188) failures++;
    }


    {
        uint16_t r = 53058 + 42717 + 36318 + 37792 + 50642 + 17212 + 42080 + 36897;
        if (r != 54572) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 12: result = 69; break;
        case 15: result = 109; break;
        case 8: result = 138; break;
        default: result = 72; break;
        }
        if (result != 72) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(114,3) != 117) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {238,101,60153,165};
        if (s.a != (uint8_t)238) failures++;
    }


    {
        uint8_t x = 159;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 162;
        if (buf[9] != 162) failures++;
    }


    {
        uint16_t x = 149;
        x = x + 103;
        if (x != 252) failures++;
    }


    {
        volatile uint8_t port = 86;
        uint8_t r = port;
        if (r != 86) failures++;
    }


    {
        uint8_t v = 240;
        v |= 4;
        if (v != 244) failures++;
    }


    {
        volatile uint8_t port = 68;
        uint8_t r = port;
        if (r != 68) failures++;
    }


    {
        uint32_t a = 3356566562UL;
        uint32_t b = 2969755471UL;
        uint32_t r = a ^ b;
        if (r != 2031352685UL) failures++;
    }


    {
        uint32_t a = 4103448547UL;
        uint32_t b = 2224032307UL;
        uint32_t r = a - b;
        if (r != 1879416240UL) failures++;
    }


    {
        uint16_t r = add2(207,236) + add2(236,35) + add2(207,35);
        if (r != 956) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(136,189) != 65483) failures++;
    }


    {
        uint16_t r = call6(69,180,190,237,90,12);
        if (r != 778) failures++;
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
        uint8_t src[2] = {166,39};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 166) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 11: result = 214; break;
        case 16: result = 113; break;
        case 1: result = 14; break;
        case 18: result = 43; break;
        case 13: result = 21; break;
        case 0: result = 13; break;
        default: result = 83; break;
        }
        if (result != 214) failures++;
    }


    {
        uint8_t m[2][3] = {{90,13,189},{45,149,56}};
        if (m[0][1] != 13) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-119) / (int16_t)((int8_t)109);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {136,185,23326,195};
        if (s.b != (uint8_t)185) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 17: result = 10; break;
        case 14: result = 188; break;
        case 1: result = 8; break;
        case 2: result = 220; break;
        case 15: result = 103; break;
        default: result = 250; break;
        }
        if (result != 220) failures++;
    }


    {
        uint8_t a[6] = {45,163,73,197,53,74};
        if (a[3] != 197) failures++;
    }


    {
        uint8_t v = 3;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t src[10] = {0,97,106,143,235,140,60,94,220,254};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[9] != 254) failures++;
    }


    {
        uint16_t r = add2(120,128) + add2(128,16) + add2(120,16);
        if (r != 528) failures++;
    }


    {
        uint32_t a = 3980342820UL;
        uint32_t b = 3272859180UL;
        uint32_t r = a | b;
        if (r != 4013948460UL) failures++;
    }


    {
        uint16_t r = call6(102,33,117,173,249,55);
        if (r != 729) failures++;
    }


    {
        uint16_t r = 15328 + 8153 + 26852 + 10629 + 57102 + 5864 + 5062 + 58478;
        if (r != 56396) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint8_t x = 7;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)160) + (uint16_t)62673;
        if (r != 62833) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 17: result = 250; break;
        case 18: result = 87; break;
        case 19: result = 219; break;
        case 13: result = 230; break;
        case 12: result = 201; break;
        case 8: result = 45; break;
        case 4: result = 74; break;
        default: result = 2; break;
        }
        if (result != 219) failures++;
    }


    {
        uint8_t buf[8] = {190,31,1,177,30,235,70,148};
        uint8_t *p = buf;
        p += 3;
        if (*p != 177) failures++;
    }


    {
        uint16_t x = 397;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)76) != 76) failures++;
    }


    {
        uint8_t x = 181;
        x <<= 3;
        if (x != 168) failures++;
    }


    {
        uint16_t x = 31594;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -41;
        int8_t b = -125;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(206,203) != 3) failures++;
    }


    {
        uint8_t x = 99;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t x = 47524;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-127) % (int16_t)((int8_t)-39);
        if ((uint16_t)r != (uint16_t)65526) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 205;
        if (buf[2] != 205) failures++;
    }


    {
        uint16_t x = 46335;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 3716351165UL;
        uint32_t b = 1286451427UL;
        uint32_t r = a - b;
        if (r != 2429899738UL) failures++;
    }


    {
        int8_t a = -42;
        int8_t b = -28;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 4482;
        volatile int16_t b = 3553;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 214;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t src[8] = {25,180,75,7,177,115,209,27};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 180) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)249) + (uint16_t)53850;
        if (r != 54099) failures++;
    }


    {
        uint16_t r = call6(53,103,36,229,86,200);
        if (r != 707) failures++;
    }


    {
        g16 = 12205;
        if (read_g16() != 12205) failures++;
    }


    {
        uint16_t x = 162;
        x = x + 210;
        if (x != 372) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 125;
        if (buf[4] != 125) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 20448 + 31429 + 59248 + 6102 + 8926 + 4323 + 61634 + 31414;
        if (r != 26916) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        uint8_t m[3][2] = {{123,58},{37,195},{72,69}};
        if (m[1][1] != 195) failures++;
    }


    {
        volatile int16_t a = 3538;
        volatile int16_t b = 16539;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {85,253,121,213,141,94,199,222};
        uint8_t *p = buf;
        p += 0;
        if (*p != 85) failures++;
    }


    {
        uint8_t src[8] = {150,101,6,110,207,98,20,218};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[4] != 207) failures++;
    }


    {
        uint16_t r = call6(101,76,109,229,229,117);
        if (r != 861) failures++;
    }


    {
        uint8_t src[4] = {229,102,51,85};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 229) failures++;
    }


    {
        uint16_t r = 14504 + 53981 + 46913 + 39439 + 62203 + 48802 + 23801 + 20131;
        if (r != 47630) failures++;
    }


    {
        volatile uint8_t port = 10;
        uint8_t r = port;
        if (r != 10) failures++;
    }


    {
        uint8_t x = 223;
        x <<= 2;
        if (x != 124) failures++;
    }


    {
        uint16_t x = 253;
        x = x + 206;
        if (x != 459) failures++;
    }


    {
        uint8_t x = 96;
        x <<= 0;
        if (x != 96) failures++;
    }


    {
        volatile int16_t a = 31113;
        volatile int16_t b = 18369;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {58,3,22,230,35,88};
        if (a[4] != 35) failures++;
    }


    {
        uint16_t r = call6(230,12,237,83,17,220);
        if (r != 799) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 244;
        if (buf[3] != 244) failures++;
    }


    {
        uint8_t src[1] = {52};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 52) failures++;
    }


    {
        g16 = 45649;
        if (read_g16() != 45649) failures++;
    }


    {
        if (((uint16_t)(((5 | 83) + (210 ^ 210)) & (16 & (203 + 169)))) != 16) failures++;
    }


    {
        volatile int16_t a = 14551;
        volatile int16_t b = 21360;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {216,88,103,8,52,21};
        if (a[5] != 21) failures++;
    }


    {
        g16 = 55876;
        if (read_g16() != 55876) failures++;
    }


    {
        volatile int16_t a = 17451;
        volatile int16_t b = -2007;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(32,238) != 65330) failures++;
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
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 10) failures++;
    }


    {
        volatile int16_t a = -25018;
        volatile int16_t b = -2548;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[2][3] = {{67,231,144},{183,116,152}};
        if (m[0][0] != 67) failures++;
    }


    {
        volatile uint8_t port = 236;
        uint8_t r = port;
        if (r != 236) failures++;
    }


    {
        uint16_t r = 12855 + 18627 + 59487 + 45205 + 54177 + 35779 + 5724 + 52991;
        if (r != 22701) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {50,80,63089,127};
        if (s.b != (uint8_t)80) failures++;
    }


    {
        volatile uint8_t port = 238;
        uint8_t r = port;
        if (r != 238) failures++;
    }


    {
        uint16_t x = 189;
        x = x + 106;
        if (x != 295) failures++;
    }


    {
        uint8_t v = 241;
        v ^= 1;
        if (v != 240) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 207;
        if (buf[8] != 207) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint16_t r = call6(207,34,220,133,25,23);
        if (r != 642) failures++;
    }


    {
        uint8_t m[2][3] = {{52,210,78},{55,112,80}};
        if (m[0][0] != 52) failures++;
    }


    {
        volatile int16_t a = -12064;
        volatile int16_t b = -26944;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 219;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
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
        uint16_t r = add2(82,50) + add2(50,141) + add2(82,141);
        if (r != 546) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 215;
        if (buf[13] != 215) failures++;
    }


    {
        int8_t a = 109;
        int8_t b = 4;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 124;
        v ^= 64;
        if (v != 60) failures++;
    }


    {
        if (((uint16_t)(((194 + 148) ^ 60) - ((202 | 239) + (173 | 11)))) != 65484) failures++;
    }


    {
        uint16_t x = 140;
        x = x + 120;
        if (x != 260) failures++;
    }


    {
        uint16_t x = 30;
        x = x + 83;
        if (x != 113) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint32_t a = 3937732813UL;
        uint32_t b = 935437129UL;
        uint32_t r = a + b;
        if (r != 578202646UL) failures++;
    }


    {
        uint8_t m[3][4] = {{255,2,67,215},{87,249,124,85},{225,49,3,106}};
        if (m[2][3] != 106) failures++;
    }


    {
        uint8_t v = 120;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 3950477230UL;
        uint32_t b = 3106834015UL;
        uint32_t r = a - b;
        if (r != 843643215UL) failures++;
    }


    {
        uint16_t r = 7664 + 3140 + 39417 + 40140 + 39916 + 11110 + 61274 + 49288;
        if (r != 55341) failures++;
    }


    {
        uint8_t a[6] = {247,42,18,41,25,212};
        if (a[2] != 18) failures++;
    }


    {
        uint32_t a = 1929666580UL;
        uint32_t b = 2379757864UL;
        uint32_t r = a + b;
        if (r != 14457148UL) failures++;
    }


    {
        uint8_t x = 51;
        x <<= 4;
        if (x != 48) failures++;
    }


    {
        uint16_t x = 18664;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 134;
        x = x + 229;
        if (x != 363) failures++;
    }


    {
        if (((uint16_t)25) != 25) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t a[6] = {204,246,204,57,141,82};
        if (a[4] != 141) failures++;
    }


    {
        uint8_t m[3][3] = {{168,232,127},{77,253,150},{129,43,116}};
        if (m[1][2] != 150) failures++;
    }


    {
        uint32_t a = 3585975048UL;
        uint32_t b = 1107654587UL;
        uint32_t r = a ^ b;
        if (r != 2545472691UL) failures++;
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
        while (!((++v) & 8)) count++;
        count++;
        if (count != 8) failures++;
    }


    {
        uint16_t x = 4884;
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {36,225,58553,143};
        if (s.c != (uint16_t)58553) failures++;
    }


    {
        uint8_t v = 142;
        v &= ~(uint8_t)64;
        if (v != 142) failures++;
    }


    {
        g16 = 29894;
        if (read_g16() != 29894) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 5: result = 77; break;
        case 19: result = 0; break;
        case 15: result = 62; break;
        case 12: result = 122; break;
        case 7: result = 2; break;
        case 13: result = 233; break;
        case 4: result = 242; break;
        case 11: result = 169; break;
        default: result = 189; break;
        }
        if (result != 2) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(186,185) != 371) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 7: result = 32; break;
        case 1: result = 147; break;
        case 12: result = 56; break;
        case 18: result = 109; break;
        case 13: result = 230; break;
        case 11: result = 48; break;
        case 14: result = 52; break;
        case 6: result = 195; break;
        default: result = 38; break;
        }
        if (result != 48) failures++;
    }


    {
        if (((uint16_t)((11 | 151) & (192 - 110))) != 18) failures++;
    }


    {
        uint8_t m[2][3] = {{151,16,136},{104,245,15}};
        if (m[1][2] != 15) failures++;
    }


    {
        g16 = 64138;
        if (read_g16() != 64138) failures++;
    }


    {
        uint8_t buf[8] = {121,65,0,193,244,121,48,155};
        uint8_t *p = buf;
        p += 6;
        if (*p != 48) failures++;
    }


    {
        uint8_t v = 2;
        v ^= 8;
        if (v != 10) failures++;
    }


    {
        volatile int16_t a = -6832;
        volatile int16_t b = 21976;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[15] = {187,115,118,117,9,139,155,198,162,58,236,157,123,184,27};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[10] != 236) failures++;
    }


    {
        if (((uint16_t)(((150 & 182) & (94 | 197)) & ((176 | 210) ^ (8 & 162)))) != 146) failures++;
    }


    {
        if (((uint16_t)(15 ^ 54)) != 57) failures++;
    }


    {
        uint8_t a[6] = {62,82,126,146,178,247};
        if (a[5] != 247) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
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
        for (uint16_t j = 0; j < 14; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 231;
        if (buf[14] != 231) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 89;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 102;
        v ^= 4;
        if (v != 98) failures++;
    }


    {
        uint8_t src[16] = {182,144,202,49,124,255,18,222,214,246,158,162,3,16,60,116};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[1] != 144) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(110,122) != 232) failures++;
    }


    {
        uint8_t x = 13;
        x <<= 2;
        if (x != 52) failures++;
    }


    {
        uint8_t a[6] = {36,102,63,116,93,117};
        if (a[1] != 102) failures++;
    }


    {
        g16 = 3620;
        if (read_g16() != 3620) failures++;
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
        if (fn(15,179) != 194) failures++;
    }


    {
        uint8_t x = 1;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t src[7] = {99,253,191,192,232,144,84};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[2] != 191) failures++;
    }


    {
        uint32_t a = 3159934891UL;
        uint32_t b = 3730166691UL;
        uint32_t r = a | b;
        if (r != 4267562923UL) failures++;
    }


    {
        uint8_t a[6] = {168,174,109,7,177,161};
        if (a[4] != 177) failures++;
    }


    {
        uint16_t x = 52376;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[2][2] = {{220,212},{2,196}};
        if (m[0][1] != 212) failures++;
    }


    {
        uint16_t x = 22446;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)76) % (int16_t)((int8_t)-104);
        if ((uint16_t)r != (uint16_t)76) failures++;
    }


    {
        uint16_t r = 7811 + 15504 + 417 + 35699 + 8872 + 43764 + 64152 + 17621;
        if (r != 62768) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)110) / (int16_t)((int8_t)-42);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint16_t x = 60;
        x = x + 91;
        if (x != 151) failures++;
    }


    {
        int8_t a = 23;
        int8_t b = 85;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint16_t r = call6(176,106,39,202,57,66);
        if (r != 646) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t m[2][4] = {{214,27,124,45},{84,236,75,202}};
        if (m[0][1] != 27) failures++;
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
        if (fn(31,169) != 65398) failures++;
    }


    {
        g16 = 54756;
        if (read_g16() != 54756) failures++;
    }


    {
        uint32_t a = 2705846792UL;
        uint32_t b = 996122720UL;
        uint32_t r = a + b;
        if (r != 3701969512UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(91,166) != 65461) failures++;
    }


    {
        volatile uint8_t port = 196;
        uint8_t r = port;
        if (r != 196) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(74,17) + add2(17,114) + add2(74,114);
        if (r != 410) failures++;
    }


    {
        uint8_t m[2][4] = {{215,165,237,135},{6,29,61,135}};
        if (m[1][1] != 29) failures++;
    }


    {
        uint8_t v = 35;
        int r = (v & 8) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)81) / (int16_t)((int8_t)-45);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t buf[8] = {11,139,45,13,57,117,183,126};
        uint8_t *p = buf;
        p += 0;
        if (*p != 11) failures++;
    }


    {
        if (((uint16_t)234) != 234) failures++;
    }


    {
        uint8_t x = 96;
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
        if (((uint16_t)(((44 ^ 120) + (248 | 216)) - 207)) != 125) failures++;
    }


    {
        volatile int16_t a = 11421;
        volatile int16_t b = 21345;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 4124662788UL;
        uint32_t b = 3643135966UL;
        uint32_t r = a ^ b;
        if (r != 754748378UL) failures++;
    }


    {
        uint16_t x = 163;
        x = x + 194;
        if (x != 357) failures++;
    }


    {
        uint16_t r = 54477 + 22504 + 17654 + 13376 + 22250 + 33898 + 56890 + 20157;
        if (r != 44598) failures++;
    }


    {
        uint8_t m[3][4] = {{210,214,244,196},{168,78,6,17},{6,51,184,42}};
        if (m[0][1] != 214) failures++;
    }


    {
        uint8_t m[2][4] = {{99,10,208,130},{204,205,131,195}};
        if (m[0][2] != 208) failures++;
    }


    {
        uint32_t a = 2045489540UL;
        uint32_t b = 1003060312UL;
        uint32_t r = a - b;
        if (r != 1042429228UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {77,249,25853,131};
        if (s.c != (uint16_t)25853) failures++;
    }


    {
        uint8_t v = 112;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int8_t a = 103;
        int8_t b = -28;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(((121 ^ 199) - 188) & 120)) != 0) failures++;
    }


    {
        if (((uint16_t)(241 ^ ((41 | 207) + (10 & 91)))) != 8) failures++;
    }


    {
        uint16_t r = call6(228,61,66,208,180,107);
        if (r != 850) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)136) + (uint16_t)19287;
        if (r != 19423) failures++;
    }


    {
        uint16_t r = call6(56,181,20,236,223,100);
        if (r != 816) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(248,179) != 427) failures++;
    }


    {
        g16 = 22311;
        if (read_g16() != 22311) failures++;
    }


    {
        uint16_t x = 115;
        x = x + 229;
        if (x != 344) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -31997;
        volatile int16_t b = 22666;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {255,45,28,41,230,150,81,34};
        uint8_t *p = buf;
        p += 3;
        if (*p != 41) failures++;
    }


    {
        uint8_t buf[8] = {186,152,132,202,194,170,92,150};
        uint8_t *p = buf;
        p += 4;
        if (*p != 194) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-68) / (int16_t)((int8_t)52);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(29,86) != 65479) failures++;
    }


    {
        volatile int16_t a = -18742;
        volatile int16_t b = 8449;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 42588 + 32307 + 45757 + 19383 + 30911 + 9644 + 54428 + 16949;
        if (r != 55359) failures++;
    }


    {
        if (((uint16_t)((28 + (185 | 89)) ^ (239 - 144))) != 330) failures++;
    }


    {
        if (((uint16_t)(((20 - 29) + 41) | (224 ^ 117))) != 181) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)46) + (uint16_t)46939;
        if (r != 46985) failures++;
    }


    {
        uint8_t m[2][4] = {{10,148,19,252},{244,65,195,198}};
        if (m[0][1] != 148) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 81;
        if (buf[14] != 81) failures++;
    }


    {
        volatile int16_t a = 15475;
        volatile int16_t b = 10962;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[10] = {245,93,159,100,119,173,103,149,106,75};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[4] != 119) failures++;
    }


    {
        uint8_t buf[8] = {212,212,240,196,150,78,58,187};
        uint8_t *p = buf;
        p += 1;
        if (*p != 212) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)90) + (uint16_t)18740;
        if (r != 18830) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        volatile int16_t a = -15214;
        volatile int16_t b = -29801;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 4: result = 72; break;
        case 17: result = 16; break;
        case 9: result = 46; break;
        case 5: result = 180; break;
        default: result = 55; break;
        }
        if (result != 46) failures++;
    }


    {
        uint16_t r = 35172 + 2479 + 2815 + 55039 + 23225 + 22052 + 62436 + 10605;
        if (r != 17215) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 208; break;
        case 14: result = 105; break;
        case 0: result = 102; break;
        case 3: result = 169; break;
        case 18: result = 63; break;
        case 1: result = 14; break;
        default: result = 59; break;
        }
        if (result != 208) failures++;
    }


    {
        if (((uint16_t)(31 & ((16 - 146) ^ (203 + 104)))) != 13) failures++;
    }


    {
        uint8_t m[3][4] = {{110,162,149,75},{128,120,62,170},{196,39,168,2}};
        if (m[2][1] != 39) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(21,227) != 248) failures++;
    }


    {
        uint16_t r = add2(61,87) + add2(87,178) + add2(61,178);
        if (r != 652) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {123,42,22956,39};
        if (s.a != (uint8_t)123) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 240;
        if (buf[14] != 240) failures++;
    }


    {
        uint8_t x = 71;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint16_t r = add2(121,252) + add2(252,191) + add2(121,191);
        if (r != 1128) failures++;
    }


    {
        uint8_t src[15] = {53,236,163,181,196,196,140,58,216,144,182,87,55,104,190};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[0] != 53) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 17: result = 216; break;
        case 14: result = 185; break;
        case 6: result = 38; break;
        case 11: result = 119; break;
        case 15: result = 2; break;
        case 8: result = 9; break;
        case 2: result = 204; break;
        case 5: result = 48; break;
        default: result = 82; break;
        }
        if (result != 9) failures++;
    }


    {
        uint16_t x = 197;
        x = x + 235;
        if (x != 432) failures++;
    }


    {
        uint16_t x = 187;
        x = x + 251;
        if (x != 438) failures++;
    }


    {
        uint8_t v = 216;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 64;
        if (buf[0] != 64) failures++;
    }


    {
        uint8_t src[1] = {140};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 140) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 17: result = 192; break;
        case 18: result = 85; break;
        case 4: result = 132; break;
        case 19: result = 131; break;
        case 13: result = 93; break;
        case 15: result = 240; break;
        case 5: result = 88; break;
        case 11: result = 125; break;
        default: result = 201; break;
        }
        if (result != 240) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(181,129) != 52) failures++;
    }


    {
        uint8_t buf[8] = {249,78,55,158,20,44,149,136};
        uint8_t *p = buf;
        p += 6;
        if (*p != 149) failures++;
    }


    {
        uint8_t src[1] = {162};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 162) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 7934;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = -6;
        int8_t b = -105;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 18: result = 123; break;
        case 9: result = 147; break;
        case 5: result = 107; break;
        case 11: result = 108; break;
        case 14: result = 42; break;
        case 13: result = 23; break;
        default: result = 127; break;
        }
        if (result != 107) failures++;
    }


    {
        uint8_t v = 184;
        int r = (v & 128) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t src[5] = {139,167,14,116,255};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[4] != 255) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(143,21) != 164) failures++;
    }


    {
        uint16_t r = call6(166,22,84,43,90,162);
        if (r != 567) failures++;
    }


    {
        int8_t a = 62;
        int8_t b = -66;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 246;
        uint8_t r = port;
        if (r != 246) failures++;
    }


    {
        uint8_t x = 177;
        x <<= 4;
        if (x != 16) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 1: result = 53; break;
        case 14: result = 209; break;
        case 8: result = 115; break;
        case 2: result = 62; break;
        default: result = 126; break;
        }
        if (result != 53) failures++;
    }


    {
        uint8_t m[4][3] = {{212,139,73},{200,195,64},{31,219,102},{108,169,221}};
        if (m[2][2] != 102) failures++;
    }


    {
        uint16_t x = 58537;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[4][2] = {{73,147},{217,42},{34,17},{41,185}};
        if (m[3][1] != 185) failures++;
    }


    {
        uint8_t buf[8] = {93,156,122,147,7,239,6,18};
        uint8_t *p = buf;
        p += 1;
        if (*p != 156) failures++;
    }


    {
        uint32_t a = 2223175944UL;
        uint32_t b = 1871735556UL;
        uint32_t r = a & b;
        if (r != 75524352UL) failures++;
    }


    {
        uint8_t v = 89;
        v &= ~(uint8_t)1;
        if (v != 88) failures++;
    }


    {
        uint32_t a = 2920576743UL;
        uint32_t b = 1244241068UL;
        uint32_t r = a & b;
        if (r != 167777444UL) failures++;
    }


    {
        uint8_t buf[8] = {192,85,172,195,60,223,5,181};
        uint8_t *p = buf;
        p += 4;
        if (*p != 60) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)141) + (uint16_t)64109;
        if (r != 64250) failures++;
    }


    {
        uint16_t r = call6(232,37,170,43,226,174);
        if (r != 882) failures++;
    }


    {
        uint8_t m[3][4] = {{84,185,238,8},{137,82,35,200},{173,11,193,138}};
        if (m[0][2] != 238) failures++;
    }


    {
        uint16_t x = 52284;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 230;
        x <<= 1;
        if (x != 204) failures++;
    }


    {
        uint16_t r = add2(93,138) + add2(138,196) + add2(93,196);
        if (r != 854) failures++;
    }


    {
        uint8_t a[6] = {210,228,118,5,219,234};
        if (a[1] != 228) failures++;
    }


    {
        uint32_t a = 2696637588UL;
        uint32_t b = 1947301083UL;
        uint32_t r = a ^ b;
        if (r != 3567917135UL) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 51;
        if (buf[7] != 51) failures++;
    }


    {
        uint8_t m[3][4] = {{143,222,195,94},{223,199,166,255},{207,195,9,48}};
        if (m[1][0] != 223) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(200,145) != 345) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(28,57) != 65507) failures++;
    }


    {
        int8_t a = 126;
        int8_t b = -55;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 241;
        uint8_t r = port;
        if (r != 241) failures++;
    }


    {
        uint8_t a[6] = {66,240,204,149,18,142};
        if (a[5] != 142) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[2][3] = {{55,213,102},{1,164,179}};
        if (m[1][2] != 179) failures++;
    }


    {
        uint8_t buf[8] = {31,17,17,37,251,202,127,159};
        uint8_t *p = buf;
        p += 7;
        if (*p != 159) failures++;
    }


    {
        uint16_t r = 9746 + 31873 + 51260 + 10862 + 49780 + 55680 + 169 + 12437;
        if (r != 25199) failures++;
    }


    {
        uint16_t r = 26604 + 17754 + 19525 + 57201 + 39284 + 19082 + 24224 + 38396;
        if (r != 45462) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 202;
        if (buf[15] != 202) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-45) % (int16_t)((int8_t)49);
        if ((uint16_t)r != (uint16_t)65491) failures++;
    }


    {
        uint16_t r = call6(126,54,69,4,173,138);
        if (r != 564) failures++;
    }


    {
        g16 = 62570;
        if (read_g16() != 62570) failures++;
    }


    {
        uint8_t v = 1;
        v ^= 128;
        if (v != 129) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 2: result = 177; break;
        case 16: result = 136; break;
        case 6: result = 119; break;
        case 19: result = 199; break;
        case 12: result = 178; break;
        default: result = 140; break;
        }
        if (result != 178) failures++;
    }


    {
        uint8_t x = 129;
        x <<= 0;
        if (x != 129) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)19) % (int16_t)((int8_t)117);
        if ((uint16_t)r != (uint16_t)19) failures++;
    }


    {
        uint16_t r = call6(186,245,68,251,183,71);
        if (r != 1004) failures++;
    }


    {
        uint8_t buf[8] = {83,209,171,106,208,218,173,99};
        uint8_t *p = buf;
        p += 5;
        if (*p != 218) failures++;
    }


    {
        int8_t a = -111;
        int8_t b = 127;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 155;
        uint8_t r = port;
        if (r != 155) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-115) % (int16_t)((int8_t)3);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t m[2][2] = {{252,90},{230,171}};
        if (m[1][1] != 171) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t src[5] = {104,11,223,241,17};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[1] != 11) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t x = 165;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(60,58) != 2) failures++;
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
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 2) sum += j;
        if (sum != 6) failures++;
    }


    {
        g16 = 20821;
        if (read_g16() != 20821) failures++;
    }


    {
        uint16_t r = add2(171,231) + add2(231,159) + add2(171,159);
        if (r != 1122) failures++;
    }


    {
        uint8_t src[14] = {211,191,161,81,132,107,111,81,178,36,208,57,87,216};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[12] != 87) failures++;
    }


    {
        uint16_t x = 22663;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 13;
        int8_t b = 26;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 120;
        uint8_t r = port;
        if (r != 120) failures++;
    }


    {
        uint16_t x = 52;
        x = x + 195;
        if (x != 247) failures++;
    }


    {
        uint16_t x = 134;
        x = x + 114;
        if (x != 248) failures++;
    }


    {
        uint8_t a[6] = {55,77,65,117,170,171};
        if (a[2] != 65) failures++;
    }


    {
        uint8_t m[4][3] = {{132,202,85},{150,84,168},{217,65,209},{129,8,8}};
        if (m[2][2] != 209) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 10: result = 53; break;
        case 1: result = 240; break;
        case 0: result = 22; break;
        case 18: result = 11; break;
        case 8: result = 113; break;
        case 9: result = 235; break;
        default: result = 194; break;
        }
        if (result != 113) failures++;
    }


    {
        uint16_t x = 11891;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][4] = {{163,191,18,19},{251,137,44,53},{89,139,115,22}};
        if (m[0][0] != 163) failures++;
    }


    {
        volatile uint8_t port = 240;
        uint8_t r = port;
        if (r != 240) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {113,76,65435,87};
        if (s.b != (uint8_t)76) failures++;
    }


    {
        volatile uint8_t port = 180;
        uint8_t r = port;
        if (r != 180) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 47;
        if (buf[12] != 47) failures++;
    }


    {
        uint32_t a = 4166496476UL;
        uint32_t b = 1683098413UL;
        uint32_t r = a & b;
        if (r != 1615986700UL) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 134;
        if (buf[6] != 134) failures++;
    }


    {
        uint8_t a[6] = {40,216,61,1,57,7};
        if (a[5] != 7) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 7: result = 200; break;
        case 5: result = 119; break;
        case 15: result = 180; break;
        default: result = 52; break;
        }
        if (result != 52) failures++;
    }


    {
        uint16_t r = add2(58,70) + add2(70,213) + add2(58,213);
        if (r != 682) failures++;
    }


    {
        volatile int16_t a = -8827;
        volatile int16_t b = -28185;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {46,183,37489,212};
        if (s.a != (uint8_t)46) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 1: result = 44; break;
        case 11: result = 172; break;
        case 15: result = 45; break;
        case 3: result = 82; break;
        case 5: result = 153; break;
        case 0: result = 242; break;
        default: result = 30; break;
        }
        if (result != 82) failures++;
    }


    {
        uint32_t a = 3575229345UL;
        uint32_t b = 19274962UL;
        uint32_t r = a + b;
        if (r != 3594504307UL) failures++;
    }


    {
        uint16_t r = 27296 + 33587 + 20536 + 59579 + 6278 + 27006 + 34329 + 6559;
        if (r != 18562) failures++;
    }


    {
        int8_t a = -62;
        int8_t b = -113;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 92;
        uint8_t r = port;
        if (r != 92) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(154,118) != 36) failures++;
    }


    {
        g16 = 46928;
        if (read_g16() != 46928) failures++;
    }


    {
        uint16_t x = 222;
        x = x + 9;
        if (x != 231) failures++;
    }


    {
        uint16_t r = add2(149,182) + add2(182,117) + add2(149,117);
        if (r != 896) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)157) + (uint16_t)43897;
        if (r != 44054) failures++;
    }


    {
        uint8_t m[4][3] = {{61,143,53},{250,7,114},{5,76,130},{143,161,163}};
        if (m[2][2] != 130) failures++;
    }


    {
        uint16_t x = 35362;
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
        uint8_t m[4][4] = {{187,142,157,120},{203,145,113,142},{178,84,40,178},{240,100,16,120}};
        if (m[2][1] != 84) failures++;
    }


    {
        uint8_t v = 241;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)(177 ^ ((27 + 105) & 30))) != 181) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)224) + (uint16_t)42870;
        if (r != 43094) failures++;
    }


    {
        uint16_t r = 36426 + 643 + 64202 + 26066 + 40274 + 47465 + 44794 + 28388;
        if (r != 26114) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 152;
        if (buf[10] != 152) failures++;
    }


    {
        uint16_t x = 47367;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 1: result = 166; break;
        case 3: result = 169; break;
        case 5: result = 250; break;
        default: result = 18; break;
        }
        if (result != 250) failures++;
    }


    {
        uint16_t r = call6(103,89,207,16,249,213);
        if (r != 877) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {62,166,49038,102};
        if (s.a != (uint8_t)62) failures++;
    }


    {
        uint8_t buf[8] = {155,239,140,124,152,169,32,212};
        uint8_t *p = buf;
        p += 0;
        if (*p != 155) failures++;
    }


    {
        uint8_t a[6] = {187,120,196,116,127,38};
        if (a[0] != 187) failures++;
    }


    {
        uint8_t v = 86;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 202; break;
        case 17: result = 27; break;
        case 8: result = 97; break;
        case 18: result = 100; break;
        case 13: result = 206; break;
        case 1: result = 116; break;
        default: result = 120; break;
        }
        if (result != 202) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(67,96) != 65507) failures++;
    }


    {
        uint8_t x = 179;
        x <<= 0;
        if (x != 179) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 75;
        if (buf[7] != 75) failures++;
    }


    {
        uint8_t buf[8] = {58,10,114,14,207,2,61,161};
        uint8_t *p = buf;
        p += 3;
        if (*p != 14) failures++;
    }


    {
        uint8_t a[6] = {128,169,148,60,81,180};
        if (a[5] != 180) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 3: result = 169; break;
        case 2: result = 109; break;
        case 0: result = 66; break;
        case 19: result = 169; break;
        case 11: result = 222; break;
        case 9: result = 33; break;
        case 4: result = 238; break;
        default: result = 165; break;
        }
        if (result != 238) failures++;
    }


    {
        uint32_t a = 211306779UL;
        uint32_t b = 2706117466UL;
        uint32_t r = a | b;
        if (r != 2916899675UL) failures++;
    }


    {
        uint16_t x = 52010;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-124) % (int16_t)((int8_t)6);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        uint8_t src[14] = {76,50,116,85,161,218,166,67,16,226,231,97,148,164};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[11] != 97) failures++;
    }


    {
        g16 = 56268;
        if (read_g16() != 56268) failures++;
    }


    {
        uint16_t r = call6(48,255,246,104,2,108);
        if (r != 763) failures++;
    }


    {
        uint8_t src[15] = {70,68,62,231,23,74,133,90,232,206,1,134,27,78,209};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[1] != 68) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)25) != 25) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 129;
        if (buf[14] != 129) failures++;
    }


    {
        if (((uint16_t)175) != 175) failures++;
    }


    {
        g16 = 12379;
        if (read_g16() != 12379) failures++;
    }


    {
        uint16_t x = 82;
        x = x + 117;
        if (x != 199) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)61) + (uint16_t)44406;
        if (r != 44467) failures++;
    }


    {
        uint8_t v = 34;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 94) failures++;
    }


    {
        uint16_t r = 22964 + 65388 + 46789 + 65456 + 36720 + 15619 + 2043 + 16634;
        if (r != 9469) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)62) % (int16_t)((int8_t)26);
        if ((uint16_t)r != (uint16_t)10) failures++;
    }


    {
        g16 = 53142;
        if (read_g16() != 53142) failures++;
    }


    {
        uint8_t buf[8] = {91,180,27,151,222,128,226,38};
        uint8_t *p = buf;
        p += 6;
        if (*p != 226) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 1354871961UL;
        uint32_t b = 2935617120UL;
        uint32_t r = a + b;
        if (r != 4290489081UL) failures++;
    }


    {
        int8_t a = 41;
        int8_t b = 111;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[2][4] = {{165,237,15,248},{135,154,45,156}};
        if (m[0][0] != 165) failures++;
    }


    {
        uint16_t r = add2(160,151) + add2(151,195) + add2(160,195);
        if (r != 1012) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = 2987;
        volatile int16_t b = 15830;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(((214 | 174) - (85 & 205)) | (168 - 59))) != 253) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-116) / (int16_t)((int8_t)116);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 48;
        if (buf[1] != 48) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {161,183,7132,139};
        if (s.a != (uint8_t)161) failures++;
    }


    {
        uint8_t v = 229;
        v |= 1;
        if (v != 229) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(183,19) != 164) failures++;
    }


    {
        uint16_t x = 2191;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -31104;
        volatile int16_t b = 21227;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(12,15) + add2(15,199) + add2(12,199);
        if (r != 452) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(80,121) != 65495) failures++;
    }


    {
        uint8_t x = 28;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(0,186) != 65350) failures++;
    }


    {
        volatile uint8_t port = 87;
        uint8_t r = port;
        if (r != 87) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 15176 + 501 + 45673 + 63515 + 55056 + 65224 + 40837 + 55924;
        if (r != 14226) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-46) / (int16_t)((int8_t)-39);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        volatile int16_t a = -8449;
        volatile int16_t b = -18648;
        int r = (a == b);
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
        uint16_t x = 7926;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 89;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 7) failures++;
    }


    {
        uint8_t a[6] = {51,32,162,82,227,182};
        if (a[0] != 51) failures++;
    }


    {
        uint16_t r = call6(234,161,164,244,216,165);
        if (r != 1184) failures++;
    }


    {
        uint8_t src[7] = {38,59,74,243,60,144,156};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[3] != 243) failures++;
    }


    {
        volatile int16_t a = 9004;
        volatile int16_t b = -11472;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 8: result = 26; break;
        case 5: result = 21; break;
        case 4: result = 203; break;
        case 12: result = 40; break;
        case 15: result = 79; break;
        case 1: result = 113; break;
        default: result = 58; break;
        }
        if (result != 40) failures++;
    }


    {
        if (((uint16_t)151) != 151) failures++;
    }


    {
        uint16_t r = add2(78,76) + add2(76,158) + add2(78,158);
        if (r != 624) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 131;
        if (buf[7] != 131) failures++;
    }


    {
        uint16_t x = 39892;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(88,102) != 190) failures++;
    }


    {
        uint16_t x = 37911;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 93;
        int8_t b = 14;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = -56;
        int8_t b = -48;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(89,182,175,33,31,240);
        if (r != 750) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 39;
        if (buf[11] != 39) failures++;
    }


    {
        uint8_t a[6] = {208,17,151,26,236,128};
        if (a[2] != 151) failures++;
    }


    {
        uint16_t x = 178;
        x = x + 152;
        if (x != 330) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {142,23,208,156,118,207,181,94};
        uint8_t *p = buf;
        p += 3;
        if (*p != 156) failures++;
    }


    {
        uint8_t x = 79;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 46645 + 8977 + 64937 + 23057 + 35114 + 34828 + 56433 + 34834;
        if (r != 42681) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint8_t buf[8] = {31,53,241,119,231,188,6,143};
        uint8_t *p = buf;
        p += 2;
        if (*p != 241) failures++;
    }


    {
        uint32_t a = 1171466718UL;
        uint32_t b = 3125130971UL;
        uint32_t r = a & b;
        if (r != 4268250UL) failures++;
    }


    {
        uint8_t v = 0;
        v ^= 32;
        if (v != 32) failures++;
    }


    {
        uint8_t v = 203;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {218,208,125,105,92,249,130,61};
        uint8_t *p = buf;
        p += 5;
        if (*p != 249) failures++;
    }


    {
        uint16_t r = call6(21,108,51,248,153,167);
        if (r != 748) failures++;
    }


    {
        int8_t a = 111;
        int8_t b = -13;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][3] = {{146,104,71},{246,146,198},{59,253,19},{35,90,247}};
        if (m[2][2] != 19) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(122,76) != 198) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {194,188,4146,212};
        if (s.b != (uint8_t)188) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {221,159,26340,230};
        if (s.a != (uint8_t)221) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(164,208) != 372) failures++;
    }


    {
        uint8_t v = 202;
        v &= ~(uint8_t)1;
        if (v != 202) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {57,13,43952,33};
        if (s.d != (uint8_t)33) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint32_t a = 843152747UL;
        uint32_t b = 1797979979UL;
        uint32_t r = a & b;
        if (r != 570457419UL) failures++;
    }


    {
        uint8_t a[6] = {183,123,214,58,61,59};
        if (a[5] != 59) failures++;
    }


    {
        if (((uint16_t)90) != 90) failures++;
    }


    {
        uint16_t r = 21105 + 20365 + 18688 + 34178 + 58011 + 30423 + 49796 + 25123;
        if (r != 61081) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)246) + (uint16_t)52933;
        if (r != 53179) failures++;
    }


    {
        uint8_t src[1] = {153};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 153) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(108,3,178,96,3,166);
        if (r != 554) failures++;
    }


    {
        uint8_t a[6] = {60,252,6,13,239,179};
        if (a[4] != 239) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-117) / (int16_t)((int8_t)85);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -106;
        int8_t b = 122;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(145,80) + add2(80,29) + add2(145,29);
        if (r != 508) failures++;
    }


    {
        uint32_t a = 4294020478UL;
        uint32_t b = 3006558027UL;
        uint32_t r = a ^ b;
        if (r != 1288036917UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint8_t a[6] = {195,158,2,100,48,50};
        if (a[0] != 195) failures++;
    }


    {
        int8_t a = 123;
        int8_t b = -48;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 30;
        x = x + 249;
        if (x != 279) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 75;
        if (buf[13] != 75) failures++;
    }


    {
        uint8_t m[3][3] = {{54,234,175},{246,86,144},{188,225,214}};
        if (m[2][2] != 214) failures++;
    }


    {
        uint8_t m[3][3] = {{172,112,185},{113,167,187},{62,172,156}};
        if (m[2][1] != 172) failures++;
    }


    {
        uint8_t src[10] = {45,74,120,96,105,152,29,35,11,50};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[4] != 105) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 247;
        if (buf[1] != 247) failures++;
    }


    {
        g16 = 45438;
        if (read_g16() != 45438) failures++;
    }


    {
        uint16_t x = 4737;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 237;
        uint8_t r = port;
        if (r != 237) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)80) + (uint16_t)54968;
        if (r != 55048) failures++;
    }


    {
        uint16_t x = 53295;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 63985;
        if (read_g16() != 63985) failures++;
    }


    {
        uint16_t x = 25168;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(96,214) != 65418) failures++;
    }


    {
        uint8_t buf[8] = {22,255,169,90,88,231,99,110};
        uint8_t *p = buf;
        p += 4;
        if (*p != 88) failures++;
    }


    {
        volatile int16_t a = -10009;
        volatile int16_t b = 8003;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 126;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(125,171) != 296) failures++;
    }


    {
        uint16_t r = add2(146,221) + add2(221,239) + add2(146,239);
        if (r != 1212) failures++;
    }


    {
        uint8_t src[11] = {155,151,76,143,76,204,163,67,223,228,81};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[5] != 204) failures++;
    }


    {
        int8_t a = 21;
        int8_t b = 114;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 72;
        v ^= 2;
        if (v != 74) failures++;
    }


    {
        volatile int16_t a = -11905;
        volatile int16_t b = -31431;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {15,72,23,203,51,66,39,25};
        uint8_t *p = buf;
        p += 6;
        if (*p != 39) failures++;
    }

    return failures;
}