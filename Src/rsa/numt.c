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
  "$Id: numt.c,v 1.4 2002/04/25 06:13:52 cperciva Exp $";
#endif

#include "ptypes.h"
#include "numt.h"
#include "smpa.h"

#define BIT(x,n) ((x[(n)/8]>>((n)%8))&0x01)

void numt_powmod(uint8 * x,uint8 * n,uint8 * m,uint8 * y,
                        double * T1,uint32 len,uint32 hibit,uint32 lobit)
{
    uint32 j;

    smpa_init(T1,len);

    smpa_in(x,T1+len*2,len);

    for(j=1;j<len;j++) *(y+j)=0;*y=1;
    smpa_in(y,T1+len*1,len);
    for(j=0;j<len;j++) *(T1+len*3+j)=*(T1+len*(1+BIT(n,hibit))+j);

    smpa_premul(T1+len*1,len,T1);
    smpa_premul(T1+len*2,len,T1);

    smpa_in(m,T1+len*4,len);
    smpa_prediv(T1+len*4,T1+len*5,T1+len*6,T1+len*7,len,T1);

    for(j=hibit;j>lobit;j--) {
        smpa_premul(T1+len*3,len,T1);
        smpa_mulmod(T1+len*3,T1+len*3,T1+len*3,
                    T1+len*4,T1+len*5,T1+len*6,
                    T1+len*7,len,T1);
        smpa_premul(T1+len*3,len,T1);
        smpa_mulmod(T1+len*(1+BIT(n,j-1)),T1+len*3,T1+len*3,
                    T1+len*4,T1+len*5,T1+len*6,
                    T1+len*7,len,T1);
    };

    smpa_out(T1+len*3,y,len);
}

static int numt_spsp(uint8 * a,uint8 * p,double * T1,uint8 * T2,uint32 len)
{
    uint32 i,j;
    uint8 r1;
    int r;

    r=1;j=0;
    for(i=1;i<len*8;i++) {
        j+=i*r*BIT(p,i);
        r-=r*BIT(p,i);
    };

    numt_powmod(a,p,p,T2,T1,len,len*8-1,j);

    smpa_init(T1,len);
    smpa_in(T2,T1+len*3,len);
    smpa_in(p,T1+len*4,len);
    smpa_prediv(T1+len*4,T1+len*5,T1+len*6,T1+len*7,len,T1);

    r1=T2[0]-1;
    for(i=1;i<len;i++) r1|=T2[i];
    // r1 = 0 <==> T2 = 1
    r=r1;

    for(;j>0;j--) {
        r1=T2[0]+1-p[0];
        for(i=1;i<len;i++) r1|=T2[i]-p[i];
        // r1 = 0 <==> T2+1 = p
        r=r*r1%257;

        smpa_premul(T1+len*3,len,T1);
        smpa_mulmod(T1+len*3,T1+len*3,T1+len*3,
                    T1+len*4,T1+len*5,T1+len*6,
                    T1+len*7,len,T1);
        smpa_out(T1+len*3,T2,len);
    };

    return r;
}

static uint8 prim32dat[] = {2,0,0,0,3,0,0,0,5,0,0,0,7,0,0,0,11,0,0,0};

static void numt_prim32(uint8 * r,uint8 * p,double * T1)
{
    uint32 i;
    int r1;

    for(i=0;i<4;i++) p[i]=r[i];
    p[0]|=0x01;p[3]|=0x80;

    do {
        i=int32_little(*(uint32 *)(p))+2;
        *(uint32 *)(p)=int32_little(i|0x80000000);
        r1=0;
        for(i=0;i<5;i++) r1+=numt_spsp(prim32dat+4*i,p,T1,r,4);
    } while(r1);
}

static void numt_primap(uint8 * p1,uint8 * m,uint8 * a,uint8 * b,uint8 * p,double * T1,uint8 * T2,uint32 len)
{
    uint32 j;
    uint32 c;
    uint8 r1;

    do {
        smpa_init(T1,len/2);

        do {
            c=1;
            for(j=0;j<len/2;j++) {
                c+=m[j];
                m[j]=c%256;
                c=c/256;
            };

            smpa_in(a,T1+len/2,len/2);
            smpa_premul(T1+len/2,len/2,T1);

            smpa_in(m,T1+len,len/2);
            smpa_premul(T1+len,len/2,T1);

            smpa_mul(T1+len/2,T1+len,T1+len/2,len/2,T1);

            smpa_in(b,T1+len,len/2);
            smpa_add(T1+len/2,T1+len,T1+len/2,len/2);
            smpa_out(T1+len/2,T2,len/2);

            smpa_in(p1,T1+len,len/2);
            smpa_premul(T1+len,len/2,T1);

            smpa_premul(T1+len/2,len/2,T1);
            smpa_mul(T1+len/2,T1+len,T1+len/2,len/2,T1);
            smpa_doubleout(T1+len/2,p,len/2);

            if(p[len-1]&0x80) {
                for(j=0;j<len/2-1;j++) m[j]=(m[j]>>1)|(m[j+1]<<7);
                m[len/2-1]=m[len/2-1]>>1;
            };
        } while(p[len-1]&0x80);

        for(j=len-1;j>0;j--) p[j]=(p[j]<<1)|(p[j-1]>>7);
        p[0]=(p[0]<<1)+1;

        for(j=0;j<len;j++) T2[len+j]=0;
        T2[len]=4;

        numt_powmod(T2+len,p,p,T2+len*2,T1,len,len*8-1,1);

        r1=T2[len*2]-1;
        for(j=1;j<len;j++) r1|=T2[len*2+j];
        if(r1) continue;

        numt_powmod(T2+len,T2,p,T2+len*2,T1,len,len*4-1,0);

        r1=T2[len*2]-1;
        for(j=1;j<len;j++) r1|=T2[len*2+j];
        if(!r1) {r1=1;continue;};

        c=0;
        for(j=0;j<len;j++) {
            c+=T2[len*2+j]+255;
            T2[len*2+j]=c%256;
            c=c/256;
        };

        smpa_in(p,T1+len*0,len);
        smpa_in(T2+len*2,T1+len*1,len);
        smpa_gcd(T1+len*0,T1+len*1,
                 T1+len*2,T1+len*3,T1+len*4,
                 T1+len*5,len);
        smpa_out(T1+len*2,T2+len*2,len);

        r1=T2[len*2]-1;
        for(j=1;j<len;j++) r1|=T2[len*2+j];
    } while(r1);
}

