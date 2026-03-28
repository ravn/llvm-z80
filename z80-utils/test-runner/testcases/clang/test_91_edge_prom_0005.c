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
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {172,212,43917,39};
        if (s.a != (uint8_t)172) failures++;
    }


    {
        uint8_t v = 146;
        v &= ~(uint8_t)2;
        if (v != 144) failures++;
    }


    {
        uint8_t m[4][4] = {{115,115,42,223},{119,63,166,0},{49,138,150,147},{201,19,132,138}};
        if (m[0][3] != 223) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-127) / (int16_t)((int8_t)93);
        if ((uint16_t)r != (uint16_t)65535) failures++;
    }


    {
        int8_t a = -85;
        int8_t b = 8;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)18) + (uint16_t)45452;
        if (r != 45470) failures++;
    }


    {
        uint8_t v = 43;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t a[6] = {74,18,207,75,9,225};
        if (a[3] != 75) failures++;
    }


    {
        uint8_t m[4][2] = {{128,67},{169,78},{140,214},{83,45}};
        if (m[1][1] != 78) failures++;
    }


    {
        uint16_t x = 193;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 10; j += 2) sum += j;
        if (sum != 20) failures++;
    }


    {
        uint16_t x = 165;
        x = x + 63;
        if (x != 228) failures++;
    }


    {
        uint8_t v = 207;
        v |= 64;
        if (v != 207) failures++;
    }


    {
        uint16_t r = 50304 + 42901 + 40864 + 31068 + 36390 + 15894 + 37715 + 60389;
        if (r != 53381) failures++;
    }


    {
        uint8_t x = 252;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        volatile int16_t a = 22619;
        volatile int16_t b = -12264;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 85;
        x = x + 7;
        if (x != 92) failures++;
    }


    {
        uint16_t r = 22922 + 44620 + 37549 + 33125 + 14002 + 5256 + 40377 + 49394;
        if (r != 50637) failures++;
    }


    {
        uint8_t x = 3;
        x <<= 1;
        if (x != 6) failures++;
    }


    {
        uint8_t v = 213;
        v |= 128;
        if (v != 213) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(114,213) != 327) failures++;
    }


    {
        uint8_t a[6] = {39,220,201,252,190,242};
        if (a[2] != 201) failures++;
    }


    {
        uint8_t a[6] = {44,86,206,243,101,149};
        if (a[2] != 206) failures++;
    }


    {
        uint8_t src[6] = {147,242,26,101,118,52};
        uint8_t dst[6];
        for (uint8_t j = 0; j < 6; j++) dst[j] = src[j];
        if (dst[0] != 147) failures++;
    }


    {
        g16 = 52022;
        if (read_g16() != 52022) failures++;
    }


    {
        volatile uint8_t port = 178;
        uint8_t r = port;
        if (r != 178) failures++;
    }


    {
        uint32_t a = 3093116091UL;
        uint32_t b = 2140627938UL;
        uint32_t r = a - b;
        if (r != 952488153UL) failures++;
    }


    {
        uint8_t x = 174;
        x <<= 0;
        if (x != 174) failures++;
    }


    {
        uint16_t r = 53541 + 9945 + 23969 + 6943 + 41040 + 20188 + 61895 + 34589;
        if (r != 55502) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)254) + (uint16_t)40733;
        if (r != 40987) failures++;
    }


    {
        g16 = 33195;
        if (read_g16() != 33195) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)89) + (uint16_t)20689;
        if (r != 20778) failures++;
    }


    {
        volatile int16_t a = -14577;
        volatile int16_t b = -13750;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 247;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 1) sum += j;
        if (sum != 91) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 61;
        if (buf[6] != 61) failures++;
    }


    {
        uint32_t a = 2355474510UL;
        uint32_t b = 1859268215UL;
        uint32_t r = a - b;
        if (r != 496206295UL) failures++;
    }


    {
        uint16_t r = 53171 + 59889 + 33436 + 5876 + 22255 + 35749 + 58987 + 60263;
        if (r != 1946) failures++;
    }


    {
        if (((uint16_t)111) != 111) failures++;
    }


    {
        uint16_t r = call6(100,113,15,174,158,64);
        if (r != 624) failures++;
    }


    {
        uint8_t buf[8] = {155,109,254,122,120,168,21,14};
        uint8_t *p = buf;
        p += 2;
        if (*p != 254) failures++;
    }


    {
        uint8_t input = 18;
        uint8_t result;
        switch (input) {
        case 18: result = 218; break;
        case 0: result = 87; break;
        case 19: result = 116; break;
        case 12: result = 178; break;
        default: result = 14; break;
        }
        if (result != 218) failures++;
    }


    {
        uint8_t x = 203;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint8_t v = 199;
        v ^= 32;
        if (v != 231) failures++;
    }


    {
        uint8_t buf[8] = {130,148,160,214,5,224,172,222};
        uint8_t *p = buf;
        p += 1;
        if (*p != 148) failures++;
    }


    {
        g16 = 4584;
        if (read_g16() != 4584) failures++;
    }


    {
        uint16_t r = call6(228,68,24,255,68,141);
        if (r != 784) failures++;
    }


    {
        int8_t a = -49;
        int8_t b = 36;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t m[2][4] = {{94,182,109,181},{219,68,102,253}};
        if (m[1][0] != 219) failures++;
    }


    {
        volatile uint8_t port = 183;
        uint8_t r = port;
        if (r != 183) failures++;
    }


    {
        uint16_t x = 207;
        x = x + 21;
        if (x != 228) failures++;
    }


    {
        int8_t a = -61;
        int8_t b = -3;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[14] = {178,111,36,29,91,149,22,127,120,69,34,154,223,37};
        uint8_t dst[14];
        for (uint8_t j = 0; j < 14; j++) dst[j] = src[j];
        if (dst[5] != 149) failures++;
    }


    {
        uint8_t buf[15];
        for (uint8_t j = 0; j < 15; j++) buf[j] = 132;
        if (buf[14] != 132) failures++;
    }


    {
        uint16_t x = 36;
        x = x + 112;
        if (x != 148) failures++;
    }


    {
        uint8_t buf[8] = {123,130,221,71,14,91,90,101};
        uint8_t *p = buf;
        p += 7;
        if (*p != 101) failures++;
    }


    {
        volatile int16_t a = 22793;
        volatile int16_t b = 17001;
        int r = (a > b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = 18577;
        volatile int16_t b = -39;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 49;
        if (buf[4] != 49) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {127,105,48655,187};
        if (s.a != (uint8_t)127) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        uint8_t v = 98;
        uint16_t count = 0;
        while (!((++v) & 32)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = 18870 + 60493 + 44313 + 62819 + 11890 + 37448 + 56417 + 23846;
        if (r != 53952) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 23;
        if (x != 221) failures++;
    }


    {
        int8_t a = 7;
        int8_t b = 70;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[1] = {121};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 121) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {226,48,8569,158};
        if (s.a != (uint8_t)226) failures++;
    }


    {
        uint8_t buf[5];
        for (uint8_t j = 0; j < 5; j++) buf[j] = 143;
        if (buf[4] != 143) failures++;
    }


    {
        uint16_t r = add2(131,125) + add2(125,43) + add2(131,43);
        if (r != 598) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 11: result = 144; break;
        case 18: result = 75; break;
        case 10: result = 61; break;
        case 16: result = 84; break;
        case 19: result = 233; break;
        case 6: result = 115; break;
        default: result = 42; break;
        }
        if (result != 115) failures++;
    }


    {
        if (((uint16_t)(95 & 81)) != 81) failures++;
    }


    {
        uint16_t x = 172;
        x = x + 212;
        if (x != 384) failures++;
    }


    {
        uint8_t a[6] = {66,226,126,245,120,182};
        if (a[2] != 126) failures++;
    }


    {
        uint16_t r = add2(184,238) + add2(238,47) + add2(184,47);
        if (r != 938) failures++;
    }


    {
        uint8_t buf[8] = {84,30,131,121,41,188,83,164};
        uint8_t *p = buf;
        p += 5;
        if (*p != 188) failures++;
    }


    {
        uint16_t r = call6(59,157,169,31,40,221);
        if (r != 677) failures++;
    }


    {
        uint16_t x = 18971;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile uint8_t port = 191;
        uint8_t r = port;
        if (r != 191) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t m[2][4] = {{147,116,191,27},{92,251,143,73}};
        if (m[1][3] != 73) failures++;
    }


    {
        uint32_t a = 89469630UL;
        uint32_t b = 570121518UL;
        uint32_t r = a | b;
        if (r != 637501374UL) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {85,126,58244,27};
        if (s.b != (uint8_t)126) failures++;
    }


    {
        volatile uint8_t port = 242;
        uint8_t r = port;
        if (r != 242) failures++;
    }


    {
        uint8_t buf[1];
        for (uint8_t j = 0; j < 1; j++) buf[j] = 81;
        if (buf[0] != 81) failures++;
    }


    {
        uint16_t x = 37766;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 237;
        if (buf[13] != 237) failures++;
    }


    {
        uint32_t a = 103243218UL;
        uint32_t b = 3155410686UL;
        uint32_t r = a & b;
        if (r != 67312850UL) failures++;
    }


    {
        uint32_t a = 2063386297UL;
        uint32_t b = 822821821UL;
        uint32_t r = a + b;
        if (r != 2886208118UL) failures++;
    }


    {
        uint16_t r = 19366 + 35775 + 50249 + 31158 + 30730 + 37557 + 10991 + 12639;
        if (r != 31857) failures++;
    }


    {
        uint8_t input = 9;
        uint8_t result;
        switch (input) {
        case 4: result = 254; break;
        case 5: result = 52; break;
        case 18: result = 215; break;
        case 16: result = 110; break;
        case 15: result = 195; break;
        case 9: result = 65; break;
        default: result = 107; break;
        }
        if (result != 65) failures++;
    }


    {
        int8_t a = 46;
        int8_t b = 64;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {100,55,99,157,197,64};
        if (a[4] != 197) failures++;
    }


    {
        uint16_t x = 31349;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t v = 31;
        v &= ~(uint8_t)32;
        if (v != 31) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 14; j += 4) sum += j;
        if (sum != 24) failures++;
    }


    {
        volatile uint8_t port = 173;
        uint8_t r = port;
        if (r != 173) failures++;
    }


    {
        uint16_t r = 17952 + 39163 + 19926 + 15427 + 49631 + 32272 + 3432 + 527;
        if (r != 47258) failures++;
    }


    {
        uint8_t src[7] = {22,150,99,88,209,32,199};
        uint8_t dst[7];
        for (uint8_t j = 0; j < 7; j++) dst[j] = src[j];
        if (dst[5] != 32) failures++;
    }


    {
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 39;
        if (buf[11] != 39) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 68;
        v |= 4;
        if (v != 68) failures++;
    }


    {
        uint8_t a[6] = {233,108,226,43,252,59};
        if (a[3] != 43) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-13) / (int16_t)((int8_t)54);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 183;
        uint8_t r = port;
        if (r != 183) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)244) + (uint16_t)54663;
        if (r != 54907) failures++;
    }


    {
        volatile uint8_t port = 22;
        uint8_t r = port;
        if (r != 22) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)30) + (uint16_t)34304;
        if (r != 34334) failures++;
    }


    {
        uint8_t a[6] = {230,26,161,205,172,212};
        if (a[0] != 230) failures++;
    }


    {
        g16 = 44502;
        if (read_g16() != 44502) failures++;
    }


    {
        uint32_t a = 3848576620UL;
        uint32_t b = 1089131089UL;
        uint32_t r = a | b;
        if (r != 3857636989UL) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)115) / (int16_t)((int8_t)-122);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t m[4][2] = {{201,94},{237,152},{9,62},{235,230}};
        if (m[3][1] != 230) failures++;
    }


    {
        volatile int16_t a = 22132;
        volatile int16_t b = -13066;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint16_t x = 55186;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 98;
        if (buf[3] != 98) failures++;
    }


    {
        volatile uint8_t port = 47;
        uint8_t r = port;
        if (r != 47) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 4;
        do { cnt++; } while (--k);
        if (cnt != 4) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 3: result = 11; break;
        case 8: result = 248; break;
        case 4: result = 14; break;
        case 12: result = 142; break;
        case 7: result = 131; break;
        default: result = 142; break;
        }
        if (result != 248) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 164;
        if (buf[13] != 164) failures++;
    }


    {
        uint8_t buf[8] = {175,23,96,111,163,34,46,218};
        uint8_t *p = buf;
        p += 4;
        if (*p != 163) failures++;
    }


    {
        uint8_t x = 83;
        x <<= 1;
        if (x != 166) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)56) / (int16_t)((int8_t)-37);
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
        uint8_t cnt = 0;
        uint8_t k = 5;
        do { cnt++; } while (--k);
        if (cnt != 5) failures++;
    }


    {
        volatile uint8_t port = 52;
        uint8_t r = port;
        if (r != 52) failures++;
    }


    {
        uint8_t x = 145;
        x <<= 5;
        if (x != 32) failures++;
    }


    {
        volatile uint8_t port = 155;
        uint8_t r = port;
        if (r != 155) failures++;
    }


    {
        uint16_t x = 77;
        x = x + 0;
        if (x != 77) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 15; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {43,177,38542,169};
        if (s.a != (uint8_t)43) failures++;
    }


    {
        uint8_t buf[8] = {38,52,195,155,27,167,161,96};
        uint8_t *p = buf;
        p += 3;
        if (*p != 155) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 11; j += 5) sum += j;
        if (sum != 15) failures++;
    }


    {
        uint8_t v = 98;
        v ^= 128;
        if (v != 226) failures++;
    }


    {
        uint32_t a = 928459970UL;
        uint32_t b = 1095453548UL;
        uint32_t r = a + b;
        if (r != 2023913518UL) failures++;
    }


    {
        uint8_t m[2][2] = {{250,161},{59,164}};
        if (m[1][1] != 164) failures++;
    }


    {
        g16 = 61432;
        if (read_g16() != 61432) failures++;
    }


    {
        uint8_t v = 40;
        v &= ~(uint8_t)1;
        if (v != 40) failures++;
    }


    {
        uint8_t x = 190;
        x <<= 4;
        if (x != 224) failures++;
    }


    {
        g16 = 29931;
        if (read_g16() != 29931) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-42) / (int16_t)((int8_t)-62);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {217,233,47085,192};
        if (s.a != (uint8_t)217) failures++;
    }


    {
        uint16_t x = 110;
        x = x + 114;
        if (x != 224) failures++;
    }


    {
        uint8_t src[1] = {134};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 134) failures++;
    }


    {
        volatile int16_t a = -25003;
        volatile int16_t b = -24735;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[4][2] = {{156,89},{248,118},{99,130},{22,26}};
        if (m[0][1] != 89) failures++;
    }


    {
        uint8_t v = 50;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        if (((uint16_t)165) != 165) failures++;
    }


    {
        uint8_t a[6] = {187,87,212,36,82,61};
        if (a[2] != 212) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 2: result = 33; break;
        case 17: result = 108; break;
        case 8: result = 228; break;
        case 12: result = 76; break;
        case 11: result = 84; break;
        case 19: result = 83; break;
        default: result = 57; break;
        }
        if (result != 57) failures++;
    }


    {
        uint16_t x = 24121;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(255,217) != 38) failures++;
    }


    {
        uint8_t v = 193;
        v |= 4;
        if (v != 197) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-30) / (int16_t)((int8_t)-84);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        volatile uint8_t port = 167;
        uint8_t r = port;
        if (r != 167) failures++;
    }


    {
        if (((uint16_t)(((9 - 181) + (206 | 210)) - 254)) != 65332) failures++;
    }


    {
        uint32_t a = 1570545828UL;
        uint32_t b = 4227131348UL;
        uint32_t r = a | b;
        if (r != 4294764532UL) failures++;
    }


    {
        uint16_t x = 18147;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 55430;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t src[1] = {211};
        uint8_t dst[1];
        for (uint8_t j = 0; j < 1; j++) dst[j] = src[j];
        if (dst[0] != 211) failures++;
    }


    {
        uint16_t r = call6(18,234,221,174,169,9);
        if (r != 825) failures++;
    }


    {
        uint8_t a[6] = {54,210,80,237,221,239};
        if (a[2] != 80) failures++;
    }


    {
        g16 = 28939;
        if (read_g16() != 28939) failures++;
    }


    {
        uint8_t src[8] = {15,166,229,54,228,62,51,81};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[3] != 54) failures++;
    }


    {
        uint8_t a[6] = {142,180,87,73,245,208};
        if (a[0] != 142) failures++;
    }


    {
        int8_t a = -71;
        int8_t b = -62;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        g16 = 1531;
        if (read_g16() != 1531) failures++;
    }


    {
        uint8_t a[6] = {68,213,162,246,12,113};
        if (a[2] != 162) failures++;
    }


    {
        uint16_t r = add2(16,126) + add2(126,174) + add2(16,174);
        if (r != 632) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {108,53,33338,39};
        if (s.d != (uint8_t)39) failures++;
    }


    {
        uint8_t m[4][3] = {{191,227,134},{183,154,128},{37,84,97},{0,215,14}};
        if (m[3][1] != 215) failures++;
    }


    {
        uint16_t r = call6(163,117,160,45,97,154);
        if (r != 736) failures++;
    }


    {
        uint16_t x = 63867;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(253,228,239,140,62,146);
        if (r != 1068) failures++;
    }


    {
        volatile int16_t a = -1104;
        volatile int16_t b = -12484;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 10;
        do { cnt++; } while (--k);
        if (cnt != 10) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {54,142,26,210,114,247,149,19};
        uint8_t *p = buf;
        p += 4;
        if (*p != 114) failures++;
    }


    {
        g16 = 38813;
        if (read_g16() != 38813) failures++;
    }


    {
        if (((uint16_t)156) != 156) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 151;
        uint16_t count = 0;
        while (!((++v) & 2)) count++;
        count++;
        if (count != 3) failures++;
    }


    {
        volatile uint8_t port = 190;
        uint8_t r = port;
        if (r != 190) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        int8_t a = -25;
        int8_t b = -26;
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
        volatile uint8_t port = 81;
        uint8_t r = port;
        if (r != 81) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint8_t src[10] = {69,82,229,95,169,136,171,82,56,86};
        uint8_t dst[10];
        for (uint8_t j = 0; j < 10; j++) dst[j] = src[j];
        if (dst[5] != 136) failures++;
    }


    {
        uint8_t buf[7];
        for (uint8_t j = 0; j < 7; j++) buf[j] = 38;
        if (buf[6] != 38) failures++;
    }


    {
        uint8_t input = 12;
        uint8_t result;
        switch (input) {
        case 9: result = 107; break;
        case 6: result = 244; break;
        case 4: result = 117; break;
        case 12: result = 231; break;
        case 5: result = 58; break;
        case 13: result = 91; break;
        default: result = 138; break;
        }
        if (result != 231) failures++;
    }


    {
        if (((uint16_t)19) != 19) failures++;
    }


    {
        uint8_t v = 163;
        v |= 2;
        if (v != 163) failures++;
    }


    {
        g16 = 3469;
        if (read_g16() != 3469) failures++;
    }


    {
        uint8_t v = 74;
        v ^= 1;
        if (v != 75) failures++;
    }


    {
        uint32_t a = 3552826934UL;
        uint32_t b = 2233647825UL;
        uint32_t r = a | b;
        if (r != 3622033143UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 26;
        v &= ~(uint8_t)16;
        if (v != 10) failures++;
    }


    {
        uint16_t r = add2(215,107) + add2(107,223) + add2(215,223);
        if (r != 1090) failures++;
    }


    {
        uint8_t v = 25;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
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
        uint16_t x = 245;
        x = x + 25;
        if (x != 270) failures++;
    }


    {
        uint32_t a = 2560102158UL;
        uint32_t b = 793484162UL;
        uint32_t r = a ^ b;
        if (r != 3084096652UL) failures++;
    }


    {
        uint8_t x = 90;
        x <<= 2;
        if (x != 104) failures++;
    }


    {
        volatile uint8_t port = 187;
        uint8_t r = port;
        if (r != 187) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {178,152,24745,249};
        if (s.b != (uint8_t)152) failures++;
    }


    {
        uint16_t x = 19375;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)93) / (int16_t)((int8_t)-40);
        if ((uint16_t)r != (uint16_t)65534) failures++;
    }


    {
        uint16_t x = 198;
        x = x + 173;
        if (x != 371) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-36) / (int16_t)((int8_t)46);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(168,78) != 246) failures++;
    }


    {
        uint16_t x = 63562;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[2][2] = {{149,60},{228,239}};
        if (m[1][0] != 228) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 2) sum += j;
        if (sum != 2) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {155,206,28149,56};
        if (s.b != (uint8_t)206) failures++;
    }


    {
        uint16_t r = add2(47,238) + add2(238,150) + add2(47,150);
        if (r != 870) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 17; j += 4) sum += j;
        if (sum != 40) failures++;
    }


    {
        uint16_t x = 173;
        x = x + 226;
        if (x != 399) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)121) % (int16_t)((int8_t)-102);
        if ((uint16_t)r != (uint16_t)19) failures++;
    }


    {
        if (((uint16_t)((201 + (133 | 143)) | ((31 | 237) - 104))) != 479) failures++;
    }


    {
        uint16_t r = add2(143,142) + add2(142,175) + add2(143,175);
        if (r != 920) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(138,109) != 247) failures++;
    }


    {
        uint16_t x = 56821;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t m[3][4] = {{240,155,60,205},{139,21,164,122},{221,211,68,225}};
        if (m[1][0] != 139) failures++;
    }


    {
        int8_t a = 11;
        int8_t b = 106;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        volatile int16_t a = -4951;
        volatile int16_t b = -27467;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 22;
        do { cnt++; } while (--k);
        if (cnt != 22) failures++;
    }


    {
        uint8_t v = 206;
        v ^= 128;
        if (v != 78) failures++;
    }


    {
        volatile uint8_t port = 67;
        uint8_t r = port;
        if (r != 67) failures++;
    }


    {
        volatile uint8_t port = 2;
        uint8_t r = port;
        if (r != 2) failures++;
    }


    {
        uint16_t r = add2(184,238) + add2(238,179) + add2(184,179);
        if (r != 1202) failures++;
    }


    {
        uint8_t buf[8] = {19,16,245,181,26,91,47,241};
        uint8_t *p = buf;
        p += 2;
        if (*p != 245) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 1: result = 90; break;
        case 5: result = 175; break;
        case 16: result = 189; break;
        case 9: result = 217; break;
        case 7: result = 216; break;
        case 10: result = 0; break;
        case 8: result = 20; break;
        default: result = 223; break;
        }
        if (result != 223) failures++;
    }


    {
        uint8_t v = 75;
        int r = (v & 64) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 125;
        uint8_t r = port;
        if (r != 125) failures++;
    }


    {
        int8_t a = 38;
        int8_t b = 36;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)35) / (int16_t)((int8_t)9);
        if ((uint16_t)r != (uint16_t)3) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 64;
        x = x + 37;
        if (x != 101) failures++;
    }


    {
        uint16_t x = 58699;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = call6(75,95,146,43,222,199);
        if (r != 780) failures++;
    }


    {
        int8_t a = 0;
        int8_t b = -22;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)9) + (uint16_t)42249;
        if (r != 42258) failures++;
    }


    {
        uint8_t a[6] = {31,215,94,189,174,60};
        if (a[3] != 189) failures++;
    }


    {
        uint8_t a[6] = {66,75,174,208,96,231};
        if (a[1] != 75) failures++;
    }


    {
        if (((uint16_t)4) != 4) failures++;
    }


    {
        uint16_t x = 170;
        x = x + 130;
        if (x != 300) failures++;
    }


    {
        uint8_t buf[8] = {213,99,49,136,22,183,47,231};
        uint8_t *p = buf;
        p += 7;
        if (*p != 231) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)169) + (uint16_t)41968;
        if (r != 42137) failures++;
    }


    {
        uint16_t x = 204;
        x = x + 255;
        if (x != 459) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-113) / (int16_t)((int8_t)-78);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t v = 131;
        v ^= 64;
        if (v != 195) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {99,223,28847,251};
        if (s.d != (uint8_t)251) failures++;
    }


    {
        uint16_t r = call6(9,51,122,85,164,175);
        if (r != 606) failures++;
    }


    {
        g16 = 61191;
        if (read_g16() != 61191) failures++;
    }


    {
        uint8_t m[3][4] = {{41,157,48,160},{192,15,59,102},{114,167,127,164}};
        if (m[0][2] != 48) failures++;
    }


    {
        uint16_t r = add2(93,87) + add2(87,88) + add2(93,88);
        if (r != 536) failures++;
    }


    {
        uint8_t m[2][2] = {{143,221},{181,138}};
        if (m[0][1] != 221) failures++;
    }


    {
        uint16_t r = 29974 + 10574 + 21190 + 27808 + 37148 + 25925 + 49277 + 26949;
        if (r != 32237) failures++;
    }


    {
        g16 = 7209;
        if (read_g16() != 7209) failures++;
    }


    {
        uint16_t r = 55581 + 33812 + 3357 + 9229 + 59882 + 34589 + 13800 + 61526;
        if (r != 9632) failures++;
    }


    {
        uint8_t v = 174;
        int r = (v & 4) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t r = 5212 + 49456 + 65171 + 18584 + 50685 + 29976 + 54099 + 43761;
        if (r != 54800) failures++;
    }


    {
        volatile int16_t a = -15720;
        volatile int16_t b = 15846;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)122) % (int16_t)((int8_t)2);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 12: result = 179; break;
        case 2: result = 245; break;
        case 16: result = 103; break;
        case 11: result = 19; break;
        default: result = 144; break;
        }
        if (result != 144) failures++;
    }


    {
        if (((uint16_t)(((128 | 191) ^ 214) | ((236 ^ 61) ^ (215 - 21)))) != 123) failures++;
    }


    {
        uint8_t m[2][2] = {{102,19},{13,26}};
        if (m[1][1] != 26) failures++;
    }


    {
        uint8_t v = 148;
        int r = (v & 4) ? 1 : 0;
        if (r != 1) failures++;
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
        uint8_t buf[12];
        for (uint8_t j = 0; j < 12; j++) buf[j] = 140;
        if (buf[11] != 140) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-99) % (int16_t)((int8_t)-126);
        if ((uint16_t)r != (uint16_t)65437) failures++;
    }


    {
        uint32_t a = 3812350039UL;
        uint32_t b = 136599531UL;
        uint32_t r = a ^ b;
        if (r != 3944714172UL) failures++;
    }


    {
        uint8_t a[6] = {135,79,232,237,83,86};
        if (a[2] != 232) failures++;
    }


    {
        uint8_t v = 238;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)46) + (uint16_t)12255;
        if (r != 12301) failures++;
    }


    {
        uint8_t buf[8] = {251,111,92,66,60,130,175,213};
        uint8_t *p = buf;
        p += 6;
        if (*p != 175) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 16: result = 36; break;
        case 8: result = 52; break;
        case 19: result = 178; break;
        case 5: result = 22; break;
        case 4: result = 230; break;
        case 12: result = 247; break;
        case 17: result = 97; break;
        default: result = 237; break;
        }
        if (result != 22) failures++;
    }


    {
        uint8_t src[2] = {49,134};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 134) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 78;
        uint8_t r = port;
        if (r != 78) failures++;
    }


    {
        uint16_t x = 191;
        x = x + 124;
        if (x != 315) failures++;
    }


    {
        uint8_t x = 139;
        x <<= 6;
        if (x != 192) failures++;
    }


    {
        uint8_t src[2] = {43,108};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 43) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)228) + (uint16_t)63651;
        if (r != 63879) failures++;
    }


    {
        uint8_t buf[8] = {230,162,159,48,218,232,75,193};
        uint8_t *p = buf;
        p += 0;
        if (*p != 230) failures++;
    }


    {
        uint16_t x = 18710;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)121) + (uint16_t)55938;
        if (r != 56059) failures++;
    }


    {
        volatile uint8_t port = 24;
        uint8_t r = port;
        if (r != 24) failures++;
    }


    {
        uint16_t x = 53247;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)98) + (uint16_t)2755;
        if (r != 2853) failures++;
    }


    {
        uint8_t a[6] = {203,125,253,222,12,177};
        if (a[3] != 222) failures++;
    }


    {
        uint16_t r = call6(55,216,14,217,243,238);
        if (r != 983) failures++;
    }


    {
        uint8_t v = 252;
        uint16_t count = 0;
        while (!((++v) & 4)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        int8_t a = -16;
        int8_t b = 60;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t v = 123;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = add2(25,65) + add2(65,125) + add2(25,125);
        if (r != 430) failures++;
    }


    {
        uint8_t v = 17;
        v &= ~(uint8_t)1;
        if (v != 16) failures++;
    }


    {
        uint16_t x = 14;
        x = x + 214;
        if (x != 228) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 6; j += 3) sum += j;
        if (sum != 3) failures++;
    }


    {
        uint16_t x = 3616;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 23728;
        if (read_g16() != 23728) failures++;
    }


    {
        uint8_t m[2][3] = {{249,186,78},{170,68,0}};
        if (m[0][2] != 78) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 17;
        do { cnt++; } while (--k);
        if (cnt != 17) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 98;
        v &= ~(uint8_t)1;
        if (v != 98) failures++;
    }


    {
        uint16_t r = 44723 + 3743 + 30611 + 42086 + 4161 + 14882 + 64047 + 14838;
        if (r != 22483) failures++;
    }


    {
        uint16_t x = 202;
        x = x + 153;
        if (x != 355) failures++;
    }


    {
        uint16_t r = call6(161,25,63,176,44,7);
        if (r != 476) failures++;
    }


    {
        uint8_t a[6] = {15,101,194,203,168,146};
        if (a[3] != 203) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)8) + (uint16_t)13400;
        if (r != 13408) failures++;
    }


    {
        uint16_t x = 18873;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t buf[8] = {169,124,44,106,40,2,31,174};
        uint8_t *p = buf;
        p += 5;
        if (*p != 2) failures++;
    }


    {
        volatile int16_t a = -21263;
        volatile int16_t b = -7002;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)129) + (uint16_t)36401;
        if (r != 36530) failures++;
    }


    {
        volatile uint8_t port = 107;
        uint8_t r = port;
        if (r != 107) failures++;
    }


    {
        uint8_t m[2][4] = {{222,108,144,226},{73,127,189,166}};
        if (m[1][2] != 189) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)87) + (uint16_t)11065;
        if (r != 11152) failures++;
    }


    {
        uint8_t x = 8;
        x <<= 2;
        if (x != 32) failures++;
    }


    {
        uint8_t a[6] = {156,148,227,184,111,111};
        if (a[4] != 111) failures++;
    }


    {
        uint8_t v = 164;
        v |= 8;
        if (v != 172) failures++;
    }


    {
        uint32_t a = 1972546800UL;
        uint32_t b = 4232137424UL;
        uint32_t r = a - b;
        if (r != 2035376672UL) failures++;
    }


    {
        uint16_t x = 106;
        x = x + 152;
        if (x != 258) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-105) % (int16_t)((int8_t)57);
        if ((uint16_t)r != (uint16_t)65488) failures++;
    }


    {
        uint16_t r = add2(39,91) + add2(91,8) + add2(39,8);
        if (r != 276) failures++;
    }


    {
        uint16_t r = 12410 + 47269 + 31577 + 7509 + 18268 + 42753 + 33803 + 24488;
        if (r != 21469) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 3; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = call6(25,65,9,183,88,82);
        if (r != 452) failures++;
    }


    {
        uint8_t v = 187;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        g16 = 56766;
        if (read_g16() != 56766) failures++;
    }


    {
        uint16_t r = call6(39,15,135,156,19,122);
        if (r != 486) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 3: result = 210; break;
        case 1: result = 161; break;
        case 2: result = 24; break;
        case 19: result = 165; break;
        default: result = 4; break;
        }
        if (result != 24) failures++;
    }


    {
        if (((uint16_t)130) != 130) failures++;
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
        uint8_t cnt = 0;
        uint8_t k = 12;
        do { cnt++; } while (--k);
        if (cnt != 12) failures++;
    }


    {
        uint8_t x = 210;
        x <<= 2;
        if (x != 72) failures++;
    }


    {
        uint16_t r = call6(66,160,249,14,149,171);
        if (r != 809) failures++;
    }


    {
        uint32_t a = 4245963774UL;
        uint32_t b = 2916374009UL;
        uint32_t r = a + b;
        if (r != 2867370487UL) failures++;
    }


    {
        uint8_t v = 238;
        int r = (v & 1) ? 1 : 0;
        if (r != 0) failures++;
    }


    {
        uint8_t v = 89;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t m[3][3] = {{65,198,177},{207,213,113},{60,187,200}};
        if (m[2][0] != 60) failures++;
    }


    {
        uint16_t r = call6(223,197,4,222,83,202);
        if (r != 931) failures++;
    }


    {
        uint8_t buf[8] = {193,211,53,19,0,58,199,247};
        uint8_t *p = buf;
        p += 5;
        if (*p != 58) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {217,27,52719,111};
        if (s.d != (uint8_t)111) failures++;
    }


    {
        if (((uint16_t)54) != 54) failures++;
    }


    {
        if (((uint16_t)244) != 244) failures++;
    }


    {
        uint32_t a = 2166234264UL;
        uint32_t b = 3382377777UL;
        uint32_t r = a | b;
        if (r != 3382640057UL) failures++;
    }


    {
        uint8_t input = 8;
        uint8_t result;
        switch (input) {
        case 11: result = 13; break;
        case 3: result = 208; break;
        case 6: result = 246; break;
        case 8: result = 85; break;
        default: result = 253; break;
        }
        if (result != 85) failures++;
    }


    {
        uint16_t x = 40519;
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
        int16_t r = (int16_t)((int8_t)-48) / (int16_t)((int8_t)-112);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t src[11] = {129,193,247,117,191,103,178,58,77,191,52};
        uint8_t dst[11];
        for (uint8_t j = 0; j < 11; j++) dst[j] = src[j];
        if (dst[6] != 178) failures++;
    }


    {
        uint16_t r = 36930 + 54902 + 14019 + 43572 + 26302 + 57700 + 38015 + 57417;
        if (r != 1177) failures++;
    }


    {
        uint16_t x = 20194;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        g16 = 34765;
        if (read_g16() != 34765) failures++;
    }


    {
        volatile int16_t a = 31338;
        volatile int16_t b = -1861;
        int r = (a >= b);
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
        uint16_t r = (uint16_t)((uint8_t)67) + (uint16_t)9888;
        if (r != 9955) failures++;
    }


    {
        uint8_t v = 85;
        v &= ~(uint8_t)8;
        if (v != 85) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(162,69) != 93) failures++;
    }


    {
        uint8_t a[6] = {223,160,77,134,177,163};
        if (a[1] != 160) failures++;
    }


    {
        uint8_t m[3][2] = {{237,117},{57,63},{51,128}};
        if (m[1][1] != 63) failures++;
    }


    {
        uint8_t m[3][3] = {{108,218,2},{117,196,173},{5,201,238}};
        if (m[2][1] != 201) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-55) / (int16_t)((int8_t)114);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        uint8_t v = 153;
        v &= ~(uint8_t)4;
        if (v != 153) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)99) + (uint16_t)685;
        if (r != 784) failures++;
    }


    {
        uint8_t buf[2];
        for (uint8_t j = 0; j < 2; j++) buf[j] = 202;
        if (buf[1] != 202) failures++;
    }


    {
        uint8_t a[6] = {83,232,75,153,193,30};
        if (a[0] != 83) failures++;
    }


    {
        uint8_t x = 232;
        x <<= 3;
        if (x != 64) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {2,17,41711,150};
        if (s.d != (uint8_t)150) failures++;
    }


    {
        uint8_t input = 3;
        uint8_t result;
        switch (input) {
        case 13: result = 72; break;
        case 3: result = 44; break;
        case 19: result = 86; break;
        case 7: result = 125; break;
        case 17: result = 155; break;
        case 5: result = 126; break;
        default: result = 208; break;
        }
        if (result != 44) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {202,54,24825,181};
        if (s.d != (uint8_t)181) failures++;
    }


    {
        volatile int16_t a = 9455;
        volatile int16_t b = 15999;
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
        int8_t a = 30;
        int8_t b = 66;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {12,2,159,194,157,151};
        if (a[0] != 12) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 20; j += 2) sum += j;
        if (sum != 90) failures++;
    }


    {
        volatile int16_t a = -10178;
        volatile int16_t b = -6542;
        int r = (a > b);
        if (r != 0) failures++;
    }


    {
        uint32_t a = 2978899846UL;
        uint32_t b = 2038306523UL;
        uint32_t r = a | b;
        if (r != 4194203615UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(110,222) != 332) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 229;
        x = x + 150;
        if (x != 379) failures++;
    }


    {
        uint16_t x = 78;
        x = x + 242;
        if (x != 320) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(122,103) != 225) failures++;
    }


    {
        uint16_t r = 52875 + 44266 + 63321 + 29786 + 23600 + 43537 + 58886 + 64872;
        if (r != 53463) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(250,51) != 301) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 9;
        do { cnt++; } while (--k);
        if (cnt != 9) failures++;
    }


    {
        int8_t a = -11;
        int8_t b = 53;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t src[2] = {171,114};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[1] != 114) failures++;
    }


    {
        uint32_t a = 598630578UL;
        uint32_t b = 1805657096UL;
        uint32_t r = a & b;
        if (r != 597696512UL) failures++;
    }


    {
        uint32_t a = 508938900UL;
        uint32_t b = 661054747UL;
        uint32_t r = a - b;
        if (r != 4142851449UL) failures++;
    }


    {
        uint8_t v = 49;
        v |= 1;
        if (v != 49) failures++;
    }


    {
        volatile uint8_t port = 218;
        uint8_t r = port;
        if (r != 218) failures++;
    }


    {
        volatile int16_t a = -17862;
        volatile int16_t b = 18424;
        int r = (a == b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 37;
        v ^= 1;
        if (v != 36) failures++;
    }


    {
        uint8_t buf[8] = {12,95,107,194,13,218,222,224};
        uint8_t *p = buf;
        p += 2;
        if (*p != 107) failures++;
    }


    {
        uint16_t x = 146;
        x = x + 80;
        if (x != 226) failures++;
    }


    {
        uint16_t x = 202;
        x = x + 181;
        if (x != 383) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(197,90) != 287) failures++;
    }


    {
        uint16_t x = 19482;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(60,0) + add2(0,250) + add2(60,250);
        if (r != 620) failures++;
    }


    {
        g16 = 1200;
        if (read_g16() != 1200) failures++;
    }


    {
        uint8_t src[3] = {199,48,81};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[0] != 199) failures++;
    }


    {
        volatile int16_t a = 29739;
        volatile int16_t b = 23075;
        int r = (a <= b);
        if (r != 0) failures++;
    }


    {
        uint8_t v = 13;
        v |= 32;
        if (v != 45) failures++;
    }


    {
        volatile uint8_t port = 121;
        uint8_t r = port;
        if (r != 121) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(54,59) != 65531) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)230) + (uint16_t)13766;
        if (r != 13996) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t x = 135;
        x = x + 240;
        if (x != 375) failures++;
    }


    {
        uint8_t v = 182;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 2) failures++;
    }


    {
        uint16_t r = add2(99,2) + add2(2,175) + add2(99,175);
        if (r != 552) failures++;
    }


    {
        uint16_t x = 246;
        x = x + 250;
        if (x != 496) failures++;
    }


    {
        uint16_t r = 6782 + 1657 + 38019 + 30044 + 37970 + 24109 + 27859 + 15949;
        if (r != 51317) failures++;
    }


    {
        uint32_t a = 4257586156UL;
        uint32_t b = 2486709437UL;
        uint32_t r = a | b;
        if (r != 4261265405UL) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = 51900 + 54293 + 64977 + 57595 + 45367 + 65300 + 24579 + 55514;
        if (r != 26309) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-16) % (int16_t)((int8_t)-93);
        if ((uint16_t)r != (uint16_t)65520) failures++;
    }


    {
        uint8_t v = 188;
        v |= 128;
        if (v != 188) failures++;
    }


    {
        uint32_t a = 1595643905UL;
        uint32_t b = 1758879859UL;
        uint32_t r = a & b;
        if (r != 1209139201UL) failures++;
    }


    {
        uint8_t buf[8] = {30,195,53,126,95,92,82,202};
        uint8_t *p = buf;
        p += 4;
        if (*p != 95) failures++;
    }


    {
        uint8_t v = 221;
        uint16_t count = 0;
        while (!((++v) & 16)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t v = 94;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 34) failures++;
    }


    {
        uint16_t r = call6(140,162,39,108,203,121);
        if (r != 773) failures++;
    }


    {
        uint32_t a = 2399888910UL;
        uint32_t b = 1295483608UL;
        uint32_t r = a | b;
        if (r != 3477071582UL) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 26;
        do { cnt++; } while (--k);
        if (cnt != 26) failures++;
    }


    {
        volatile uint8_t port = 203;
        uint8_t r = port;
        if (r != 203) failures++;
    }


    {
        uint8_t v = 197;
        uint16_t count = 0;
        while (!((++v) & 128)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        volatile int16_t a = 24302;
        volatile int16_t b = -3605;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        volatile uint8_t port = 241;
        uint8_t r = port;
        if (r != 241) failures++;
    }


    {
        volatile uint8_t port = 218;
        uint8_t r = port;
        if (r != 218) failures++;
    }


    {
        uint8_t src[8] = {157,179,161,167,145,93,176,162};
        uint8_t dst[8];
        for (uint8_t j = 0; j < 8; j++) dst[j] = src[j];
        if (dst[4] != 145) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t buf[8] = {27,66,123,113,65,64,50,101};
        uint8_t *p = buf;
        p += 6;
        if (*p != 50) failures++;
    }


    {
        uint8_t input = 99;
        uint8_t result;
        switch (input) {
        case 10: result = 227; break;
        case 18: result = 149; break;
        case 0: result = 111; break;
        default: result = 126; break;
        }
        if (result != 126) failures++;
    }


    {
        uint8_t x = 20;
        x <<= 3;
        if (x != 160) failures++;
    }


    {
        uint16_t x = 112;
        x = x + 8;
        if (x != 120) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 12; j += 3) sum += j;
        if (sum != 18) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 1;
        do { cnt++; } while (--k);
        if (cnt != 1) failures++;
    }


    {
        uint8_t a[6] = {202,7,190,232,78,158};
        if (a[3] != 232) failures++;
    }


    {
        uint8_t v = 224;
        v &= ~(uint8_t)128;
        if (v != 96) failures++;
    }


    {
        uint16_t x = 177;
        x = x + 238;
        if (x != 415) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)221) + (uint16_t)30189;
        if (r != 30410) failures++;
    }


    {
        if (((uint16_t)175) != 175) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 4; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint32_t a = 4129924523UL;
        uint32_t b = 2521627787UL;
        uint32_t r = a - b;
        if (r != 1608296736UL) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(177,218) != 65495) failures++;
    }


    {
        volatile uint8_t port = 229;
        uint8_t r = port;
        if (r != 229) failures++;
    }


    {
        uint8_t a[6] = {159,127,75,35,245,241};
        if (a[1] != 127) failures++;
    }


    {
        uint16_t x = 47023;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {103,100,1839,14};
        if (s.b != (uint8_t)100) failures++;
    }


    {
        uint8_t input = 6;
        uint8_t result;
        switch (input) {
        case 6: result = 231; break;
        case 17: result = 102; break;
        case 10: result = 23; break;
        default: result = 169; break;
        }
        if (result != 231) failures++;
    }


    {
        uint8_t buf[14];
        for (uint8_t j = 0; j < 14; j++) buf[j] = 193;
        if (buf[13] != 193) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)245) + (uint16_t)55132;
        if (r != 55377) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 15;
        do { cnt++; } while (--k);
        if (cnt != 15) failures++;
    }


    {
        uint8_t m[2][2] = {{8,120},{97,18}};
        if (m[1][0] != 97) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)37) / (int16_t)((int8_t)88);
        if ((uint16_t)r != (uint16_t)0) failures++;
    }


    {
        g16 = 8026;
        if (read_g16() != 8026) failures++;
    }


    {
        g16 = 58178;
        if (read_g16() != 58178) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile int16_t a = 21074;
        volatile int16_t b = 26801;
        int r = (a >= b);
        if (r != 0) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-126) % (int16_t)((int8_t)84);
        if ((uint16_t)r != (uint16_t)65494) failures++;
    }


    {
        uint8_t m[2][3] = {{26,167,97},{50,228,132}};
        if (m[1][1] != 228) failures++;
    }


    {
        uint8_t v = 2;
        v ^= 2;
        if (v != 0) failures++;
    }


    {
        uint8_t m[2][3] = {{128,182,107},{168,65,216}};
        if (m[1][2] != 216) failures++;
    }


    {
        if (((uint16_t)(((122 & 140) & 157) ^ 211)) != 219) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-113) % (int16_t)((int8_t)47);
        if ((uint16_t)r != (uint16_t)65517) failures++;
    }


    {
        uint8_t m[2][2] = {{50,38},{56,113}};
        if (m[1][0] != 56) failures++;
    }


    {
        uint8_t a[6] = {156,182,131,43,49,214};
        if (a[3] != 43) failures++;
    }


    {
        uint16_t r = call6(187,25,34,75,15,93);
        if (r != 429) failures++;
    }


    {
        uint8_t input = 16;
        uint8_t result;
        switch (input) {
        case 16: result = 184; break;
        case 11: result = 46; break;
        case 7: result = 57; break;
        case 5: result = 43; break;
        case 0: result = 128; break;
        case 19: result = 118; break;
        case 18: result = 146; break;
        case 15: result = 33; break;
        default: result = 17; break;
        }
        if (result != 184) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {79,85,21240,76};
        if (s.b != (uint8_t)85) failures++;
    }


    {
        uint16_t x = 105;
        x = x + 12;
        if (x != 117) failures++;
    }


    {
        uint16_t r = 36922 + 44182 + 728 + 7801 + 57472 + 14415 + 34170 + 29950;
        if (r != 29032) failures++;
    }


    {
        volatile uint8_t port = 238;
        uint8_t r = port;
        if (r != 238) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)115) / (int16_t)((int8_t)-32);
        if ((uint16_t)r != (uint16_t)65533) failures++;
    }


    {
        uint16_t r = add2(82,8) + add2(8,80) + add2(82,80);
        if (r != 340) failures++;
    }


    {
        uint16_t x = 160;
        x = x + 67;
        if (x != 227) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {186,112,59707,34};
        if (s.d != (uint8_t)34) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint8_t v = 186;
        uint16_t count = 0;
        while (!((++v) & 64)) count++;
        count++;
        if (count != 6) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)73) % (int16_t)((int8_t)-71);
        if ((uint16_t)r != (uint16_t)2) failures++;
    }


    {
        uint16_t r = add2(147,110) + add2(110,165) + add2(147,165);
        if (r != 844) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 3) sum += j;
        if (sum != 0) failures++;
    }


    {
        volatile uint8_t port = 167;
        uint8_t r = port;
        if (r != 167) failures++;
    }


    {
        uint8_t buf[8] = {220,170,30,214,126,224,85,95};
        uint8_t *p = buf;
        p += 3;
        if (*p != 214) failures++;
    }


    {
        uint16_t r = add2(152,139) + add2(139,229) + add2(152,229);
        if (r != 1040) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        volatile uint8_t port = 186;
        uint8_t r = port;
        if (r != 186) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)252) + (uint16_t)3253;
        if (r != 3505) failures++;
    }


    {
        uint8_t input = 2;
        uint8_t result;
        switch (input) {
        case 17: result = 1; break;
        case 10: result = 27; break;
        case 2: result = 188; break;
        default: result = 149; break;
        }
        if (result != 188) failures++;
    }


    {
        uint8_t m[4][3] = {{59,66,146},{92,162,219},{232,181,244},{98,154,112}};
        if (m[1][2] != 219) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(66,64) != 2) failures++;
    }


    {
        uint16_t x = 41012;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 19;
        x = x + 72;
        if (x != 91) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 15: result = 249; break;
        case 5: result = 81; break;
        case 19: result = 11; break;
        case 4: result = 146; break;
        case 11: result = 132; break;
        case 17: result = 31; break;
        case 9: result = 32; break;
        default: result = 234; break;
        }
        if (result != 81) failures++;
    }


    {
        int8_t a = -39;
        int8_t b = 52;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint8_t a[6] = {96,48,160,167,225,43};
        if (a[4] != 225) failures++;
    }


    {
        uint8_t a[6] = {116,19,160,65,35,83};
        if (a[3] != 65) failures++;
    }


    {
        uint16_t x = 38118;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t input = 4;
        uint8_t result;
        switch (input) {
        case 6: result = 198; break;
        case 5: result = 151; break;
        case 11: result = 232; break;
        case 1: result = 177; break;
        case 4: result = 81; break;
        case 10: result = 157; break;
        case 3: result = 29; break;
        default: result = 111; break;
        }
        if (result != 81) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {175,61,3891,169};
        if (s.a != (uint8_t)175) failures++;
    }


    {
        uint8_t buf[4];
        for (uint8_t j = 0; j < 4; j++) buf[j] = 90;
        if (buf[3] != 90) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)53) % (int16_t)((int8_t)-15);
        if ((uint16_t)r != (uint16_t)8) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 2;
        do { cnt++; } while (--k);
        if (cnt != 2) failures++;
    }


    {
        uint8_t x = 177;
        x <<= 7;
        if (x != 128) failures++;
    }


    {
        uint16_t x = 4882;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint8_t x = 57;
        x <<= 6;
        if (x != 64) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 21;
        do { cnt++; } while (--k);
        if (cnt != 21) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(133,118) != 251) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {21,185,51632,60};
        if (s.b != (uint8_t)185) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {14,37,51422,42};
        if (s.c != (uint16_t)51422) failures++;
    }


    {
        uint16_t r = add2(186,65) + add2(65,4) + add2(186,4);
        if (r != 510) failures++;
    }


    {
        uint8_t m[3][3] = {{249,123,83},{109,222,75},{157,231,176}};
        if (m[1][0] != 109) failures++;
    }


    {
        uint16_t r = 18297 + 9486 + 2155 + 59580 + 22648 + 41272 + 55255 + 49399;
        if (r != 61484) failures++;
    }


    {
        volatile int16_t a = -27567;
        volatile int16_t b = 15768;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {183,141,28019,43};
        if (s.a != (uint8_t)183) failures++;
    }


    {
        if (((uint16_t)(((228 ^ 178) + 123) + 215)) != 424) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {43,91,65258,99};
        if (s.c != (uint16_t)65258) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {199,91,64957,158};
        if (s.a != (uint8_t)199) failures++;
    }


    {
        volatile uint8_t port = 89;
        uint8_t r = port;
        if (r != 89) failures++;
    }


    {
        uint8_t src[3] = {180,133,216};
        uint8_t dst[3];
        for (uint8_t j = 0; j < 3; j++) dst[j] = src[j];
        if (dst[1] != 133) failures++;
    }


    {
        uint16_t x = 0x00FF;
        uint16_t y = x + 1;
        volatile uint8_t t = 0;
        uint16_t z = y + 1;

        if (z != 0x0101) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)211) + (uint16_t)15677;
        if (r != 15888) failures++;
    }


    {
        g16 = 37499;
        if (read_g16() != 37499) failures++;
    }


    {
        if (((uint16_t)233) != 233) failures++;
    }


    {
        uint8_t input = 7;
        uint8_t result;
        switch (input) {
        case 19: result = 186; break;
        case 14: result = 70; break;
        case 2: result = 160; break;
        case 7: result = 89; break;
        case 16: result = 189; break;
        default: result = 202; break;
        }
        if (result != 89) failures++;
    }


    {
        int8_t a = 2;
        int8_t b = -20;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        int8_t a = 58;
        int8_t b = -97;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        volatile int16_t a = 22180;
        volatile int16_t b = -10515;
        int r = (a != b);
        if (r != 1) failures++;
    }


    {
        uint8_t buf[8] = {161,160,235,219,201,185,138,7};
        uint8_t *p = buf;
        p += 1;
        if (*p != 160) failures++;
    }


    {
        uint8_t a[6] = {153,36,130,81,33,125};
        if (a[3] != 81) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(187,81) != 106) failures++;
    }


    {
        uint16_t x = 90;
        x = x + 46;
        if (x != 136) failures++;
    }


    {
        g16 = 55005;
        if (read_g16() != 55005) failures++;
    }


    {
        uint16_t x = 82;
        x = x + 38;
        if (x != 120) failures++;
    }


    {
        volatile uint8_t port = 147;
        uint8_t r = port;
        if (r != 147) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)196) + (uint16_t)23835;
        if (r != 24031) failures++;
    }


    {
        int8_t a = 83;
        int8_t b = -19;
        int r = (a < b);
        if (r != 0) failures++;
    }


    {
        uint8_t m[2][4] = {{174,229,176,198},{142,171,159,116}};
        if (m[0][3] != 198) failures++;
    }


    {
        uint16_t x = 7742;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t r = add2(11,103) + add2(103,145) + add2(11,145);
        if (r != 518) failures++;
    }


    {
        uint16_t x = 64890;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {104,77,17354,5};
        if (s.b != (uint8_t)77) failures++;
    }


    {
        volatile uint8_t port = 120;
        uint8_t r = port;
        if (r != 120) failures++;
    }


    {
        uint16_t r = 11882 + 3846 + 61094 + 44288 + 34037 + 35188 + 38875 + 50563;
        if (r != 17629) failures++;
    }


    {
        if (((uint16_t)(((230 - 176) + 242) - ((209 ^ 132) | 109))) != 171) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 4) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint16_t r = add2(71,200) + add2(200,106) + add2(71,106);
        if (r != 754) failures++;
    }


    {
        g16 = 30127;
        if (read_g16() != 30127) failures++;
    }


    {
        uint8_t buf[8] = {138,6,70,36,56,249,197,235};
        uint8_t *p = buf;
        p += 6;
        if (*p != 197) failures++;
    }


    {
        uint16_t r = call6(225,53,36,185,160,90);
        if (r != 749) failures++;
    }


    {
        uint8_t cnt = 0;
        uint8_t k = 20;
        do { cnt++; } while (--k);
        if (cnt != 20) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {16,70,40003,218};
        if (s.c != (uint16_t)40003) failures++;
    }


    {
        uint16_t x = 43187;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t x = 42636;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        volatile int16_t a = 7513;
        volatile int16_t b = 369;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 5;
        uint8_t result;
        switch (input) {
        case 18: result = 244; break;
        case 16: result = 1; break;
        case 9: result = 102; break;
        case 5: result = 227; break;
        default: result = 148; break;
        }
        if (result != 227) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 2; j += 5) sum += j;
        if (sum != 0) failures++;
    }


    {
        struct { uint8_t a; uint8_t b; uint16_t c; uint8_t d; } s = {37,79,33999,227};
        if (s.b != (uint8_t)79) failures++;
    }


    {
        uint8_t input = 19;
        uint8_t result;
        switch (input) {
        case 17: result = 111; break;
        case 6: result = 68; break;
        case 10: result = 43; break;
        case 19: result = 220; break;
        case 5: result = 234; break;
        case 4: result = 136; break;
        default: result = 101; break;
        }
        if (result != 220) failures++;
    }


    {
        uint8_t x = 102;
        x <<= 4;
        if (x != 96) failures++;
    }


    {
        uint16_t x = 0;
        x = x + 204;
        if (x != 204) failures++;
    }


    {
        uint16_t x = 33;
        x = x + 248;
        if (x != 281) failures++;
    }


    {
        int8_t a = -103;
        int8_t b = -99;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t r = call6(115,175,195,63,3,72);
        if (r != 623) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)67) + (uint16_t)8311;
        if (r != 8378) failures++;
    }


    {
        uint8_t buf[8] = {68,170,104,209,136,2,95,253};
        uint8_t *p = buf;
        p += 5;
        if (*p != 2) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(14,67) != 65483) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(225,198) != 423) failures++;
    }


    {
        uint8_t a[6] = {230,238,65,105,4,74};
        if (a[4] != 4) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &add2;
        if (fn(45,5) != 50) failures++;
    }


    {
        volatile int16_t a = 18193;
        volatile int16_t b = -21976;
        int r = (a >= b);
        if (r != 1) failures++;
    }


    {
        int8_t a = -87;
        int8_t b = 32;
        int r = (a < b);
        if (r != 1) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(35,210) != 65361) failures++;
    }


    {
        volatile uint8_t port = 141;
        uint8_t r = port;
        if (r != 141) failures++;
    }


    {
        uint8_t v = 0;
        uint16_t count = 0;
        while (!((++v) & 1)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint16_t r = 13531 + 32770 + 29857 + 7549 + 63251 + 6196 + 502 + 12371;
        if (r != 34955) failures++;
    }


    {
        uint8_t v = 253;
        int r = (v & 4) ? 1 : 0;
        if (r != 1) failures++;
    }


    {
        uint16_t x = 53;
        x = x + 151;
        if (x != 204) failures++;
    }


    {
        uint8_t x = 0;
        x <<= 6;
        if (x != 0) failures++;
    }


    {
        uint8_t v = 173;
        uint16_t count = 0;
        while (!((++v) & 8)) count++;
        count++;
        if (count != 1) failures++;
    }


    {
        uint8_t x = 114;
        x <<= 6;
        if (x != 128) failures++;
    }


    {
        uint16_t r = call6(200,132,191,65,29,60);
        if (r != 677) failures++;
    }


    {
        uint16_t x = 14329;
        uint16_t *p = &x;

        uint16_t a = *p;
        uint16_t b = a + 1;
        uint16_t c = *p;

        if (a != c) failures++;
    }


    {
        uint16_t sum = 0;
        for (uint16_t j = 0; j < 1; j += 2) sum += j;
        if (sum != 0) failures++;
    }


    {
        uint8_t v = 229;
        v ^= 1;
        if (v != 228) failures++;
    }


    {
        uint16_t r = (uint16_t)((uint8_t)225) + (uint16_t)28373;
        if (r != 28598) failures++;
    }


    {
        uint16_t (*fn)(uint16_t, uint16_t) = &sub2;
        if (fn(167,148) != 19) failures++;
    }


    {
        int16_t r = (int16_t)((int8_t)-83) / (int16_t)((int8_t)-58);
        if ((uint16_t)r != (uint16_t)1) failures++;
    }


    {
        uint8_t src[2] = {221,158};
        uint8_t dst[2];
        for (uint8_t j = 0; j < 2; j++) dst[j] = src[j];
        if (dst[0] != 221) failures++;
    }


    {
        uint8_t m[2][3] = {{84,245,13},{171,230,104}};
        if (m[0][1] != 245) failures++;
    }


    {
        uint8_t buf[8] = {205,238,226,179,134,33,13,112};
        uint8_t *p = buf;
        p += 2;
        if (*p != 226) failures++;
    }


    {
        volatile int16_t a = -14141;
        volatile int16_t b = 4101;
        int r = (a <= b);
        if (r != 1) failures++;
    }


    {
        uint8_t input = 11;
        uint8_t result;
        switch (input) {
        case 5: result = 133; break;
        case 17: result = 145; break;
        case 11: result = 179; break;
        case 2: result = 75; break;
        case 16: result = 252; break;
        default: result = 16; break;
        }
        if (result != 179) failures++;
    }

    return failures;
}