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
  "$Id: aes.c,v 1.3 2002/04/25 06:13:53 cperciva Exp $";
#endif

#include "ptypes.h"
#include "aestab.h"
#include "aes.h"

void aes_keyexpand128(uint8 * key,uint8 * keyexp)
{
    uint32 i;
    uint8 Rcon;

    *(uint32 *)(keyexp   ) = *(uint32 *)(key   );
    *(uint32 *)(keyexp+4 ) = *(uint32 *)(key+4 );
    *(uint32 *)(keyexp+8 ) = *(uint32 *)(key+8 );
    *(uint32 *)(keyexp+12) = *(uint32 *)(key+12);

    Rcon=0x01;

    for(i=1;i<11;i++) {
        *(keyexp+i*16  )=*(keyexp+i*16-16)^aestab_SBox[*(keyexp+i*16-3)]
                                          ^Rcon;
        *(keyexp+i*16+1)=*(keyexp+i*16-15)^aestab_SBox[*(keyexp+i*16-2)];
        *(keyexp+i*16+2)=*(keyexp+i*16-14)^aestab_SBox[*(keyexp+i*16-1)];
        *(keyexp+i*16+3)=*(keyexp+i*16-13)^aestab_SBox[*(keyexp+i*16-4)];
        Rcon=((Rcon<<1)&0xFE)^
             ((Rcon>>3)&0x10)^
             ((Rcon>>4)&0x08)^
             ((Rcon>>6)&0x02)^
             ((Rcon>>7));
        *(uint32 *)(keyexp+i*16+4) =*(uint32 *)(keyexp+i*16-12)
                                   ^*(uint32 *)(keyexp+i*16   );
        *(uint32 *)(keyexp+i*16+8) =*(uint32 *)(keyexp+i*16-8 )
                                   ^*(uint32 *)(keyexp+i*16+4 );
        *(uint32 *)(keyexp+i*16+12)=*(uint32 *)(keyexp+i*16-4 )
                                   ^*(uint32 *)(keyexp+i*16+8 );
    };
}

void aes_keyexpand256(uint8 * key,uint8 * keyexp)
{
    uint32 i;
    uint8 Rcon;

    *(uint32 *)(keyexp   ) = *(uint32 *)(key   );
    *(uint32 *)(keyexp+4 ) = *(uint32 *)(key+4 );
    *(uint32 *)(keyexp+8 ) = *(uint32 *)(key+8 );
    *(uint32 *)(keyexp+12) = *(uint32 *)(key+12);
    *(uint32 *)(keyexp+16) = *(uint32 *)(key+16);
    *(uint32 *)(keyexp+20) = *(uint32 *)(key+20);
    *(uint32 *)(keyexp+24) = *(uint32 *)(key+24);
    *(uint32 *)(keyexp+28) = *(uint32 *)(key+28);

    Rcon=0x01;

    for(i=1;i<7;i++) {
        *(keyexp+i*32   )=*(keyexp+i*32-32)^aestab_SBox[*(keyexp+i*32-3)]
                                           ^Rcon;
        *(keyexp+i*32+1 )=*(keyexp+i*32-31)^aestab_SBox[*(keyexp+i*32-2)];
        *(keyexp+i*32+2 )=*(keyexp+i*32-30)^aestab_SBox[*(keyexp+i*32-1)];
        *(keyexp+i*32+3 )=*(keyexp+i*32-29)^aestab_SBox[*(keyexp+i*32-4)];
        Rcon=((Rcon<<1)&0xFE)^
             ((Rcon>>3)&0x10)^
             ((Rcon>>4)&0x08)^
             ((Rcon>>6)&0x02)^
             ((Rcon>>7));
        *(uint32 *)(keyexp+i*32+4 )=*(uint32 *)(keyexp+i*32-28)
                                   ^*(uint32 *)(keyexp+i*32   );
        *(uint32 *)(keyexp+i*32+8 )=*(uint32 *)(keyexp+i*32-24)
                                   ^*(uint32 *)(keyexp+i*32+4 );
        *(uint32 *)(keyexp+i*32+12)=*(uint32 *)(keyexp+i*32-20)
                                   ^*(uint32 *)(keyexp+i*32+8 );

        *(keyexp+i*32+16)=*(keyexp+i*32-16)^aestab_SBox[*(keyexp+i*32+12)];
        *(keyexp+i*32+17)=*(keyexp+i*32-15)^aestab_SBox[*(keyexp+i*32+13)];
        *(keyexp+i*32+18)=*(keyexp+i*32-14)^aestab_SBox[*(keyexp+i*32+14)];
        *(keyexp+i*32+19)=*(keyexp+i*32-13)^aestab_SBox[*(keyexp+i*32+15)];
        *(uint32 *)(keyexp+i*32+20)=*(uint32 *)(keyexp+i*32-12)
                                   ^*(uint32 *)(keyexp+i*32+16);
        *(uint32 *)(keyexp+i*32+24)=*(uint32 *)(keyexp+i*32-8 )
                                   ^*(uint32 *)(keyexp+i*32+20);
        *(uint32 *)(keyexp+i*32+28)=*(uint32 *)(keyexp+i*32-4 )
                                   ^*(uint32 *)(keyexp+i*32+24);
    };

    *(keyexp+224)=*(keyexp+192)^aestab_SBox[*(keyexp+221)]^Rcon;
    *(keyexp+225)=*(keyexp+193)^aestab_SBox[*(keyexp+222)];
    *(keyexp+226)=*(keyexp+194)^aestab_SBox[*(keyexp+223)];
    *(keyexp+227)=*(keyexp+195)^aestab_SBox[*(keyexp+220)];
    *(uint32 *)(keyexp+228)=*(uint32 *)(keyexp+196)^*(uint32 *)(keyexp+224);
    *(uint32 *)(keyexp+232)=*(uint32 *)(keyexp+200)^*(uint32 *)(keyexp+228);
    *(uint32 *)(keyexp+236)=*(uint32 *)(keyexp+204)^*(uint32 *)(keyexp+232);
}

