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
  "$Id: smpa.c,v 1.8 2002/04/25 06:13:52 cperciva Exp $";
#endif

#include "ptypes.h"
#include "smpa.h"
#include "fft.h"

#define MAGIC 6755399441055744.
#define MAGIC1 32767.5
#define MAGIC2 MAGIC*65536.
#define SCARRY(c,x) {x+=c;c=x+MAGIC2;c-=MAGIC2;x-=c;c/=65536;}
#define UCARRY(c,x) {x+=c;c=(x-MAGIC1)+MAGIC2;c-=MAGIC2;x-=c;c/=65536;}
#define EVEN(c,x) (c=x+MAGIC*2,c-=MAGIC*2,x==c)

static void smpa_half(double * A1,uint32 len)
{
    uint32 i;
    double c;

    for(i=0;i<len;i++) *(A1+i)*=0.5;
    for(i=len-2;i>0;i-=2) {
        c=*(A1+i);
        *(A1+i)+=MAGIC;
        *(A1+i)-=MAGIC;
        *(A1+i-2)+=(c-*(A1+i))*65536;
    };
    c=0;
    for(i=0;i<len;i+=2) SCARRY(c,*(A1+i));
    *(A1+len-2)+=c*65536;
}

static double smpa_compare(double * A1,double * B1,uint32 len)
{
    uint32 i;
    double c;

    c=0;
    for(i=0;i<len;i+=2) {
        c+=*(A1+i)-*(B1+i)-MAGIC1;
        c/=65536;
        c+=MAGIC;
        c-=MAGIC;
    };
    return c;
}

static double smpa_positive(double *A1,uint32 len)
{
    uint32 i;
    double c;

    c=-1;
    for(i=0;i<len;i+=2) {
        c+=*(A1+i)-MAGIC1;
        c/=65536;
        c+=MAGIC;
        c-=MAGIC;
    };
    return c;
}

void smpa_init(double * LUT,uint32 len)
{
    fft_makelut(LUT,len/2);
    fft_makeraclut(LUT+len/2,len/2);
}

void smpa_in(uint8 * A,double * A1,uint32 len)
{
    uint32 i;
    double c;

    c=0;
    for(i=0;i<len;i+=4) {
        *(A1+i)=int32_little(*(uint32 *)(A+i));
        *(A1+i+1)=0;
        SCARRY(c,*(A1+i));
        *(A1+i+2)=0;
        *(A1+i+3)=0;
        SCARRY(c,*(A1+i+2));
    };
    *(A1+len-2)+=c*65536;
}    

void smpa_add(double * A1,double * B1,double * C1,uint32 len)
{
    uint32 i;
    double c;

    c=0;
    for(i=0;i<len;i+=2) {
        *(C1+i)=*(A1+i)+*(B1+i);
        SCARRY(c,*(C1+i));
    };
    for(i=1;i<len;i+=2) {
        *(C1+i)=*(A1+i)+*(B1+i);
        SCARRY(c,*(C1+i));
    };
    *(C1+len-1)+=c*65536;
}

void smpa_sub(double * A1,double * B1,double * C1,uint32 len)
{
    uint32 i;
    double c;

    c=0;
    for(i=0;i<len;i+=2) {
        *(C1+i)=*(A1+i)-*(B1+i);
        SCARRY(c,*(C1+i));
    };
    for(i=1;i<len;i+=2) {
        *(C1+i)=*(A1+i)-*(B1+i);
        SCARRY(c,*(C1+i));
    };
    *(C1+len-1)+=c*65536;
}

void smpa_premul(double * A1,uint32 len,double * LUT)
{
    fft_racw(A1,len/2,1,LUT+len/2);
    fft_fft(A1,len/2,LUT);
}

void smpa_mul(double * A1,double * B1,double * C1,
                                    uint32 len,double * LUT)
{
    uint32 i;
    double c;

    fft_mulpw(A1,B1,C1,len/2);

    fft_ifft(C1,len/2,LUT);
    fft_iracw(C1,len/2,1,LUT+len/2);
    fft_normalize(C1,len/2);

    c=0;
    for(i=0;i<len;i+=2) SCARRY(c,*(C1+i));
    for(i=1;i<len;i+=2) SCARRY(c,*(C1+i));
    *(C1+len-1)+=c*65536;
}

