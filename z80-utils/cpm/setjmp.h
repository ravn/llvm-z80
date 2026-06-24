/* setjmp.h — CP/M/Z80; saves BC DE HL IX IY SP and return address */
#ifndef _CPM_SETJMP_H
#define _CPM_SETJMP_H

/* jmp_buf: [0]=ret-lo [1]=ret-hi [2]=SP-lo [3]=SP-hi
            [4]=BC [5] [6]=DE [7] [8]=HL [9] [10]=IX [11] [12]=IY [13] */
typedef unsigned char jmp_buf[14];

int  setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#endif