void aes_key2dec(uint8 * src,uint8 *dest,uint8 rounds)
{
    uint32 i;
    uint32 k,k1,k2;

    *(uint32 *)(dest   )=*(uint32 *)(src   );
    *(uint32 *)(dest+4 )=*(uint32 *)(src+4 );
    *(uint32 *)(dest+8 )=*(uint32 *)(src+8 );
    *(uint32 *)(dest+12)=*(uint32 *)(src+12);

    for(i=4;i<rounds*4;i++) {
        k=int32_little(*(uint32 *)(src+i*4));
        k2=((k<<1)&0xFEFEFEFE)^
           ((k>>3)&0x10101010)^
           ((k>>4)&0x08080808)^
           ((k>>6)&0x02020202)^
           ((k>>7)&0x01010101);
        k1=k^k2^(k2<<24)^(k2>>8);
        k2=((k1<<1)&0xFEFEFEFE)^
           ((k1>>3)&0x10101010)^
           ((k1>>4)&0x08080808)^
           ((k1>>6)&0x02020202)^
           ((k1>>7)&0x01010101);
        k2=((k2<<1)&0xFEFEFEFE)^
           ((k2>>3)&0x10101010)^
           ((k2>>4)&0x08080808)^
           ((k2>>6)&0x02020202)^
           ((k2>>7)&0x01010101);
        k2^=k^(k<<24)^(k>>8);
        k2^=(k2<<16)^(k2>>16)^k1;
        *(uint32 *)(dest+i*4)=int32_little(k2);
    };

    *(uint32 *)(dest+rounds*16   )=*(uint32 *)(src+rounds*16   );
    *(uint32 *)(dest+rounds*16+4 )=*(uint32 *)(src+rounds*16+4 );
    *(uint32 *)(dest+rounds*16+8 )=*(uint32 *)(src+rounds*16+8 );
    *(uint32 *)(dest+rounds*16+12)=*(uint32 *)(src+rounds*16+12);
}