void smpa_prediv(double * M1,double * M2,double * K1,
                        double * T1,uint32 len,double * LUT)
{
    uint32 i,j;
    double c,c1,c2;


    for(i=0;i<len;i++) *(K1+i)=0;
    *(K1)=4294967296./M1[len-2];*(K1)+=MAGIC;*(K1)-=MAGIC;

    for(i=2;i<len;i+=i) {
        for(j=0;j<i*2;j++) *(M2+j)=*(M1+len-i*2+j);

        fft_racw(M2,i,len/(i*2),LUT+len/2);
        fft_fft(M2,i,LUT);

        fft_racw(K1,i,len/(i*2),LUT+len/2);
        fft_fft(K1,i,LUT);

        fft_mulpw(K1,M2,M2,i);
        fft_ifft(M2,i,LUT);
        fft_iracw(M2,i,len/(i*2),LUT+len/2);
        fft_normalize(M2,i);

        c=0;
        for(j=0;j<i*2;j+=2) {
            *(M2+j)=-*(M2+j);
            SCARRY(c,*(M2+j));
        };
        for(j=1;j<i;j+=2) {
            *(M2+j)=-*(M2+j);
            SCARRY(c,*(M2+j));
        };
        *(M2+i-1)+=(2+c)*65536;

        fft_racw(M2,i,len/(i*2),LUT+len/2);
        fft_fft(M2,i,LUT);

        fft_mulpw(K1,M2,M2,i);
        fft_ifft(M2,i,LUT);
        fft_iracw(M2,i,len/(i*2),LUT+len/2);
        fft_normalize(M2,i);

        c=0;
        for(j=0;j<i*2;j+=2) {
            c=(c+*(M2+j))/65536+MAGIC;
            c-=MAGIC;
            *(K1+j)=*(M2+j+1);
            *(K1+j+1)=0;
        };
        for(j=0;j<i*2;j+=2) SCARRY(c,*(K1+j));
        *(K1+i*2-2)+=c*65536;
    };

    for(j=0;j<len;j++) *(M2+j)=*(M1+j);

    fft_racw(M2,len/2,1,LUT+len/2);
    fft_fft(M2,len/2,LUT);

    fft_racw(K1,len/2,1,LUT+len/2);
    fft_fft(K1,len/2,LUT);

    fft_mulpw(K1,M2,T1,len/2);
    fft_ifft(T1,len/2,LUT);
    fft_iracw(T1,len/2,1,LUT+len/2);
    fft_normalize(T1,len/2);

    c=0;
    for(j=0;j<len;j+=2) {
        c=(c-*(T1+j))/65536+MAGIC;
        c-=MAGIC;
        *(T1+j)=-*(T1+j+1);
        *(T1+j+1)=0;
    };
    for(j=0;j<len;j+=2) SCARRY(c,*(T1+j));
    *(T1+len-2)+=(2+c)*65536;

    fft_racw(T1,len/2,1,LUT+len/2);
    fft_fft(T1,len/2,LUT);

    fft_mulpw(K1,T1,T1,len/2);
    fft_ifft(T1,len/2,LUT);
    fft_iracw(T1,len/2,1,LUT+len/2);
    fft_normalize(T1,len/2);

    c=0;
    for(j=0;j<len;j+=2) {
        c=(c+*(T1+j))/65536+MAGIC;
        c-=MAGIC;
        *(T1+j)=*(T1+j+1);
        *(T1+j+1)=0;
    };
    for(j=0;j<len;j+=2) SCARRY(c,*(T1+j));
    *(T1+len-2)+=c*65536;

    for(j=0;j<len;j++) *(K1+j)=*(T1+j);

    fft_racw(T1,len/2,1,LUT+len/2);
    fft_fft(T1,len/2,LUT);

    fft_mulpw(M2,T1,T1,len/2);
    fft_ifft(T1,len/2,LUT);
    fft_iracw(T1,len/2,1,LUT+len/2);
    fft_normalize(T1,len/2);

    do {
        c1=0;c2=0;
        for(j=0;j<len;j+=2) {
            c1=(c1+*(T1+j)-MAGIC1)/65536+MAGIC;
            c1-=MAGIC;
            c2=(c2+*(T1+j)+*(M1+j)-MAGIC1)/65536+MAGIC;
            c2-=MAGIC;
        };
        for(j=1;j<len;j+=2) {
            c1=(c1+*(T1+j)-MAGIC1)/65536+MAGIC;
            c1-=MAGIC;
            c2=(c2+*(T1+j)-MAGIC1)/65536+MAGIC;
            c2-=MAGIC;
        };
        if(c1>0) {
            for(j=0;j<len;j+=2) *(T1+j)-=*(M1+j);
            *(K1)-=1;
        };
        if(c2<1) {
            for(j=0;j<len;j+=2) *(T1+j)+=*(M1+j);
            *(K1)+=1;
        };
    } while((c1>0)||(c2<1));

    fft_racw(K1,len/2,1,LUT+len/2);
    fft_fft(K1,len/2,LUT);    
}

