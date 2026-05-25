/* #189 reduction: i32 conditional xor const, no shift, IY-on. xoronly(0xFF) low16==0x00FF */
typedef unsigned char uint8_t; typedef unsigned long uint32_t;
uint32_t f(uint32_t crc){ for(uint8_t j=0;j<8;j++){ if(crc&1) crc^=0xEDB88320UL; } return crc; }
int main(){ return (int)(f(0xFFUL)&0xFFFFu); } /* expect 0x00FF */