void numt_prim(uint8 * r,uint8 * p,double * T1,uint8 * T2,uint32 len)
{
    uint32 j;

    if(len==4) {
        numt_prim32(r,p,T1);
        return;
    };

    numt_prim(r,T2,T1,T2+len/2,len/2);

    for(j=0;j<len/2-1;j++) r[j]=0;r[len/2-1]=0x80;
    r[len-1]|=0x80;

    smpa_init(T1,len/2);
    smpa_in(T2,T1+len/2,len/2);
    smpa_prediv(T1+len/2,T1+len,T1+3*len/2,T1+len*2,len/2,T1);
    smpa_in(r,T1+4*len/2,len/2);
    smpa_in(r+len/2,T1+5*len/2,len/2);
    smpa_premul(T1+4*len/2,len/2,T1);
    smpa_premul(T1+5*len/2,len/2,T1);
    smpa_mul(T1+4*len/2,T1+5*len/2,T1+4*len/2,len/2,T1);

    smpa_div(T1+4*len/2,T1+6*len/2,T1+7*len/2,
             T1+len/2,T1+2*len/2,T1+3*len/2,
             T1+5*len/2,len/2,T1);

    smpa_out(T1+6*len/2,T2+len/2,len/2);
    T2[len]=1;for(j=len+1;j<2*len;j++) T2[j]=0;

    numt_primap(T2,T2+len/2,T2+len,T2+3*len/2,p,T1,T2+len*2,len);
}

void numt_sprim(uint8 * r,uint8 * p,double * T1,uint8 * T2,uint32 len)
{
    uint32 j;
    uint32 x;

    numt_prim(r,T2,T1,T2+len/2,len/2);

    for(j=0;j<len/2-1;j++) r[j]=0;r[len/2-1]=0x80;
    r[len-1]=(r[len-1]|0xC0)-1;

    smpa_init(T1,len/2);
    smpa_in(T2,T1+len/2,len/2);
    smpa_prediv(T1+len/2,T1+len,T1+3*len/2,T1+len*2,len/2,T1);
    smpa_in(r,T1+4*len/2,len/2);
    smpa_in(r+len/2,T1+5*len/2,len/2);
    smpa_premul(T1+4*len/2,len/2,T1);
    smpa_premul(T1+5*len/2,len/2,T1);
    smpa_mul(T1+4*len/2,T1+5*len/2,T1+4*len/2,len/2,T1);

    smpa_div(T1+4*len/2,T1+6*len/2,T1+7*len/2,
             T1+len/2,T1+2*len/2,T1+3*len/2,
             T1+5*len/2,len/2,T1);

    smpa_out(T1+6*len/2,T2+len/2,len/2);

    numt_prim(r+len/2,T2+len,T1,T2+3*len/2,len/4);
    for(j=len/4;j<len/2;j++) T2[len+j]=0;

    smpa_in(T2,T1,len/2);
    smpa_in(T2+len,T1+len/2,len/2);
    smpa_gcd(T1+len/2,T1,
             T1+len,T1+3*len/2,T1+2*len,
             T1+5*len/2,len/2);

    for(j=0;j<len/2;j++) T2[3*len/2+j]=0;
    smpa_in(T2+3*len/2,T1+len,len/2);
    smpa_sub(T1+len,T1+2*len,T1+len,len/2);
    smpa_out(T1+len,T2+3*len/2,len/2);

    smpa_init(T1,len/2);
    smpa_in(T2+len/2,T1+len/2,len/2);
    smpa_in(T2+3*len/2,T1+len,len/2);
    smpa_sub(T1+len/2,T1+len,T1+len/2,len/2);

    for(j=0;j<len/2;j++) T2[2*len+j]=0;
    T2[2*len+len/4]=1;
    smpa_in(T2+len*2,T1+3*len/2,len/2);
    smpa_premul(T1+3*len/2,len/2,T1);

    smpa_in(T2+len,T1+len,len/2);
    smpa_premul(T1+len,len/2,T1);
    smpa_mul(T1+len,T1+3*len/2,T1+len,len/2,T1);
    smpa_out(T1+len,T2+len/2,len/2);

    smpa_premul(T1+len/2,len/2,T1);
    smpa_mul(T1+len/2,T1+3*len/2,T1+len/2,len/2,T1);

    smpa_in(T2+len/2,T1+len,len/2);
    smpa_prediv(T1+len,T1+3*len/2,T1+2*len,T1+5*len/2,len/2,T1);
    smpa_div(T1+len/2,T1+5*len/2,T1+6*len/2,
             T1+2*len/2,T1+3*len/2,T1+4*len/2,
             T1+7*len/2,len/2,T1);

    smpa_out(T1+5*len/2,T2+len/2,len/2);

    do {
        numt_primap(T2,T2+len/2,T2+len,T2+3*len/2,p,T1,T2+len*2,len);

        x=0;
        for(j=len;j>0;j--) x=(x*256+p[j-1])%65537;
    } while(x==1);
}