void smpa_mulmod(double * A1,double * B1,double * C1,
                        double * M1,double * M2,double * K1,
                        double * T1,uint32 len,double * LUT)
{
    uint32 i,j;
    double c,c1,c2;

    fft_mulpw(A1,B1,C1,len/2);
    fft_ifft(C1,len/2,LUT);
    fft_iracw(C1,len/2,1,LUT+len/2);
    fft_normalize(C1,len/2);

    c=0;
    for(j=0;j<len;j+=2) {
        c=(c+*(C1+j))/65536+MAGIC;
        c-=MAGIC;
        *(T1+j)=+*(C1+j+1);
        *(T1+j+1)=0;
    };
    for(j=0;j<len;j+=2) SCARRY(c,*(T1+j));
    *(T1+len-2)+=c*65536;

    fft_racw(T1,len/2,1,LUT+len/2);
    fft_fft(T1,len/2,LUT);

    fft_mulpw(K1,T1,T1,len/2);
    fft_ifft(T1,len/2,LUT);
    fft_iracw(T1,len/2,1,LUT+len/2);
    fft_normalize(T1,len/2);

    c=0;
    for(j=0;j<len;j+=2) {
        c=(c+*(T1+j))/65536+MAGIC;
        c-=MAGIC;
        *(T1+j)=*(T1+j+1);
        *(T1+j+1)=0;
    };
    for(j=0;j<len;j+=2) SCARRY(c,*(T1+j));
    *(T1+len-2)+=c*65536;

    fft_racw(T1,len/2,1,LUT+len/2);
    fft_fft(T1,len/2,LUT);

    fft_mulpw(M2,T1,T1,len/2);
    fft_ifft(T1,len/2,LUT);
    fft_iracw(T1,len/2,1,LUT+len/2);
    fft_normalize(T1,len/2);

    for(j=0;j<len;j++) *(C1+j)-=*(T1+j);

    for(i=0;i<4;i++) {
        c1=0;c2=0;
        for(j=0;j<len;j+=2) {
            c1=(c1+*(C1+j)-MAGIC1)/65536+MAGIC;
            c1-=MAGIC;
            c2=(c2+*(C1+j)-*(M1+j)-MAGIC1)/65536+MAGIC;
            c2-=MAGIC;
        };
        for(j=1;j<len;j+=2) {
            c1=(c1+*(C1+j)-MAGIC1)/65536+MAGIC;
            c1-=MAGIC;
            c2=(c2+*(C1+j)-MAGIC1)/65536+MAGIC;
            c2-=MAGIC;
        };

        c=0;
        if(c1<0) c+=1;
        if(c2>=0) c-=1;

        for(j=0;j<len;j+=2) *(C1+j)+=*(M1+j)*c;
    };

    c=0;
    for(j=0;j<len;j+=2) SCARRY(c,*(C1+j));
    for(j=1;j<len;j+=2) *(C1+j)=0;
    if(*(C1+len-2)<0) *(C1+len-2)+=65536;
}

