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
        g16 = 3451;
        if (read_g16() != 3451) failures++;
    }


    {
        uint16_t r = 41403 + 31489 + 21537 + 55237 + 3448 + 44804 + 44755 + 42071;
        if (r != 22600) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)51) / (int16_t)((int8_t)120);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t r = 22367 + 36550 + 18773 + 28081 + 12330 + 58815 + 61541 + 54532;
        if (r != 30845) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 15: result = 92; break;
        case 5: result = 204; break;
        case 2: result = 167; break;
        case 10: result = 15; break;
        case 7: result = 43; break;
        case 1: result = 188; break;
        case 17: result = 43; break;
        default: result = 31; break;
        }
        if (result != 167) failures++;
    }


    {
        uint8_t v = 161;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 31) failures++;
    }


    {
        uint16_t x = 33729;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 113;
        x <<= 2;
        if (x != 196) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {8,144,40399,125};
        if (s.b != (uint8_t)144) failures++;
    }


    {
        volatile int16_t a = -24450;
        volatile int16_t b = 20001;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)72) + (uint16_t)40935;
        if (r != 41007) failures++;
    }


    {
        if (((uint16_t)(((202 & 8) | (220 & 175)) ^ ((203 - 227) - 16))) != 65364) failures++;
    }


    {
        uint8_t src[2] = {137,83};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 137) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        if (((uint16_t)(((211 - 89) + (145 & 244)) & (54 ^ (172 - 156)))) != 2) failures++;
    }


    {
        uint16_t r = call6(202,47,9,150,72,199);
        if (r != 679) failures++;
    }


    {
        uint16_t r = 51574 + 29232 + 53607 + 63617 + 62647 + 55033 + 24948 + 63224;
        if (r != 10666) failures++;
    }


    {
        uint8_t v = 89;
        v ^= 128;
        if (v != 217) failures++;
    }


    {
        uint8_t buf[8] = {229,49,17,247,230,175,16,15};
        uint8_t *p = buf;
        p += 6;
        if (*p != 16) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)171) + (uint16_t)9139;
        if (r != 9310) failures++;
    }


    {
        if (((uint16_t)(90 & ((179 & 83) & 132))) != 0) failures++;
    }


    {
        uint16_t x = 25472;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 57;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint32_t a = 2476632664UL;
        uint32_t b = 459709438UL;
        uint32_t r = a | b;
        if (r != 2617179134UL) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 228;
        if (buf[10] != 228) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)123) + (uint16_t)65202;
        if (r != 65325) failures++;
    }


    {
        uint8_t v = 4;
        int r = (v & 16) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint32_t a = 3957055470UL;
        uint32_t b = 1180111828UL;
        uint32_t r = a ^ b;
        if (r != 2911685690UL) failures++;
    }


    {
        uint8_t buf[8] = {100,129,232,218,62,170,163,96};
        uint8_t *p = buf;
        p += 3;
        if (*p != 218) failures++;
    }


    {
        uint8_t v = 204;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 48;
        x = x + 5;
        if (x != 53) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)104) % (int16_t)((int8_t)91);
        if ((uint16_t)r != (uint16_t)13) failures++;
    }


    {
        g16 = 35245;
        if (read_g16() != 35245) failures++;
    }


    {
        volatile int16_t a = -5408;
        volatile int16_t b = -24853;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][2] = {{123,118},{229,166},{196,16},{202,196}};
        if (m[0][0] != 123) failures++;
    }


    {
        uint16_t r = 55633 + 45284 + 34063 + 13277 + 52458 + 26817 + 44203 + 7585;
        if (r != 17176) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(186,227) != 65495) failures++;
    }


    {
        uint32_t a = 842179090UL;
        uint32_t b = 979763533UL;
        uint32_t r = a | b;
        if (r != 980853599UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)101) % (int16_t)((int8_t)-24);
        if ((uint16_t)r != (uint16_t)5) failures++;
    }


    {
        uint16_t r = add2(193,182) + add2(182,244) + add2(193,244);
        if (r != 1238) failures++;
    }


    {
        uint8_t buf[8] = {76,86,55,245,22,207,8,133};
        uint8_t *p = buf;
        p += 1;
        if (*p != 86) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 11: result = 132; break;
        case 14: result = 52; break;
        case 8: result = 70; break;
        case 18: result = 127; break;
        case 1: result = 24; break;
        case 17: result = 151; break;
        case 5: result = 132; break;
        default: result = 178; break;
        }
        if (result != 132) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 4;
        if (buf[3] != 4) failures++;
    }


    {
        if (((uint16_t)(79 ^ ((186 | 43) | (7 & 187)))) != 244) failures++;
    }


    {
        int8_t a = -25;
        int8_t b = -26;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {161,210,4551,201};
        if (s.b != (uint8_t)210) failures++;
    }


    {
        if (((uint16_t)((149 ^ 89) & 43)) != 8) failures++;
    }


    {
        volatile uint8_t port = 233;
        uint8_t r = port;
        if (r != 233) failures++;
    }


    {
        uint16_t r = call6(132,20,115,139,243,43);
        if (r != 692) failures++;
    }


    {
        uint8_t buf[8] = {150,83,112,30,177,41,114,228};
        uint8_t *p = buf;
        p += 2;
        if (*p != 112) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(127,253) != 380) failures++;
    }


    {
        uint8_t m[4][4] = {{141,210,14,243},{121,236,51,68},{61,197,40,221},{188,162,180,2}};
        if (m[1][0] != 121) failures++;
    }


    {
        uint32_t a = 1953890938UL;
        uint32_t b = 2841655578UL;
        uint32_t r = a - b;
        if (r != 3407202656UL) failures++;
    }


    {
        volatile uint8_t port = 54;
        uint8_t r = port;
        if (r != 54) failures++;
    }


    {
        g16 = 13248;
        if (read_g16() != 13248) failures++;
    }


    {
        uint8_t v = 84;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 4) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)116) % (int16_t)((int8_t)5);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        g16 = 10284;
        if (read_g16() != 10284) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 1) sum += j;
        if (sum != 55) failures++;
    }


    {
        uint8_t v = 51;
        v |= 4;
        if (v != 55) failures++;
    }


    {
        uint8_t v = 111;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = call6(48,180,3,3,195,124);
        if (r != 553) failures++;
    }


    {
        uint8_t input = 15;
        uint8_t result;
        switch (input) {
        case 18: result = 101; break;
        case 6: result = 218; break;
        case 10: result = 69; break;
        case 15: result = 202; break;
        case 7: result = 184; break;
        default: result = 181; break;
        }
        if (result != 202) failures++;
    }


    {
        if (((uint16_t)((75 & (115 & 169)) + ((111 | 246) + 101))) != 357) failures++;
    }


    {
        uint8_t x = 135;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {47,82,29331,82};
        if (s.b != (uint8_t)82) failures++;
    }


    {
        uint32_t a = 3298718322UL;
        uint32_t b = 2308861244UL;
        uint32_t r = a | b;
        if (r != 3449716606UL) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        volatile uint8_t port = 97;
        uint8_t r = port;
        if (r != 97) failures++;
    }


    {
        g16 = 60297;
        if (read_g16() != 60297) failures++;
    }


    {
        uint16_t r = 63382 + 60476 + 25746 + 48049 + 3456 + 6681 + 20189 + 27401;
        if (r != 58772) failures++;
    }


    {
        if (((uint16_t)242) != 242) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 18;
        do { cnt++; } while (--k);
        if (cnt != 18) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)210) + (uint16_t)30753;
        if (r != 30963) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(61,138) != 199) failures++;
    }


    {
        uint16_t r = 54489 + 9437 + 12122 + 35823 + 54097 + 14426 + 25159 + 33704;
        if (r != 42649) failures++;
    }


    {
        uint16_t r = 42745 + 56293 + 27779 + 9962 + 16245 + 60711 + 18056 + 9532;
        if (r != 44715) failures++;
    }


    {
        uint16_t r = 7173 + 29583 + 62997 + 3440 + 5087 + 58254 + 19689 + 13790;
        if (r != 3405) failures++;
    }


    {
        uint8_t buf[8] = {30,200,86,4,79,49,13,102};
        uint8_t *p = buf;
        p += 3;
        if (*p != 4) failures++;
    }


    {
        uint32_t a = 3669809362UL;
        uint32_t b = 1233562973UL;
        uint32_t r = a - b;
        if (r != 2436246389UL) failures++;
    }


    {
        uint8_t src[13] = {5,238,132,29,38,79,59,172,20,219,86,7,161};
        uint8_t dst[13];
        for (uint8_t j = 0; j < 13; j++) dst[j] = src[j];
        if (dst[9] != 219) failures++;
    }


    {
        volatile uint8_t port = 44;
        uint8_t r = port;
        if (r != 44) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        g16 = 15668;
        if (read_g16() != 15668) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        uint16_t x = 57791;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)100) + (uint16_t)63253;
        if (r != 63353) failures++;
    }


    {
        uint16_t r = call6(32,94,172,216,150,227);
        if (r != 891) failures++;
    }


    {
        volatile uint8_t port = 177;
        uint8_t r = port;
        if (r != 177) failures++;
    }


    {
        g16 = 26112;
        if (read_g16() != 26112) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint8_t v = 124;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(32,155) != 187) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 3: result = 39; break;
        case 17: result = 1; break;
        case 14: result = 34; break;
        case 10: result = 16; break;
        case 9: result = 17; break;
        case 18: result = 238; break;
        case 13: result = 247; break;
        default: result = 229; break;
        }
        if (result != 34) failures++;
    }


    {
        uint8_t a[6] = {127,54,102,176,29,223};
        if (a[2] != 102) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)128) + (uint16_t)18816;
        if (r != 18944) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {198,249,56558,191};
        if (s.a != (uint8_t)198) failures++;
    }


    {
        uint8_t x = 111;
        x <<= 0;
        if (x != 111) failures++;
    }


    {
        uint16_t x = 36467;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(181,150,250,203,165,53);
        if (r != 1002) failures++;
    }


    {
        volatile uint8_t port = 214;
        uint8_t r = port;
        if (r != 214) failures++;
    }


    {
        uint8_t v = 128;
        int r = (v & 8) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][3] = {{140,108,146},{176,248,48},{132,249,214},{237,216,182}};
        if (m[3][2] != 182) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(30,48) != 65518) failures++;
    }


    {
        if (((uint16_t)(((119 + 34) & (159 & 171)) & ((201 - 239) + 15))) != 137) failures++;
    }


    {
        volatile uint8_t port = 166;
        uint8_t r = port;
        if (r != 166) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -79;
        int8_t b = -126;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(243,158) != 85) failures++;
    }


    {
        uint8_t a[6] = {248,27,18,197,188,32};
        if (a[4] != 188) failures++;
    }


    {
        uint8_t v = 129;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 31) failures++;
    }


    {
        uint16_t r = add2(66,171) + add2(171,254) + add2(66,254);
        if (r != 982) failures++;
    }


    {
        uint8_t x = 73;
        x <<= 2;
        if (x != 36) failures++;
    }


    {
        uint16_t r = 28428 + 60666 + 33824 + 61105 + 7606 + 59202 + 17491 + 33679;
        if (r != 39857) failures++;
    }


    {
        uint8_t v = 25;
        v &= ~(uint8_t)4;
        if (v != 25) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {227,148,50313,55};
        if (s.d != (uint8_t)55) failures++;
    }


    {
        uint8_t v = 169;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(34,242) != 276) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)229) + (uint16_t)16529;
        if (r != 16758) failures++;
    }


    {
        int8_t a = -8;
        int8_t b = 106;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)108) != 108) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {28,55,9269,252};
        if (s.a != (uint8_t)28) failures++;
    }


    {
        uint32_t a = 507030777UL;
        uint32_t b = 2407335752UL;
        uint32_t r = a + b;
        if (r != 2914366529UL) failures++;
    }


    {
        uint8_t buf[8];
        for (uint8_t j = 0; j < 8; j++) buf[j] = 90;
        if (buf[7] != 90) failures++;
    }


    {
        uint16_t r = call6(179,46,168,158,248,222);
        if (r != 1021) failures++;
    }


    {
        uint16_t x = 54;
        x = x + 157;
        if (x != 211) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 13;
        do { cnt++; } while (--k);
        if (cnt != 13) failures++;
    }


    {
        uint8_t a[6] = {160,230,73,19,105,141};
        if (a[5] != 141) failures++;
    }


    {
        g16 = 16481;
        if (read_g16() != 16481) failures++;
    }


    {
        uint8_t src[5] = {14,11,51,58,59};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 14) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 152;
        if (buf[2] != 152) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)145) + (uint16_t)19199;
        if (r != 19344) failures++;
    }


    {
        uint16_t x = 2136;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 63529;
        if (read_g16() != 63529) failures++;
    }


    {
        uint8_t m[4][4] = {{107,162,167,22},{165,86,173,248},{37,206,214,239},{88,103,55,17}};
        if (m[0][2] != 167) failures++;
    }


    {
        uint16_t r = 35143 + 35278 + 49467 + 56581 + 14939 + 33373 + 10831 + 46906;
        if (r != 20374) failures++;
    }


    {
        uint8_t m[2][3] = {{23,57,109},{224,243,180}};
        if (m[1][0] != 224) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 228;
        if (buf[2] != 228) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 23;
        do { cnt++; } while (--k);
        if (cnt != 23) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {63,125,63078,189};
        if (s.b != (uint8_t)125) failures++;
    }


    {
        uint16_t r = call6(178,65,115,255,138,72);
        if (r != 823) failures++;
    }


    {
        int8_t a = 89;
        int8_t b = -93;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[6] = {194,252,38,172,224,125};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[1] != 252) failures++;
    }


    {
        if (((uint16_t)(58 | 30)) != 62) failures++;
    }


    {
        uint8_t v = 163;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = 43506 + 41120 + 14133 + 50305 + 53892 + 59028 + 1135 + 57294;
        if (r != 58269) failures++;
    }


    {
        uint8_t v = 211;
        v ^= 128;
        if (v != 83) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t a[6] = {59,128,62,131,161,15};
        if (a[5] != 15) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)15) + (uint16_t)60779;
        if (r != 60794) failures++;
    }


    {
        uint16_t x = 9078;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 19114 + 31180 + 17861 + 47665 + 5375 + 3025 + 23839 + 20123;
        if (r != 37110) failures++;
    }


    {
        uint16_t r = add2(194,125) + add2(125,13) + add2(194,13);
        if (r != 664) failures++;
    }


    {
        uint8_t a[6] = {248,159,223,209,138,0};
        if (a[4] != 138) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)226) + (uint16_t)15946;
        if (r != 16172) failures++;
    }


    {
        uint16_t r = 46743 + 17939 + 773 + 21358 + 38981 + 45108 + 15920 + 33817;
        if (r != 24031) failures++;
    }


    {
        g16 = 47921;
        if (read_g16() != 47921) failures++;
    }


    {
        uint8_t buf[8] = {64,84,255,187,166,217,3,93};
        uint8_t *p = buf;
        p += 2;
        if (*p != 255) failures++;
    }


    {
        uint8_t x = 165;
        x <<= 3;
        if (x != 40) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 15: result = 52; break;
        case 16: result = 247; break;
        case 8: result = 14; break;
        case 10: result = 76; break;
        case 4: result = 124; break;
        default: result = 33; break;
        }
        if (result != 33) failures++;
    }


    {
        int8_t a = -113;
        int8_t b = 57;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 84;
        x = x + 237;
        if (x != 321) failures++;
    }


    {
        uint16_t r = 12156 + 758 + 56518 + 22697 + 53170 + 42266 + 18590 + 17878;
        if (r != 27425) failures++;
    }


    {
        uint8_t m[2][2] = {{179,145},{106,87}};
        if (m[0][0] != 179) failures++;
    }


    {
        uint8_t buf[13];
        for (uint8_t j = 0; j < 13; j++) buf[j] = 37;
        if (buf[12] != 37) failures++;
    }


    {
        if (((uint16_t)4) != 4) failures++;
    }


    {
        if (((uint16_t)204) != 204) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint8_t x = 112;
        x <<= 0;
        if (x != 112) failures++;
    }


    {
        uint16_t r = add2(177,195) + add2(195,109) + add2(177,109);
        if (r != 962) failures++;
    }


    {
        uint8_t buf[8] = {208,89,134,12,176,204,171,15};
        uint8_t *p = buf;
        p += 2;
        if (*p != 134) failures++;
    }


    {
        uint8_t src[6] = {90,163,65,24,230,126};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[0] != 90) failures++;
    }


    {
        uint8_t v = 38;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {61,129,36,113,74,29,192,125};
        uint8_t *p = buf;
        p += 2;
        if (*p != 36) failures++;
    }


    {
        uint8_t a[6] = {82,255,66,211,0,222};
        if (a[3] != 211) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 1) sum += j;
        if (sum != 66) failures++;
    }


    {
        uint8_t m[4][3] = {{19,179,6},{37,45,70},{4,14,195},{130,6,22}};
        if (m[3][1] != 6) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(55,134) != 189) failures++;
    }


    {
        uint8_t m[4][4] = {{196,11,50,193},{53,122,229,41},{17,106,1,37},{112,18,246,8}};
        if (m[2][2] != 1) failures++;
    }


    {
        uint8_t m[3][3] = {{112,11,240},{25,220,101},{219,79,230}};
        if (m[2][0] != 219) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        uint8_t m[3][3] = {{108,100,199},{198,2,242},{46,31,196}};
        if (m[0][0] != 108) failures++;
    }


    {
        volatile int16_t a = 18810;
        volatile int16_t b = -16725;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 95;
        uint8_t r = port;
        if (r != 95) failures++;
    }


    {
        if (((uint16_t)(((207 & 220) ^ 65) + 253)) != 394) failures++;
    }


    {
        uint16_t x = 135;
        x = x + 218;
        if (x != 353) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)101) % (int16_t)((int8_t)-29);
        if ((uint16_t)r != (uint16_t)14) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {47,69,25764,10};
        if (s.b != (uint8_t)69) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {141,246,12376,25};
        if (s.d != (uint8_t)25) failures++;
    }


    {
        uint8_t v = 42;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)100) + (uint16_t)45573;
        if (r != 45673) failures++;
    }


    {
        uint8_t x = 234;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        if (((uint16_t)(((73 - 178) ^ (250 - 46)) | ((201 + 32) ^ (220 ^ 212)))) != 65531) failures++;
    }


    {
        uint8_t src[1] = {73};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 73) failures++;
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
        uint16_t x = 142;
        x = x + 131;
        if (x != 273) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {85,229,54366,138};
        if (s.a != (uint8_t)85) failures++;
    }


    {
        uint16_t r = 22169 + 28362 + 10451 + 27375 + 17056 + 32763 + 31693 + 30258;
        if (r != 3519) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 27;
        do { cnt++; } while (--k);
        if (cnt != 27) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)92) + (uint16_t)28066;
        if (r != 28158) failures++;
    }


    {
        uint16_t x = 43457;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 195;
        uint8_t r = port;
        if (r != 195) failures++;
    }


    {
        uint16_t r = 36381 + 26827 + 40748 + 3572 + 22017 + 25678 + 58171 + 63854;
        if (r != 15104) failures++;
    }


    {
        uint16_t r = call6(57,189,136,121,23,193);
        if (r != 719) failures++;
    }


    {
        if (((uint16_t)(178 + 179)) != 357) failures++;
    }


    {
        volatile uint8_t port = 218;
        uint8_t r = port;
        if (r != 218) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {27,249,58807,94};
        if (s.b != (uint8_t)249) failures++;
    }


    {
        if (((uint16_t)(((76 & 243) | 82) | (47 + (8 & 193)))) != 127) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(49,178) != 227) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-99) / (int16_t)((int8_t)-121);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t src[15] = {180,149,207,128,49,145,109,180,255,81,8,20,56,178,9};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[1] != 149) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)72) + (uint16_t)32316;
        if (r != 32388) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)153) + (uint16_t)15470;
        if (r != 15623) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint8_t a[6] = {184,98,126,177,55,21};
        if (a[3] != 177) failures++;
    }


    {
        uint8_t a[6] = {232,110,125,160,50,203};
        if (a[1] != 110) failures++;
    }


    {
        uint16_t r = 8253 + 48404 + 10844 + 42808 + 57178 + 20842 + 861 + 7348;
        if (r != 65466) failures++;
    }


    {
        uint16_t r = call6(53,111,167,198,233,102);
        if (r != 864) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(160,229) != 389) failures++;
    }


    {
        volatile int16_t a = 23890;
        volatile int16_t b = -12677;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t src[5] = {27,151,89,92,216};
        uint8_t dst[5];
        for (uint8_t j = 0; j < 5; j++) dst[j] = src[j];
        if (dst[0] != 27) failures++;
    }


    {
        g16 = 24996;
        if (read_g16() != 24996) failures++;
    }


    {
        uint32_t a = 4237670758UL;
        uint32_t b = 80977202UL;
        uint32_t r = a - b;
        if (r != 4156693556UL) failures++;
    }


    {
        uint32_t a = 3905683300UL;
        uint32_t b = 1683691233UL;
        uint32_t r = a & b;
        if (r != 1615533664UL) failures++;
    }


    {
        uint8_t v = 206;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 205;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t buf[8] = {67,163,12,130,37,117,4,226};
        uint8_t *p = buf;
        p += 0;
        if (*p != 67) failures++;
    }


    {
        volatile int16_t a = 30004;
        volatile int16_t b = -23036;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t x = 165;
        x = x + 161;
        if (x != 326) failures++;
    }


    {
        g16 = 30992;
        if (read_g16() != 30992) failures++;
    }


    {
        g16 = 34039;
        if (read_g16() != 34039) failures++;
    }


    {
        uint16_t r = call6(83,55,69,67,204,254);
        if (r != 732) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 85;
        if (buf[10] != 85) failures++;
    }


    {
        int8_t a = 36;
        int8_t b = -45;
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
        uint8_t x = 244;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)90) + (uint16_t)40224;
        if (r != 40314) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint8_t v = 228;
        v ^= 8;
        if (v != 236) failures++;
    }


    {
        int8_t a = 75;
        int8_t b = 32;
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
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(141,21) != 120) failures++;
    }


    {
        uint8_t buf[8] = {105,150,112,158,132,142,243,25};
        uint8_t *p = buf;
        p += 4;
        if (*p != 132) failures++;
    }


    {
        uint16_t r = 37353 + 8232 + 46381 + 44912 + 17449 + 26427 + 33231 + 15093;
        if (r != 32470) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)60) + (uint16_t)26824;
        if (r != 26884) failures++;
    }


    {
        uint16_t r = 19165 + 41246 + 2987 + 45860 + 21973 + 53726 + 64144 + 54303;
        if (r != 41260) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-102) / (int16_t)((int8_t)34);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        int8_t a = 84;
        int8_t b = -91;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = -2300;
        volatile int16_t b = 19554;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 3;
        do { cnt++; } while (--k);
        if (cnt != 3) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 71;
        v ^= 8;
        if (v != 79) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)94) + (uint16_t)14953;
        if (r != 15047) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = call6(44,64,167,15,139,71);
        if (r != 500) failures++;
    }


    {
        uint16_t r = 55252 + 53137 + 45162 + 23692 + 31361 + 4587 + 52611 + 32381;
        if (r != 36039) failures++;
    }


    {
        uint8_t buf[8] = {248,163,10,49,253,17,165,204};
        uint8_t *p = buf;
        p += 7;
        if (*p != 204) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(213,224) != 437) failures++;
    }


    {
        uint8_t m[2][3] = {{116,71,99},{188,102,77}};
        if (m[1][1] != 102) failures++;
    }


    {
        uint16_t r = 26079 + 36642 + 19575 + 5158 + 40873 + 55425 + 31201 + 5352;
        if (r != 23697) failures++;
    }


    {
        volatile int16_t a = -29262;
        volatile int16_t b = -14324;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)235) + (uint16_t)57563;
        if (r != 57798) failures++;
    }


    {
        uint16_t r = add2(9,172) + add2(172,4) + add2(9,4);
        if (r != 370) failures++;
    }


    {
        uint8_t x = 39;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {7,79,5143,6};
        if (s.b != (uint8_t)79) failures++;
    }


    {
        uint32_t a = 715199066UL;
        uint32_t b = 1363182746UL;
        uint32_t r = a - b;
        if (r != 3646983616UL) failures++;
    }


    {
        if (((uint16_t)(((177 + 140) - 13) ^ (72 & (160 - 142)))) != 304) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 13: result = 138; break;
        case 9: result = 234; break;
        case 4: result = 83; break;
        case 14: result = 99; break;
        case 2: result = 187; break;
        case 11: result = 43; break;
        case 5: result = 116; break;
        case 8: result = 122; break;
        default: result = 190; break;
        }
        if (result != 83) failures++;
    }


    {
        uint8_t input = 10;
        uint8_t result;
        switch (input) {
        case 8: result = 239; break;
        case 0: result = 31; break;
        case 5: result = 134; break;
        case 10: result = 196; break;
        case 1: result = 207; break;
        case 17: result = 28; break;
        case 4: result = 115; break;
        default: result = 127; break;
        }
        if (result != 196) failures++;
    }


    {
        uint32_t a = 3795035168UL;
        uint32_t b = 979543280UL;
        uint32_t r = a + b;
        if (r != 479611152UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint8_t x = 221;
        x <<= 4;
        if (x != 208) failures++;
    }


    {
        uint8_t buf[8] = {182,9,164,107,172,154,161,61};
        uint8_t *p = buf;
        p += 3;
        if (*p != 107) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(46,243) != 65339) failures++;
    }


    {
        uint8_t v = 33;
        v |= 64;
        if (v != 97) failures++;
    }


    {
        volatile uint8_t port = 175;
        uint8_t r = port;
        if (r != 175) failures++;
    }


    {
        uint8_t v = 218;
        int r = (v & 8) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {182,64,38032,91};
        if (s.c != (uint16_t)38032) failures++;
    }


    {
        g16 = 18346;
        if (read_g16() != 18346) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t x = 96;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint16_t r = add2(78,39) + add2(39,58) + add2(78,58);
        if (r != 350) failures++;
    }


    {
        uint16_t r = add2(184,20) + add2(20,108) + add2(184,108);
        if (r != 624) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 3: result = 225; break;
        case 5: result = 193; break;
        case 12: result = 94; break;
        case 1: result = 246; break;
        case 14: result = 90; break;
        case 0: result = 77; break;
        case 4: result = 51; break;
        case 17: result = 19; break;
        default: result = 80; break;
        }
        if (result != 90) failures++;
    }


    {
        uint8_t buf[8] = {38,161,226,131,173,157,91,206};
        uint8_t *p = buf;
        p += 3;
        if (*p != 131) failures++;
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
        if (fn(35,115) != 150) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint8_t x = 82;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-84) % (int16_t)((int8_t)-62);
        if ((uint16_t)r != (uint16_t)65514) failures++;
    }


    {
        volatile uint8_t port = 16;
        uint8_t r = port;
        if (r != 16) failures++;
    }


    {
        volatile uint8_t port = 55;
        uint8_t r = port;
        if (r != 55) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(239,139) != 100) failures++;
    }


    {
        volatile int16_t a = -12745;
        volatile int16_t b = 20780;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t buf[8] = {39,136,169,71,132,121,36,221};
        uint8_t *p = buf;
        p += 1;
        if (*p != 136) failures++;
    }


    {
        uint8_t src[14] = {203,63,140,139,46,125,105,6,136,15,184,202,27,198};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[12] != 27) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 13; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t src[15] = {133,250,174,46,19,91,62,221,93,52,230,222,82,251,108};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[3] != 46) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 7;
        do { cnt++; } while (--k);
        if (cnt != 7) failures++;
    }


    {
        uint16_t r = 40994 + 54889 + 4658 + 13499 + 54433 + 41142 + 17083 + 50070;
        if (r != 14624) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {221,96,59130,194};
        if (s.c != (uint16_t)59130) failures++;
    }


    {
        uint16_t r = 32692 + 23198 + 41003 + 20093 + 52357 + 28624 + 4241 + 39271;
        if (r != 44871) failures++;
    }


    {
        uint8_t m[3][2] = {{36,187},{104,114},{25,127}};
        if (m[2][1] != 127) failures++;
    }


    {
        g16 = 17747;
        if (read_g16() != 17747) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 5: result = 109; break;
        case 2: result = 137; break;
        case 18: result = 94; break;
        case 13: result = 49; break;
        default: result = 32; break;
        }
        if (result != 49) failures++;
    }


    {
        uint16_t r = call6(198,97,218,11,121,156);
        if (r != 801) failures++;
    }


    {
        uint16_t x = 150;
        x = x + 63;
        if (x != 213) failures++;
    }


    {
        uint8_t a[6] = {92,142,50,195,15,118};
        if (a[0] != 92) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-58) / (int16_t)((int8_t)-44);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint16_t x = 127;
        x = x + 34;
        if (x != 161) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(236,161) != 397) failures++;
    }


    {
        uint16_t r = call6(0,186,30,40,80,179);
        if (r != 515) failures++;
    }


    {
        uint8_t x = 182;
        x <<= 5;
        if (x != 192) failures++;
    }


    {
        uint16_t r = add2(45,134) + add2(134,229) + add2(45,229);
        if (r != 816) failures++;
    }


    {
        uint8_t src[3] = {138,117,84};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 117) failures++;
    }


    {
        uint8_t m[3][2] = {{89,124},{189,82},{92,22}};
        if (m[1][1] != 82) failures++;
    }


    {
        uint8_t v = 251;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = call6(218,60,224,58,177,82);
        if (r != 819) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)24) % (int16_t)((int8_t)-112);
        if ((uint16_t)r != (uint16_t)24) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {197,253,50458,243};
        if (s.c != (uint16_t)50458) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-77) % (int16_t)((int8_t)-108);
        if ((uint16_t)r != (uint16_t)65459) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 205;
        if (buf[14] != 205) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(104,49) != 153) failures++;
    }


    {
        uint8_t m[4][3] = {{88,94,239},{233,252,110},{46,229,146},{187,225,6}};
        if (m[3][1] != 225) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)174) + (uint16_t)61381;
        if (r != 61555) failures++;
    }


    {
        uint8_t v = 33;
        v |= 4;
        if (v != 37) failures++;
    }


    {
        uint16_t x = 224;
        x = x + 86;
        if (x != 310) failures++;
    }


    {
        uint8_t v = 25;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)135) != 135) failures++;
    }


    {
        int8_t a = -70;
        int8_t b = 24;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        int8_t a = 2;
        int8_t b = 10;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        if (((uint16_t)131) != 131) failures++;
    }


    {
        uint8_t m[4][3] = {{46,26,124},{128,221,156},{131,230,194},{161,229,77}};
        if (m[0][0] != 46) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 29;
        if (buf[14] != 29) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)169) + (uint16_t)43443;
        if (r != 43612) failures++;
    }


    {
        uint16_t x = 61064;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 62205;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = 46487 + 42291 + 40301 + 63382 + 43270 + 45359 + 5920 + 47802;
        if (r != 7132) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 5305;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[15] = {236,121,87,151,198,19,172,168,244,199,172,104,164,112,122};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[10] != 172) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -128;
        int8_t b = 111;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 19; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        uint16_t r = call6(110,34,125,3,220,64);
        if (r != 556) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(161,202) != 65495) failures++;
    }


    {
        uint8_t src[3] = {74,44,221};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 74) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 2) sum += j;
        if (sum != 12) failures++;
    }


    {
        uint8_t m[2][2] = {{5,132},{86,175}};
        if (m[0][1] != 132) failures++;
    }


    {
        uint16_t r = add2(138,134) + add2(134,149) + add2(138,149);
        if (r != 842) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(161,234) != 65463) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {229,222,51518,163};
        if (s.d != (uint8_t)163) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-127) % (int16_t)((int8_t)-40);
        if ((uint16_t)r != (uint16_t)65529) failures++;
    }


    {
        uint8_t x = 50;
        x <<= 5;
        if (x != 64) failures++;
    }


    {
        if (((uint16_t)((133 ^ (244 + 55)) ^ ((96 - 179) & 228))) != 266) failures++;
    }


    {
        uint8_t buf[11];
        for (uint8_t j = 0; j < 11; j++) buf[j] = 100;
        if (buf[10] != 100) failures++;
    }


    {
        uint8_t buf[8] = {27,195,240,203,188,96,119,239};
        uint8_t *p = buf;
        p += 1;
        if (*p != 195) failures++;
    }


    {
        uint8_t v = 217;
        int r = (v & 128) ? 1 : 0;
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
        uint8_t a[6] = {177,77,16,48,82,17};
        if (a[3] != 48) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)120) / (int16_t)((int8_t)-25);
        if ((uint16_t)r != (uint16_t)65532) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 1) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)30) + (uint16_t)28113;
        if (r != 28143) failures++;
    }


    {
        volatile uint8_t port = 45;
        uint8_t r = port;
        if (r != 45) failures++;
    }


    {
        uint8_t v = 139;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t x = 227;
        x = x + 158;
        if (x != 385) failures++;
    }


    {
        if (((uint16_t)(35 + ((45 & 40) ^ (93 - 123)))) != 65517) failures++;
    }


    {
        int8_t a = 32;
        int8_t b = -82;
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
        uint8_t a[6] = {108,84,164,106,255,39};
        if (a[1] != 84) failures++;
    }


    {
        int8_t a = -10;
        int8_t b = 61;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)115) + (uint16_t)13011;
        if (r != 13126) failures++;
    }


    {
        if (((uint16_t)(((39 ^ 73) | 102) - ((102 & 91) + (216 + 250)))) != 65114) failures++;
    }


    {
        uint8_t buf[8] = {172,217,236,187,37,95,230,169};
        uint8_t *p = buf;
        p += 4;
        if (*p != 37) failures++;
    }


    {
        uint16_t r = call6(65,154,154,174,88,219);
        if (r != 854) failures++;
    }


    {
        uint8_t m[2][4] = {{84,57,223,245},{120,108,213,243}};
        if (m[0][2] != 223) failures++;
    }


    {
        uint8_t x = 77;
        x <<= 3;
        if (x != 104) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(80,154) != 65462) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {217,170,63431,70};
        if (s.c != (uint16_t)63431) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 231;
        if (buf[15] != 231) failures++;
    }


    {
        uint16_t r = call6(122,77,156,123,168,57);
        if (r != 703) failures++;
    }


    {
        uint8_t v = 195;
        v |= 128;
        if (v != 195) failures++;
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
        g16 = 19913;
        if (read_g16() != 19913) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)68) + (uint16_t)20339;
        if (r != 20407) failures++;
    }


    {
        uint16_t r = 2640 + 63207 + 5743 + 51792 + 60126 + 60979 + 30817 + 6582;
        if (r != 19742) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {59,120,34218,170};
        if (s.d != (uint8_t)170) failures++;
    }


    {
        uint8_t buf[8] = {73,65,50,52,31,116,33,111};
        uint8_t *p = buf;
        p += 7;
        if (*p != 111) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)68) + (uint16_t)27637;
        if (r != 27705) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 6;
        do { cnt++; } while (--k);
        if (cnt != 6) failures++;
    }


    {
        uint8_t v = 102;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 10) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-3) % (int16_t)((int8_t)45);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        g16 = 2790;
        if (read_g16() != 2790) failures++;
    }


    {
        g16 = 11484;
        if (read_g16() != 11484) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 1) sum += j;
        if (sum != 21) failures++;
    }


    {
        uint8_t buf[8] = {50,161,29,253,85,134,218,25};
        uint8_t *p = buf;
        p += 3;
        if (*p != 253) failures++;
    }


    {
        uint16_t r = 13581 + 60967 + 33383 + 37196 + 15844 + 39575 + 12518 + 11411;
        if (r != 27867) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint8_t buf[10];
        for (uint8_t j = 0; j < 10; j++) buf[j] = 165;
        if (buf[9] != 165) failures++;
    }


    {
        uint16_t r = add2(122,142) + add2(142,45) + add2(122,45);
        if (r != 618) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint8_t m[2][4] = {{135,158,227,24},{221,95,119,99}};
        if (m[1][3] != 99) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(61,61) != 122) failures++;
    }


    {
        uint8_t x = 230;
        x <<= 3;
        if (x != 48) failures++;
    }


    {
        uint16_t r = 16797 + 59432 + 2209 + 4481 + 33673 + 52969 + 28163 + 34513;
        if (r != 35629) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {233,162,12530,184};
        if (s.b != (uint8_t)162) failures++;
    }


    {
        uint8_t src[7] = {187,210,146,246,188,128,123};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[4] != 188) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 1) sum += j;
        if (sum != 28) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 129;
        if (buf[15] != 129) failures++;
    }


    {
        uint16_t r = call6(222,143,52,66,122,103);
        if (r != 708) failures++;
    }


    {
        g16 = 31547;
        if (read_g16() != 31547) failures++;
    }


    {
        uint8_t buf[8] = {83,222,164,3,27,216,82,178};
        uint8_t *p = buf;
        p += 3;
        if (*p != 3) failures++;
    }


    {
        uint16_t r = call6(182,145,140,125,247,220);
        if (r != 1059) failures++;
    }


    {
        volatile int16_t a = 18452;
        volatile int16_t b = 17162;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(74,105) != 65505) failures++;
    }


    {
        volatile int16_t a = 224;
        volatile int16_t b = 27738;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t x = 30;
        x <<= 3;
        if (x != 240) failures++;
    }


    {
        uint8_t v = 217;
        v ^= 1;
        if (v != 216) failures++;
    }


    {
        uint16_t r = 64719 + 52650 + 5065 + 55653 + 46683 + 41891 + 1499 + 33598;
        if (r != 39614) failures++;
    }


    {
        volatile int16_t a = -12699;
        volatile int16_t b = -13766;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 7444;
        volatile int16_t b = -12780;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {9,253,7060,71};
        if (s.a != (uint8_t)9) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t src[2] = {122,103};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 122) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 8; j += 5) sum += j;
        if (sum != 5) failures++;
    }


    {
        uint16_t r = call6(78,99,150,130,254,4);
        if (r != 715) failures++;
    }


    {
        uint32_t a = 3988876585UL;
        uint32_t b = 1541161903UL;
        uint32_t r = a ^ b;
        if (r != 3055365766UL) failures++;
    }


    {
        uint8_t x = 131;
        x <<= 4;
        if (x != 48) failures++;
    }


    {
        uint32_t a = 2686332736UL;
        uint32_t b = 241042027UL;
        uint32_t r = a | b;
        if (r != 2925408107UL) failures++;
    }


    {
        uint16_t r = call6(244,156,193,130,29,231);
        if (r != 983) failures++;
    }


    {
        uint8_t v = 73;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)122) + (uint16_t)2357;
        if (r != 2479) failures++;
    }


    {
        uint8_t x = 53;
        x <<= 0;
        if (x != 53) failures++;
    }


    {
        uint16_t x = 0;
        x = x + 175;
        if (x != 175) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 44;
        if (buf[3] != 44) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 8: result = 27; break;
        case 13: result = 223; break;
        case 5: result = 28; break;
        case 12: result = 242; break;
        case 1: result = 52; break;
        case 16: result = 237; break;
        default: result = 25; break;
        }
        if (result != 25) failures++;
    }


    {
        int8_t a = 86;
        int8_t b = 119;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 15472;
        if (read_g16() != 15472) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 1) sum += j;
        if (sum != 45) failures++;
    }


    {
        uint8_t v = 179;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {61,213,147,109,216,126};
        if (a[0] != 61) failures++;
    }


    {
        uint32_t a = 693194481UL;
        uint32_t b = 3013321717UL;
        uint32_t r = a - b;
        if (r != 1974840060UL) failures++;
    }


    {
        if (((uint16_t)2) != 2) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(226,247) != 65515) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        volatile uint8_t port = 224;
        uint8_t r = port;
        if (r != 224) failures++;
    }


    {
        uint32_t a = 1956749093UL;
        uint32_t b = 1119886131UL;
        uint32_t r = a | b;
        if (r != 1994497847UL) failures++;
    }


    {
        g16 = 8266;
        if (read_g16() != 8266) failures++;
    }


    {
        uint16_t x = 48557;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t a[6] = {102,99,203,103,46,26};
        if (a[3] != 103) failures++;
    }


    {
        uint16_t x = 138;
        x = x + 141;
        if (x != 279) failures++;
    }


    {
        uint16_t r = 22930 + 19712 + 51312 + 34744 + 32915 + 50466 + 5649 + 47189;
        if (r != 2773) failures++;
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
        uint16_t x = 178;
        x = x + 188;
        if (x != 366) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        uint8_t buf[8] = {122,39,66,139,143,253,167,85};
        uint8_t *p = buf;
        p += 5;
        if (*p != 253) failures++;
    }


    {
        uint8_t src[15] = {76,52,50,111,193,50,249,181,205,191,244,240,243,215,253};
        uint8_t dst[15];
        for (uint8_t j = 0; j < 15; j++) dst[j] = src[j];
        if (dst[11] != 240) failures++;
    }


    {
        uint16_t x = 16651;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(65,98) != 65503) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        g16 = 14265;
        if (read_g16() != 14265) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 227;
        if (buf[1] != 227) failures++;
    }


    {
        g16 = 5542;
        if (read_g16() != 5542) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {205,255,63609,163};
        if (s.a != (uint8_t)205) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[4][2] = {{68,16},{93,119},{73,82},{137,86}};
        if (m[3][0] != 137) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {59,194,61715,215};
        if (s.b != (uint8_t)194) failures++;
    }


    {
        uint16_t x = 88;
        x = x + 65;
        if (x != 153) failures++;
    }


    {
        uint8_t input = 14;
        uint8_t result;
        switch (input) {
        case 12: result = 7; break;
        case 11: result = 4; break;
        case 14: result = 164; break;
        default: result = 233; break;
        }
        if (result != 164) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        if (((uint16_t)(((153 | 139) + 40) ^ (160 - (171 ^ 38)))) != 208) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(106,190) != 65452) failures++;
    }


    {
        uint8_t buf[8] = {120,236,156,6,39,12,161,253};
        uint8_t *p = buf;
        p += 1;
        if (*p != 236) failures++;
    }


    {
        uint16_t x = 232;
        x = x + 106;
        if (x != 338) failures++;
    }


    {
        volatile uint8_t port = 237;
        uint8_t r = port;
        if (r != 237) failures++;
    }


    {
        uint8_t v = 115;
        v ^= 16;
        if (v != 99) failures++;
    }


    {
        uint16_t r = 63350 + 37112 + 54037 + 11677 + 17966 + 37924 + 36130 + 36364;
        if (r != 32416) failures++;
    }


    {
        uint8_t m[3][2] = {{130,60},{171,66},{17,212}};
        if (m[0][1] != 60) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(44,176) != 220) failures++;
    }


    {
        uint8_t buf[8] = {17,147,78,124,73,147,0,245};
        uint8_t *p = buf;
        p += 6;
        if (*p != 0) failures++;
    }


    {
        uint8_t a[6] = {95,162,228,116,218,31};
        if (a[3] != 116) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)205) + (uint16_t)18869;
        if (r != 19074) failures++;
    }


    {
        int8_t a = 123;
        int8_t b = -40;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 177;
        v ^= 2;
        if (v != 179) failures++;
    }


    {
        g16 = 47135;
        if (read_g16() != 47135) failures++;
    }


    {
        uint16_t r = add2(126,194) + add2(194,79) + add2(126,79);
        if (r != 798) failures++;
    }


    {
        uint16_t r = add2(25,83) + add2(83,72) + add2(25,72);
        if (r != 360) failures++;
    }


    {
        volatile int16_t a = -3419;
        volatile int16_t b = -10532;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {143,248,16,162,233,228};
        if (a[5] != 228) failures++;
    }


    {
        uint32_t a = 673538741UL;
        uint32_t b = 610264340UL;
        uint32_t r = a & b;
        if (r != 537223188UL) failures++;
    }


    {
        uint16_t r = call6(152,122,142,72,210,152);
        if (r != 850) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 16;
        do { cnt++; } while (--k);
        if (cnt != 16) failures++;
    }


    {
        uint8_t v = 100;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint8_t buf[8] = {85,251,98,238,239,59,169,114};
        uint8_t *p = buf;
        p += 0;
        if (*p != 85) failures++;
    }


    {
        uint8_t src[3] = {119,167,172};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 119) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 18; j += 1) sum += j;
        if (sum != 153) failures++;
    }


    {
        uint8_t buf[16];
        for (uint8_t j = 0; j < 16; j++) buf[j] = 15;
        if (buf[15] != 15) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = -15207;
        volatile int16_t b = 22404;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = 32932 + 16281 + 22380 + 59318 + 59947 + 16147 + 38360 + 27527;
        if (r != 10748) failures++;
    }


    {
        uint16_t r = add2(71,228) + add2(228,52) + add2(71,52);
        if (r != 702) failures++;
    }


    {
        int8_t a = 8;
        int8_t b = 26;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 0: result = 226; break;
        case 11: result = 42; break;
        case 5: result = 42; break;
        case 17: result = 64; break;
        case 13: result = 13; break;
        default: result = 229; break;
        }
        if (result != 42) failures++;
    }


    {
        uint16_t r = add2(82,30) + add2(30,197) + add2(82,197);
        if (r != 618) failures++;
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
        uint8_t src[16] = {159,97,111,145,189,19,221,83,6,135,121,25,1,188,99,97};
        uint8_t dst[16];
        for (uint8_t j = 0; j < 16; j++) dst[j] = src[j];
        if (dst[2] != 111) failures++;
    }


    {
        uint8_t buf[3];
        for (uint8_t j = 0; j < 3; j++) buf[j] = 83;
        if (buf[2] != 83) failures++;
    }


    {
        g16 = 28104;
        if (read_g16() != 28104) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-99) % (int16_t)((int8_t)63);
        if ((uint16_t)r != (uint16_t)65500) failures++;
    }


    {
        uint8_t x = 92;
        x <<= 2;
        if (x != 112) failures++;
    }


    {
        uint8_t a[6] = {249,96,20,218,110,128};
        if (a[5] != 128) failures++;
    }


    {
        uint8_t a[6] = {90,36,35,20,181,74};
        if (a[1] != 36) failures++;
    }


    {
        uint16_t x = 137;
        x = x + 107;
        if (x != 244) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)57) + (uint16_t)2195;
        if (r != 2252) failures++;
    }


    {
        uint16_t r = call6(237,216,174,99,5,12);
        if (r != 743) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {91,210,4899,18};
        if (s.b != (uint8_t)210) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(181,184) != 65533) failures++;
    }


    {
        uint8_t x = 41;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(220,8) != 212) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 3) sum += j;
        if (sum != 63) failures++;
    }


    {
        volatile int16_t a = -4297;
        volatile int16_t b = -13275;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)60) + (uint16_t)10168;
        if (r != 10228) failures++;
    }


    {
        uint8_t v = 125;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(241,211) != 30) failures++;
    }


    {
        uint8_t src[3] = {219,56,134};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 56) failures++;
    }


    {
        uint8_t m[4][4] = {{69,204,192,4},{133,42,208,242},{165,252,113,72},{18,140,248,37}};
        if (m[0][3] != 4) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {37,4,61466,17};
        if (s.d != (uint8_t)17) failures++;
    }


    {
        uint16_t r = call6(239,164,69,21,101,234);
        if (r != 828) failures++;
    }


    {
        uint16_t r = 59540 + 48473 + 63749 + 30050 + 12687 + 18267 + 50689 + 1241;
        if (r != 22552) failures++;
    }


    {
        if (((uint16_t)(((186 - 121) + (78 | 209)) | ((48 + 66) | 67))) != 371) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(25,108) != 65453) failures++;
    }


    {
        volatile uint8_t port = 101;
        uint8_t r = port;
        if (r != 101) failures++;
    }


    {
        uint8_t a[6] = {0,46,75,108,14,195};
        if (a[4] != 14) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {96,61,6315,94};
        if (s.c != (uint16_t)6315) failures++;
    }


    {
        uint8_t x = 65;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 238;
        x = x + 140;
        if (x != 378) failures++;
    }


    {
        if (((uint16_t)(((192 & 116) | (229 & 65)) + ((159 | 113) + 216))) != 536) failures++;
    }


    {
        if (((uint16_t)199) != 199) failures++;
    }


    {
        uint8_t src[14] = {140,149,48,137,101,120,161,223,135,123,207,63,24,104};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[2] != 48) failures++;
    }


    {
        g16 = 3351;
        if (read_g16() != 3351) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(132,105) != 237) failures++;
    }


    {
        uint16_t r = call6(27,28,63,172,237,4);
        if (r != 531) failures++;
    }


    {
        uint16_t x = 144;
        x = x + 189;
        if (x != 333) failures++;
    }


    {
        volatile uint8_t port = 150;
        uint8_t r = port;
        if (r != 150) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 8: result = 248; break;
        case 13: result = 67; break;
        case 0: result = 110; break;
        case 19: result = 186; break;
        case 16: result = 49; break;
        case 4: result = 197; break;
        default: result = 165; break;
        }
        if (result != 248) failures++;
    }


    {
        uint8_t buf[8] = {171,144,246,57,100,157,220,221};
        uint8_t *p = buf;
        p += 4;
        if (*p != 100) failures++;
    }


    {
        uint8_t a[6] = {81,83,43,150,50,71};
        if (a[0] != 81) failures++;
    }


    {
        uint8_t input = 13;
        uint8_t result;
        switch (input) {
        case 5: result = 110; break;
        case 9: result = 176; break;
        case 14: result = 63; break;
        case 13: result = 207; break;
        case 8: result = 201; break;
        default: result = 57; break;
        }
        if (result != 207) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(168,13) != 155) failures++;
    }


    {
        int8_t a = -127;
        int8_t b = -50;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -3613;
        volatile int16_t b = 11099;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {85,245,64734,32};
        if (s.c != (uint16_t)64734) failures++;
    }


    {
        uint8_t m[2][3] = {{178,16,92},{163,8,14}};
        if (m[0][0] != 178) failures++;
    }


    {
        uint16_t r = 36348 + 39096 + 2104 + 1453 + 18664 + 26112 + 50210 + 41571;
        if (r != 18950) failures++;
    }


    {
        uint8_t x = 20;
        x <<= 5;
        if (x != 128) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)65) + (uint16_t)54443;
        if (r != 54508) failures++;
    }


    {
        uint16_t x = 11005;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 31231;
        if (read_g16() != 31231) failures++;
    }


    {
        uint16_t r = call6(73,162,255,16,26,67);
        if (r != 599) failures++;
    }


    {
        uint8_t m[3][4] = {{190,208,12,40},{130,87,155,246},{78,22,54,39}};
        if (m[2][0] != 78) failures++;
    }


    {
        uint16_t r = 54128 + 32621 + 12009 + 16209 + 61191 + 42384 + 16537 + 55106;
        if (r != 28041) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(63,10) != 53) failures++;
    }


    {
        uint8_t x = 174;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {108,67,113,46,3,148,145,247};
        uint8_t *p = buf;
        p += 4;
        if (*p != 3) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 29;
        do { cnt++; } while (--k);
        if (cnt != 29) failures++;
    }


    {
        uint8_t x = 102;
        x <<= 7;
        if (x != 0) failures++;
    }


    {
        uint16_t x = 59141;
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
        case 9: result = 135; break;
        case 1: result = 92; break;
        case 2: result = 72; break;
        case 13: result = 23; break;
        case 19: result = 119; break;
        default: result = 162; break;
        }
        if (result != 23) failures++;
    }


    {
        uint16_t x = 18783;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {24,161,7599,225};
        if (s.d != (uint8_t)225) failures++;
    }


    {
        uint8_t v = 74;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t x = 178;
        x = x + 242;
        if (x != 420) failures++;
    }


    {
        if (((uint16_t)139) != 139) failures++;
    }


    {
        uint8_t m[3][2] = {{159,177},{148,99},{179,157}};
        if (m[0][0] != 159) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 1: result = 120; break;
        case 2: result = 43; break;
        case 19: result = 75; break;
        case 12: result = 196; break;
        case 0: result = 218; break;
        default: result = 242; break;
        }
        if (result != 75) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 7; j += 3) sum += j;
        if (sum != 9) failures++;
    }


    {
        g16 = 37700;
        if (read_g16() != 37700) failures++;
    }


    {
        int8_t a = 102;
        int8_t b = -81;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 123;
        int r = (v & 1) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        int8_t a = 33;
        int8_t b = 10;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t a[6] = {234,170,122,254,234,194};
        if (a[5] != 194) failures++;
    }


    {
        uint8_t m[3][4] = {{233,187,237,150},{249,157,151,75},{135,17,32,230}};
        if (m[1][2] != 151) failures++;
    }


    {
        uint8_t m[4][2] = {{208,163},{69,150},{143,185},{186,55}};
        if (m[1][1] != 150) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 11;
        do { cnt++; } while (--k);
        if (cnt != 11) failures++;
    }


    {
        uint16_t r = add2(67,194) + add2(194,215) + add2(67,215);
        if (r != 952) failures++;
    }


    {
        uint8_t v = 210;
        v &= ~(uint8_t)128;
        if (v != 82) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(29,50) != 65515) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {166,192,32387,240};
        if (s.c != (uint16_t)32387) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 8;
        do { cnt++; } while (--k);
        if (cnt != 8) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)46) + (uint16_t)52574;
        if (r != 52620) failures++;
    }


    {
        uint8_t a[6] = {228,57,255,249,111,139};
        if (a[4] != 111) failures++;
    }

    return failures;
}