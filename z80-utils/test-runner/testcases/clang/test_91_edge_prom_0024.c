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
        uint8_t src[16] = {83,1,238,203,255,22,58,129,55,9,137,130,130,92,248,48};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[5] != 22) failures++;
    }


    {
        g16 = 48503;
        if (read_g16() != 48503) failures++;
    }


    {
        uint16_t x = 75;
        x = x + 61;
        if (x != 136) failures++;
    }


    {
        volatile int16_t a = -13324;
        volatile int16_t b = -30649;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(80,92) + add2(92,251) + add2(80,251);
        if (r != 846) failures++;
    }


    {
        uint16_t r = 10762 + 50901 + 29392 + 56611 + 28721 + 58754 + 65225 + 26143;
        if (r != 64365) failures++;
    }


    {
        volatile int16_t a = -26264;
        volatile int16_t b = 4833;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {210,236,56,143,167,248,236,182};
        uint8_t *p = buf;
        p += 4;
        if (*p != 167) failures++;
    }


    {
        int8_t a = 69;
        int8_t b = 126;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 2291;
        volatile int16_t b = 3694;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 165;
        uint8_t r = port;
        if (r != 165) failures++;
    }


    {
        uint8_t v = 236;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        volatile uint8_t port = 131;
        uint8_t r = port;
        if (r != 131) failures++;
    }


    {
        g16 = 1406;
        if (read_g16() != 1406) failures++;
    }


    {
        volatile int16_t a = -11823;
        volatile int16_t b = 1208;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        uint8_t x = 74;
        x <<= 2;
        if (x != 40) failures++;
    }


    {
        uint16_t r = call6(16,47,253,57,140,29);
        if (r != 542) failures++;
    }


    {
        uint16_t r = call6(134,119,242,197,228,158);
        if (r != 1078) failures++;
    }


    {
        volatile int16_t a = 31845;
        volatile int16_t b = -31262;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)234) + (uint16_t)6956;
        if (r != 7190) failures++;
    }


    {
        uint8_t v = 93;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 35) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint8_t v = 224;
        int r = (v & 128) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint32_t a = 3657888532UL;
        uint32_t b = 3422926111UL;
        uint32_t r = a - b;
        if (r != 234962421UL) failures++;
    }


    {
        uint8_t buf[8] = {22,13,146,198,59,207,0,251};
        uint8_t *p = buf;
        p += 2;
        if (*p != 146) failures++;
    }


    {
        uint8_t v = 13;
        v &= ~(uint8_t)8;
        if (v != 5) failures++;
    }


    {
        uint32_t a = 1354987514UL;
        uint32_t b = 3111118568UL;
        uint32_t r = a ^ b;
        if (r != 3920400658UL) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 205;
        if (buf[4] != 205) failures++;
    }


    {
        g16 = 65039;
        if (read_g16() != 65039) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 9; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t m[2][4] = {{97,122,143,187},{108,171,114,210}};
        if (m[0][3] != 187) failures++;
    }


    {
        g16 = 51109;
        if (read_g16() != 51109) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 1) sum += j;
        if (sum != 6) failures++;
    }


    {
        uint8_t v = 55;
        v |= 64;
        if (v != 119) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-4) % (int16_t)((int8_t)15);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        uint32_t a = 3396795163UL;
        uint32_t b = 1064036604UL;
        uint32_t r = a - b;
        if (r != 2332758559UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)117) / (int16_t)((int8_t)87);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t buf[8] = {223,156,174,128,3,159,143,185};
        uint8_t *p = buf;
        p += 5;
        if (*p != 159) failures++;
    }


    {
        volatile int16_t a = 32202;
        volatile int16_t b = -27581;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[7] = {88,238,83,65,132,240,223};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[4] != 132) failures++;
    }


    {
        uint8_t m[4][2] = {{59,77},{4,182},{246,200},{255,90}};
        if (m[0][0] != 59) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 17: result = 159; break;
        case 11: result = 99; break;
        case 1: result = 181; break;
        case 15: result = 26; break;
        case 13: result = 168; break;
        case 9: result = 39; break;
        case 19: result = 172; break;
        case 16: result = 117; break;
        default: result = 110; break;
        }
        if (result != 26) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 2: result = 237; break;
        case 6: result = 209; break;
        case 10: result = 70; break;
        case 17: result = 57; break;
        case 8: result = 216; break;
        default: result = 53; break;
        }
        if (result != 53) failures++;
    }


    {
        uint8_t buf[8] = {78,142,182,161,216,43,158,95};
        uint8_t *p = buf;
        p += 2;
        if (*p != 182) failures++;
    }


    {
        g16 = 41539;
        if (read_g16() != 41539) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 18: result = 228; break;
        case 7: result = 245; break;
        case 8: result = 93; break;
        case 12: result = 220; break;
        case 19: result = 95; break;
        case 5: result = 18; break;
        case 11: result = 209; break;
        default: result = 141; break;
        }
        if (result != 228) failures++;
    }


    {
        int8_t a = 20;
        int8_t b = 17;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 11581;
        volatile int16_t b = 14711;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = add2(26,40) + add2(40,36) + add2(26,36);
        if (r != 204) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(245,101) != 144) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)50) + (uint16_t)53535;
        if (r != 53585) failures++;
    }


    {
        uint8_t v = 250;
        v |= 32;
        if (v != 250) failures++;
    }


    {
        uint32_t a = 4145610263UL;
        uint32_t b = 4103373544UL;
        uint32_t r = a ^ b;
        if (r != 59538687UL) failures++;
    }


    {
        uint16_t r = call6(57,131,166,251,208,172);
        if (r != 985) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 27;
        if (buf[12] != 27) failures++;
    }


    {
        volatile int16_t a = -4826;
        volatile int16_t b = 32437;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[4][3] = {{186,214,213},{87,131,85},{69,84,180},{136,229,250}};
        if (m[0][0] != 186) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {73,36,17585,31};
        if (s.a != (uint8_t)73) failures++;
    }


    {
        uint8_t v = 178;
        v &= ~(uint8_t)32;
        if (v != 146) failures++;
    }


    {
        uint8_t src[8] = {47,191,226,115,81,217,0,74};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[7] != 74) failures++;
    }


    {
        uint16_t r = 28951 + 36407 + 44661 + 57739 + 308 + 15356 + 55659 + 43037;
        if (r != 19974) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 5) sum += j;
        if (sum != 30) failures++;
    }


    {
        uint8_t x = 31;
        x <<= 3;
        if (x != 248) failures++;
    }


    {
        if (((uint16_t)(((19 & 6) ^ (7 - 51)) | 219)) != 65503) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {221,221,10066,149};
        if (s.d != (uint8_t)149) failures++;
    }


    {
        uint32_t a = 1980327998UL;
        uint32_t b = 843668877UL;
        uint32_t r = a + b;
        if (r != 2823996875UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-99) / (int16_t)((int8_t)64);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        uint32_t a = 2686865304UL;
        uint32_t b = 1135524816UL;
        uint32_t r = a - b;
        if (r != 1551340488UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = add2(34,67) + add2(67,100) + add2(34,100);
        if (r != 402) failures++;
    }


    {
        uint16_t r = call6(223,57,16,204,34,154);
        if (r != 688) failures++;
    }


    {
        int8_t a = 32;
        int8_t b = 116;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 41114;
        if (read_g16() != 41114) failures++;
    }


    {
        uint16_t r = call6(1,166,85,102,218,119);
        if (r != 691) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {247,190,16111,91};
        if (s.c != (uint16_t)16111) failures++;
    }


    {
        uint16_t r = call6(74,213,151,51,192,197);
        if (r != 878) failures++;
    }


    {
        uint8_t buf[8] = {251,65,126,153,96,246,232,173};
        uint8_t *p = buf;
        p += 3;
        if (*p != 153) failures++;
    }


    {
        uint16_t r = 28464 + 34831 + 47984 + 4895 + 13435 + 59960 + 780 + 40231;
        if (r != 33972) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)95) % (int16_t)((int8_t)-67);
        if ((uint16_t)r != (uint16_t)28) failures++;
    }


    {
        volatile int16_t a = -28186;
        volatile int16_t b = 32303;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {65,22,17,197,31,245};
        if (a[1] != 22) failures++;
    }


    {
        uint16_t x = 30;
        x = x + 254;
        if (x != 284) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = add2(10,89) + add2(89,194) + add2(10,194);
        if (r != 586) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = 28758;
        volatile int16_t b = 1022;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 30;
        do { cnt++; } while (--k);
        if (cnt != 30) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {104,235,8978,252};
        if (s.a != (uint8_t)104) failures++;
    }


    {
        uint8_t v = 19;
        v ^= 16;
        if (v != 3) failures++;
    }


    {
        uint8_t buf[8] = {248,181,135,75,152,4,243,104};
        uint8_t *p = buf;
        p += 7;
        if (*p != 104) failures++;
    }


    {
        uint8_t v = 227;
        v &= ~(uint8_t)64;
        if (v != 163) failures++;
    }


    {
        uint8_t src[6] = {110,231,195,62,1,56};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[1] != 231) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {222,126,64152,51};
        if (s.b != (uint8_t)126) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)102) / (int16_t)((int8_t)100);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-90) / (int16_t)((int8_t)-53);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t a[6] = {59,20,219,210,9,65};
        if (a[0] != 59) failures++;
    }


    {
        uint16_t r = call6(72,221,90,65,255,43);
        if (r != 746) failures++;
    }


    {
        uint8_t a[6] = {53,245,121,87,20,126};
        if (a[4] != 20) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 240;
        v ^= 64;
        if (v != 176) failures++;
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
        uint16_t r = add2(165,170) + add2(170,242) + add2(165,242);
        if (r != 1154) failures++;
    }


    {
        g16 = 23817;
        if (read_g16() != 23817) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)205) + (uint16_t)56788;
        if (r != 56993) failures++;
    }


    {
        uint16_t r = add2(149,4) + add2(4,98) + add2(149,98);
        if (r != 502) failures++;
    }


    {
        g16 = 15673;
        if (read_g16() != 15673) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 16; j += 1) sum += j;
        if (sum != 120) failures++;
    }


    {
        uint16_t r = call6(22,60,73,255,149,90);
        if (r != 649) failures++;
    }


    {
        uint16_t r = add2(43,105) + add2(105,170) + add2(43,170);
        if (r != 636) failures++;
    }


    {
        uint16_t r = call6(107,46,190,168,159,35);
        if (r != 705) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)167) + (uint16_t)33142;
        if (r != 33309) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)229) + (uint16_t)64932;
        if (r != 65161) failures++;
    }


    {
        uint16_t r = call6(163,241,240,134,208,107);
        if (r != 1093) failures++;
    }


    {
        uint16_t r = call6(234,142,97,20,247,172);
        if (r != 912) failures++;
    }


    {
        uint16_t r = call6(31,18,106,19,242,108);
        if (r != 524) failures++;
    }


    {
        if (((uint16_t)193) != 193) failures++;
    }


    {
        uint16_t r = call6(212,203,114,35,213,245);
        if (r != 1022) failures++;
    }


    {
        g16 = 17153;
        if (read_g16() != 17153) failures++;
    }


    {
        uint32_t a = 1246093908UL;
        uint32_t b = 2999093155UL;
        uint32_t r = a & b;
        if (r != 37782016UL) failures++;
    }


    {
        uint32_t a = 1956086663UL;
        uint32_t b = 345025674UL;
        uint32_t r = a ^ b;
        if (r != 1611083533UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 2) sum += j;
        if (sum != 72) failures++;
    }


    {
        volatile int16_t a = 22565;
        volatile int16_t b = -26391;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -13542;
        volatile int16_t b = -29568;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint32_t a = 2703470343UL;
        uint32_t b = 10159288UL;
        uint32_t r = a - b;
        if (r != 2693311055UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)10) % (int16_t)((int8_t)-114);
        if ((uint16_t)r != (uint16_t)10) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {241,80,60453,173};
        if (s.c != (uint16_t)60453) failures++;
    }


    {
        int8_t a = -5;
        int8_t b = -9;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(71,109) != 180) failures++;
    }


    {
        uint16_t r = call6(23,26,163,171,95,159);
        if (r != 637) failures++;
    }


    {
        uint8_t a[6] = {182,152,232,68,248,247};
        if (a[1] != 152) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)117) + (uint16_t)46716;
        if (r != 46833) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(156,64) != 220) failures++;
    }


    {
        uint8_t m[4][4] = {{38,96,170,228},{222,129,186,235},{68,53,17,91},{102,13,218,216}};
        if (m[1][3] != 235) failures++;
    }


    {
        g16 = 19857;
        if (read_g16() != 19857) failures++;
    }


    {
        uint16_t r = 47447 + 35036 + 51116 + 63793 + 57890 + 45160 + 2494 + 9251;
        if (r != 50043) failures++;
    }


    {
        uint8_t x = 203;
        x <<= 0;
        if (x != 203) failures++;
    }


    {
        uint32_t a = 1195809546UL;
        uint32_t b = 291239353UL;
        uint32_t r = a & b;
        if (r != 21139720UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t r = 22667 + 219 + 51155 + 21364 + 11699 + 52505 + 51197 + 31532;
        if (r != 45730) failures++;
    }


    {
        uint8_t src[1] = {43};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 43) failures++;
    }


    {
        uint16_t x = 150;
        x = x + 78;
        if (x != 228) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)25) + (uint16_t)47315;
        if (r != 47340) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(251,69) != 320) failures++;
    }


    {
        uint32_t a = 3588354274UL;
        uint32_t b = 3577825645UL;
        uint32_t r = a - b;
        if (r != 10528629UL) failures++;
    }


    {
        volatile uint8_t port = 8;
        uint8_t r = port;
        if (r != 8) failures++;
    }


    {
        uint8_t m[3][2] = {{50,143},{118,141},{126,61}};
        if (m[1][1] != 141) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 10: result = 19; break;
        case 11: result = 46; break;
        case 14: result = 81; break;
        case 9: result = 80; break;
        case 3: result = 236; break;
        case 13: result = 135; break;
        case 1: result = 92; break;
        default: result = 172; break;
        }
        if (result != 80) failures++;
    }


    {
        uint8_t v = 203;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 19;
        x = x + 28;
        if (x != 47) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)28) + (uint16_t)9969;
        if (r != 9997) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 16: result = 13; break;
        case 8: result = 246; break;
        case 13: result = 233; break;
        default: result = 175; break;
        }
        if (result != 246) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {93,63,2962,59};
        if (s.d != (uint8_t)59) failures++;
    }


    {
        uint16_t r = add2(9,167) + add2(167,109) + add2(9,109);
        if (r != 570) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 144;
        if (buf[9] != 144) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-14) % (int16_t)((int8_t)-112);
        if ((uint16_t)r != (uint16_t)65522) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-71) / (int16_t)((int8_t)-95);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 231;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)194) + (uint16_t)14209;
        if (r != 14403) failures++;
    }


    {
        uint8_t buf[8] = {165,220,165,145,24,213,213,199};
        uint8_t *p = buf;
        p += 3;
        if (*p != 145) failures++;
    }


    {
        uint16_t r = call6(141,45,6,205,179,203);
        if (r != 779) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-83) % (int16_t)((int8_t)121);
        if ((uint16_t)r != (uint16_t)65453) failures++;
    }


    {
        if (((uint16_t)((115 + 95) + ((191 ^ 118) + 18))) != 429) failures++;
    }


    {
        uint16_t r = add2(14,232) + add2(232,132) + add2(14,132);
        if (r != 756) failures++;
    }


    {
        volatile uint8_t port = 186;
        uint8_t r = port;
        if (r != 186) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 1) sum += j;
        if (sum != 190) failures++;
    }


    {
        uint8_t a[6] = {173,241,90,248,90,43};
        if (a[0] != 173) failures++;
    }


    {
        uint8_t m[2][4] = {{130,240,126,251},{238,86,15,199}};
        if (m[1][2] != 15) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 220;
        x = x + 204;
        if (x != 424) failures++;
    }


    {
        uint16_t r = call6(49,20,97,20,105,126);
        if (r != 417) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 3: result = 186; break;
        case 13: result = 252; break;
        case 9: result = 202; break;
        case 7: result = 250; break;
        default: result = 112; break;
        }
        if (result != 202) failures++;
    }


    {
        uint8_t v = 99;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)(((238 | 153) + (77 | 78)) ^ ((18 + 192) | (231 - 190)))) != 437) failures++;
    }


    {
        uint8_t a[6] = {73,241,15,23,116,249};
        if (a[2] != 15) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 243;
        if (buf[10] != 243) failures++;
    }


    {
        uint16_t r = 20307 + 19335 + 35624 + 57393 + 852 + 45697 + 23309 + 19783;
        if (r != 25692) failures++;
    }


    {
        uint16_t r = 19315 + 50129 + 54418 + 15761 + 40330 + 2930 + 55512 + 35522;
        if (r != 11773) failures++;
    }


    {
        if (((uint16_t)(((52 ^ 89) & (49 - 186)) ^ ((141 & 238) - (222 - 155)))) != 44) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(81,250) != 65367) failures++;
    }


    {
        int8_t a = -125;
        int8_t b = 48;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {227,92,1,204,58,138};
        if (a[3] != 204) failures++;
    }


    {
        uint8_t x = 254;
        x <<= 2;
        if (x != 248) failures++;
    }


    {
        g16 = 64944;
        if (read_g16() != 64944) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(207,218) != 425) failures++;
    }


    {
        volatile uint8_t port = 153;
        uint8_t r = port;
        if (r != 153) failures++;
    }


    {
        uint8_t src[2] = {40,84};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 84) failures++;
    }


    {
        uint16_t x = 60507;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {20,184,30192,146};
        if (s.b != (uint8_t)184) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)163) + (uint16_t)6210;
        if (r != 6373) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 2: result = 109; break;
        case 18: result = 45; break;
        case 4: result = 73; break;
        case 5: result = 151; break;
        case 10: result = 52; break;
        case 11: result = 150; break;
        case 1: result = 206; break;
        case 14: result = 112; break;
        default: result = 155; break;
        }
        if (result != 112) failures++;
    }


    {
        uint8_t src[15] = {220,185,188,24,199,114,43,30,169,27,176,134,106,165,130};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[11] != 134) failures++;
    }


    {
        uint16_t r = add2(26,141) + add2(141,100) + add2(26,100);
        if (r != 534) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 75;
        if (buf[10] != 75) failures++;
    }


    {
        uint8_t buf[8] = {203,236,129,123,70,28,99,53};
        uint8_t *p = buf;
        p += 7;
        if (*p != 53) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(127,10) != 117) failures++;
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
        if (fn(159,151) != 310) failures++;
    }


    {
        g16 = 30045;
        if (read_g16() != 30045) failures++;
    }


    {
        uint8_t v = 96;
        v &= ~(uint8_t)4;
        if (v != 96) failures++;
    }


    {
        g16 = 16795;
        if (read_g16() != 16795) failures++;
    }


    {
        uint16_t r = call6(176,160,164,177,104,49);
        if (r != 830) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(14,176) != 190) failures++;
    }


    {
        uint8_t m[4][3] = {{135,69,212},{169,3,84},{7,198,73},{187,126,182}};
        if (m[0][0] != 135) failures++;
    }


    {
        uint8_t m[4][3] = {{203,134,0},{236,35,252},{159,128,123},{98,43,104}};
        if (m[0][0] != 203) failures++;
    }


    {
        if (((uint16_t)162) != 162) failures++;
    }


    {
        uint8_t src[11] = {145,207,238,16,205,193,137,176,54,69,183};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[6] != 137) failures++;
    }


    {
        uint8_t a[6] = {128,90,94,164,201,142};
        if (a[5] != 142) failures++;
    }


    {
        uint8_t v = 247;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 5) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 164; break;
        case 15: result = 50; break;
        case 4: result = 228; break;
        case 8: result = 68; break;
        default: result = 88; break;
        }
        if (result != 164) failures++;
    }


    {
        uint8_t x = 207;
        x <<= 1;
        if (x != 158) failures++;
    }


    {
        uint8_t m[4][4] = {{187,21,125,89},{71,248,220,225},{142,84,128,187},{171,86,41,223}};
        if (m[2][2] != 128) failures++;
    }


    {
        int8_t a = -76;
        int8_t b = 39;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 236;
        uint8_t r = port;
        if (r != 236) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {157,126,2978,17};
        if (s.b != (uint8_t)126) failures++;
    }


    {
        uint16_t r = 20978 + 63613 + 25897 + 52158 + 56583 + 24671 + 51887 + 53483;
        if (r != 21590) failures++;
    }


    {
        uint8_t a[6] = {241,45,248,22,162,33};
        if (a[1] != 45) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 183;
        if (buf[0] != 183) failures++;
    }


    {
        g16 = 46102;
        if (read_g16() != 46102) failures++;
    }


    {
        uint16_t r = call6(133,21,100,133,169,229);
        if (r != 785) failures++;
    }


    {
        uint16_t r = add2(188,163) + add2(163,155) + add2(188,155);
        if (r != 1012) failures++;
    }


    {
        uint8_t x = 217;
        x <<= 3;
        if (x != 200) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 63;
        if (buf[0] != 63) failures++;
    }


    {
        if (((uint16_t)((150 | (248 ^ 83)) ^ 235)) != 84) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 2) sum += j;
        if (sum != 56) failures++;
    }


    {
        uint8_t m[3][3] = {{76,149,66},{175,54,255},{116,172,90}};
        if (m[2][2] != 90) failures++;
    }


    {
        uint16_t x = 19414;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 59277;
        if (read_g16() != 59277) failures++;
    }


    {
        uint8_t a[6] = {253,153,62,132,229,108};
        if (a[2] != 62) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 19: result = 201; break;
        case 7: result = 14; break;
        case 18: result = 243; break;
        case 15: result = 208; break;
        default: result = 143; break;
        }
        if (result != 243) failures++;
    }


    {
        uint8_t input = 17;
        uint8_t result;
        switch (input) {
        case 9: result = 228; break;
        case 0: result = 191; break;
        case 16: result = 169; break;
        case 3: result = 28; break;
        case 13: result = 93; break;
        case 18: result = 163; break;
        case 17: result = 44; break;
        case 5: result = 212; break;
        default: result = 25; break;
        }
        if (result != 44) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        uint8_t v = 103;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 9) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {175,196,241,141,173,221,34,67};
        uint8_t *p = buf;
        p += 4;
        if (*p != 173) failures++;
    }


    {
        uint8_t v = 23;
        v |= 4;
        if (v != 23) failures++;
    }


    {
        volatile uint8_t port = 122;
        uint8_t r = port;
        if (r != 122) failures++;
    }


    {
        uint8_t src[6] = {191,33,48,113,254,239};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[5] != 239) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 24;
        do { cnt++; } while (--k);
        if (cnt != 24) failures++;
    }


    {
        uint8_t a[6] = {119,187,126,166,52,189};
        if (a[5] != 189) failures++;
    }


    {
        int8_t a = 110;
        int8_t b = -16;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = call6(225,224,4,165,175,20);
        if (r != 813) failures++;
    }


    {
        volatile uint8_t port = 204;
        uint8_t r = port;
        if (r != 204) failures++;
    }


    {
        uint16_t x = 3256;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(219,37,111,17,185,84);
        if (r != 653) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        int8_t a = 55;
        int8_t b = -83;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)14) + (uint16_t)47129;
        if (r != 47143) failures++;
    }


    {
        uint8_t x = 74;
        x <<= 4;
        if (x != 160) failures++;
    }


    {
        volatile int16_t a = -8535;
        volatile int16_t b = -11017;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(144,4) != 148) failures++;
    }


    {
        volatile uint8_t port = 146;
        uint8_t r = port;
        if (r != 146) failures++;
    }


    {
        uint16_t r = 16117 + 51697 + 52022 + 60726 + 45667 + 24034 + 27709 + 5245;
        if (r != 21073) failures++;
    }


    {
        int8_t a = 108;
        int8_t b = -119;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t x = 15;
        x <<= 4;
        if (x != 240) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)105) + (uint16_t)1009;
        if (r != 1114) failures++;
    }


    {
        int8_t a = 18;
        int8_t b = -109;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[1] = {3};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 3) failures++;
    }


    {
        g16 = 51631;
        if (read_g16() != 51631) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(46,168) != 65414) failures++;
    }


    {
        uint16_t r = add2(44,146) + add2(146,192) + add2(44,192);
        if (r != 764) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {4,15,60892,100};
        if (s.c != (uint16_t)60892) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)106) % (int16_t)((int8_t)126);
        if ((uint16_t)r != (uint16_t)106) failures++;
    }


    {
        volatile uint8_t port = 55;
        uint8_t r = port;
        if (r != 55) failures++;
    }


    {
        g16 = 52791;
        if (read_g16() != 52791) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)86) % (int16_t)((int8_t)-84);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)203) + (uint16_t)45771;
        if (r != 45974) failures++;
    }


    {
        uint32_t a = 728186381UL;
        uint32_t b = 2073627423UL;
        uint32_t r = a ^ b;
        if (r != 1358837010UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 5; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        g16 = 59640;
        if (read_g16() != 59640) failures++;
    }


    {
        uint16_t x = 63506;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[16] = {223,27,5,253,244,144,121,253,141,216,7,208,82,180,91,188};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[10] != 7) failures++;
    }


    {
        volatile uint8_t port = 152;
        uint8_t r = port;
        if (r != 152) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {141,221,54238,245};
        if (s.c != (uint16_t)54238) failures++;
    }


    {
        if (((uint16_t)(25 ^ 54)) != 47) failures++;
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
        if (fn(179,246) != 65469) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)55) / (int16_t)((int8_t)-98);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 202;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int8_t a = 98;
        int8_t b = 78;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[2][3] = {{134,145,151},{62,74,215}};
        if (m[1][2] != 215) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint8_t a[6] = {203,108,73,215,122,149};
        if (a[3] != 215) failures++;
    }


    {
        uint8_t x = 69;
        x <<= 0;
        if (x != 69) failures++;
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
        uint8_t buf[8] = {7,17,126,35,59,167,197,118};
        uint8_t *p = buf;
        p += 4;
        if (*p != 59) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)77) + (uint16_t)49403;
        if (r != 49480) failures++;
    }


    {
        uint16_t r = 6617 + 45461 + 60105 + 37595 + 60045 + 25420 + 56726 + 52157;
        if (r != 16446) failures++;
    }


    {
        uint8_t a[6] = {205,154,229,181,30,184};
        if (a[5] != 184) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 223;
        if (buf[0] != 223) failures++;
    }


    {
        volatile int16_t a = -25226;
        volatile int16_t b = 32282;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {59,207,118,54,63,147,165,71};
        uint8_t *p = buf;
        p += 5;
        if (*p != 147) failures++;
    }


    {
        uint16_t r = 43023 + 51403 + 50319 + 2272 + 56169 + 21087 + 58460 + 20342;
        if (r != 40931) failures++;
    }


    {
        uint8_t a[6] = {68,167,28,221,200,62};
        if (a[0] != 68) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 9: result = 184; break;
        case 7: result = 147; break;
        case 13: result = 73; break;
        default: result = 72; break;
        }
        if (result != 147) failures++;
    }


    {
        uint16_t r = add2(25,177) + add2(177,226) + add2(25,226);
        if (r != 856) failures++;
    }


    {
        uint8_t buf[8] = {159,178,84,185,233,166,247,100};
        uint8_t *p = buf;
        p += 0;
        if (*p != 159) failures++;
    }


    {
        uint16_t r = call6(43,186,38,128,123,242);
        if (r != 760) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        int8_t a = 0;
        int8_t b = 19;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 21617 + 21503 + 57582 + 60370 + 55946 + 22653 + 46308 + 23630;
        if (r != 47465) failures++;
    }


    {
        uint16_t r = 12372 + 50385 + 39248 + 7591 + 14533 + 25595 + 6795 + 31568;
        if (r != 57015) failures++;
    }


    {
        int8_t a = 44;
        int8_t b = 116;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)(181 & (157 ^ 134))) != 17) failures++;
    }


    {
        uint16_t r = call6(175,133,166,234,213,254);
        if (r != 1175) failures++;
    }


    {
        uint8_t v = 87;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 9) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t x = 12;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 8: result = 201; break;
        case 1: result = 252; break;
        case 0: result = 195; break;
        case 18: result = 81; break;
        default: result = 142; break;
        }
        if (result != 81) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)43) + (uint16_t)49824;
        if (r != 49867) failures++;
    }


    {
        uint8_t v = 216;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 8) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 4) sum += j;
        if (sum != 24) failures++;
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
        int8_t a = 127;
        int8_t b = -29;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        if (((uint16_t)(226 - ((96 + 2) ^ (31 & 190)))) != 102) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 241;
        if (buf[13] != 241) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)113) + (uint16_t)25929;
        if (r != 26042) failures++;
    }


    {
        uint16_t r = 55570 + 53438 + 48666 + 23281 + 58655 + 32259 + 16024 + 50733;
        if (r != 10946) failures++;
    }


    {
        uint8_t v = 230;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {193,78,20275,168};
        if (s.d != (uint8_t)168) failures++;
    }


    {
        uint8_t v = 174;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)183) != 183) failures++;
    }


    {
        uint8_t x = 231;
        x <<= 2;
        if (x != 156) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 14: result = 248; break;
        case 0: result = 201; break;
        case 13: result = 221; break;
        default: result = 102; break;
        }
        if (result != 248) failures++;
    }


    {
        uint16_t x = 50992;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][3] = {{149,155,219},{32,185,16},{12,250,224}};
        if (m[1][0] != 32) failures++;
    }


    {
        volatile uint8_t port = 54;
        uint8_t r = port;
        if (r != 54) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(104,12) != 116) failures++;
    }


    {
        uint8_t src[15] = {93,89,166,36,191,84,60,219,231,63,13,9,13,17,55};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[11] != 9) failures++;
    }


    {
        int8_t a = 66;
        int8_t b = 67;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {70,88,62,93,218,103,222,10};
        uint8_t *p = buf;
        p += 2;
        if (*p != 62) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 3) sum += j;
        if (sum != 30) failures++;
    }


    {
        volatile int16_t a = -11327;
        volatile int16_t b = -9733;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        volatile uint8_t port = 179;
        uint8_t r = port;
        if (r != 179) failures++;
    }


    {
        uint8_t v = 153;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t buf[8] = {68,114,6,65,58,167,79,30};
        uint8_t *p = buf;
        p += 4;
        if (*p != 58) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {163,138,16649,189};
        if (s.a != (uint8_t)163) failures++;
    }


    {
        uint16_t r = add2(47,241) + add2(241,124) + add2(47,124);
        if (r != 824) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 12: result = 176; break;
        case 10: result = 16; break;
        case 19: result = 94; break;
        case 9: result = 143; break;
        case 0: result = 232; break;
        case 17: result = 93; break;
        case 3: result = 150; break;
        default: result = 233; break;
        }
        if (result != 94) failures++;
    }


    {
        uint8_t a[6] = {7,91,38,96,119,213};
        if (a[3] != 96) failures++;
    }


    {
        volatile uint8_t port = 41;
        uint8_t r = port;
        if (r != 41) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint8_t m[3][3] = {{196,50,185},{224,156,87},{130,0,210}};
        if (m[2][2] != 210) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)23) + (uint16_t)11636;
        if (r != 11659) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {112,213,51347,239};
        if (s.b != (uint8_t)213) failures++;
    }


    {
        if (((uint16_t)125) != 125) failures++;
    }


    {
        g16 = 25998;
        if (read_g16() != 25998) failures++;
    }


    {
        g16 = 59671;
        if (read_g16() != 59671) failures++;
    }


    {
        uint16_t x = 37464;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)75) % (int16_t)((int8_t)-113);
        if ((uint16_t)r != (uint16_t)75) failures++;
    }


    {
        uint8_t a[6] = {46,216,90,149,97,183};
        if (a[4] != 97) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        uint16_t r = add2(200,98) + add2(98,18) + add2(200,18);
        if (r != 632) failures++;
    }


    {
        uint16_t x = 50620;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 11: result = 113; break;
        case 17: result = 10; break;
        case 14: result = 98; break;
        case 18: result = 31; break;
        case 4: result = 149; break;
        case 15: result = 251; break;
        case 13: result = 57; break;
        default: result = 23; break;
        }
        if (result != 57) failures++;
    }


    {
        uint8_t v = 173;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 19) failures++;
    }


    {
        volatile int16_t a = -18030;
        volatile int16_t b = 13456;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -119;
        int8_t b = 76;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {173,203,54088,248};
        if (s.c != (uint16_t)54088) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 16: result = 99; break;
        case 11: result = 200; break;
        case 2: result = 226; break;
        case 5: result = 49; break;
        case 4: result = 37; break;
        case 1: result = 88; break;
        case 18: result = 151; break;
        case 12: result = 218; break;
        default: result = 130; break;
        }
        if (result != 49) failures++;
    }


    {
        g16 = 22975;
        if (read_g16() != 22975) failures++;
    }


    {
        uint16_t r = add2(213,68) + add2(68,173) + add2(213,173);
        if (r != 908) failures++;
    }


    {
        if (((uint16_t)(((180 ^ 210) + (145 & 8)) ^ (74 + (189 ^ 192)))) != 161) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(242,222) != 464) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 152;
        if (buf[3] != 152) failures++;
    }


    {
        uint16_t r = call6(17,255,64,132,65,149);
        if (r != 682) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-104) % (int16_t)((int8_t)-43);
        if ((uint16_t)r != (uint16_t)65518) failures++;
    }


    {
        uint8_t v = 195;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 13) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)214) + (uint16_t)10617;
        if (r != 10831) failures++;
    }


    {
        uint8_t x = 255;
        x <<= 2;
        if (x != 252) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 15: result = 247; break;
        case 1: result = 150; break;
        case 8: result = 212; break;
        case 12: result = 55; break;
        case 2: result = 74; break;
        case 19: result = 0; break;
        default: result = 144; break;
        }
        if (result != 55) failures++;
    }


    {
        volatile uint8_t port = 249;
        uint8_t r = port;
        if (r != 249) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)75) + (uint16_t)36202;
        if (r != 36277) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 151;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 9) failures++;
    }


    {
        uint8_t v = 64;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 32) failures++;
    }


    {
        uint16_t x = 45;
        x = x + 22;
        if (x != 67) failures++;
    }


    {
        uint8_t buf[8] = {88,242,44,185,1,111,52,68};
        uint8_t *p = buf;
        p += 0;
        if (*p != 88) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)90) % (int16_t)((int8_t)110);
        if ((uint16_t)r != (uint16_t)90) failures++;
    }


    {
        g16 = 54463;
        if (read_g16() != 54463) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(84,238) != 65382) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 14;
        do { cnt++; } while (--k);
        if (cnt != 14) failures++;
    }


    {
        volatile uint8_t port = 215;
        uint8_t r = port;
        if (r != 215) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 28;
        do { cnt++; } while (--k);
        if (cnt != 28) failures++;
    }


    {
        if (((uint16_t)32) != 32) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(0,62) != 62) failures++;
    }


    {
        if (((uint16_t)246) != 246) failures++;
    }


    {
        uint8_t src[11] = {92,87,147,124,9,238,33,86,243,167,243};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[4] != 9) failures++;
    }


    {
        uint8_t x = 224;
        x <<= 4;
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
        g16 = 64751;
        if (read_g16() != 64751) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint32_t a = 3933690777UL;
        uint32_t b = 1179134609UL;
        uint32_t r = a ^ b;
        if (r != 2889838856UL) failures++;
    }


    {
        uint8_t m[4][2] = {{179,234},{58,5},{216,222},{173,179}};
        if (m[0][0] != 179) failures++;
    }


    {
        uint16_t r = 62633 + 36875 + 24803 + 56144 + 39969 + 59617 + 64066 + 17979;
        if (r != 34406) failures++;
    }


    {
        uint16_t r = call6(190,123,138,254,129,227);
        if (r != 1061) failures++;
    }


    {
        uint16_t r = 10023 + 1014 + 8452 + 9136 + 8673 + 50170 + 7719 + 35504;
        if (r != 65155) failures++;
    }


    {
        int8_t a = 124;
        int8_t b = -48;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 242;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 47;
        x = x + 102;
        if (x != 149) failures++;
    }


    {
        uint8_t m[2][3] = {{27,174,114},{3,151,218}};
        if (m[0][1] != 174) failures++;
    }


    {
        uint8_t a[6] = {133,20,159,107,125,202};
        if (a[5] != 202) failures++;
    }


    {
        volatile uint8_t port = 31;
        uint8_t r = port;
        if (r != 31) failures++;
    }


    {
        if (((uint16_t)((12 + (208 + 125)) - 68)) != 277) failures++;
    }


    {
        volatile uint8_t port = 37;
        uint8_t r = port;
        if (r != 37) failures++;
    }


    {
        if (((uint16_t)(((55 | 68) + 147) & (226 | 137))) != 10) failures++;
    }


    {
        uint16_t r = 126 + 47593 + 10714 + 12637 + 51408 + 54701 + 41977 + 28519;
        if (r != 51067) failures++;
    }


    {
        uint8_t v = 245;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint32_t a = 2522241547UL;
        uint32_t b = 3420328059UL;
        uint32_t r = a & b;
        if (r != 2186678283UL) failures++;
    }


    {
        uint32_t a = 959968577UL;
        uint32_t b = 2053544273UL;
        uint32_t r = a - b;
        if (r != 3201391600UL) failures++;
    }


    {
        volatile int16_t a = -8864;
        volatile int16_t b = -17135;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 176;
        if (buf[1] != 176) failures++;
    }


    {
        if (((uint16_t)244) != 244) failures++;
    }


    {
        g16 = 44426;
        if (read_g16() != 44426) failures++;
    }


    {
        volatile int16_t a = -15784;
        volatile int16_t b = -2602;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {19,140,14286,236};
        if (s.c != (uint16_t)14286) failures++;
    }


    {
        uint8_t x = 72;
        x <<= 2;
        if (x != 32) failures++;
    }


    {
        uint8_t v = 15;
        v ^= 128;
        if (v != 143) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 3253 + 826 + 42891 + 57609 + 13461 + 50360 + 56653 + 46878;
        if (r != 9787) failures++;
    }


    {
        g16 = 35733;
        if (read_g16() != 35733) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)100) % (int16_t)((int8_t)26);
        if ((uint16_t)r != (uint16_t)22) failures++;
    }


    {
        uint16_t r = 11552 + 28508 + 53485 + 49302 + 49950 + 14178 + 206 + 52875;
        if (r != 63448) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)5) % (int16_t)((int8_t)115);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint16_t x = 7;
        x = x + 15;
        if (x != 22) failures++;
    }


    {
        uint8_t m[3][4] = {{39,52,41,161},{165,5,107,181},{200,61,186,233}};
        if (m[1][0] != 165) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 4) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t v = 107;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 21) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {217,159,43931,135};
        if (s.c != (uint16_t)43931) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 3: result = 252; break;
        case 6: result = 201; break;
        case 18: result = 167; break;
        case 4: result = 251; break;
        case 0: result = 189; break;
        case 16: result = 29; break;
        default: result = 146; break;
        }
        if (result != 167) failures++;
    }


    {
        uint8_t m[4][2] = {{127,218},{100,195},{96,60},{233,133}};
        if (m[1][1] != 195) failures++;
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
        case 2: result = 115; break;
        case 6: result = 36; break;
        case 11: result = 3; break;
        default: result = 36; break;
        }
        if (result != 36) failures++;
    }


    {
        uint8_t m[3][4] = {{135,68,8,210},{152,132,78,38},{254,171,90,54}};
        if (m[0][0] != 135) failures++;
    }


    {
        uint8_t a[6] = {126,233,60,9,164,51};
        if (a[3] != 9) failures++;
    }


    {
        uint16_t x = 63327;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 12325;
        if (read_g16() != 12325) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(254,247) != 501) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 141;
        x = x + 50;
        if (x != 191) failures++;
    }


    {
        uint8_t a[6] = {35,83,124,66,62,228};
        if (a[2] != 124) failures++;
    }


    {
        uint8_t a[6] = {255,138,55,77,152,3};
        if (a[3] != 77) failures++;
    }


    {
        uint16_t x = 236;
        x = x + 212;
        if (x != 448) failures++;
    }


    {
        uint8_t v = 114;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 110;
        x = x + 169;
        if (x != 279) failures++;
    }


    {
        int8_t a = 74;
        int8_t b = -84;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][4] = {{146,116,213,105},{116,229,186,160},{34,196,232,213},{18,21,82,27}};
        if (m[0][1] != 116) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)14) % (int16_t)((int8_t)113);
        if ((uint16_t)r != (uint16_t)14) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 234;
        if (buf[0] != 234) failures++;
    }


    {
        if (((uint16_t)((37 + 1) ^ (91 - (36 | 124)))) != 65529) failures++;
    }


    {
        g16 = 3594;
        if (read_g16() != 3594) failures++;
    }


    {
        uint8_t a[6] = {83,248,82,213,206,48};
        if (a[2] != 82) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(213,39) != 252) failures++;
    }


    {
        uint16_t r = add2(47,129) + add2(129,192) + add2(47,192);
        if (r != 736) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 188;
        if (buf[6] != 188) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)253) + (uint16_t)63487;
        if (r != 63740) failures++;
    }


    {
        uint16_t r = 35503 + 5170 + 14872 + 23976 + 53939 + 5057 + 53484 + 12600;
        if (r != 7993) failures++;
    }


    {
        if (((uint16_t)((251 + 102) + 199)) != 552) failures++;
    }


    {
        int8_t a = -108;
        int8_t b = 107;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = add2(165,198) + add2(198,77) + add2(165,77);
        if (r != 880) failures++;
    }


    {
        uint8_t v = 92;
        int r = (v & 128) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {151,84,15,182,79,190,180,158};
        uint8_t *p = buf;
        p += 5;
        if (*p != 190) failures++;
    }


    {
        if (((uint16_t)(((169 | 66) & (70 | 139)) & (100 | 165))) != 193) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-4) % (int16_t)((int8_t)-46);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        int8_t a = 53;
        int8_t b = 82;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint32_t a = 1268487177UL;
        uint32_t b = 4194803437UL;
        uint32_t r = a ^ b;
        if (r != 2979793636UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t x = 53;
        x <<= 3;
        if (x != 168) failures++;
    }


    {
        uint8_t src[6] = {154,174,126,51,138,41};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[1] != 174) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 78;
        if (buf[1] != 78) failures++;
    }


    {
        uint8_t v = 93;
        v ^= 2;
        if (v != 95) failures++;
    }


    {
        volatile uint8_t port = 234;
        uint8_t r = port;
        if (r != 234) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-34) / (int16_t)((int8_t)-33);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t x = 40543;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 43275;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 180;
        v ^= 16;
        if (v != 164) failures++;
    }


    {
        uint8_t a[6] = {43,77,94,102,196,142};
        if (a[5] != 142) failures++;
    }


    {
        uint8_t src[16] = {208,116,159,153,16,221,69,63,143,94,59,187,235,212,50,57};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[11] != 187) failures++;
    }


    {
        uint16_t r = call6(155,198,20,19,38,210);
        if (r != 640) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(140,144) != 65532) failures++;
    }


    {
        uint8_t src[14] = {82,122,167,17,118,121,171,163,10,128,88,58,46,250};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[13] != 250) failures++;
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
        volatile uint8_t port = 222;
        uint8_t r = port;
        if (r != 222) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)110) + (uint16_t)5180;
        if (r != 5290) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 166;
        v ^= 128;
        if (v != 38) failures++;
    }


    {
        g16 = 49829;
        if (read_g16() != 49829) failures++;
    }


    {
        g16 = 13299;
        if (read_g16() != 13299) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-109) / (int16_t)((int8_t)-82);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 228;
        if (buf[4] != 228) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(86,73) != 13) failures++;
    }


    {
        uint8_t v = 8;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)91) != 91) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)86) % (int16_t)((int8_t)21);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(8,24) != 32) failures++;
    }


    {
        uint8_t src[3] = {133,102,107};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 133) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {49,162,33599,116};
        if (s.c != (uint16_t)33599) failures++;
    }


    {
        int8_t a = -125;
        int8_t b = 16;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 75;
        x <<= 2;
        if (x != 44) failures++;
    }


    {
        uint16_t x = 48;
        x = x + 47;
        if (x != 95) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)70) + (uint16_t)51584;
        if (r != 51654) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)223) + (uint16_t)25506;
        if (r != 25729) failures++;
    }


    {
        uint8_t v = 174;
        v ^= 64;
        if (v != 238) failures++;
    }


    {
        uint8_t v = 14;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile uint8_t port = 170;
        uint8_t r = port;
        if (r != 170) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {74,144,244,90,17,69,64,183};
        uint8_t *p = buf;
        p += 3;
        if (*p != 90) failures++;
    }


    {
        uint32_t a = 2065792175UL;
        uint32_t b = 2659602776UL;
        uint32_t r = a + b;
        if (r != 430427655UL) failures++;
    }


    {
        uint8_t buf[8] = {182,65,38,41,191,198,143,90};
        uint8_t *p = buf;
        p += 4;
        if (*p != 191) failures++;
    }


    {
        volatile uint8_t port = 174;
        uint8_t r = port;
        if (r != 174) failures++;
    }


    {
        volatile int16_t a = -28014;
        volatile int16_t b = -9345;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 193;
        v ^= 1;
        if (v != 192) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        g16 = 42007;
        if (read_g16() != 42007) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(29,120) != 149) failures++;
    }


    {
        uint16_t x = 170;
        x = x + 119;
        if (x != 289) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {155,68,14190,75};
        if (s.c != (uint16_t)14190) failures++;
    }


    {
        uint8_t buf[8] = {64,110,61,199,175,202,206,188};
        uint8_t *p = buf;
        p += 2;
        if (*p != 61) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {218,29,19809,103};
        if (s.d != (uint8_t)103) failures++;
    }


    {
        uint8_t x = 19;
        x <<= 4;
        if (x != 48) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 101;
        if (buf[9] != 101) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint32_t a = 1171452580UL;
        uint32_t b = 3059239179UL;
        uint32_t r = a | b;
        if (r != 4158323631UL) failures++;
    }


    {
        uint8_t a[6] = {88,111,243,167,30,198};
        if (a[4] != 30) failures++;
    }


    {
        uint16_t x = 11;
        x = x + 131;
        if (x != 142) failures++;
    }


    {
        uint8_t src[6] = {33,44,35,82,55,73};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[0] != 33) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 126;
        if (buf[3] != 126) failures++;
    }


    {
        uint8_t src[1] = {175};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 175) failures++;
    }


    {
        if (((uint16_t)((206 + (102 - 43)) | ((88 | 138) + 250))) != 477) failures++;
    }


    {
        uint8_t a[6] = {63,133,124,13,255,226};
        if (a[5] != 226) failures++;
    }


    {
        uint8_t buf[8] = {2,22,68,193,52,62,22,178};
        uint8_t *p = buf;
        p += 1;
        if (*p != 22) failures++;
    }


    {
        uint16_t r = 44830 + 12712 + 49523 + 38474 + 10168 + 29867 + 2966 + 3861;
        if (r != 61329) failures++;
    }


    {
        volatile uint8_t port = 24;
        uint8_t r = port;
        if (r != 24) failures++;
    }


    {
        uint8_t v = 107;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)114) + (uint16_t)24901;
        if (r != 25015) failures++;
    }


    {
        uint16_t r = call6(93,219,1,6,199,117);
        if (r != 635) failures++;
    }


    {
        uint32_t a = 2380032789UL;
        uint32_t b = 1390986004UL;
        uint32_t r = a ^ b;
        if (r != 3744770049UL) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 207;
        if (buf[12] != 207) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 18: result = 214; break;
        case 3: result = 187; break;
        case 8: result = 66; break;
        case 7: result = 71; break;
        case 5: result = 8; break;
        case 15: result = 92; break;
        default: result = 201; break;
        }
        if (result != 187) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 14: result = 114; break;
        case 0: result = 37; break;
        case 5: result = 230; break;
        case 11: result = 191; break;
        case 10: result = 221; break;
        case 3: result = 97; break;
        case 16: result = 229; break;
        default: result = 243; break;
        }
        if (result != 191) failures++;
    }


    {
        uint8_t v = 220;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 230;
        x = x + 136;
        if (x != 366) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)163) + (uint16_t)24626;
        if (r != 24789) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)109) % (int16_t)((int8_t)-73);
        if ((uint16_t)r != (uint16_t)36) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(160,115) != 45) failures++;
    }


    {
        uint8_t v = 130;
        v |= 8;
        if (v != 138) failures++;
    }


    {
        uint16_t x = 106;
        x = x + 229;
        if (x != 335) failures++;
    }


    {
        uint16_t x = 250;
        x = x + 152;
        if (x != 402) failures++;
    }


    {
        if (((uint16_t)(((249 - 201) + (231 ^ 138)) + ((75 | 100) | (138 - 219)))) != 140) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 11: result = 248; break;
        case 14: result = 229; break;
        case 18: result = 43; break;
        case 10: result = 221; break;
        case 5: result = 134; break;
        default: result = 223; break;
        }
        if (result != 134) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {93,194,60680,191};
        if (s.b != (uint8_t)194) failures++;
    }


    {
        uint8_t src[7] = {48,147,16,98,24,140,56};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[1] != 147) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 242;
        if (buf[9] != 242) failures++;
    }


    {
        if (((uint16_t)((135 ^ 173) ^ ((21 | 102) - (60 - 123)))) != 156) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 1) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint16_t r = add2(76,43) + add2(43,208) + add2(76,208);
        if (r != 654) failures++;
    }


    {
        uint8_t src[9] = {119,222,101,71,214,248,142,90,101};
        uint8_t dst[9];
        for (uint8_t j = 0; j < 9; j++) dst[j] = src[j];
        if (dst[5] != 248) failures++;
    }


    {
        int8_t a = -126;
        int8_t b = -116;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {206,57,254,137,8,54};
        if (a[3] != 137) failures++;
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
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint8_t v = 135;
        v &= ~(uint8_t)64;
        if (v != 135) failures++;
    }


    {
        uint16_t x = 42;
        x = x + 49;
        if (x != 91) failures++;
    }


    {
        uint8_t x = 22;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t r = 2088 + 5449 + 19017 + 37974 + 62990 + 62704 + 12669 + 19573;
        if (r != 25856) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 19;
        if (buf[3] != 19) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)5) != 5) failures++;
    }


    {
        int8_t a = 108;
        int8_t b = 26;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(82,29) != 53) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 18: result = 14; break;
        case 9: result = 208; break;
        case 6: result = 112; break;
        case 15: result = 131; break;
        case 13: result = 235; break;
        case 1: result = 205; break;
        case 14: result = 33; break;
        default: result = 58; break;
        }
        if (result != 33) failures++;
    }


    {
        int8_t a = -125;
        int8_t b = -76;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 215;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 9) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-44) / (int16_t)((int8_t)16);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint8_t m[2][4] = {{184,184,10,202},{21,26,206,56}};
        if (m[0][1] != 184) failures++;
    }


    {
        uint16_t r = 63681 + 5764 + 59811 + 55607 + 13772 + 55620 + 18020 + 60386;
        if (r != 4981) failures++;
    }


    {
        uint32_t a = 3252190794UL;
        uint32_t b = 1976964337UL;
        uint32_t r = a + b;
        if (r != 934187835UL) failures++;
    }


    {
        uint8_t a[6] = {195,120,233,100,98,40};
        if (a[3] != 100) failures++;
    }


    {
        uint16_t x = 12961;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 254;
        if (buf[1] != 254) failures++;
    }


    {
        uint8_t v = 201;
        int r = (v & 2) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 174;
        v ^= 8;
        if (v != 166) failures++;
    }

    return failures;
}