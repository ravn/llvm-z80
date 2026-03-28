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
        int16_t r = (int16_t)((int8_t)28) % (int16_t)((int8_t)48);
        if ((uint16_t)r != (uint16_t)28) failures++;
    }


    {
        uint8_t a[6] = {150,226,42,175,72,108};
        if (a[1] != 226) failures++;
    }


    {
        int8_t a = -65;
        int8_t b = -118;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 9: result = 221; break;
        case 15: result = 249; break;
        case 4: result = 181; break;
        case 13: result = 40; break;
        case 17: result = 77; break;
        case 2: result = 99; break;
        case 5: result = 171; break;
        case 14: result = 86; break;
        default: result = 182; break;
        }
        if (result != 99) failures++;
    }


    {
        uint16_t r = call6(83,77,117,190,235,79);
        if (r != 781) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)70) + (uint16_t)62153;
        if (r != 62223) failures++;
    }


    {
        volatile int16_t a = 24407;
        volatile int16_t b = -19736;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 250;
        x = x + 236;
        if (x != 486) failures++;
    }


    {
        uint8_t v = 23;
        int r = (v & 8) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(242,231) != 11) failures++;
    }


    {
        uint8_t v = 51;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint8_t m[3][4] = {{92,234,221,80},{44,72,181,202},{87,215,184,157}};
        if (m[1][1] != 72) failures++;
    }


    {
        uint16_t x = 23350;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 1: result = 242; break;
        case 15: result = 236; break;
        case 5: result = 116; break;
        case 12: result = 37; break;
        case 6: result = 13; break;
        case 10: result = 243; break;
        case 8: result = 182; break;
        case 7: result = 170; break;
        default: result = 244; break;
        }
        if (result != 243) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint8_t m[4][3] = {{246,228,118},{24,135,52},{188,103,149},{191,202,78}};
        if (m[3][1] != 202) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)205) + (uint16_t)378;
        if (r != 583) failures++;
    }


    {
        uint16_t x = 35;
        x = x + 155;
        if (x != 190) failures++;
    }


    {
        uint8_t buf[8] = {233,108,117,182,63,114,69,209};
        uint8_t *p = buf;
        p += 4;
        if (*p != 63) failures++;
    }


    {
        uint16_t x = 36297;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 140;
        uint8_t r = port;
        if (r != 140) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)100) / (int16_t)((int8_t)64);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t a[6] = {83,232,18,136,222,21};
        if (a[3] != 136) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 252;
        if (buf[2] != 252) failures++;
    }


    {
        uint16_t r = 13284 + 46748 + 13683 + 46723 + 35720 + 43114 + 22755 + 52474;
        if (r != 12357) failures++;
    }


    {
        uint8_t m[3][2] = {{150,216},{31,186},{106,168}};
        if (m[2][0] != 106) failures++;
    }


    {
        uint8_t a[6] = {107,147,89,190,64,74};
        if (a[3] != 190) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 1: result = 8; break;
        case 18: result = 183; break;
        case 2: result = 225; break;
        case 12: result = 255; break;
        case 11: result = 52; break;
        case 8: result = 41; break;
        case 3: result = 177; break;
        case 17: result = 180; break;
        default: result = 127; break;
        }
        if (result != 180) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {132,234,41510,144};
        if (s.c != (uint16_t)41510) failures++;
    }


    {
        uint8_t m[4][4] = {{61,245,164,157},{92,19,248,97},{28,67,249,169},{182,84,243,22}};
        if (m[1][3] != 97) failures++;
    }


    {
        g16 = 13574;
        if (read_g16() != 13574) failures++;
    }


    {
        uint8_t x = 230;
        x <<= 3;
        if (x != 48) failures++;
    }


    {
        g16 = 3738;
        if (read_g16() != 3738) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint16_t r = add2(123,112) + add2(112,89) + add2(123,89);
        if (r != 648) failures++;
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
        volatile uint8_t port = 213;
        uint8_t r = port;
        if (r != 213) failures++;
    }


    {
        uint8_t x = 207;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(135,227) != 65444) failures++;
    }


    {
        uint8_t src[5] = {106,110,29,127,138};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[2] != 29) failures++;
    }


    {
        uint16_t r = 32687 + 45562 + 64837 + 56026 + 34272 + 32484 + 16542 + 41222;
        if (r != 61488) failures++;
    }


    {
        uint16_t r = call6(211,174,189,91,75,52);
        if (r != 792) failures++;
    }


    {
        uint16_t r = add2(32,189) + add2(189,83) + add2(32,83);
        if (r != 608) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        volatile uint8_t port = 28;
        uint8_t r = port;
        if (r != 28) failures++;
    }


    {
        uint8_t a[6] = {81,46,148,120,253,85};
        if (a[4] != 253) failures++;
    }


    {
        uint8_t v = 142;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint8_t a[6] = {87,112,72,247,176,2};
        if (a[5] != 2) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint8_t v = 252;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 32966;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 132;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 60) failures++;
    }


    {
        uint16_t r = 28752 + 34087 + 12197 + 41302 + 2433 + 12337 + 13270 + 52923;
        if (r != 693) failures++;
    }


    {
        uint8_t m[2][4] = {{13,138,89,220},{255,178,237,18}};
        if (m[1][1] != 178) failures++;
    }


    {
        g16 = 42421;
        if (read_g16() != 42421) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint8_t v = 197;
        int r = (v & 32) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t x = 24;
        x <<= 3;
        if (x != 192) failures++;
    }


    {
        if (((uint16_t)(((21 ^ 42) + (74 - 31)) | ((220 | 72) + 64))) != 382) failures++;
    }


    {
        uint16_t r = add2(58,130) + add2(130,152) + add2(58,152);
        if (r != 680) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)75) + (uint16_t)51271;
        if (r != 51346) failures++;
    }


    {
        int8_t a = -108;
        int8_t b = 43;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {57,201,23,236,198,33};
        if (a[5] != 33) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(133,237) != 65432) failures++;
    }


    {
        uint8_t a[6] = {73,201,39,163,229,33};
        if (a[5] != 33) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t buf[8] = {38,55,209,234,113,158,192,13};
        uint8_t *p = buf;
        p += 4;
        if (*p != 113) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 19: result = 34; break;
        case 6: result = 64; break;
        case 8: result = 103; break;
        case 11: result = 123; break;
        case 3: result = 90; break;
        default: result = 193; break;
        }
        if (result != 90) failures++;
    }


    {
        uint8_t v = 22;
        v |= 4;
        if (v != 22) failures++;
    }


    {
        uint8_t buf[8] = {249,239,94,28,195,87,111,133};
        uint8_t *p = buf;
        p += 6;
        if (*p != 111) failures++;
    }


    {
        uint16_t r = call6(156,158,153,29,33,74);
        if (r != 603) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t r = 15425 + 64773 + 58136 + 43042 + 62951 + 58893 + 18588 + 17435;
        if (r != 11563) failures++;
    }


    {
        uint8_t src[9] = {174,65,235,248,58,242,223,110,90};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[4] != 58) failures++;
    }


    {
        volatile uint8_t port = 182;
        uint8_t r = port;
        if (r != 182) failures++;
    }


    {
        volatile int16_t a = 3716;
        volatile int16_t b = 5829;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(27,245) != 65318) failures++;
    }


    {
        uint16_t x = 45;
        x = x + 117;
        if (x != 162) failures++;
    }


    {
        uint8_t src[9] = {206,128,157,137,118,164,125,111,49};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 157) failures++;
    }


    {
        uint16_t r = 40851 + 44605 + 60018 + 30577 + 36390 + 60004 + 64699 + 51551;
        if (r != 61015) failures++;
    }


    {
        int8_t a = 38;
        int8_t b = 82;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[3][2] = {{17,123},{245,174},{71,108}};
        if (m[0][0] != 17) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(186,233) != 419) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 106;
        if (buf[4] != 106) failures++;
    }


    {
        uint32_t a = 557129350UL;
        uint32_t b = 863101082UL;
        uint32_t r = a ^ b;
        if (r != 306511388UL) failures++;
    }


    {
        uint32_t a = 1618805483UL;
        uint32_t b = 3767407902UL;
        uint32_t r = a + b;
        if (r != 1091246089UL) failures++;
    }


    {
        uint8_t v = 9;
        v &= ~(uint8_t)4;
        if (v != 9) failures++;
    }


    {
        uint32_t a = 4252825408UL;
        uint32_t b = 609911724UL;
        uint32_t r = a & b;
        if (r != 609780480UL) failures++;
    }


    {
        if (((uint16_t)(((179 & 89) + (107 + 190)) + ((72 & 59) | 3))) != 325) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 8: result = 8; break;
        case 16: result = 141; break;
        case 1: result = 165; break;
        case 19: result = 223; break;
        case 11: result = 93; break;
        case 17: result = 230; break;
        case 13: result = 216; break;
        case 3: result = 54; break;
        default: result = 60; break;
        }
        if (result != 216) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)119) / (int16_t)((int8_t)124);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(86,255) != 65367) failures++;
    }


    {
        uint8_t a[6] = {206,213,173,208,242,234};
        if (a[3] != 208) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 1) sum += j;
        if (sum != 153) failures++;
    }


    {
        uint8_t x = 215;
        x <<= 5;
        if (x != 224) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 1: result = 209; break;
        case 10: result = 203; break;
        case 15: result = 201; break;
        default: result = 97; break;
        }
        if (result != 201) failures++;
    }


    {
        uint8_t v = 247;
        v &= ~(uint8_t)4;
        if (v != 243) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(186,71) != 115) failures++;
    }


    {
        uint8_t src[4] = {169,86,39,121};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[2] != 39) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)10) % (int16_t)((int8_t)-53);
        if ((uint16_t)r != (uint16_t)10) failures++;
    }


    {
        uint16_t r = 49692 + 38011 + 1126 + 15714 + 32314 + 8698 + 53296 + 9900;
        if (r != 12143) failures++;
    }


    {
        uint8_t v = 160;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 8) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 11: result = 67; break;
        case 15: result = 147; break;
        case 12: result = 55; break;
        case 7: result = 16; break;
        case 9: result = 26; break;
        default: result = 113; break;
        }
        if (result != 67) failures++;
    }


    {
        if (((uint16_t)(((57 | 143) - 119) ^ ((76 - 220) - (13 | 111)))) != 65353) failures++;
    }


    {
        uint8_t m[2][4] = {{126,157,175,218},{72,14,195,7}};
        if (m[1][2] != 195) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {167,124,163,122,107,231,251,109};
        uint8_t *p = buf;
        p += 7;
        if (*p != 109) failures++;
    }


    {
        if (((uint16_t)(((50 ^ 192) & (104 ^ 237)) + ((42 + 252) - 119))) != 303) failures++;
    }


    {
        int8_t a = -119;
        int8_t b = -47;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(69,135,148,201,3,148);
        if (r != 704) failures++;
    }


    {
        uint8_t v = 161;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 7) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 11;
        if (buf[12] != 11) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 14: result = 232; break;
        case 12: result = 169; break;
        case 13: result = 38; break;
        case 2: result = 179; break;
        default: result = 72; break;
        }
        if (result != 38) failures++;
    }


    {
        uint16_t r = add2(24,159) + add2(159,145) + add2(24,145);
        if (r != 656) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(81,159) != 240) failures++;
    }


    {
        uint8_t v = 67;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)130) + (uint16_t)52307;
        if (r != 52437) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint16_t r = 38914 + 22519 + 64444 + 3420 + 63970 + 48070 + 15227 + 26595;
        if (r != 21015) failures++;
    }


    {
        uint8_t v = 182;
        v &= ~(uint8_t)32;
        if (v != 150) failures++;
    }


    {
        int8_t a = -59;
        int8_t b = -108;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)21) + (uint16_t)12017;
        if (r != 12038) failures++;
    }


    {
        uint32_t a = 2295213024UL;
        uint32_t b = 1256945607UL;
        uint32_t r = a ^ b;
        if (r != 3257228327UL) failures++;
    }


    {
        uint32_t a = 3912559713UL;
        uint32_t b = 2764739472UL;
        uint32_t r = a + b;
        if (r != 2382331889UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)124) / (int16_t)((int8_t)73);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(191,18) != 173) failures++;
    }


    {
        volatile uint8_t port = 208;
        uint8_t r = port;
        if (r != 208) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-67) % (int16_t)((int8_t)-79);
        if ((uint16_t)r != (uint16_t)65469) failures++;
    }


    {
        uint16_t r = add2(83,249) + add2(249,195) + add2(83,195);
        if (r != 1054) failures++;
    }


    {
        uint16_t x = 10;
        x = x + 175;
        if (x != 185) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint32_t a = 2633763632UL;
        uint32_t b = 21102688UL;
        uint32_t r = a ^ b;
        if (r != 2646477648UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)77) + (uint16_t)21270;
        if (r != 21347) failures++;
    }


    {
        uint8_t src[3] = {114,169,17};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 169) failures++;
    }


    {
        uint16_t r = 28832 + 22788 + 57203 + 20191 + 33951 + 24636 + 65066 + 12385;
        if (r != 2908) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[12] = {205,2,53,201,137,216,167,137,41,196,121,103};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[3] != 201) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 132;
        if (buf[1] != 132) failures++;
    }


    {
        uint16_t r = 28581 + 39990 + 12302 + 59500 + 45089 + 44254 + 36141 + 14234;
        if (r != 17947) failures++;
    }


    {
        volatile uint8_t port = 179;
        uint8_t r = port;
        if (r != 179) failures++;
    }


    {
        uint32_t a = 963075271UL;
        uint32_t b = 1522853829UL;
        uint32_t r = a + b;
        if (r != 2485929100UL) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 21;
        if (buf[8] != 21) failures++;
    }


    {
        if (((uint16_t)(208 | ((222 ^ 136) + (37 + 4)))) != 255) failures++;
    }


    {
        uint32_t a = 583665961UL;
        uint32_t b = 2945673249UL;
        uint32_t r = a | b;
        if (r != 2950392105UL) failures++;
    }


    {
        uint8_t m[4][2] = {{47,110},{91,104},{229,210},{136,189}};
        if (m[3][1] != 189) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(7,61) != 68) failures++;
    }


    {
        uint8_t src[9] = {111,242,108,201,163,198,225,211,221};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[0] != 111) failures++;
    }


    {
        volatile uint8_t port = 128;
        uint8_t r = port;
        if (r != 128) failures++;
    }


    {
        uint16_t r = 58606 + 47995 + 12215 + 36486 + 37332 + 10153 + 39695 + 17520;
        if (r != 63394) failures++;
    }


    {
        uint16_t x = 171;
        x = x + 163;
        if (x != 334) failures++;
    }


    {
        uint8_t src[1] = {50};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 50) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 9;
        if (buf[12] != 9) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)123) % (int16_t)((int8_t)-1);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile int16_t a = 7863;
        volatile int16_t b = -4862;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 144;
        x = x + 30;
        if (x != 174) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)97) % (int16_t)((int8_t)124);
        if ((uint16_t)r != (uint16_t)97) failures++;
    }


    {
        uint8_t src[14] = {154,220,168,210,27,42,38,84,136,255,143,19,27,171};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[5] != 42) failures++;
    }


    {
        uint16_t r = call6(205,178,119,41,179,143);
        if (r != 865) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 103;
        if (buf[14] != 103) failures++;
    }


    {
        uint16_t r = call6(220,222,88,26,46,175);
        if (r != 777) failures++;
    }


    {
        uint8_t v = 154;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        uint16_t x = 23596;
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {200,152,19228,72};
        if (s.b != (uint8_t)152) failures++;
    }


    {
        uint16_t x = 33107;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 243;
        if (buf[12] != 243) failures++;
    }


    {
        uint16_t r = call6(91,157,92,8,15,150);
        if (r != 513) failures++;
    }


    {
        uint16_t r = 2268 + 57873 + 2937 + 20167 + 16934 + 5000 + 54429 + 10001;
        if (r != 38537) failures++;
    }


    {
        uint8_t m[2][4] = {{157,238,0,103},{203,159,33,196}};
        if (m[0][2] != 0) failures++;
    }


    {
        uint8_t x = 83;
        x <<= 0;
        if (x != 83) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)22) + (uint16_t)18164;
        if (r != 18186) failures++;
    }


    {
        uint16_t x = 251;
        x = x + 138;
        if (x != 389) failures++;
    }


    {
        uint32_t a = 770375612UL;
        uint32_t b = 2806992495UL;
        uint32_t r = a | b;
        if (r != 2951741439UL) failures++;
    }


    {
        uint8_t buf[8] = {241,21,18,168,189,75,55,197};
        uint8_t *p = buf;
        p += 4;
        if (*p != 189) failures++;
    }


    {
        if (((uint16_t)100) != 100) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 62; break;
        case 15: result = 237; break;
        case 10: result = 248; break;
        case 18: result = 67; break;
        default: result = 47; break;
        }
        if (result != 62) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 10: result = 97; break;
        case 3: result = 162; break;
        case 19: result = 105; break;
        case 2: result = 146; break;
        case 12: result = 255; break;
        case 0: result = 200; break;
        case 11: result = 102; break;
        case 15: result = 70; break;
        default: result = 81; break;
        }
        if (result != 162) failures++;
    }


    {
        uint16_t r = 32096 + 38998 + 18060 + 16995 + 31213 + 17449 + 54349 + 41958;
        if (r != 54510) failures++;
    }


    {
        int8_t a = 33;
        int8_t b = -41;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {161,194,226,136,14,21,175,199};
        uint8_t *p = buf;
        p += 3;
        if (*p != 136) failures++;
    }


    {
        uint8_t a[6] = {57,138,226,251,175,99};
        if (a[4] != 175) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 1) sum += j;
        if (sum != 78) failures++;
    }


    {
        uint16_t r = call6(181,108,119,119,86,161);
        if (r != 774) failures++;
    }


    {
        uint8_t m[4][4] = {{32,40,94,121},{225,2,103,41},{239,201,14,41},{22,42,82,190}};
        if (m[1][2] != 103) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 6: result = 66; break;
        case 14: result = 235; break;
        case 9: result = 49; break;
        case 10: result = 96; break;
        case 16: result = 109; break;
        default: result = 35; break;
        }
        if (result != 235) failures++;
    }


    {
        g16 = 58577;
        if (read_g16() != 58577) failures++;
    }


    {
        volatile uint8_t port = 127;
        uint8_t r = port;
        if (r != 127) failures++;
    }


    {
        uint16_t r = 54110 + 33727 + 12134 + 22700 + 11907 + 55407 + 34184 + 15280;
        if (r != 42841) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[2] = {98,89};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 98) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {127,197,42023,146};
        if (s.c != (uint16_t)42023) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 192;
        if (buf[10] != 192) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(144,129) != 273) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 6: result = 98; break;
        case 15: result = 138; break;
        case 19: result = 214; break;
        case 5: result = 37; break;
        default: result = 72; break;
        }
        if (result != 72) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(50,152) != 65434) failures++;
    }


    {
        volatile uint8_t port = 129;
        uint8_t r = port;
        if (r != 129) failures++;
    }


    {
        volatile int16_t a = 18555;
        volatile int16_t b = 15065;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 71;
        uint8_t r = port;
        if (r != 71) failures++;
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
        uint16_t x = 77;
        x = x + 198;
        if (x != 275) failures++;
    }


    {
        volatile uint8_t port = 221;
        uint8_t r = port;
        if (r != 221) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 11: result = 38; break;
        case 4: result = 88; break;
        case 15: result = 32; break;
        case 5: result = 246; break;
        case 8: result = 156; break;
        default: result = 223; break;
        }
        if (result != 32) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)29) + (uint16_t)28869;
        if (r != 28898) failures++;
    }


    {
        uint8_t x = 121;
        x <<= 4;
        if (x != 144) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        int8_t a = -81;
        int8_t b = -68;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 13: result = 202; break;
        case 8: result = 240; break;
        case 1: result = 25; break;
        case 17: result = 154; break;
        case 14: result = 26; break;
        case 18: result = 16; break;
        case 10: result = 99; break;
        case 2: result = 119; break;
        default: result = 158; break;
        }
        if (result != 240) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 13: result = 36; break;
        case 8: result = 165; break;
        case 12: result = 238; break;
        case 0: result = 21; break;
        default: result = 153; break;
        }
        if (result != 153) failures++;
    }


    {
        uint8_t x = 51;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        volatile uint8_t port = 197;
        uint8_t r = port;
        if (r != 197) failures++;
    }


    {
        uint8_t buf[8] = {57,151,161,27,247,144,224,166};
        uint8_t *p = buf;
        p += 5;
        if (*p != 144) failures++;
    }


    {
        uint8_t x = 75;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(142,247) != 389) failures++;
    }


    {
        uint16_t x = 24210;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)108) % (int16_t)((int8_t)2);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t x = 228;
        x <<= 4;
        if (x != 64) failures++;
    }


    {
        uint8_t m[3][4] = {{183,220,210,224},{44,212,214,224},{19,95,121,238}};
        if (m[0][2] != 210) failures++;
    }


    {
        volatile uint8_t port = 48;
        uint8_t r = port;
        if (r != 48) failures++;
    }


    {
        uint8_t m[2][3] = {{99,136,210},{82,71,64}};
        if (m[0][0] != 99) failures++;
    }


    {
        uint8_t x = 187;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 32444;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int8_t a = 35;
        int8_t b = 40;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][2] = {{56,218},{0,223},{243,228},{17,226}};
        if (m[0][0] != 56) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 11793 + 29106 + 50928 + 5020 + 12870 + 2666 + 26331 + 12190;
        if (r != 19832) failures++;
    }


    {
        uint32_t a = 2321977592UL;
        uint32_t b = 926973818UL;
        uint32_t r = a + b;
        if (r != 3248951410UL) failures++;
    }


    {
        g16 = 55512;
        if (read_g16() != 55512) failures++;
    }


    {
        uint16_t x = 48917;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(5,138) != 143) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {180,11,7933,5};
        if (s.a != (uint8_t)180) failures++;
    }


    {
        uint8_t v = 217;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][4] = {{20,180,212,236},{215,247,26,207}};
        if (m[0][2] != 212) failures++;
    }


    {
        if (((uint16_t)2) != 2) failures++;
    }


    {
        uint8_t m[2][3] = {{28,163,100},{34,216,249}};
        if (m[1][2] != 249) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 35;
        if (buf[12] != 35) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {155,9,29621,99};
        if (s.d != (uint8_t)99) failures++;
    }


    {
        uint8_t x = 11;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        volatile uint8_t port = 100;
        uint8_t r = port;
        if (r != 100) failures++;
    }


    {
        uint16_t r = call6(152,139,167,122,54,34);
        if (r != 668) failures++;
    }


    {
        uint16_t r = call6(74,10,164,140,20,187);
        if (r != 595) failures++;
    }


    {
        volatile uint8_t port = 144;
        uint8_t r = port;
        if (r != 144) failures++;
    }


    {
        uint8_t src[16] = {41,200,19,7,203,88,53,147,139,174,101,140,214,9,20,177};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[2] != 19) failures++;
    }


    {
        volatile uint8_t port = 81;
        uint8_t r = port;
        if (r != 81) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint16_t x = 43;
        x = x + 105;
        if (x != 148) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(73,189,151,117,91,24);
        if (r != 645) failures++;
    }


    {
        uint8_t src[9] = {177,252,185,93,152,181,141,172,205};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[2] != 185) failures++;
    }


    {
        uint8_t x = 129;
        x <<= 0;
        if (x != 129) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 45;
        if (buf[5] != 45) failures++;
    }


    {
        uint8_t src[11] = {75,55,185,237,230,123,48,203,98,246,7};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[9] != 246) failures++;
    }


    {
        uint8_t v = 105;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {11,155,26545,3};
        if (s.b != (uint8_t)155) failures++;
    }


    {
        uint16_t x = 65;
        x = x + 97;
        if (x != 162) failures++;
    }


    {
        if (((uint16_t)223) != 223) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t x = 32;
        x <<= 1;
        if (x != 64) failures++;
    }


    {
        uint8_t m[4][3] = {{80,200,30},{9,7,243},{120,37,229},{167,24,247}};
        if (m[3][0] != 167) failures++;
    }


    {
        volatile int16_t a = -4834;
        volatile int16_t b = 30730;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 18: result = 65; break;
        case 0: result = 135; break;
        case 6: result = 68; break;
        case 17: result = 248; break;
        case 15: result = 195; break;
        case 11: result = 233; break;
        case 13: result = 23; break;
        case 3: result = 137; break;
        default: result = 20; break;
        }
        if (result != 135) failures++;
    }


    {
        volatile uint8_t port = 142;
        uint8_t r = port;
        if (r != 142) failures++;
    }


    {
        uint16_t r = call6(155,9,212,31,124,112);
        if (r != 643) failures++;
    }


    {
        uint8_t a[6] = {118,155,195,251,9,246};
        if (a[3] != 251) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(56,199) != 255) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {146,76,62582,206};
        if (s.a != (uint8_t)146) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-8) / (int16_t)((int8_t)-101);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = add2(4,114) + add2(114,52) + add2(4,52);
        if (r != 340) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 4: result = 103; break;
        case 6: result = 68; break;
        case 8: result = 123; break;
        case 2: result = 65; break;
        case 7: result = 89; break;
        case 1: result = 165; break;
        case 12: result = 48; break;
        default: result = 97; break;
        }
        if (result != 97) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t a[6] = {39,128,208,208,83,89};
        if (a[0] != 39) failures++;
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
        if (((uint16_t)71) != 71) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 1) sum += j;
        if (sum != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {28,98,7390,4};
        if (s.d != (uint8_t)4) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-50) / (int16_t)((int8_t)-106);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 8: result = 33; break;
        case 17: result = 22; break;
        case 2: result = 238; break;
        default: result = 8; break;
        }
        if (result != 22) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        g16 = 27385;
        if (read_g16() != 27385) failures++;
    }


    {
        uint8_t x = 248;
        x <<= 4;
        if (x != 128) failures++;
    }


    {
        uint16_t r = 50747 + 50470 + 25980 + 64310 + 8717 + 24687 + 62971 + 10704;
        if (r != 36442) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t v = 26;
        v ^= 8;
        if (v != 18) failures++;
    }


    {
        uint8_t a[6] = {194,171,11,225,182,219};
        if (a[4] != 182) failures++;
    }


    {
        int8_t a = -76;
        int8_t b = -29;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 43;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint32_t a = 2889856497UL;
        uint32_t b = 4092173683UL;
        uint32_t r = a | b;
        if (r != 4294950387UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t v = 246;
        v |= 32;
        if (v != 246) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 109;
        if (buf[7] != 109) failures++;
    }


    {
        g16 = 29077;
        if (read_g16() != 29077) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)95) % (int16_t)((int8_t)-7);
        if ((uint16_t)r != (uint16_t)4) failures++;
    }


    {
        g16 = 35980;
        if (read_g16() != 35980) failures++;
    }


    {
        uint16_t x = 153;
        x = x + 226;
        if (x != 379) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(100,129) != 65507) failures++;
    }


    {
        volatile uint8_t port = 130;
        uint8_t r = port;
        if (r != 130) failures++;
    }


    {
        uint8_t v = 87;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][2] = {{101,2},{24,25}};
        if (m[0][1] != 2) failures++;
    }


    {
        uint8_t v = 108;
        v &= ~(uint8_t)16;
        if (v != 108) failures++;
    }


    {
        uint16_t x = 158;
        x = x + 41;
        if (x != 199) failures++;
    }


    {
        uint16_t r = 27098 + 8556 + 9063 + 20728 + 14006 + 51552 + 430 + 33727;
        if (r != 34088) failures++;
    }


    {
        uint8_t a[6] = {29,29,204,56,166,4};
        if (a[4] != 166) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 1: result = 13; break;
        case 7: result = 251; break;
        case 18: result = 36; break;
        case 14: result = 63; break;
        case 19: result = 187; break;
        case 6: result = 17; break;
        case 0: result = 85; break;
        case 10: result = 194; break;
        default: result = 117; break;
        }
        if (result != 251) failures++;
    }


    {
        uint16_t r = 3625 + 18425 + 20348 + 34457 + 41199 + 33027 + 51403 + 61739;
        if (r != 2079) failures++;
    }


    {
        uint8_t v = 209;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 15) failures++;
    }


    {
        uint8_t a[6] = {2,179,119,96,158,187};
        if (a[5] != 187) failures++;
    }


    {
        uint16_t r = add2(36,86) + add2(86,26) + add2(36,26);
        if (r != 296) failures++;
    }


    {
        uint8_t a[6] = {13,182,119,80,165,209};
        if (a[1] != 182) failures++;
    }


    {
        uint16_t x = 68;
        x = x + 70;
        if (x != 138) failures++;
    }


    {
        uint8_t m[2][2] = {{43,159},{182,196}};
        if (m[1][1] != 196) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)192) + (uint16_t)46276;
        if (r != 46468) failures++;
    }


    {
        g16 = 32759;
        if (read_g16() != 32759) failures++;
    }


    {
        g16 = 40882;
        if (read_g16() != 40882) failures++;
    }


    {
        uint32_t a = 3525283505UL;
        uint32_t b = 3441604209UL;
        uint32_t r = a - b;
        if (r != 83679296UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-73) % (int16_t)((int8_t)98);
        if ((uint16_t)r != (uint16_t)65463) failures++;
    }


    {
        uint8_t a[6] = {170,230,78,187,116,105};
        if (a[0] != 170) failures++;
    }


    {
        uint8_t m[2][4] = {{46,237,69,175},{233,204,60,3}};
        if (m[1][0] != 233) failures++;
    }


    {
        uint8_t src[6] = {148,206,234,78,178,193};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[1] != 206) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-1) / (int16_t)((int8_t)-76);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint32_t a = 396477779UL;
        uint32_t b = 1094223444UL;
        uint32_t r = a + b;
        if (r != 1490701223UL) failures++;
    }


    {
        uint32_t a = 3565478997UL;
        uint32_t b = 1629084801UL;
        uint32_t r = a & b;
        if (r != 1073792001UL) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 6;
        if (buf[6] != 6) failures++;
    }


    {
        uint8_t buf[8] = {176,13,204,101,25,234,32,126};
        uint8_t *p = buf;
        p += 6;
        if (*p != 32) failures++;
    }


    {
        volatile uint8_t port = 237;
        uint8_t r = port;
        if (r != 237) failures++;
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
        if (fn(223,214) != 9) failures++;
    }


    {
        uint8_t src[5] = {213,155,243,242,230};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 242) failures++;
    }


    {
        uint16_t r = add2(173,64) + add2(64,126) + add2(173,126);
        if (r != 726) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(92,96) != 65532) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-48) % (int16_t)((int8_t)-128);
        if ((uint16_t)r != (uint16_t)65488) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {140,150,39413,165};
        if (s.a != (uint8_t)140) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 45;
        if (buf[5] != 45) failures++;
    }


    {
        uint16_t r = add2(24,123) + add2(123,72) + add2(24,72);
        if (r != 438) failures++;
    }


    {
        uint8_t buf[8] = {196,59,76,47,173,10,226,67};
        uint8_t *p = buf;
        p += 5;
        if (*p != 10) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)74) + (uint16_t)39485;
        if (r != 39559) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {185,59,54011,116};
        if (s.c != (uint16_t)54011) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {135,23,41513,21};
        if (s.d != (uint8_t)21) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(240,236) != 476) failures++;
    }


    {
        uint8_t v = 125;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][4] = {{25,225,183,4},{232,105,164,48}};
        if (m[1][2] != 164) failures++;
    }


    {
        uint16_t r = add2(132,10) + add2(10,112) + add2(132,112);
        if (r != 508) failures++;
    }


    {
        uint8_t m[4][4] = {{4,67,208,35},{99,59,239,53},{12,172,43,251},{20,134,150,104}};
        if (m[2][3] != 251) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 41425;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 125;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t x = 40636;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(214,104,161,125,215,73);
        if (r != 892) failures++;
    }


    {
        int8_t a = 50;
        int8_t b = 31;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {116,15,36274,95};
        if (s.c != (uint16_t)36274) failures++;
    }


    {
        uint32_t a = 3499317491UL;
        uint32_t b = 2419379801UL;
        uint32_t r = a + b;
        if (r != 1623729996UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)173) + (uint16_t)35023;
        if (r != 35196) failures++;
    }


    {
        uint32_t a = 3496166268UL;
        uint32_t b = 2346575640UL;
        uint32_t r = a - b;
        if (r != 1149590628UL) failures++;
    }


    {
        uint16_t r = add2(108,132) + add2(132,207) + add2(108,207);
        if (r != 894) failures++;
    }


    {
        volatile uint8_t port = 242;
        uint8_t r = port;
        if (r != 242) failures++;
    }


    {
        uint16_t r = call6(24,11,34,148,223,40);
        if (r != 480) failures++;
    }


    {
        uint8_t src[13] = {150,69,28,6,124,68,84,23,176,34,11,247,207};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[5] != 68) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        uint8_t v = 145;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 171;
        if (buf[4] != 171) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(71,120) != 65487) failures++;
    }


    {
        uint16_t r = 12295 + 8771 + 56854 + 12983 + 39963 + 9065 + 38059 + 31502;
        if (r != 12884) failures++;
    }


    {
        uint16_t r = 18452 + 11403 + 13111 + 26322 + 24927 + 19383 + 43860 + 35461;
        if (r != 61847) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 6: result = 196; break;
        case 11: result = 61; break;
        case 18: result = 214; break;
        case 7: result = 207; break;
        case 14: result = 158; break;
        case 19: result = 128; break;
        case 3: result = 178; break;
        case 2: result = 29; break;
        default: result = 164; break;
        }
        if (result != 61) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 6: result = 151; break;
        case 0: result = 150; break;
        case 18: result = 65; break;
        default: result = 222; break;
        }
        if (result != 150) failures++;
    }


    {
        uint8_t m[4][3] = {{190,237,236},{222,15,193},{209,107,213},{238,33,173}};
        if (m[2][0] != 209) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-90) % (int16_t)((int8_t)-87);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 19: result = 222; break;
        case 13: result = 108; break;
        case 17: result = 7; break;
        case 5: result = 243; break;
        case 18: result = 133; break;
        case 2: result = 125; break;
        case 8: result = 71; break;
        case 0: result = 157; break;
        default: result = 240; break;
        }
        if (result != 222) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)41) / (int16_t)((int8_t)-96);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 223;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 17) failures++;
    }


    {
        if (((uint16_t)(79 | ((224 - 237) - (170 & 34)))) != 65503) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)15) + (uint16_t)64133;
        if (r != 64148) failures++;
    }


    {
        volatile int16_t a = -21977;
        volatile int16_t b = 20013;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 95;
        int r = (v & 32) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t m[2][4] = {{148,51,82,209},{178,248,11,91}};
        if (m[1][2] != 11) failures++;
    }


    {
        uint8_t buf[8] = {5,139,83,177,67,190,10,177};
        uint8_t *p = buf;
        p += 0;
        if (*p != 5) failures++;
    }


    {
        uint8_t v = 247;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 74;
        uint8_t r = port;
        if (r != 74) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {178,42,35317,51};
        if (s.c != (uint16_t)35317) failures++;
    }


    {
        uint8_t src[5] = {250,136,140,171,99};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[4] != 99) failures++;
    }


    {
        uint8_t m[2][2] = {{118,111},{37,157}};
        if (m[0][1] != 111) failures++;
    }


    {
        uint16_t x = 180;
        x = x + 206;
        if (x != 386) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        int8_t a = 50;
        int8_t b = -59;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {121,29,16133,242};
        if (s.d != (uint8_t)242) failures++;
    }


    {
        uint8_t v = 144;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 48) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {158,108,25015,160};
        if (s.a != (uint8_t)158) failures++;
    }


    {
        uint8_t buf[8] = {210,109,59,149,244,0,138,97};
        uint8_t *p = buf;
        p += 5;
        if (*p != 0) failures++;
    }


    {
        uint16_t r = add2(197,79) + add2(79,181) + add2(197,181);
        if (r != 914) failures++;
    }


    {
        uint16_t r = add2(146,57) + add2(57,91) + add2(146,91);
        if (r != 588) failures++;
    }


    {
        uint8_t v = 125;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        volatile int16_t a = -20091;
        volatile int16_t b = 2602;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 6133;
        volatile int16_t b = -9239;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 127;
        x = x + 124;
        if (x != 251) failures++;
    }


    {
        uint8_t src[11] = {69,225,81,143,122,39,57,199,187,161,64};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[7] != 199) failures++;
    }


    {
        volatile uint8_t port = 103;
        uint8_t r = port;
        if (r != 103) failures++;
    }


    {
        volatile uint8_t port = 183;
        uint8_t r = port;
        if (r != 183) failures++;
    }


    {
        uint16_t r = call6(84,185,106,225,221,231);
        if (r != 1052) failures++;
    }


    {
        uint16_t x = 5074;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)37) + (uint16_t)55495;
        if (r != 55532) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        uint16_t x = 242;
        x = x + 194;
        if (x != 436) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 71;
        if (buf[7] != 71) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {250,116,7522,237};
        if (s.b != (uint8_t)116) failures++;
    }


    {
        int8_t a = -8;
        int8_t b = -14;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 30;
        v ^= 4;
        if (v != 26) failures++;
    }


    {
        uint8_t v = 112;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 16) failures++;
    }


    {
        uint8_t a[6] = {41,60,222,166,247,94};
        if (a[0] != 41) failures++;
    }


    {
        if (((uint16_t)(117 ^ ((46 ^ 250) + 134))) != 303) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 79;
        if (buf[14] != 79) failures++;
    }


    {
        uint8_t src[15] = {226,60,99,107,249,155,234,143,70,92,233,2,18,71,122};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[6] != 234) failures++;
    }


    {
        uint8_t x = 159;
        x <<= 1;
        if (x != 62) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 165;
        if (buf[10] != 165) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        uint8_t v = 92;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {60,245,207,54,146,185,51,97};
        uint8_t *p = buf;
        p += 5;
        if (*p != 185) failures++;
    }


    {
        uint16_t r = call6(24,28,28,110,141,191);
        if (r != 522) failures++;
    }


    {
        uint8_t v = 196;
        v |= 4;
        if (v != 196) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)123) + (uint16_t)33819;
        if (r != 33942) failures++;
    }


    {
        uint8_t m[4][4] = {{60,0,134,229},{132,156,193,46},{157,133,180,113},{1,160,180,149}};
        if (m[3][2] != 180) failures++;
    }


    {
        uint8_t buf[8] = {143,1,152,110,180,163,172,116};
        uint8_t *p = buf;
        p += 7;
        if (*p != 116) failures++;
    }


    {
        uint16_t r = add2(102,149) + add2(149,100) + add2(102,100);
        if (r != 702) failures++;
    }


    {
        volatile int16_t a = -13845;
        volatile int16_t b = -22194;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 33525;
        if (read_g16() != 33525) failures++;
    }


    {
        uint8_t x = 171;
        x <<= 5;
        if (x != 96) failures++;
    }


    {
        uint16_t r = call6(143,143,79,61,7,146);
        if (r != 579) failures++;
    }


    {
        int8_t a = -62;
        int8_t b = 125;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 7: result = 233; break;
        case 13: result = 29; break;
        case 16: result = 100; break;
        default: result = 140; break;
        }
        if (result != 100) failures++;
    }


    {
        uint16_t x = 12;
        x = x + 55;
        if (x != 67) failures++;
    }


    {
        volatile uint8_t port = 36;
        uint8_t r = port;
        if (r != 36) failures++;
    }


    {
        uint8_t x = 183;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t a[6] = {100,221,59,57,249,84};
        if (a[4] != 249) failures++;
    }


    {
        uint8_t m[2][3] = {{142,255,34},{96,126,99}};
        if (m[0][2] != 34) failures++;
    }


    {
        uint16_t r = 25787 + 791 + 48278 + 13007 + 45783 + 63685 + 58542 + 5685;
        if (r != 64950) failures++;
    }


    {
        volatile uint8_t port = 122;
        uint8_t r = port;
        if (r != 122) failures++;
    }


    {
        uint8_t v = 191;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 17) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(3,59) != 65480) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 208;
        if (buf[3] != 208) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 58;
        if (buf[15] != 58) failures++;
    }


    {
        volatile uint8_t port = 125;
        uint8_t r = port;
        if (r != 125) failures++;
    }


    {
        uint16_t r = add2(194,39) + add2(39,125) + add2(194,125);
        if (r != 716) failures++;
    }


    {
        uint8_t v = 87;
        v ^= 32;
        if (v != 119) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 142;
        if (buf[15] != 142) failures++;
    }


    {
        uint8_t buf[8] = {168,219,87,186,39,80,96,0};
        uint8_t *p = buf;
        p += 0;
        if (*p != 168) failures++;
    }


    {
        uint16_t r = add2(162,3) + add2(3,81) + add2(162,81);
        if (r != 492) failures++;
    }


    {
        int8_t a = -41;
        int8_t b = 9;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 121;
        uint8_t r = port;
        if (r != 121) failures++;
    }


    {
        uint16_t r = 60463 + 41758 + 34421 + 44104 + 37816 + 16246 + 21721 + 18481;
        if (r != 12866) failures++;
    }


    {
        uint8_t m[4][4] = {{12,125,253,192},{47,100,115,25},{71,107,215,228},{21,100,218,56}};
        if (m[2][2] != 215) failures++;
    }


    {
        uint8_t v = 209;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 204;
        v &= ~(uint8_t)4;
        if (v != 200) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint8_t v = 222;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
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
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 16: result = 222; break;
        case 15: result = 150; break;
        case 19: result = 175; break;
        case 6: result = 84; break;
        case 14: result = 145; break;
        case 11: result = 25; break;
        case 1: result = 133; break;
        default: result = 157; break;
        }
        if (result != 133) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {61,63,25051,59};
        if (s.d != (uint8_t)59) failures++;
    }


    {
        if (((uint16_t)(171 ^ (28 | 85))) != 246) failures++;
    }


    {
        int8_t a = -23;
        int8_t b = 17;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 107;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 21) failures++;
    }


    {
        uint8_t v = 16;
        v &= ~(uint8_t)64;
        if (v != 16) failures++;
    }


    {
        volatile int16_t a = -22900;
        volatile int16_t b = -22832;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 109;
        x = x + 161;
        if (x != 270) failures++;
    }


    {
        uint8_t v = 42;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-105) / (int16_t)((int8_t)124);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        int8_t a = 44;
        int8_t b = -28;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 27176 + 21692 + 23011 + 53250 + 26385 + 27195 + 33728 + 9857;
        if (r != 25686) failures++;
    }


    {
        if (((uint16_t)7) != 7) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(44,162) != 65418) failures++;
    }


    {
        uint16_t x = 203;
        x = x + 31;
        if (x != 234) failures++;
    }


    {
        uint16_t r = 32731 + 45590 + 50057 + 26876 + 55073 + 56564 + 54906 + 63177;
        if (r != 57294) failures++;
    }


    {
        uint16_t x = 161;
        x = x + 181;
        if (x != 342) failures++;
    }


    {
        uint8_t m[4][3] = {{224,171,50},{201,209,82},{22,116,179},{185,9,76}};
        if (m[0][1] != 171) failures++;
    }


    {
        uint8_t x = 110;
        x <<= 0;
        if (x != 110) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)56) + (uint16_t)9747;
        if (r != 9803) failures++;
    }


    {
        if (((uint16_t)(((210 | 108) & 93) ^ ((48 & 160) - (176 - 156)))) != 80) failures++;
    }


    {
        int8_t a = 24;
        int8_t b = 37;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 7755;
        volatile int16_t b = -17531;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[3][2] = {{237,86},{184,57},{198,79}};
        if (m[0][1] != 86) failures++;
    }


    {
        uint8_t v = 198;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[2][2] = {{12,19},{178,66}};
        if (m[1][0] != 178) failures++;
    }


    {
        int8_t a = 43;
        int8_t b = 64;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t buf[8] = {252,183,106,202,28,77,178,193};
        uint8_t *p = buf;
        p += 0;
        if (*p != 252) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {129,119,13195,105};
        if (s.a != (uint8_t)129) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)22) / (int16_t)((int8_t)58);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint32_t a = 590466422UL;
        uint32_t b = 4099882442UL;
        uint32_t r = a | b;
        if (r != 4152360446UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        g16 = 55527;
        if (read_g16() != 55527) failures++;
    }


    {
        uint8_t v = 127;
        v &= ~(uint8_t)2;
        if (v != 125) failures++;
    }


    {
        uint16_t x = 50265;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[11] = {123,122,169,83,137,70,8,124,86,173,124};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[9] != 173) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)152) + (uint16_t)48973;
        if (r != 49125) failures++;
    }


    {
        uint8_t v = 204;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t x = 139;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        int8_t a = -68;
        int8_t b = 47;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 1) sum += j;
        if (sum != 105) failures++;
    }


    {
        volatile uint8_t port = 185;
        uint8_t r = port;
        if (r != 185) failures++;
    }


    {
        uint8_t v = 93;
        v |= 16;
        if (v != 93) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {154,106,37805,185};
        if (s.b != (uint8_t)106) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t a[6] = {232,46,44,111,2,182};
        if (a[0] != 232) failures++;
    }


    {
        int8_t a = -59;
        int8_t b = -87;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        g16 = 29851;
        if (read_g16() != 29851) failures++;
    }


    {
        if (((uint16_t)((102 ^ (56 - 85)) & 105)) != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t m[3][4] = {{88,51,109,217},{205,0,111,237},{43,56,180,246}};
        if (m[2][2] != 180) failures++;
    }


    {
        uint8_t v = 131;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint16_t r = add2(44,239) + add2(239,191) + add2(44,191);
        if (r != 948) failures++;
    }


    {
        if (((uint16_t)(((81 ^ 247) - 156) ^ ((96 ^ 6) - (6 - 175)))) != 261) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint32_t a = 70370552UL;
        uint32_t b = 1701547880UL;
        uint32_t r = a & b;
        if (r != 69305448UL) failures++;
    }


    {
        uint8_t x = 94;
        x <<= 2;
        if (x != 120) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)172) + (uint16_t)5194;
        if (r != 5366) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)71) % (int16_t)((int8_t)-60);
        if ((uint16_t)r != (uint16_t)11) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(48,14) != 62) failures++;
    }


    {
        uint16_t r = call6(175,217,71,217,11,65);
        if (r != 756) failures++;
    }


    {
        uint16_t r = 37949 + 209 + 36682 + 64707 + 3410 + 30751 + 62934 + 41682;
        if (r != 16180) failures++;
    }


    {
        uint8_t x = 169;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t r = call6(14,230,29,66,19,130);
        if (r != 488) failures++;
    }


    {
        uint16_t x = 199;
        x = x + 143;
        if (x != 342) failures++;
    }


    {
        g16 = 55783;
        if (read_g16() != 55783) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 4: result = 203; break;
        case 16: result = 138; break;
        case 9: result = 99; break;
        case 1: result = 152; break;
        case 8: result = 180; break;
        case 6: result = 236; break;
        case 15: result = 149; break;
        case 11: result = 240; break;
        default: result = 28; break;
        }
        if (result != 203) failures++;
    }


    {
        uint8_t m[2][3] = {{62,195,63},{190,209,217}};
        if (m[0][2] != 63) failures++;
    }


    {
        uint16_t r = add2(224,137) + add2(137,104) + add2(224,104);
        if (r != 930) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 5: result = 126; break;
        case 4: result = 76; break;
        case 19: result = 235; break;
        case 13: result = 221; break;
        case 16: result = 167; break;
        case 14: result = 46; break;
        case 2: result = 14; break;
        default: result = 0; break;
        }
        if (result != 221) failures++;
    }


    {
        int8_t a = 22;
        int8_t b = -68;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 16708 + 40803 + 39717 + 37447 + 7505 + 14446 + 49636 + 12093;
        if (r != 21747) failures++;
    }


    {
        uint16_t r = call6(17,28,236,136,81,174);
        if (r != 672) failures++;
    }


    {
        uint8_t a[6] = {226,13,25,125,8,107};
        if (a[3] != 125) failures++;
    }


    {
        if (((uint16_t)(186 - ((49 - 105) ^ (242 | 138)))) != 392) failures++;
    }


    {
        uint8_t m[2][3] = {{158,183,39},{128,166,151}};
        if (m[1][1] != 166) failures++;
    }


    {
        uint16_t r = 42622 + 5377 + 26627 + 32975 + 30971 + 2014 + 6337 + 18548;
        if (r != 34399) failures++;
    }


    {
        uint32_t a = 3158266848UL;
        uint32_t b = 1985869525UL;
        uint32_t r = a & b;
        if (r != 874336960UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)79) + (uint16_t)25451;
        if (r != 25530) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint8_t v = 73;
        v ^= 128;
        if (v != 201) failures++;
    }


    {
        int8_t a = -108;
        int8_t b = 51;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 13: result = 53; break;
        case 1: result = 17; break;
        case 12: result = 127; break;
        case 5: result = 79; break;
        case 14: result = 200; break;
        case 19: result = 218; break;
        case 6: result = 48; break;
        case 7: result = 87; break;
        default: result = 131; break;
        }
        if (result != 218) failures++;
    }


    {
        volatile uint8_t port = 144;
        uint8_t r = port;
        if (r != 144) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {190,176,37968,21};
        if (s.b != (uint8_t)176) failures++;
    }


    {
        uint8_t a[6] = {13,45,194,129,125,246};
        if (a[2] != 194) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(211,3) != 214) failures++;
    }


    {
        uint16_t r = call6(25,228,203,64,24,33);
        if (r != 577) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        g16 = 60541;
        if (read_g16() != 60541) failures++;
    }


    {
        uint16_t r = add2(115,39) + add2(39,205) + add2(115,205);
        if (r != 718) failures++;
    }


    {
        uint16_t r = add2(152,81) + add2(81,55) + add2(152,55);
        if (r != 576) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {108,210,17803,171};
        if (s.d != (uint8_t)171) failures++;
    }


    {
        uint8_t m[3][4] = {{87,8,191,155},{138,82,206,16},{80,30,117,102}};
        if (m[1][3] != 16) failures++;
    }


    {
        uint16_t x = 33772;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 11875;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = -31357;
        volatile int16_t b = -2557;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 9: result = 220; break;
        case 2: result = 129; break;
        case 12: result = 206; break;
        case 0: result = 176; break;
        case 7: result = 24; break;
        case 13: result = 159; break;
        case 4: result = 180; break;
        default: result = 94; break;
        }
        if (result != 220) failures++;
    }


    {
        uint8_t buf[8] = {183,22,233,243,240,134,160,149};
        uint8_t *p = buf;
        p += 7;
        if (*p != 149) failures++;
    }


    {
        uint8_t x = 92;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 24672;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 11912;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 1080888870UL;
        uint32_t b = 1673594374UL;
        uint32_t r = a + b;
        if (r != 2754483244UL) failures++;
    }


    {
        uint8_t x = 59;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint8_t x = 81;
        x <<= 4;
        if (x != 16) failures++;
    }


    {
        uint16_t r = 29053 + 20360 + 32687 + 43619 + 44694 + 13668 + 20550 + 29687;
        if (r != 37710) failures++;
    }


    {
        uint8_t v = 209;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(136,193) + add2(193,33) + add2(136,33);
        if (r != 724) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 113;
        int8_t b = 30;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 23885;
        volatile int16_t b = -4564;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[12] = {74,30,189,111,100,212,68,133,16,121,153,97};
        uint8_t dst[12];
        for (uint8_t j = 0; j < 12; j++) dst[j] = src[j];
        if (dst[8] != 16) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(76 - ((124 ^ 233) ^ (156 - 180)))) != 207) failures++;
    }


    {
        uint8_t m[2][2] = {{117,93},{48,236}};
        if (m[1][0] != 48) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)235) + (uint16_t)9671;
        if (r != 9906) failures++;
    }


    {
        uint16_t x = 13133;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        volatile int16_t a = 7247;
        volatile int16_t b = 27418;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {56,111,22,242,225,62,182,171};
        uint8_t *p = buf;
        p += 6;
        if (*p != 182) failures++;
    }


    {
        uint8_t v = 217;
        v &= ~(uint8_t)1;
        if (v != 216) failures++;
    }


    {
        uint16_t r = call6(245,84,144,252,151,222);
        if (r != 1098) failures++;
    }


    {
        uint16_t r = add2(137,165) + add2(165,84) + add2(137,84);
        if (r != 772) failures++;
    }


    {
        if (((uint16_t)254) != 254) failures++;
    }


    {
        volatile int16_t a = -13148;
        volatile int16_t b = 31361;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {43,76,33,233,188,208,248,235};
        uint8_t *p = buf;
        p += 1;
        if (*p != 76) failures++;
    }


    {
        volatile int16_t a = 20111;
        volatile int16_t b = -2531;
        int r = (a > b);
        if (r != 1) failures++;
    }

    return failures;
}