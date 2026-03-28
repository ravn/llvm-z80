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
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(139,20) != 159) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[3][3] = {{68,149,60},{42,230,135},{93,179,226}};
        if (m[0][0] != 68) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 160;
        if (buf[11] != 160) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        int8_t a = -82;
        int8_t b = -121;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 517485374UL;
        uint32_t b = 1205580286UL;
        uint32_t r = a + b;
        if (r != 1723065660UL) failures++;
    }


    {
        uint16_t r = call6(151,241,52,44,76,106);
        if (r != 670) failures++;
    }


    {
        g16 = 62269;
        if (read_g16() != 62269) failures++;
    }


    {
        uint16_t x = 58;
        x = x + 188;
        if (x != 246) failures++;
    }


    {
        g16 = 56434;
        if (read_g16() != 56434) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 1) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint32_t a = 80098936UL;
        uint32_t b = 1203753500UL;
        uint32_t r = a | b;
        if (r != 1207957116UL) failures++;
    }


    {
        uint8_t m[2][3] = {{133,194,47},{51,40,78}};
        if (m[1][1] != 40) failures++;
    }


    {
        if (((uint16_t)(((101 + 84) | (15 ^ 202)) ^ (99 - (238 - 37)))) != 65383) failures++;
    }


    {
        uint16_t x = 31814;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint16_t x = 19534;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)(84 & (188 | (149 - 101)))) != 20) failures++;
    }


    {
        volatile int16_t a = -24447;
        volatile int16_t b = -24076;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = 56580 + 58163 + 23898 + 3366 + 53516 + 3840 + 59925 + 15748;
        if (r != 12892) failures++;
    }


    {
        if (((uint16_t)(((203 & 131) + (63 & 212)) & ((3 | 40) - 86))) != 149) failures++;
    }


    {
        g16 = 36891;
        if (read_g16() != 36891) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 92;
        if (buf[1] != 92) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 4: result = 168; break;
        case 11: result = 60; break;
        case 15: result = 213; break;
        default: result = 23; break;
        }
        if (result != 213) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 116;
        if (buf[5] != 116) failures++;
    }


    {
        uint16_t r = add2(157,41) + add2(41,148) + add2(157,148);
        if (r != 692) failures++;
    }


    {
        uint16_t r = add2(24,80) + add2(80,76) + add2(24,76);
        if (r != 360) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        if (((uint16_t)78) != 78) failures++;
    }


    {
        uint8_t x = 123;
        x <<= 1;
        if (x != 246) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 9: result = 30; break;
        case 1: result = 234; break;
        case 15: result = 196; break;
        case 4: result = 45; break;
        case 5: result = 208; break;
        default: result = 14; break;
        }
        if (result != 30) failures++;
    }


    {
        uint8_t x = 231;
        x <<= 1;
        if (x != 206) failures++;
    }


    {
        uint16_t x = 48903;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {221,1,198,13,219,221};
        if (a[4] != 219) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 2238594180UL;
        uint32_t b = 2918663505UL;
        uint32_t r = a & b;
        if (r != 2238055424UL) failures++;
    }


    {
        uint16_t x = 21542;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {19,216,175,171,108,81};
        if (a[5] != 81) failures++;
    }


    {
        volatile uint8_t port = 118;
        uint8_t r = port;
        if (r != 118) failures++;
    }


    {
        g16 = 55254;
        if (read_g16() != 55254) failures++;
    }


    {
        volatile uint8_t port = 42;
        uint8_t r = port;
        if (r != 42) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {31,116,40097,166};
        if (s.c != (uint16_t)40097) failures++;
    }


    {
        uint16_t r = add2(45,63) + add2(63,190) + add2(45,190);
        if (r != 596) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint16_t x = 227;
        x = x + 129;
        if (x != 356) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t v = 242;
        v ^= 1;
        if (v != 243) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 33524;
        if (read_g16() != 33524) failures++;
    }


    {
        uint16_t r = call6(134,158,41,183,187,124);
        if (r != 827) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-11) % (int16_t)((int8_t)-112);
        if ((uint16_t)r != (uint16_t)65525) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-6) / (int16_t)((int8_t)5);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t src[2] = {90,242};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 242) failures++;
    }


    {
        uint8_t v = 178;
        int r = (v & 4) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint16_t x = 4888;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 66;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-85) / (int16_t)((int8_t)17);
        if ((uint16_t)r != (uint16_t)65531) failures++;
    }


    {
        uint8_t v = 104;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 8) failures++;
    }


    {
        uint8_t m[2][2] = {{121,69},{24,164}};
        if (m[0][0] != 121) failures++;
    }


    {
        int8_t a = -11;
        int8_t b = -15;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 210;
        if (x != 408) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 17: result = 179; break;
        case 13: result = 245; break;
        case 1: result = 157; break;
        default: result = 151; break;
        }
        if (result != 179) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)13) / (int16_t)((int8_t)108);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 39;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        if (((uint16_t)94) != 94) failures++;
    }


    {
        uint16_t x = 243;
        x = x + 20;
        if (x != 263) failures++;
    }


    {
        uint8_t x = 199;
        x <<= 4;
        if (x != 112) failures++;
    }


    {
        g16 = 29310;
        if (read_g16() != 29310) failures++;
    }


    {
        g16 = 10637;
        if (read_g16() != 10637) failures++;
    }


    {
        uint8_t v = 152;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 224;
        if (buf[6] != 224) failures++;
    }


    {
        uint32_t a = 947917448UL;
        uint32_t b = 3787524569UL;
        uint32_t r = a ^ b;
        if (r != 3644922705UL) failures++;
    }


    {
        volatile int16_t a = -6785;
        volatile int16_t b = -31852;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 54;
        x = x + 62;
        if (x != 116) failures++;
    }


    {
        g16 = 39710;
        if (read_g16() != 39710) failures++;
    }


    {
        uint8_t v = 203;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t src[2] = {45,202};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 45) failures++;
    }


    {
        uint8_t x = 170;
        x <<= 2;
        if (x != 168) failures++;
    }


    {
        uint8_t buf[8] = {118,158,193,214,110,235,240,118};
        uint8_t *p = buf;
        p += 4;
        if (*p != 110) failures++;
    }


    {
        uint8_t src[10] = {168,227,219,60,33,237,57,223,148,255};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[5] != 237) failures++;
    }


    {
        g16 = 12430;
        if (read_g16() != 12430) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 4) sum += j;
        if (sum != 4) failures++;
    }


    {
        uint8_t m[3][2] = {{119,252},{122,138},{91,120}};
        if (m[2][1] != 120) failures++;
    }


    {
        volatile uint8_t port = 216;
        uint8_t r = port;
        if (r != 216) failures++;
    }


    {
        uint16_t x = 55366;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 28;
        x <<= 3;
        if (x != 224) failures++;
    }


    {
        uint16_t x = 88;
        x = x + 163;
        if (x != 251) failures++;
    }


    {
        g16 = 57146;
        if (read_g16() != 57146) failures++;
    }


    {
        uint8_t v = 40;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)42) + (uint16_t)18782;
        if (r != 18824) failures++;
    }


    {
        uint8_t v = 206;
        v ^= 32;
        if (v != 238) failures++;
    }


    {
        uint8_t input = 1;
        uint8_t result;
        switch (input) {
        case 9: result = 139; break;
        case 14: result = 211; break;
        case 3: result = 27; break;
        case 17: result = 129; break;
        case 1: result = 205; break;
        case 7: result = 96; break;
        case 18: result = 23; break;
        default: result = 43; break;
        }
        if (result != 205) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 212;
        if (buf[13] != 212) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 230;
        if (buf[10] != 230) failures++;
    }


    {
        uint16_t r = 15622 + 52437 + 34397 + 12493 + 47308 + 39220 + 20017 + 52790;
        if (r != 12140) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 19: result = 254; break;
        case 18: result = 115; break;
        case 3: result = 61; break;
        case 5: result = 190; break;
        case 4: result = 219; break;
        case 15: result = 133; break;
        default: result = 49; break;
        }
        if (result != 49) failures++;
    }


    {
        uint16_t r = call6(182,214,164,168,231,147);
        if (r != 1106) failures++;
    }


    {
        uint16_t x = 43718;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 192;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 2: result = 180; break;
        case 1: result = 247; break;
        case 4: result = 11; break;
        case 14: result = 14; break;
        case 7: result = 251; break;
        case 5: result = 161; break;
        case 6: result = 146; break;
        case 9: result = 235; break;
        default: result = 80; break;
        }
        if (result != 235) failures++;
    }


    {
        g16 = 59528;
        if (read_g16() != 59528) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {102,10,52315,217};
        if (s.c != (uint16_t)52315) failures++;
    }


    {
        if (((uint16_t)46) != 46) failures++;
    }


    {
        uint16_t r = 19810 + 11466 + 8456 + 7684 + 59449 + 44135 + 39403 + 40926;
        if (r != 34721) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 169;
        if (buf[5] != 169) failures++;
    }


    {
        uint8_t src[8] = {141,75,53,58,71,25,1,154};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[1] != 75) failures++;
    }


    {
        uint16_t x = 69;
        x = x + 13;
        if (x != 82) failures++;
    }


    {
        uint16_t r = add2(98,131) + add2(131,125) + add2(98,125);
        if (r != 708) failures++;
    }


    {
        volatile uint8_t port = 254;
        uint8_t r = port;
        if (r != 254) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint8_t src[6] = {181,1,67,177,42,10};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[0] != 181) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 5: result = 159; break;
        case 8: result = 251; break;
        case 6: result = 248; break;
        case 12: result = 102; break;
        case 11: result = 121; break;
        case 17: result = 149; break;
        default: result = 4; break;
        }
        if (result != 149) failures++;
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
        uint8_t m[3][4] = {{122,141,238,248},{131,208,176,111},{198,92,41,110}};
        if (m[1][3] != 111) failures++;
    }


    {
        uint32_t a = 4062204059UL;
        uint32_t b = 2555713191UL;
        uint32_t r = a | b;
        if (r != 4201993919UL) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 6: result = 30; break;
        case 10: result = 60; break;
        case 12: result = 120; break;
        case 15: result = 213; break;
        case 14: result = 243; break;
        default: result = 123; break;
        }
        if (result != 243) failures++;
    }


    {
        g16 = 54758;
        if (read_g16() != 54758) failures++;
    }


    {
        uint8_t src[14] = {65,112,164,144,107,69,15,73,99,102,206,35,8,243};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[0] != 65) failures++;
    }


    {
        uint16_t r = call6(154,194,120,31,245,122);
        if (r != 866) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 22117 + 42855 + 45224 + 19659 + 30551 + 29609 + 10624 + 30549;
        if (r != 34580) failures++;
    }


    {
        uint16_t r = add2(43,254) + add2(254,133) + add2(43,133);
        if (r != 860) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 6498 + 17070 + 3254 + 58685 + 19151 + 31150 + 50955 + 39924;
        if (r != 30079) failures++;
    }


    {
        uint8_t x = 34;
        x <<= 4;
        if (x != 32) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 235;
        if (buf[7] != 235) failures++;
    }


    {
        uint16_t x = 15276;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 24179;
        if (read_g16() != 24179) failures++;
    }


    {
        volatile int16_t a = -18869;
        volatile int16_t b = 2870;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        volatile int16_t a = 31473;
        volatile int16_t b = 28232;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 6444;
        volatile int16_t b = 14294;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(129,174) != 303) failures++;
    }


    {
        uint16_t r = call6(202,218,85,239,202,72);
        if (r != 1018) failures++;
    }


    {
        uint8_t src[5] = {166,1,142,186,240};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[3] != 186) failures++;
    }


    {
        g16 = 56757;
        if (read_g16() != 56757) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 19;
        do { cnt++; } while (--k);
        if (cnt != 19) failures++;
    }


    {
        uint32_t a = 1720125045UL;
        uint32_t b = 959961396UL;
        uint32_t r = a - b;
        if (r != 760163649UL) failures++;
    }


    {
        uint8_t v = 165;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t x = 36;
        x <<= 1;
        if (x != 72) failures++;
    }


    {
        uint8_t a[6] = {136,149,222,207,174,237};
        if (a[2] != 222) failures++;
    }


    {
        uint8_t v = 18;
        v ^= 16;
        if (v != 2) failures++;
    }


    {
        uint16_t r = call6(112,252,175,39,78,58);
        if (r != 714) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-74) % (int16_t)((int8_t)-31);
        if ((uint16_t)r != (uint16_t)65524) failures++;
    }


    {
        volatile uint8_t port = 0;
        uint8_t r = port;
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {218,208,212,101,240,56};
        if (a[0] != 218) failures++;
    }


    {
        uint16_t r = add2(66,78) + add2(78,50) + add2(66,50);
        if (r != 388) failures++;
    }


    {
        if (((uint16_t)((56 & (233 | 9)) | ((223 + 8) ^ (188 + 145)))) != 426) failures++;
    }


    {
        if (((uint16_t)(((133 + 16) - 124) - 185)) != 65376) failures++;
    }


    {
        volatile uint8_t port = 110;
        uint8_t r = port;
        if (r != 110) failures++;
    }


    {
        int8_t a = -87;
        int8_t b = 125;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(23,91) + add2(91,221) + add2(23,221);
        if (r != 670) failures++;
    }


    {
        if (((uint16_t)(((13 & 233) ^ (142 & 49)) & (247 ^ 225))) != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        g16 = 15527;
        if (read_g16() != 15527) failures++;
    }


    {
        uint8_t a[6] = {189,198,247,166,77,163};
        if (a[4] != 77) failures++;
    }


    {
        uint8_t buf[8] = {172,74,111,64,175,159,236,3};
        uint8_t *p = buf;
        p += 2;
        if (*p != 111) failures++;
    }


    {
        volatile uint8_t port = 61;
        uint8_t r = port;
        if (r != 61) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {90,148,62054,27};
        if (s.d != (uint8_t)27) failures++;
    }


    {
        uint8_t v = 131;
        v &= ~(uint8_t)16;
        if (v != 131) failures++;
    }


    {
        uint32_t a = 326299108UL;
        uint32_t b = 1469281392UL;
        uint32_t r = a & b;
        if (r != 319972448UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint8_t v = 121;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 25;
        do { cnt++; } while (--k);
        if (cnt != 25) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)123) % (int16_t)((int8_t)13);
        if ((uint16_t)r != (uint16_t)6) failures++;
    }


    {
        uint8_t x = 140;
        x <<= 1;
        if (x != 24) failures++;
    }


    {
        uint16_t r = 43617 + 18493 + 40322 + 35619 + 57514 + 35481 + 44753 + 6384;
        if (r != 20039) failures++;
    }


    {
        uint8_t v = 57;
        v |= 16;
        if (v != 57) failures++;
    }


    {
        uint8_t buf[8] = {131,129,158,11,149,230,203,65};
        uint8_t *p = buf;
        p += 5;
        if (*p != 230) failures++;
    }


    {
        uint8_t m[2][3] = {{210,86,208},{118,30,209}};
        if (m[0][2] != 208) failures++;
    }


    {
        uint16_t x = 241;
        x = x + 151;
        if (x != 392) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(185,10) != 175) failures++;
    }


    {
        uint16_t r = add2(224,111) + add2(111,142) + add2(224,142);
        if (r != 954) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)167) + (uint16_t)4718;
        if (r != 4885) failures++;
    }


    {
        uint16_t r = call6(149,182,172,29,58,236);
        if (r != 826) failures++;
    }


    {
        uint16_t x = 233;
        x = x + 41;
        if (x != 274) failures++;
    }


    {
        uint32_t a = 3770843314UL;
        uint32_t b = 967651211UL;
        uint32_t r = a & b;
        if (r != 545259650UL) failures++;
    }


    {
        volatile int16_t a = -26930;
        volatile int16_t b = 24849;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = -11223;
        volatile int16_t b = -30884;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 48449;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-118) / (int16_t)((int8_t)-9);
        if ((uint16_t)r != (uint16_t)13) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(101,88) != 13) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(184 & ((234 | 72) - (171 & 104)))) != 128) failures++;
    }


    {
        volatile uint8_t port = 72;
        uint8_t r = port;
        if (r != 72) failures++;
    }


    {
        uint8_t src[11] = {207,79,18,194,179,101,28,87,217,181,116};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[4] != 179) failures++;
    }


    {
        g16 = 42818;
        if (read_g16() != 42818) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[3][4] = {{126,17,150,115},{47,220,3,229},{9,115,4,229}};
        if (m[2][2] != 4) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-101) % (int16_t)((int8_t)-84);
        if ((uint16_t)r != (uint16_t)65519) failures++;
    }


    {
        uint8_t m[2][2] = {{193,232},{26,53}};
        if (m[1][0] != 26) failures++;
    }


    {
        uint16_t x = 26799;
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
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t src[11] = {237,4,65,126,130,143,50,167,109,155,186};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[2] != 65) failures++;
    }


    {
        uint16_t r = call6(93,110,228,153,85,252);
        if (r != 921) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 54;
        if (buf[5] != 54) failures++;
    }


    {
        uint16_t x = 47508;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)47) + (uint16_t)62702;
        if (r != 62749) failures++;
    }


    {
        uint8_t v = 106;
        int r = (v & 2) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {17,25,63009,124};
        if (s.c != (uint16_t)63009) failures++;
    }


    {
        g16 = 51997;
        if (read_g16() != 51997) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)65) + (uint16_t)15305;
        if (r != 15370) failures++;
    }


    {
        uint8_t m[4][3] = {{28,10,34},{252,71,62},{115,139,133},{123,144,114}};
        if (m[3][1] != 144) failures++;
    }


    {
        uint8_t m[3][4] = {{232,98,125,56},{188,212,42,161},{123,236,219,227}};
        if (m[1][1] != 212) failures++;
    }


    {
        g16 = 38852;
        if (read_g16() != 38852) failures++;
    }


    {
        volatile uint8_t port = 125;
        uint8_t r = port;
        if (r != 125) failures++;
    }


    {
        uint8_t v = 53;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {112,238,43741,25};
        if (s.d != (uint8_t)25) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 1: result = 140; break;
        case 16: result = 156; break;
        case 4: result = 236; break;
        case 14: result = 3; break;
        case 9: result = 185; break;
        case 6: result = 25; break;
        default: result = 250; break;
        }
        if (result != 25) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(167,233) != 65470) failures++;
    }


    {
        volatile uint8_t port = 9;
        uint8_t r = port;
        if (r != 9) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)58) % (int16_t)((int8_t)35);
        if ((uint16_t)r != (uint16_t)23) failures++;
    }


    {
        uint16_t r = call6(192,221,212,184,36,63);
        if (r != 908) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)224) + (uint16_t)14141;
        if (r != 14365) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 3) sum += j;
        if (sum != 45) failures++;
    }


    {
        g16 = 44712;
        if (read_g16() != 44712) failures++;
    }


    {
        uint16_t r = call6(174,2,157,66,35,117);
        if (r != 551) failures++;
    }


    {
        uint8_t x = 151;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(76,79) != 155) failures++;
    }


    {
        uint8_t buf[9];
        for (uint8_t j = 0; j < 9; j++) buf[j] = 154;
        if (buf[8] != 154) failures++;
    }


    {
        uint8_t x = 107;
        x <<= 1;
        if (x != 214) failures++;
    }


    {
        uint16_t x = 23439;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 13257;
        if (read_g16() != 13257) failures++;
    }


    {
        uint16_t r = 44325 + 55149 + 27354 + 48565 + 61534 + 12969 + 12880 + 8339;
        if (r != 8971) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 69;
        x <<= 0;
        if (x != 69) failures++;
    }


    {
        uint8_t v = 132;
        v |= 1;
        if (v != 133) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 56;
        if (buf[9] != 56) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(136,139) != 65533) failures++;
    }


    {
        uint16_t r = 63873 + 48197 + 21411 + 60918 + 37539 + 61626 + 45335 + 61949;
        if (r != 7632) failures++;
    }


    {
        uint16_t r = 15244 + 46622 + 17609 + 25444 + 46993 + 20477 + 29652 + 12473;
        if (r != 17906) failures++;
    }


    {
        volatile uint8_t port = 141;
        uint8_t r = port;
        if (r != 141) failures++;
    }


    {
        volatile int16_t a = -31175;
        volatile int16_t b = 16634;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 203;
        uint8_t r = port;
        if (r != 203) failures++;
    }


    {
        uint8_t m[2][3] = {{64,99,1},{193,88,143}};
        if (m[0][0] != 64) failures++;
    }


    {
        if (((uint16_t)(130 - ((216 + 164) | (151 | 131)))) != 65155) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)69) % (int16_t)((int8_t)127);
        if ((uint16_t)r != (uint16_t)69) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        g16 = 14958;
        if (read_g16() != 14958) failures++;
    }


    {
        uint8_t buf[8] = {145,69,250,240,227,16,146,126};
        uint8_t *p = buf;
        p += 5;
        if (*p != 16) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(222,49) != 173) failures++;
    }


    {
        uint8_t v = 199;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint8_t v = 13;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = call6(66,100,216,143,3,148);
        if (r != 676) failures++;
    }


    {
        volatile int16_t a = -13148;
        volatile int16_t b = -4006;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 78;
        x = x + 80;
        if (x != 158) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 1) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint8_t a[6] = {18,226,213,150,119,134};
        if (a[5] != 134) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {163,63,4265,17};
        if (s.a != (uint8_t)163) failures++;
    }


    {
        uint16_t r = call6(190,65,4,183,108,51);
        if (r != 601) failures++;
    }


    {
        int8_t a = -24;
        int8_t b = -3;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 25;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(222,254) != 476) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {53,136,4486,144};
        if (s.a != (uint8_t)53) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(134,37) != 171) failures++;
    }


    {
        if (((uint16_t)(128 & ((45 & 110) | (116 | 242)))) != 128) failures++;
    }


    {
        g16 = 9399;
        if (read_g16() != 9399) failures++;
    }


    {
        uint8_t v = 166;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t r = 30172 + 28997 + 35824 + 10210 + 6403 + 6537 + 8334 + 44630;
        if (r != 40035) failures++;
    }


    {
        uint8_t v = 189;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 3537972624UL;
        uint32_t b = 3516961657UL;
        uint32_t r = a | b;
        if (r != 3554787321UL) failures++;
    }


    {
        uint8_t buf[8] = {105,234,181,156,246,176,107,174};
        uint8_t *p = buf;
        p += 1;
        if (*p != 234) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(199,245) != 65490) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)135) + (uint16_t)48056;
        if (r != 48191) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)128) + (uint16_t)328;
        if (r != 456) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 13: result = 29; break;
        case 2: result = 54; break;
        case 16: result = 233; break;
        case 8: result = 129; break;
        default: result = 164; break;
        }
        if (result != 54) failures++;
    }


    {
        uint8_t x = 65;
        x <<= 4;
        if (x != 16) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(135,146) != 281) failures++;
    }


    {
        uint8_t v = 52;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
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
        int16_t r = (int16_t)((int8_t)-99) / (int16_t)((int8_t)1);
        if ((uint16_t)r != (uint16_t)65437) failures++;
    }


    {
        uint8_t v = 226;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 115;
        x = x + 112;
        if (x != 227) failures++;
    }


    {
        g16 = 42342;
        if (read_g16() != 42342) failures++;
    }


    {
        uint16_t x = 12125;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 1) sum += j;
        if (sum != 66) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(((143 & 198) & (199 | 239)) - ((134 | 254) & 68))) != 66) failures++;
    }


    {
        uint8_t v = 176;
        int r = (v & 4) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 68;
        uint8_t r = port;
        if (r != 68) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-84) % (int16_t)((int8_t)61);
        if ((uint16_t)r != (uint16_t)65513) failures++;
    }


    {
        uint8_t v = 48;
        int r = (v & 64) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 248;
        v |= 32;
        if (v != 248) failures++;
    }


    {
        uint16_t r = call6(29,118,243,163,225,32);
        if (r != 810) failures++;
    }


    {
        uint16_t x = 92;
        x = x + 0;
        if (x != 92) failures++;
    }


    {
        uint8_t v = 35;
        v |= 32;
        if (v != 35) failures++;
    }


    {
        uint32_t a = 3374677600UL;
        uint32_t b = 2472682359UL;
        uint32_t r = a + b;
        if (r != 1552392663UL) failures++;
    }


    {
        int8_t a = -108;
        int8_t b = -98;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][2] = {{49,254},{133,213},{68,241},{163,139}};
        if (m[3][0] != 163) failures++;
    }


    {
        g16 = 30077;
        if (read_g16() != 30077) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 99;
        if (buf[7] != 99) failures++;
    }


    {
        uint8_t input = 0;
        uint8_t result;
        switch (input) {
        case 0: result = 196; break;
        case 17: result = 101; break;
        case 7: result = 209; break;
        case 1: result = 171; break;
        default: result = 234; break;
        }
        if (result != 196) failures++;
    }


    {
        uint16_t r = call6(253,193,152,167,102,148);
        if (r != 1015) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint8_t x = 9;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        uint8_t a[6] = {13,105,132,145,223,246};
        if (a[0] != 13) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 8: result = 122; break;
        case 17: result = 96; break;
        case 6: result = 244; break;
        case 10: result = 109; break;
        case 12: result = 212; break;
        case 15: result = 254; break;
        case 7: result = 158; break;
        case 4: result = 205; break;
        default: result = 23; break;
        }
        if (result != 212) failures++;
    }


    {
        uint16_t r = add2(202,95) + add2(95,198) + add2(202,198);
        if (r != 990) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint16_t r = call6(235,182,24,150,254,158);
        if (r != 1003) failures++;
    }


    {
        uint8_t m[3][2] = {{205,41},{50,204},{59,132}};
        if (m[0][0] != 205) failures++;
    }


    {
        uint16_t x = 63910;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 1159375942UL;
        uint32_t b = 292111240UL;
        uint32_t r = a + b;
        if (r != 1451487182UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)23) + (uint16_t)55524;
        if (r != 55547) failures++;
    }


    {
        uint16_t x = 55622;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(127,202) + add2(202,35) + add2(127,35);
        if (r != 728) failures++;
    }


    {
        if (((uint16_t)(((4 & 60) | (220 ^ 31)) + (127 | 225))) != 454) failures++;
    }


    {
        uint16_t r = call6(50,97,130,63,124,8);
        if (r != 472) failures++;
    }


    {
        uint16_t r = add2(111,221) + add2(221,53) + add2(111,53);
        if (r != 770) failures++;
    }


    {
        volatile uint8_t port = 153;
        uint8_t r = port;
        if (r != 153) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)37) + (uint16_t)39200;
        if (r != 39237) failures++;
    }


    {
        uint16_t x = 170;
        x = x + 122;
        if (x != 292) failures++;
    }


    {
        volatile uint8_t port = 155;
        uint8_t r = port;
        if (r != 155) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)186) + (uint16_t)49651;
        if (r != 49837) failures++;
    }


    {
        uint16_t r = add2(69,60) + add2(60,125) + add2(69,125);
        if (r != 508) failures++;
    }


    {
        volatile uint8_t port = 26;
        uint8_t r = port;
        if (r != 26) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 20;
        if (buf[10] != 20) failures++;
    }


    {
        uint32_t a = 3248026787UL;
        uint32_t b = 3983290931UL;
        uint32_t r = a - b;
        if (r != 3559703152UL) failures++;
    }


    {
        volatile uint8_t port = 63;
        uint8_t r = port;
        if (r != 63) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)103) / (int16_t)((int8_t)37);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        volatile int16_t a = 16323;
        volatile int16_t b = -20112;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(172,62) != 234) failures++;
    }


    {
        uint8_t src[15] = {87,102,235,89,199,231,222,224,158,54,12,10,162,17,46};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[7] != 224) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 22;
        if (buf[15] != 22) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)32) + (uint16_t)59377;
        if (r != 59409) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(149,88) + add2(88,0) + add2(149,0);
        if (r != 474) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 247;
        if (buf[1] != 247) failures++;
    }


    {
        uint8_t buf[8] = {30,168,175,183,199,33,21,222};
        uint8_t *p = buf;
        p += 7;
        if (*p != 222) failures++;
    }


    {
        uint8_t src[5] = {226,186,31,235,107};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 226) failures++;
    }


    {
        uint8_t v = 193;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = call6(213,141,138,209,69,136);
        if (r != 906) failures++;
    }


    {
        uint8_t buf[8] = {130,229,182,15,81,22,96,154};
        uint8_t *p = buf;
        p += 3;
        if (*p != 15) failures++;
    }


    {
        uint16_t x = 42;
        x = x + 62;
        if (x != 104) failures++;
    }


    {
        uint16_t x = 114;
        x = x + 46;
        if (x != 160) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 1) sum += j;
        if (sum != 66) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)80) % (int16_t)((int8_t)69);
        if ((uint16_t)r != (uint16_t)11) failures++;
    }


    {
        uint16_t x = 49685;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {104,47,77,249,99,128,11,9};
        uint8_t *p = buf;
        p += 3;
        if (*p != 249) failures++;
    }


    {
        uint16_t x = 55810;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(160,77,172,163,115,90);
        if (r != 777) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {149,94,44490,199};
        if (s.a != (uint8_t)149) failures++;
    }


    {
        uint16_t r = add2(190,167) + add2(167,188) + add2(190,188);
        if (r != 1090) failures++;
    }


    {
        if (((uint16_t)((200 ^ 46) - 116)) != 114) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 214;
        if (buf[5] != 214) failures++;
    }


    {
        uint8_t a[6] = {218,185,138,249,205,214};
        if (a[1] != 185) failures++;
    }


    {
        uint8_t x = 234;
        x <<= 4;
        if (x != 160) failures++;
    }


    {
        uint8_t v = 73;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        volatile uint8_t port = 150;
        uint8_t r = port;
        if (r != 150) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 14406 + 1057 + 39365 + 42141 + 30244 + 44821 + 10922 + 64003;
        if (r != 50351) failures++;
    }


    {
        uint8_t src[14] = {219,202,45,102,59,98,75,249,111,170,208,135,29,48};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[10] != 208) failures++;
    }


    {
        uint16_t r = 30429 + 3841 + 44990 + 5366 + 2370 + 65493 + 46167 + 33330;
        if (r != 35378) failures++;
    }


    {
        uint8_t v = 157;
        v ^= 16;
        if (v != 141) failures++;
    }


    {
        uint8_t x = 251;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 2) sum += j;
        if (sum != 30) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-40) / (int16_t)((int8_t)-121);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint32_t a = 1905544506UL;
        uint32_t b = 416921153UL;
        uint32_t r = a ^ b;
        if (r != 1766718331UL) failures++;
    }


    {
        volatile uint8_t port = 197;
        uint8_t r = port;
        if (r != 197) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 19: result = 214; break;
        case 7: result = 86; break;
        case 4: result = 102; break;
        case 10: result = 223; break;
        case 15: result = 7; break;
        case 11: result = 126; break;
        case 12: result = 40; break;
        default: result = 159; break;
        }
        if (result != 223) failures++;
    }


    {
        uint8_t v = 124;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t x = 59455;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(218,213) + add2(213,190) + add2(218,190);
        if (r != 1242) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)115) + (uint16_t)8249;
        if (r != 8364) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-79) % (int16_t)((int8_t)-116);
        if ((uint16_t)r != (uint16_t)65457) failures++;
    }


    {
        g16 = 34775;
        if (read_g16() != 34775) failures++;
    }


    {
        uint32_t a = 624815088UL;
        uint32_t b = 920881165UL;
        uint32_t r = a + b;
        if (r != 1545696253UL) failures++;
    }


    {
        uint32_t a = 2782969753UL;
        uint32_t b = 318602870UL;
        uint32_t r = a + b;
        if (r != 3101572623UL) failures++;
    }


    {
        volatile int16_t a = -1757;
        volatile int16_t b = 26706;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t v = 253;
        v ^= 8;
        if (v != 245) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)244) + (uint16_t)59687;
        if (r != 59931) failures++;
    }


    {
        uint8_t v = 113;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {209,152,33638,62};
        if (s.d != (uint8_t)62) failures++;
    }


    {
        uint16_t x = 34475;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        if (((uint16_t)18) != 18) failures++;
    }


    {
        int8_t a = -20;
        int8_t b = -91;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(53,98,123,164,94,228);
        if (r != 760) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint8_t v = 107;
        int r = (v & 32) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(162,10) != 172) failures++;
    }


    {
        uint8_t v = 196;
        v ^= 2;
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
        volatile uint8_t port = 116;
        uint8_t r = port;
        if (r != 116) failures++;
    }


    {
        uint16_t r = add2(60,133) + add2(133,244) + add2(60,244);
        if (r != 874) failures++;
    }


    {
        int8_t a = 105;
        int8_t b = -85;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)84) + (uint16_t)28680;
        if (r != 28764) failures++;
    }


    {
        volatile int16_t a = 3207;
        volatile int16_t b = 11772;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 4: result = 190; break;
        case 17: result = 17; break;
        case 0: result = 217; break;
        case 15: result = 51; break;
        default: result = 227; break;
        }
        if (result != 51) failures++;
    }


    {
        uint16_t r = call6(66,143,156,110,119,218);
        if (r != 812) failures++;
    }


    {
        uint8_t v = 160;
        v ^= 128;
        if (v != 32) failures++;
    }


    {
        uint8_t m[3][3] = {{122,99,186},{91,214,246},{88,146,172}};
        if (m[2][0] != 88) failures++;
    }


    {
        int8_t a = 102;
        int8_t b = -22;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 35;
        uint8_t r = port;
        if (r != 35) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(192,58) != 134) failures++;
    }


    {
        volatile int16_t a = -24380;
        volatile int16_t b = 12696;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 216;
        x = x + 219;
        if (x != 435) failures++;
    }


    {
        uint8_t src[6] = {33,14,68,9,78,104};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[1] != 14) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 78;
        if (buf[11] != 78) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(153,77) != 76) failures++;
    }


    {
        volatile int16_t a = 26363;
        volatile int16_t b = -14063;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t buf[8] = {150,24,115,33,72,59,109,126};
        uint8_t *p = buf;
        p += 2;
        if (*p != 115) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 3: result = 133; break;
        case 12: result = 208; break;
        case 16: result = 189; break;
        case 2: result = 200; break;
        case 7: result = 180; break;
        case 11: result = 95; break;
        default: result = 120; break;
        }
        if (result != 95) failures++;
    }


    {
        uint32_t a = 95413026UL;
        uint32_t b = 3682170403UL;
        uint32_t r = a - b;
        if (r != 708209919UL) failures++;
    }


    {
        volatile uint8_t port = 167;
        uint8_t r = port;
        if (r != 167) failures++;
    }


    {
        volatile int16_t a = 24137;
        volatile int16_t b = 18695;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[3][2] = {{151,23},{201,226},{196,91}};
        if (m[0][1] != 23) failures++;
    }


    {
        if (((uint16_t)((12 | (91 + 102)) | ((185 + 70) + (83 ^ 88)))) != 463) failures++;
    }


    {
        uint8_t buf[8] = {83,40,216,186,204,220,15,13};
        uint8_t *p = buf;
        p += 1;
        if (*p != 40) failures++;
    }


    {
        uint16_t x = 2965;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(163,84) + add2(84,117) + add2(163,117);
        if (r != 728) failures++;
    }


    {
        uint16_t r = call6(234,157,163,20,201,43);
        if (r != 818) failures++;
    }


    {
        if (((uint16_t)(((133 | 190) - 160) + ((236 ^ 58) - (210 | 97)))) != 2) failures++;
    }


    {
        volatile uint8_t port = 216;
        uint8_t r = port;
        if (r != 216) failures++;
    }


    {
        uint8_t v = 35;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 127;
        uint8_t r = port;
        if (r != 127) failures++;
    }


    {
        uint8_t v = 110;
        v &= ~(uint8_t)64;
        if (v != 46) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint32_t a = 2135387002UL;
        uint32_t b = 309320636UL;
        uint32_t r = a ^ b;
        if (r != 1831383238UL) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)31) + (uint16_t)31508;
        if (r != 31539) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)84) % (int16_t)((int8_t)-92);
        if ((uint16_t)r != (uint16_t)84) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = 66;
        int8_t b = -52;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = -23116;
        volatile int16_t b = -29001;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)125) % (int16_t)((int8_t)15);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        uint8_t m[2][4] = {{52,176,159,80},{123,143,221,136}};
        if (m[0][0] != 52) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 139;
        if (buf[1] != 139) failures++;
    }


    {
        uint8_t buf[8] = {226,171,164,10,123,196,94,195};
        uint8_t *p = buf;
        p += 5;
        if (*p != 196) failures++;
    }


    {
        uint8_t a[6] = {176,92,245,214,28,218};
        if (a[4] != 28) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 7: result = 211; break;
        case 19: result = 212; break;
        case 4: result = 23; break;
        case 14: result = 5; break;
        default: result = 122; break;
        }
        if (result != 5) failures++;
    }


    {
        uint16_t r = 29718 + 59046 + 34176 + 47145 + 14626 + 12848 + 21942 + 32516;
        if (r != 55409) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)95) % (int16_t)((int8_t)-124);
        if ((uint16_t)r != (uint16_t)95) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)139) + (uint16_t)42;
        if (r != 181) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {235,188,29271,21};
        if (s.d != (uint8_t)21) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 213;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(98,60) != 38) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t v = 123;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {42,168,64745,151};
        if (s.c != (uint16_t)64745) failures++;
    }


    {
        uint16_t r = add2(153,70) + add2(70,163) + add2(153,163);
        if (r != 772) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t v = 209;
        int r = (v & 16) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)211) != 211) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {27,229,19728,43};
        if (s.b != (uint8_t)229) failures++;
    }


    {
        uint16_t x = 18881;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 41228;
        if (read_g16() != 41228) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {90,99,29355,138};
        if (s.a != (uint8_t)90) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(218,44) != 262) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[2][3] = {{86,243,249},{138,224,134}};
        if (m[0][2] != 249) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 237;
        if (buf[9] != 237) failures++;
    }


    {
        volatile uint8_t port = 33;
        uint8_t r = port;
        if (r != 33) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 15: result = 150; break;
        case 17: result = 191; break;
        case 14: result = 238; break;
        case 12: result = 134; break;
        default: result = 88; break;
        }
        if (result != 238) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(196,131) != 327) failures++;
    }


    {
        uint8_t v = 161;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 15) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {118,76,5742,1};
        if (s.a != (uint8_t)118) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 4: result = 215; break;
        case 7: result = 0; break;
        case 15: result = 132; break;
        case 19: result = 135; break;
        case 13: result = 255; break;
        case 10: result = 96; break;
        default: result = 118; break;
        }
        if (result != 132) failures++;
    }


    {
        uint8_t a[6] = {58,148,253,28,153,116};
        if (a[3] != 28) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)98) + (uint16_t)17732;
        if (r != 17830) failures++;
    }


    {
        uint32_t a = 648831006UL;
        uint32_t b = 1081020392UL;
        uint32_t r = a & b;
        if (r != 2883592UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(48,76) != 124) failures++;
    }


    {
        g16 = 51957;
        if (read_g16() != 51957) failures++;
    }


    {
        uint16_t r = add2(51,24) + add2(24,36) + add2(51,36);
        if (r != 222) failures++;
    }


    {
        int8_t a = 60;
        int8_t b = -118;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(186,185,241,231,185,162);
        if (r != 1190) failures++;
    }


    {
        uint32_t a = 3792660776UL;
        uint32_t b = 2138653158UL;
        uint32_t r = a - b;
        if (r != 1654007618UL) failures++;
    }


    {
        uint8_t buf[6];
        for (uint8_t j = 0; j < 6; j++) buf[j] = 180;
        if (buf[5] != 180) failures++;
    }


    {
        uint16_t x = 2884;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint32_t a = 555401877UL;
        uint32_t b = 3744921007UL;
        uint32_t r = a & b;
        if (r != 18006149UL) failures++;
    }


    {
        volatile uint8_t port = 7;
        uint8_t r = port;
        if (r != 7) failures++;
    }


    {
        uint16_t x = 22644;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)253) + (uint16_t)1381;
        if (r != 1634) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 18: result = 93; break;
        case 14: result = 49; break;
        case 15: result = 193; break;
        case 2: result = 244; break;
        case 4: result = 175; break;
        case 0: result = 98; break;
        case 5: result = 248; break;
        case 11: result = 42; break;
        default: result = 21; break;
        }
        if (result != 21) failures++;
    }


    {
        uint32_t a = 2356669970UL;
        uint32_t b = 2930293059UL;
        uint32_t r = a - b;
        if (r != 3721344207UL) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 5;
        if (buf[12] != 5) failures++;
    }


    {
        uint8_t m[2][2] = {{68,61},{176,163}};
        if (m[0][0] != 68) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 2: result = 152; break;
        case 3: result = 245; break;
        case 0: result = 117; break;
        case 12: result = 6; break;
        default: result = 205; break;
        }
        if (result != 205) failures++;
    }


    {
        if (((uint16_t)(164 + ((163 - 173) - 107))) != 47) failures++;
    }


    {
        int8_t a = -76;
        int8_t b = 22;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)74) / (int16_t)((int8_t)-106);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t buf[8] = {166,216,93,97,254,67,217,147};
        uint8_t *p = buf;
        p += 3;
        if (*p != 97) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)198) + (uint16_t)31098;
        if (r != 31296) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(46,88) != 134) failures++;
    }


    {
        uint16_t x = 205;
        x = x + 14;
        if (x != 219) failures++;
    }


    {
        volatile int16_t a = -6992;
        volatile int16_t b = 22853;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 35908;
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
        uint8_t x = 20;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint16_t r = add2(58,150) + add2(150,179) + add2(58,179);
        if (r != 774) failures++;
    }


    {
        uint16_t r = call6(199,160,30,179,235,44);
        if (r != 847) failures++;
    }


    {
        uint16_t r = 41834 + 64257 + 49969 + 20511 + 64472 + 43679 + 61464 + 18471;
        if (r != 36977) failures++;
    }


    {
        volatile uint8_t port = 241;
        uint8_t r = port;
        if (r != 241) failures++;
    }


    {
        uint16_t x = 20711;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {181,40,49157,166};
        if (s.c != (uint16_t)49157) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(198,131) != 67) failures++;
    }


    {
        uint8_t v = 162;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = 40622 + 33059 + 24146 + 10361 + 34688 + 32764 + 14068 + 14654;
        if (r != 7754) failures++;
    }


    {
        g16 = 10452;
        if (read_g16() != 10452) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(45,4) != 41) failures++;
    }


    {
        uint8_t v = 71;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 25) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 151;
        if (x != 349) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)60) + (uint16_t)58337;
        if (r != 58397) failures++;
    }


    {
        uint16_t r = call6(100,126,238,157,91,12);
        if (r != 724) failures++;
    }


    {
        if (((uint16_t)155) != 155) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)16) / (int16_t)((int8_t)-91);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        if (((uint16_t)252) != 252) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)142) + (uint16_t)57988;
        if (r != 58130) failures++;
    }


    {
        uint8_t v = 104;
        v |= 8;
        if (v != 104) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)239) + (uint16_t)9071;
        if (r != 9310) failures++;
    }


    {
        volatile uint8_t port = 128;
        uint8_t r = port;
        if (r != 128) failures++;
    }


    {
        uint8_t src[11] = {148,108,120,215,67,215,30,165,22,95,20};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[10] != 20) failures++;
    }


    {
        uint8_t x = 84;
        x <<= 0;
        if (x != 84) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 19: result = 147; break;
        case 13: result = 97; break;
        case 10: result = 71; break;
        case 16: result = 251; break;
        case 1: result = 37; break;
        case 0: result = 10; break;
        default: result = 111; break;
        }
        if (result != 147) failures++;
    }


    {
        uint32_t a = 2987761175UL;
        uint32_t b = 1365586785UL;
        uint32_t r = a & b;
        if (r != 268767745UL) failures++;
    }


    {
        uint8_t buf[8] = {44,224,246,186,12,22,201,125};
        uint8_t *p = buf;
        p += 7;
        if (*p != 125) failures++;
    }


    {
        uint8_t src[4] = {226,55,201,184};
        uint8_t dst[4];
        for (uint8_t j = 0; j < 4; j++) dst[j] = src[j];
        if (dst[0] != 226) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 195;
        x = x + 145;
        if (x != 340) failures++;
    }


    {
        uint8_t m[4][3] = {{18,157,25},{159,135,167},{42,57,180},{71,66,120}};
        if (m[3][1] != 66) failures++;
    }


    {
        uint8_t v = 137;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 55) failures++;
    }


    {
        uint8_t x = 216;
        x <<= 5;
        if (x != 0) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 67;
        if (buf[2] != 67) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)2) % (int16_t)((int8_t)-123);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint8_t v = 242;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t x = 18;
        x <<= 5;
        if (x != 64) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(45,31) != 14) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)248) + (uint16_t)14033;
        if (r != 14281) failures++;
    }


    {
        uint8_t a[6] = {255,45,157,255,150,196};
        if (a[5] != 196) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 1) sum += j;
        if (sum != 153) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 16: result = 138; break;
        case 13: result = 78; break;
        case 7: result = 113; break;
        default: result = 152; break;
        }
        if (result != 152) failures++;
    }


    {
        uint16_t r = add2(40,13) + add2(13,254) + add2(40,254);
        if (r != 614) failures++;
    }


    {
        uint8_t buf[8] = {147,122,157,235,31,204,69,19};
        uint8_t *p = buf;
        p += 2;
        if (*p != 157) failures++;
    }


    {
        uint16_t x = 24885;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 5503;
        volatile int16_t b = 4236;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 151;
        if (buf[7] != 151) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-34) / (int16_t)((int8_t)49);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {87,37,45554,231};
        if (s.c != (uint16_t)45554) failures++;
    }


    {
        uint8_t a[6] = {49,168,4,125,15,154};
        if (a[4] != 15) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {131,213,18524,173};
        if (s.a != (uint8_t)131) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 152;
        if (buf[7] != 152) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {121,214,21914,38};
        if (s.b != (uint8_t)214) failures++;
    }


    {
        uint8_t buf[8] = {42,222,132,159,12,29,163,189};
        uint8_t *p = buf;
        p += 6;
        if (*p != 163) failures++;
    }


    {
        uint32_t a = 4062151803UL;
        uint32_t b = 121116649UL;
        uint32_t r = a + b;
        if (r != 4183268452UL) failures++;
    }


    {
        uint8_t v = 196;
        v ^= 8;
        if (v != 204) failures++;
    }


    {
        volatile int16_t a = -20494;
        volatile int16_t b = 16956;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 9: result = 14; break;
        case 10: result = 244; break;
        case 5: result = 93; break;
        case 13: result = 132; break;
        default: result = 65; break;
        }
        if (result != 244) failures++;
    }


    {
        uint8_t v = 79;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {207,9,5411,70};
        if (s.a != (uint8_t)207) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)139) + (uint16_t)41343;
        if (r != 41482) failures++;
    }


    {
        uint8_t v = 150;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 227;
        x = x + 127;
        if (x != 354) failures++;
    }


    {
        uint8_t v = 215;
        v &= ~(uint8_t)128;
        if (v != 87) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 165;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 51526;
        if (read_g16() != 51526) failures++;
    }


    {
        uint16_t r = call6(24,76,208,131,86,197);
        if (r != 722) failures++;
    }


    {
        volatile int16_t a = 32288;
        volatile int16_t b = 5095;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {85,228,187,232,185,242,47,146};
        uint8_t *p = buf;
        p += 3;
        if (*p != 232) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-92) / (int16_t)((int8_t)54);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint8_t v = 197;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t m[2][2] = {{131,67},{3,167}};
        if (m[0][1] != 67) failures++;
    }


    {
        uint16_t x = 44;
        x = x + 248;
        if (x != 292) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 8: result = 12; break;
        case 6: result = 123; break;
        case 17: result = 210; break;
        default: result = 24; break;
        }
        if (result != 210) failures++;
    }


    {
        volatile uint8_t port = 88;
        uint8_t r = port;
        if (r != 88) failures++;
    }


    {
        uint8_t x = 8;
        x <<= 0;
        if (x != 8) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(161,196) != 65501) failures++;
    }


    {
        uint8_t src[6] = {57,25,241,7,89,85};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[1] != 25) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-22) % (int16_t)((int8_t)17);
        if ((uint16_t)r != (uint16_t)65531) failures++;
    }


    {
        uint16_t x = 104;
        x = x + 14;
        if (x != 118) failures++;
    }


    {
        uint32_t a = 1327149814UL;
        uint32_t b = 3383400157UL;
        uint32_t r = a + b;
        if (r != 415582675UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        g16 = 41610;
        if (read_g16() != 41610) failures++;
    }


    {
        uint16_t r = add2(138,159) + add2(159,227) + add2(138,227);
        if (r != 1048) failures++;
    }


    {
        g16 = 37765;
        if (read_g16() != 37765) failures++;
    }


    {
        uint16_t x = 49562;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }

    return failures;
}