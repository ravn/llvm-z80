typedef unsigned long u32; typedef unsigned char u8;
int main(void){
  volatile u32 val=0xA5A5A5A5UL; u8 count=0; u32 v=val;
  while(v){ count+=(u8)(v&1U); v>>=1; }
  return count; /* expect 16 */
}
