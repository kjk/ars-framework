/*
  Copyright 1999,2000,2002 Colin Percival and the University of Oxford

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
  "$Id: fft.c,v 1.5 2002/04/25 06:13:53 cperciva Exp $";
#endif

#include "ptypes.h"
#include "fft.h"
#include "fftlut.h"

void fft_makelut(double * LUT,uint32 size)
{
    uint32 i,j;
    double x,y;

    if(size==2) {
        LUT[0]=1;
        LUT[1]=0;
    } else if(size<=256) {
        for(i=0;i<size/2;i+=2) {
            x=fftlut_8[i];y=fftlut_8[i+1];
            LUT[2*i]  = x;LUT[2*i+1]=y;
            LUT[2*i+2]=-y;LUT[2*i+3]=x;
        };
    } else {
        for(i=0;i<128;i+=2)
        for(j=0;j<size/128;j+=2) {
            x=fftlut_8[i]*fftlut_14[j  ]-fftlut_8[i+1]*fftlut_14[j+1];
            y=fftlut_8[i]*fftlut_14[j+1]+fftlut_8[i+1]*fftlut_14[j  ];
            LUT[2*i+128*j]  = x;LUT[2*i+128*j+1]=y;
            LUT[2*i+128*j+2]=-y;LUT[2*i+128*j+3]=x;
        };
    };
}

static const double fft_lut_16[] = {
                   1.000000000000000000000, 0.000000000000000000000,
                   0.999999995404107312891, 0.000095873799095977346,
                   0.999999981616429293808, 0.000191747597310703307,
                   0.999999958636966069486, 0.000287621393762926508,
                   };

void fft_makeraclut(double * LUT,uint32 size)
{
    uint32 i,j,k;
    double x1,y1,x2,y2;

    for(i=0;i<size/2;i++) {
        j=i*(16384/size);

        k=j%4;
        x1=fft_lut_16[2*k];y1=fft_lut_16[2*k+1];

        k=(j/4)%64;
        k=((k<<4)&0x30)|(k&0x0c)|((k>>4)&0x03);
        k=((k>>1)&0x15)|((k<<1)&0x2a);
        x2=x1*fftlut_14[2*k]-y1*fftlut_14[2*k+1];
        y2=y1*fftlut_14[2*k]+x1*fftlut_14[2*k+1];

        k=(j/256)%64;
        k=((k<<4)&0x30)|(k&0x0c)|((k>>4)&0x03);
        k=((k>>1)&0x15)|((k<<1)&0x2a);
        x1=x2*fftlut_8[2*k]-y2*fftlut_8[2*k+1];
        y1=y2*fftlut_8[2*k]+x2*fftlut_8[2*k+1];

        LUT[2*i]=x1;LUT[2*i+1]=y1;
    };
}

void fft_racw(double * DAT,uint32 size,uint32 stride,double * LUT)
{
    uint32 i;
    double x2,y2;
    double * DAT1 = DAT + 2*size;

    for(i=1;i<size/2;i++) {
        x2=*(DAT+2*i)   * *(LUT+2*i*stride)
         - *(DAT+2*i+1) * *(LUT+2*i*stride+1);
        y2=*(DAT+2*i)   * *(LUT+2*i*stride+1)
         + *(DAT+2*i+1) * *(LUT+2*i*stride);

        *(DAT+2*i)   = x2;
        *(DAT+2*i+1) = y2;

        x2=   *(DAT1-2*i)   * *(LUT+2*i*stride+1)
            - *(DAT1-2*i+1) * *(LUT+2*i*stride);
        y2=   *(DAT1-2*i)   * *(LUT+2*i*stride)
            + *(DAT1-2*i+1) * *(LUT+2*i*stride+1);

        *(DAT1-2*i)   = x2;
        *(DAT1-2*i+1) = y2;
    };

    x2=*(DAT+size) - *(DAT+size+1);
    y2=*(DAT+size) + *(DAT+size+1);
    *(DAT+size)  =x2*0.707106781186547524401;
    *(DAT+size+1)=y2*0.707106781186547524401;
}

void fft_fft(double * DAT,uint32 size,double * LUT)
{
    uint32 S1,S2;
    uint32 i,j;
    double x2,y2;

    for(S1=1,S2=size/2;S2>0;S1*=2,S2/=2)
    for(i=0;i<S1;i++)
    for(j=0;j<S2;j++) {
        x2=   *(DAT+S2*4*i+2*j+2*S2)   * *(LUT+2*i)
            - *(DAT+S2*4*i+2*j+2*S2+1) * *(LUT+2*i+1);
        y2=   *(DAT+S2*4*i+2*j+2*S2)   * *(LUT+2*i+1)
            + *(DAT+S2*4*i+2*j+2*S2+1) * *(LUT+2*i);

        *(DAT+S2*4*i+2*j+2*S2)   = *(DAT+S2*4*i+2*j)   - x2;
        *(DAT+S2*4*i+2*j+2*S2+1) = *(DAT+S2*4*i+2*j+1) - y2;

        *(DAT+S2*4*i+2*j)        = *(DAT+S2*4*i+2*j)   + x2;
        *(DAT+S2*4*i+2*j+1)      = *(DAT+S2*4*i+2*j+1) + y2;
    };
}

void fft_mulpw(double * A,double * B,double * C,uint32 size)
{
    uint32 i;
    double x2,y2;

    for(i=0;i<size;i++) {
        x2=*(A+2*i) * *(B+2*i)   - *(A+2*i+1) * *(B+2*i+1);
        y2=*(A+2*i) * *(B+2*i+1) + *(A+2*i+1) * *(B+2*i);

        *(C+2*i)   = x2;
        *(C+2*i+1) = y2;
    };
}

void fft_ifft(double * DAT,uint32 size,double * LUT)
{
    uint32 S1,S2;
    uint32 i,j;
    double x2,y2;

    for(S1=size/2,S2=1;S1>0;S1/=2,S2*=2)
    for(i=0;i<S1;i++)
    for(j=0;j<S2;j++) {
        x2=*(DAT+S2*4*i+2*j)   - *(DAT+S2*4*i+2*j+2*S2);
        y2=*(DAT+S2*4*i+2*j+1) - *(DAT+S2*4*i+2*j+2*S2+1);

        *(DAT+S2*4*i+2*j)   += *(DAT+S2*4*i+2*j+2*S2);
        *(DAT+S2*4*i+2*j+1) += *(DAT+S2*4*i+2*j+2*S2+1);

        *(DAT+S2*4*i+2*j+2*S2)   =  x2 * *(LUT+2*i  ) + y2 * *(LUT+2*i+1);
        *(DAT+S2*4*i+2*j+2*S2+1) = -x2 * *(LUT+2*i+1) + y2 * *(LUT+2*i);
    };
}

void fft_iracw(double * DAT,uint32 size,uint32 stride,double * LUT)
{
    uint32 i;
    double x2,y2;
    double * DAT1 = DAT + 2*size;

    for(i=1;i<size/2;i++) {
        x2=   *(DAT+2*i)   * *(LUT+2*i*stride)
            + *(DAT+2*i+1) * *(LUT+2*i*stride+1);
        y2= - *(DAT+2*i)   * *(LUT+2*i*stride+1)
            + *(DAT+2*i+1) * *(LUT+2*i*stride);

        *(DAT+2*i)   = x2;
        *(DAT+2*i+1) = y2;

        x2=   *(DAT1-2*i)   * *(LUT+2*i*stride+1)
            + *(DAT1-2*i+1) * *(LUT+2*i*stride);
        y2= - *(DAT1-2*i)   * *(LUT+2*i*stride)
            + *(DAT1-2*i+1) * *(LUT+2*i*stride+1);

        *(DAT1-2*i)   = x2;
        *(DAT1-2*i+1) = y2;
    };

    x2=   *(DAT+size) + *(DAT+size+1);
    y2= - *(DAT+size) + *(DAT+size+1);
    *(DAT+size)  =x2*0.707106781186547524401;
    *(DAT+size+1)=y2*0.707106781186547524401;
}

void fft_normalize(double * DAT,uint32 size)
{
    uint32 i;
    double isize;

    isize=1.0/(double)(size);

    for(i=0;i<size*2;i++) {
        *(DAT+i) = *(DAT+i)*isize + 6755399441055744.;
        *(DAT+i) -= 6755399441055744.;
    };
}