void aes_blockenc(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds)
{
    uint32 i;
    uint8 temp[16];

    *(uint32 *)(dest   )=*(uint32 *)(src   )^*(uint32 *)(keyexp   );
    *(uint32 *)(dest+4 )=*(uint32 *)(src+4 )^*(uint32 *)(keyexp+4 );
    *(uint32 *)(dest+8 )=*(uint32 *)(src+8 )^*(uint32 *)(keyexp+8 );
    *(uint32 *)(dest+12)=*(uint32 *)(src+12)^*(uint32 *)(keyexp+12);

    for(i=1;i<rounds;i++) {
        *(uint32 *)(temp   )=*(uint32 *)(aestab_SBox0+*(dest   )*4)^
                             *(uint32 *)(aestab_SBox1+*(dest+5 )*4)^
                             *(uint32 *)(aestab_SBox2+*(dest+10)*4)^
                             *(uint32 *)(aestab_SBox3+*(dest+15)*4);
        *(uint32 *)(temp+4 )=*(uint32 *)(aestab_SBox0+*(dest+4 )*4)^
                             *(uint32 *)(aestab_SBox1+*(dest+9 )*4)^
                             *(uint32 *)(aestab_SBox2+*(dest+14)*4)^
                             *(uint32 *)(aestab_SBox3+*(dest+3 )*4);
        *(uint32 *)(temp+8 )=*(uint32 *)(aestab_SBox0+*(dest+8 )*4)^
                             *(uint32 *)(aestab_SBox1+*(dest+13)*4)^
                             *(uint32 *)(aestab_SBox2+*(dest+2 )*4)^
                             *(uint32 *)(aestab_SBox3+*(dest+7 )*4);
        *(uint32 *)(temp+12)=*(uint32 *)(aestab_SBox0+*(dest+12)*4)^
                             *(uint32 *)(aestab_SBox1+*(dest+1 )*4)^
                             *(uint32 *)(aestab_SBox2+*(dest+6 )*4)^
                             *(uint32 *)(aestab_SBox3+*(dest+11)*4);
        *(uint32 *)(dest   )=*(uint32 *)(temp   )^
                             *(uint32 *)(keyexp+i*16   );
        *(uint32 *)(dest+4 )=*(uint32 *)(temp+4 )^
                             *(uint32 *)(keyexp+i*16+4 );
        *(uint32 *)(dest+8 )=*(uint32 *)(temp+8 )^
                             *(uint32 *)(keyexp+i*16+8 );
        *(uint32 *)(dest+12)=*(uint32 *)(temp+12)^
                             *(uint32 *)(keyexp+i*16+12);
    };

    for(i=0;i<16;i++) temp[i]=aestab_SBox[*(dest+((i*5)&0x0f))];

    *(uint32 *)(dest   )=*(uint32 *)(temp   )^
                         *(uint32 *)(keyexp+rounds*16   );
    *(uint32 *)(dest+4 )=*(uint32 *)(temp+4 )^
                         *(uint32 *)(keyexp+rounds*16+4 );
    *(uint32 *)(dest+8 )=*(uint32 *)(temp+8 )^
                         *(uint32 *)(keyexp+rounds*16+8 );
    *(uint32 *)(dest+12)=*(uint32 *)(temp+12)^
                         *(uint32 *)(keyexp+rounds*16+12);

    *(uint32 *)(temp   )=0;
    *(uint32 *)(temp+4 )=0;
    *(uint32 *)(temp+8 )=0;
    *(uint32 *)(temp+12)=0;
}

