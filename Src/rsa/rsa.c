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
  "$Id: rsa.c,v 1.2 2002/04/25 06:13:52 cperciva Exp $";
#endif

#include "ptypes.h"
#include "hash.h"
#include "smpa.h"
#include "numt.h"
#include "rsa.h"

// ptext == datalen bytes
// ctext,pub == len/8 bytes
// T1 == 8*len bytes
int rsa_encrypt(uint8 * ptext,uint32 datalen,uint8 * ctext,
                        uint8 * pub,uint32 len,uint8 * T1)
{
    uint32 j;
    uint8 e[4];

    if(datalen>=len/8) return 1;

    for(j=0;j<datalen;j++) *(ctext+j)=*(ptext+j);
    for(;j&0x0F;j++) *(ctext+j)=0;
    for(;j<len/8-16;j+=16) hash_data(ctext,j,ctext+j);
    for(;j<len/8;j++) *(ctext+j)=0;

    *(uint32 *)(&e)=int32_little(65537);
    numt_powmod(ctext,e,pub,ctext,(double *)T1,len/8,16,0);

    return 0;
}

int rsa_decrypt(uint8 * ctext,uint8 * ptext,
                        uint8 * pub,uint8 * priv,uint32 len,uint8 * T1)
{
    uint32 j;

    for(j=len/8;j>0;j--) {
        if(ctext[j-1]>pub[j-1]) return 1;
        if(ctext[j-1]<pub[j-1]) break;
    };
    if(ctext[0]==pub[0]) return 1;

    numt_powmod(ctext,priv,pub,ptext,(double *)T1,len/8,len-1,0);

    return 0;
}

int rsa_sign(uint8 * ptext,uint32 datalen,uint8 * ctext,
                        uint8 * pub,uint8 * priv,uint32 len,uint8 * T1)
{
    uint32 j;

    if(datalen+32>len/8) return 1;

    for(j=0;j<datalen;j++) *(ctext+j)=*(ptext+j);
    for(;j&0x0F;j++) *(ctext+j)=0;
    hash_data(ctext,datalen,ctext+j);j+=16;
    for(;j<len/8;j++) *(ctext+j)=0;
    *(uint32 *)(ctext+len/8-16)=int32_little(datalen);

    numt_powmod(ctext,priv,pub,ctext,(double *)T1,len/8,len-1,0);

    return 0;
}

uint32 rsa_verify(uint8 * ctext,uint8 * ptext,
                        uint8 * pub,uint32 len,uint8 * T1)
{
    uint32 j,datalen;
    uint8 e[4];

    for(j=len/8;j>0;j--) {
        if(ctext[j-1]>pub[j-1]) return 0;
        if(ctext[j-1]<pub[j-1]) break;
    };
    if(ctext[0]==pub[0]) return 0;

    *(uint32 *)(&e)=int32_little(65537);
    numt_powmod(ctext,e,pub,ptext,(double *)T1,len/8,16,0);

    datalen=int32_little(*(uint32 *)(ptext+len/8-16));
    if(datalen+32>len/8) return 0;

    hash_data(ptext,datalen,T1);

    for(j=0;j<16;j++) {
        if(ptext[datalen+((len-datalen)%16)+j]!=T1[j]) return 0;
    };

    return datalen;
}

// len == key length in bits
// r,pub,priv == len/8 bytes
// T1 == 8*len bytes
void rsa_keygen(uint8 * r,uint8 * pub,uint8 * priv,uint32 len,uint8 * T1)
{
    uint32 j;
    uint32 x,y;

    numt_sprim(r       ,T1       ,(double *)(T1+len*2),T1+len,len/16);
    numt_sprim(r+len/16,T1+len/16,(double *)(T1+len*2),T1+len,len/16);

    smpa_init((double *)(T1+len),len/16);

    smpa_in(T1       ,(double *)(T1+len*2),len/16);
    smpa_in(T1+len/16,(double *)(T1+len*3),len/16);
    smpa_premul((double *)(T1+len*2),len/16,(double *)(T1+len));
    smpa_premul((double *)(T1+len*3),len/16,(double *)(T1+len));
    smpa_mul((double *)(T1+len*2),(double *)(T1+len*3),
             (double *)(T1+len*2),len/16,(double *)(T1+len));
    smpa_doubleout((double *)(T1+len*2),pub,len/16);

    *(T1       )&=0xFE;
    *(T1+len/16)&=0xFE;

    smpa_in(T1       ,(double *)(T1+len*2),len/16);
    smpa_in(T1+len/16,(double *)(T1+len*3),len/16);
    smpa_premul((double *)(T1+len*2),len/16,(double *)(T1+len));
    smpa_premul((double *)(T1+len*3),len/16,(double *)(T1+len));
    smpa_mul((double *)(T1+len*2),(double *)(T1+len*3),
             (double *)(T1+len*2),len/16,(double *)(T1+len));
    smpa_doubleout((double *)(T1+len*2),priv,len/16);

    x=0;for(j=len/8;j>0;j--) x=(x*256+priv[j-1])%65537;

    *(uint32 *)(T1  )=int32_little(x);
    *(uint32 *)(T1+4)=int32_little(65535);
    *(uint32 *)(T1+8)=int32_little(65537<<15);

    numt_powmod(T1,T1+4,T1+8,T1+12,(double *)(T1+16),4,15,0);

    x=int32_little(*(uint32 *)(T1+12))%65537;
    x=(65537-x)%65537;

    y=1;
    for(j=0;j<len/8;j++) {       y+=x*priv[j];priv[j]  =y%256  ;y/=256;   };
    for(j=len/8;j>0;j--) {y*=256;y+=priv[j-1];priv[j-1]=y/65537;y=y%65537;};
}
