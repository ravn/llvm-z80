/* Test 243 (ravn/llvm-z80#242): -O0 16-bit unsigned/signed compare with a
   zext-from-i8 operand must not be corrupted by the LD L,H;LD H,0;LD A,L peephole.
   At -O0 the zext operand reloads into H; the peephole used to delete the LD H,0
   that the compare's high-byte read depends on, so e.g. 255u > 1u returned false.
   Self-checking: returns 0x0000 iff all forms correct at every opt level. */
typedef unsigned short u16; typedef unsigned char u8;
static volatile u8 v8;
int main(void){
  static const u16 tv[]={0,1,255,128,127,0x80,0x81,254,2,100,200,55};
  u16 fail=0; int i,j;
  for(i=0;i<12;i++){
    v8=(u8)tv[i]; u8 a=v8; unsigned x=a;          /* x = zext(byte) */
    for(j=0;j<12;j++){
      u8 k=(u8)tv[j]; unsigned rk=(u8)tv[j];
      if((x>k)  != ((unsigned)(u8)tv[i] >  rk)) fail|=1<<0;
      if((x<k)  != ((unsigned)(u8)tv[i] <  rk)) fail|=1<<1;
      if((x>=k) != ((unsigned)(u8)tv[i] >= rk)) fail|=1<<2;
      if((x==k) != ((unsigned)(u8)tv[i] == rk)) fail|=1<<3;
    }
  }
  return (int)fail; /* expect 0x0000 */
}