void smpa_div(double * A1,double * Q1,double * R1,
                        double * M1,double * M2,double * K1,
                        double * T1,uint32 len,double * LUT)
{
    uint32 i,j;
    double c,c1,c2;

    for(j=0;j<len;j+=2) {
        *(T1+j)=+*(A1+j+1);
        *(T1+j+1)=0;
    };

    fft_racw(T1,len/2,1,LUT+len/2);
    fft_fft(T1,len/2,LUT);

    fft_mulpw(K1,T1,T1,len/2);
    fft_ifft(T1,len/2,LUT);
    fft_iracw(T1,len/2,1,LUT+len/2);
    fft_normalize(T1,len/2);

    c=0;
    for(j=0;j<len;j+=2) {
        c=(c+*(T1+j))/65536+MAGIC;
        c-=MAGIC;
        *(Q1+j)=*(T1+j+1);
        *(Q1+j+1)=0;
        *(T1+j)=*(T1+j+1);
        *(T1+j+1)=0;
    };
    *Q1+=c;
    for(j=0;j<len;j+=2) SCARRY(c,*(T1+j));
    *(T1+len-2)+=c*65536;

    fft_racw(T1,len/2,1,LUT+len/2);
    fft_fft(T1,len/2,LUT);

    fft_mulpw(M2,T1,T1,len/2);
    fft_ifft(T1,len/2,LUT);
    fft_iracw(T1,len/2,1,LUT+len/2);
    fft_normalize(T1,len/2);

    for(j=0;j<len;j++) *(R1+j)=*(A1+j)-*(T1+j);

    for(i=0;i<4;i++) {
        c1=0;c2=0;
        for(j=0;j<len;j+=2) {
            c1=(c1+*(R1+j)-MAGIC1)/65536+MAGIC;
            c1-=MAGIC;
            c2=(c2+*(R1+j)-*(M1+j)-MAGIC1)/65536+MAGIC;
            c2-=MAGIC;
        };
        for(j=1;j<len;j+=2) {
            c1=(c1+*(R1+j)-MAGIC1)/65536+MAGIC;
            c1-=MAGIC;
            c2=(c2+*(R1+j)-MAGIC1)/65536+MAGIC;
            c2-=MAGIC;
        };

        c=0;
        if(c1<0) c+=1;
        if(c2>=0) c-=1;

        for(j=0;j<len;j+=2) *(R1+j)+=*(M1+j)*c;
        *Q1-=c;
    };

    c=0;
    for(j=0;j<len;j+=2) SCARRY(c,*(R1+j));
    for(j=1;j<len;j+=2) SCARRY(c,*(R1+j));
    *(R1+len-2)+=*(R1+1)*65536;*(R1+1)=0;

    c=0;
    for(j=0;j<len;j+=2) SCARRY(c,*(Q1+j));
    for(j=1;j<len;j+=2) SCARRY(c,*(Q1+j));
    *(Q1+len-2)+=*(Q1+1)*65536;*(Q1+1)=0;
}

