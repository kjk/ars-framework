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
  "$Id: rand.c,v 1.3 2002/04/25 06:12:54 cperciva Exp $";
#endif

#include <string.h>
#include "hash.h"
#include "os.h"

#ifdef _OS_POSIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#else
#include <windows.h>
#endif

struct {
    uint32 outputcounter;
    uint32 mixptr;
    uint8 pool[128];
    uint8 digest[16];
} entropy;

void rand_init(void)
{
    memset(&entropy,0,sizeof(entropy));
}

void rand_addbytes(uint8 * buf,uint32 len)
{
    uint32 pos;
    uint32 i;

    for(pos=0;pos+16<len;pos+=16) {
        hash_data(buf+pos,16,entropy.digest);
        hash_data((uint8 *)&entropy,sizeof(entropy),entropy.digest);
        for(i=0;i<16;i++) entropy.pool[entropy.mixptr+i]^=entropy.digest[i];
        entropy.mixptr=(entropy.mixptr+16)%sizeof(entropy.pool);
    };

    hash_data(buf+pos,len-pos,entropy.digest);
    hash_data((uint8 *)&entropy,sizeof(entropy),entropy.digest);
    for(i=0;i<16;i++) entropy.pool[entropy.mixptr+i]^=entropy.digest[i];
    entropy.mixptr=(entropy.mixptr+16)%sizeof(entropy.pool);
}

void rand_getbytes(uint8 * buf,uint32 len)
{
    uint32 pos;

    for(pos=0;pos+16<len;pos+=16) {
        entropy.outputcounter++;
        hash_data((uint8 *)&entropy,
                   sizeof(entropy)-sizeof(entropy.digest),buf+pos);
    };

    entropy.outputcounter++;
    hash_data((uint8 *)&entropy,
                   sizeof(entropy)-sizeof(entropy.digest),entropy.digest);
    for(;pos<len;pos++) *(buf+pos)=entropy.digest[pos%16];
}

int rand_selfseed(void)
{
    uint8 buf[128];

#ifdef _OS_POSIX
    int fd;
    uint32 a,i;
    clock_t t,t0,t1;

/* Try /dev/urandom -- it might exist.  If not, fall back to using clock() */
    if((fd=open("/dev/urandom",O_RDONLY))==-1) goto ANSIC;
    if(read(fd,buf,128)!=128) goto ANSIC;
    rand_addbytes(buf,128);
    return 0;

ANSIC:
    if((CLOCKS_PER_SEC<100)||(CLOCKS_PER_SEC>10000)) return -1;
    t0=clock();
    for(i=0,t1=t0;t<t0+CLOCKS_PER_SEC*5;i++) {
        for(a=0,t=t1;t1==t;a++) t1=clock();
        *(uint32 *)(buf+(i%32)*4)=a+t1;
        if((i%32)==31) rand_addbytes(buf,128);
    };
    if(i%32) rand_addbytes(buf,(i%32)*4);
    if(i<400) return -1; else return 0;
#else
    LARGE_INTEGER freq,t,t1;
    uint32 i;

    if(!QueryPerformanceFrequency(&freq)) return -1;
    if(!QueryPerformanceCounter(&t)) return -1;
    t1.HighPart=t.HighPart+freq.HighPart;
    t1.LowPart=t.LowPart+freq.LowPart;
    if(t1.LowPart<t.LowPart) t1.HighPart++;

    if(!QueryPerformanceCounter(&t)) return -1;
    for(i=0;(t.HighPart<t1.HighPart)||
            ((t.HighPart==t1.HighPart)&&(t.LowPart<t1.LowPart));i++) {
        *(uint32 *)(buf+(i%32)*4)=t.LowPart;
        if(!QueryPerformanceCounter(&t)) return -1;
        if((i%32)==31) rand_addbytes(buf,128);
    };
    if(i%32) rand_addbytes(buf,(i%32)*4);

    if(i<5000) return -1; else return 0;
#endif
}

void rand_done(void)
{
    memset((uint8 *)&entropy,0,sizeof(entropy));
}