void aes_blockdec(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds)
{
    uint32 i;
    uint8 temp[16];

    *(uint32 *)(dest   )=*(uint32 *)(src   )^
                         *(uint32 *)(keyexp+rounds*16   );
    *(uint32 *)(dest+4 )=*(uint32 *)(src+4 )^
                         *(uint32 *)(keyexp+rounds*16+4 );
    *(uint32 *)(dest+8 )=*(uint32 *)(src+8 )^
                         *(uint32 *)(keyexp+rounds*16+8 );
    *(uint32 *)(dest+12)=*(uint32 *)(src+12)^
                         *(uint32 *)(keyexp+rounds*16+12);

    for(i=rounds-1;i>0;i--) {
        *(uint32 *)(temp   )=*(uint32 *)(aestab_ISBox0+*(dest   )*4)^
                             *(uint32 *)(aestab_ISBox1+*(dest+13)*4)^
                             *(uint32 *)(aestab_ISBox2+*(dest+10)*4)^
                             *(uint32 *)(aestab_ISBox3+*(dest+7 )*4);
        *(uint32 *)(temp+4 )=*(uint32 *)(aestab_ISBox0+*(dest+4 )*4)^
                             *(uint32 *)(aestab_ISBox1+*(dest+1 )*4)^
                             *(uint32 *)(aestab_ISBox2+*(dest+14)*4)^
                             *(uint32 *)(aestab_ISBox3+*(dest+11)*4);
        *(uint32 *)(temp+8 )=*(uint32 *)(aestab_ISBox0+*(dest+8 )*4)^
                             *(uint32 *)(aestab_ISBox1+*(dest+5 )*4)^
                             *(uint32 *)(aestab_ISBox2+*(dest+2 )*4)^
                             *(uint32 *)(aestab_ISBox3+*(dest+15)*4);
        *(uint32 *)(temp+12)=*(uint32 *)(aestab_ISBox0+*(dest+12)*4)^
                             *(uint32 *)(aestab_ISBox1+*(dest+9 )*4)^
                             *(uint32 *)(aestab_ISBox2+*(dest+6 )*4)^
                             *(uint32 *)(aestab_ISBox3+*(dest+3 )*4);
        *(uint32 *)(dest   )=*(uint32 *)(temp   )^
                             *(uint32 *)(keyexp+i*16   );
        *(uint32 *)(dest+4 )=*(uint32 *)(temp+4 )^
                             *(uint32 *)(keyexp+i*16+4 );
        *(uint32 *)(dest+8 )=*(uint32 *)(temp+8 )^
                             *(uint32 *)(keyexp+i*16+8 );
        *(uint32 *)(dest+12)=*(uint32 *)(temp+12)^
                             *(uint32 *)(keyexp+i*16+12);
    };

    for(i=0;i<16;i++) temp[i]=aestab_InvSBox[*(dest+((i*13)&0x0f))];

    *(uint32 *)(dest   )=*(uint32 *)(temp   )^*(uint32 *)(keyexp   );
    *(uint32 *)(dest+4 )=*(uint32 *)(temp+4 )^*(uint32 *)(keyexp+4 );
    *(uint32 *)(dest+8 )=*(uint32 *)(temp+8 )^*(uint32 *)(keyexp+8 );
    *(uint32 *)(dest+12)=*(uint32 *)(temp+12)^*(uint32 *)(keyexp+12);

    *(uint32 *)(temp   )=0;
    *(uint32 *)(temp+4 )=0;
    *(uint32 *)(temp+8 )=0;
    *(uint32 *)(temp+12)=0;
}

void aes_cbcenc(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds,uint32 blocks)
{
    uint8 IV[16];
    uint32 i;

    *(uint32 *)(IV   )=0;
    *(uint32 *)(IV+4 )=0;
    *(uint32 *)(IV+8 )=0;
    *(uint32 *)(IV+12)=0;

    for(i=0;i<blocks;i++) {
        *(uint32 *)(IV   )^=*(uint32 *)(src+i*16   );
        *(uint32 *)(IV+4 )^=*(uint32 *)(src+i*16+4 );
        *(uint32 *)(IV+8 )^=*(uint32 *)(src+i*16+8 );
        *(uint32 *)(IV+12)^=*(uint32 *)(src+i*16+12);
        aes_blockenc(IV,dest+i*16,keyexp,rounds);
        *(uint32 *)(IV   )=*(uint32 *)(dest+i*16   );
        *(uint32 *)(IV+4 )=*(uint32 *)(dest+i*16+4 );
        *(uint32 *)(IV+8 )=*(uint32 *)(dest+i*16+8 );
        *(uint32 *)(IV+12)=*(uint32 *)(dest+i*16+12);
    };

    *(uint32 *)(IV   )=0;
    *(uint32 *)(IV+4 )=0;
    *(uint32 *)(IV+8 )=0;
    *(uint32 *)(IV+12)=0;
}

void aes_cbcdec(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds,uint32 blocks)
{
    uint32 i;

    for(i=blocks;i>0;i--) {
        aes_blockdec(src+i*16-16,dest+i*16-16,keyexp,rounds);
        if(i>1) {
            *(uint32 *)(dest+i*16-16)^=*(uint32 *)(src+i*16-32);
            *(uint32 *)(dest+i*16-12)^=*(uint32 *)(src+i*16-28);
            *(uint32 *)(dest+i*16-8 )^=*(uint32 *)(src+i*16-24);
            *(uint32 *)(dest+i*16-4 )^=*(uint32 *)(src+i*16-20);
        };
    };
}

