/*
  Copyright 2002 Colin Percival and the University of Oxford

  Copyright in this software is held by the University of Oxford
  and Colin Percival. Both the University of Oxford and Colin Percival
  have agreed that this software may be distributed free of charge and
  in accordance with the attached licence.

  The above notwithstanding, in order that the University as a
  charitable foundation protects its assets for the benefit of its
  educational and research purposes, the University makes clear that no
  condition is made or to be implied, nor is any warranty given or to
  be implied, as to the proper functioning of this work, or that it
  will be suitable for any particular purpose or for use under any
  specific conditions.  Furthermore, the University disclaims all
  responsibility for any use which is made of this work.

  For the terms under which this work may be distributed, please see
  the adjoining file "LICENSE".
*/
#ifdef RCSID
static const char rcsid[] =
  "$Id: hash.c,v 1.4 2002/04/25 06:14:31 cperciva Exp $";
#endif

#include "ptypes.h"
#include "hash.h"
#include "hashtab.h"

#include <string.h>

#define F(x,y,z) ((x&y)|(~x&z))
#define G(x,y,z) ((z&x)|(~z&y))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y^(x|~z))

#define ROL(x,n) (x<<n|x>>(32-n))

#define OP(a,b,c,d,FF,x,s,sc) a+=FF(b,c,d)+x+sc;a=b+ROL(a,s);

#define OP4(a,b,c,d,FF,t,x0,s0,x1,s1,x2,s2,x3,s3) \
    OP(a,b,c,d,FF,x0,s0,hashtab_T[t+0]); \
    OP(d,a,b,c,FF,x1,s1,hashtab_T[t+1]); \
    OP(c,d,a,b,FF,x2,s2,hashtab_T[t+2]); \
    OP(b,c,d,a,FF,x3,s3,hashtab_T[t+3]);

static void hash_block(uint32 * state,uint8 * block)
{
    uint32 a,b,c,d;
    uint32 x[16];
    int i;

    for(i=0;i<16;i++) x[i]=int32_little(*(uint32 *)(block+i*4));

    a=state[0];b=state[1];c=state[2];d=state[3];

    OP4(a,b,c,d,F, 0, x[ 0], 7, x[ 1],12, x[ 2],17, x[ 3],22);
    OP4(a,b,c,d,F, 4, x[ 4], 7, x[ 5],12, x[ 6],17, x[ 7],22);
    OP4(a,b,c,d,F, 8, x[ 8], 7, x[ 9],12, x[10],17, x[11],22);
    OP4(a,b,c,d,F,12, x[12], 7, x[13],12, x[14],17, x[15],22);

    OP4(a,b,c,d,G,16, x[ 1], 5, x[ 6], 9, x[11],14, x[ 0],20);
    OP4(a,b,c,d,G,20, x[ 5], 5, x[10], 9, x[15],14, x[ 4],20);
    OP4(a,b,c,d,G,24, x[ 9], 5, x[14], 9, x[ 3],14, x[ 8],20);
    OP4(a,b,c,d,G,28, x[13], 5, x[ 2], 9, x[ 7],14, x[12],20);

    OP4(a,b,c,d,H,32, x[ 5], 4, x[ 8],11, x[11],16, x[14],23);
    OP4(a,b,c,d,H,36, x[ 1], 4, x[ 4],11, x[ 7],16, x[10],23);
    OP4(a,b,c,d,H,40, x[13], 4, x[ 0],11, x[ 3],16, x[ 6],23);
    OP4(a,b,c,d,H,44, x[ 9], 4, x[12],11, x[15],16, x[ 2],23);

    OP4(a,b,c,d,I,48, x[ 0], 6, x[ 7],10, x[14],15, x[ 5],21);
    OP4(a,b,c,d,I,52, x[12], 6, x[ 3],10, x[10],15, x[ 1],21);
    OP4(a,b,c,d,I,56, x[ 8], 6, x[15],10, x[ 6],15, x[13],21);
    OP4(a,b,c,d,I,60, x[ 4], 6, x[11],10, x[ 2],15, x[ 9],21);

    state[0]+=a;state[1]+=b;state[2]+=c;state[3]+=d;

    memset(x,0,64);
}

void hash_data(uint8 * data,uint32 len,uint8 * digest)
{
    uint32 state[4];
    uint8 buffer[64];
    uint32 block;

    state[0]=0x67452301;
    state[1]=0xefcdab89;
    state[2]=0x98badcfe;
    state[3]=0x10325476;

    for(block=0;block<len>>6;block++) {
        hash_block(state,data+block*64);
    };

    memcpy(buffer,data+block*64,len & 0x3F);
    if((len&0x3F)<56) {
        memset(buffer+(len&0x3F),0,56-(len&0x3F));
        buffer[len&0x3F]=0x80;
        *(uint32 *)(&buffer[56])=int32_little(len<<3);
        *(uint32 *)(&buffer[60])=int32_little(len>>29);
        hash_block(state,buffer);
        memset(buffer,0,64);
    } else {
        memset(buffer+(len&0x3F),0,64-(len&0x3F));
        buffer[len&0x3F]=0x80;
        hash_block(state,buffer);
        memset(buffer,0,56);
        *(uint32 *)(&buffer[56])=int32_little(len<<3);
        *(uint32 *)(&buffer[60])=int32_little(len>>29);
        hash_block(state,buffer);
    };

    *(uint32 *)(digest)=int32_little(state[0]);
    *(uint32 *)(digest+4)=int32_little(state[1]);
    *(uint32 *)(digest+8)=int32_little(state[2]);
    *(uint32 *)(digest+12)=int32_little(state[3]);
}