void smpa_gcd(double * X1,double * Y1,
                        double * G1,double * A1,double *B1,
                        double * T1,uint32 len)
{
    uint32 i;
    uint32 g;
    double c;

    for(i=0;i<len;i++) *(T1+len*0+i)=*(X1+i);
    for(i=0;i<len;i++) *(T1+len*1+i)=*(Y1+i);

    for(g=0;g<len*8;g++) {
        if(!EVEN(c,*(T1+len*0))) break;
        if(!EVEN(c,*(T1+len*1))) break;
        smpa_half(T1+len*0,len);
        smpa_half(T1+len*1,len);
    };

    for(i=0;i<len;i++) *(G1      +i)=*(T1+len*0+i);
    for(i=0;i<len;i++) *(T1+len*2+i)=*(T1+len*1+i);
    for(i=0;i<len;i++) *(A1+i)=0;
    for(i=0;i<len;i++) *(B1+i)=0;
    for(i=0;i<len;i++) *(T1+len*3+i)=0;
    for(i=0;i<len;i++) *(T1+len*4+i)=0;
    *(A1)=1;*(T1+len*4)=1;

    while(EVEN(c,*(G1      ))) {
        smpa_half(G1      ,len);
        if((!EVEN(c,*(A1)))||(!EVEN(c,*(B1)))) {
            for(i=0;i<len;i+=2) *(A1+i)+=*(T1+len*1+i);
            for(i=0;i<len;i+=2) *(B1+i)-=*(T1+len*0+i);
        };
        smpa_half(A1,len);
        smpa_half(B1,len);
    };
    while(EVEN(c,*(T1+len*2))) {
        smpa_half(T1+len*2,len);
        if((!EVEN(c,*(T1+len*3)))||(!EVEN(c,*(T1+len*4)))) {
            for(i=0;i<len;i+=2) *(T1+len*3+i)+=*(T1+len*1+i);
            for(i=0;i<len;i+=2) *(T1+len*4+i)-=*(T1+len*0+i);
        };
        smpa_half(T1+len*3,len);
        smpa_half(T1+len*4,len);
    };

    do {
        if(smpa_compare(T1+len*2,G1,len)>=0) {
            for(i=0;i<len;i+=2) *(T1+len*2+i)-=*(G1+i);
            for(i=0;i<len;i+=2) *(T1+len*3+i)-=*(A1+i);
            for(i=0;i<len;i+=2) *(T1+len*4+i)-=*(B1+i);
            while(EVEN(c,*(T1+len*2))&&(smpa_positive(T1+len*2,len)>=0)) {
                smpa_half(T1+len*2,len);
                if((!EVEN(c,*(T1+len*3)))||(!EVEN(c,*(T1+len*4)))) {
                    for(i=0;i<len;i+=2) *(T1+len*3+i)+=*(T1+len*1+i);
                    for(i=0;i<len;i+=2) *(T1+len*4+i)-=*(T1+len*0+i);
                };
                smpa_half(T1+len*3,len);
                smpa_half(T1+len*4,len);
            };
        } else {
            for(i=0;i<len;i+=2) *(G1+i)-=*(T1+len*2+i);
            for(i=0;i<len;i+=2) *(A1+i)-=*(T1+len*3+i);
            for(i=0;i<len;i+=2) *(B1+i)-=*(T1+len*4+i);
            while(EVEN(c,*(G1))) {
                smpa_half(G1,len);
                if((!EVEN(c,*(A1)))||(!EVEN(c,*(B1)))) {
                    for(i=0;i<len;i+=2) *(A1+i)+=*(T1+len*1+i);
                    for(i=0;i<len;i+=2) *(B1+i)-=*(T1+len*0+i);
                };
                smpa_half(A1,len);
                smpa_half(B1,len);
            };
        };
    } while(smpa_positive(T1+len*2,len)>=0);

    if(!(smpa_positive(A1,len)>=0)) {
        for(i=0;i<len;i+=2) *(A1+i)+=*(T1+len*1+i);
        for(i=0;i<len;i+=2) *(B1+i)-=*(T1+len*0+i);
        c=0;for(i=0;i<len;i+=2) SCARRY(c,*(A1+i));*(A1+len-2)+=c*65536;
        c=0;for(i=0;i<len;i+=2) SCARRY(c,*(B1+i));*(B1+len-2)+=c*65536;
    };

    for(;g>0;g--) {
        for(i=0;i<len;i+=2) *(G1+i)+=*(G1+i);
        c=0;
        for(i=0;i<len;i+=2) SCARRY(c,*(G1+i));
        *(G1+len-2)+=c*65536;
    };
}

void smpa_out(double * A1,uint8 * A,uint32 len)
{
    uint32 j,k0;
    double c;

    c=0;
    for(j=0;j<len;j+=2) UCARRY(c,*(A1+j));
    *(A1+len-2)+=c*65536;

    for(j=0;j<len;j+=4) {
        k0 = *(A1+j) + 65536 * *(A1+j+2);
        *(uint32 *)(A+j) = int32_little(k0);
    };
}

void smpa_doubleout(double * A1,uint8 * A,uint32 len)
{
    uint32 j,k0;
    double c;

    c=0;
    for(j=0;j<len;j+=2) UCARRY(c,*(A1+j));
    for(j=1;j<len;j+=2) UCARRY(c,*(A1+j));
    *(A1+len-1)+=c*65536;

    for(j=0;j<len;j+=4) {
        k0 = *(A1+j) + 65536 * *(A1+j+2);
        *(uint32 *)(A+j) = int32_little(k0);
    };
    for(j=0;j<len;j+=4) {
        k0 = *(A1+j+1) + 65536 * *(A1+j+3);
        *(uint32 *)(A+len+j) = int32_little(k0);
    };
}

