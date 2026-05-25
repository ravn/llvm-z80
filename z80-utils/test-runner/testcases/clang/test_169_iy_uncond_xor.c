/* #189 reduction: i32 (crc>>1)^const each iter, IY-on. uncond(0xFF) low16==0x2D3D */
typedef unsigned char uint8_t; typedef unsigned long uint32_t;
uint32_t f(uint32_t crc){ for(uint8_t j=0;j<8;j++) crc=(crc>>1)^0xEDB88320UL; return crc; }
int main(){ return (int)(f(0xFFUL)&0xFFFFu); } /* expect 0x2D3D */
