/*
  Copyright 2001,2002 Colin Percival and the University of Oxford

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
/*
  $Id: ptypes.h,v 1.8 2002/08/23 17:43:34 cperciva Exp $
*/
/*
  New conditional declarations should be added to this file as it is ported
  to new compilers and processors.
*/

#ifndef PTYPES_H
#define PTYPES_H

#ifdef __WATCOMC__
#ifdef __386__
#define INT8_CHAR
#define INT16_SHORTINT
#define INT32_INT
#define LITTLE_ENDIAN
#else
#error Unknown processor target in ptypes.h
#endif
#elif defined(__GNUC__)

#include <machine/endian.h>
#include <machine/ansi.h>
#if BYTE_ORDER == LITTLE_ENDIAN
#undef BIG_ENDIAN
#elif BYTE_ORDER == BIG_ENDIAN
#undef LITTLE_ENDIAN
#else
#error BYTE_ORDER not defined in machine/endian.h
#endif
typedef __int8_t sint8;
typedef __uint8_t uint8;
typedef __int16_t sint16;
typedef __uint16_t uint16;
typedef __int32_t sint32;
typedef __uint32_t uint32;

#elif defined(_WIN32)
#if _M_IX86>=300
#define INT8_CHAR
#define INT16_SHORTINT
#define INT32_INT
#define LITTLE_ENDIAN
#else
#error Unknown processor target in ptypes.h
#endif

#elif defined(__MWERKS__)
#define INT8_CHAR
#define INT16_SHORTINT
#define INT32_LONG
#ifndef BIG_ENDIAN
#define BIG_ENDIAN
#endif

#else
#error Unknown compiler in ptypes.h
#endif

#ifdef INT8_CHAR
typedef signed char sint8;
typedef unsigned char uint8;
#endif
#ifdef INT16_SHORTINT
typedef signed short int sint16;
typedef unsigned short int uint16;
#endif
#ifdef INT32_INT
typedef signed int sint32;
typedef unsigned int uint32;
#endif
#ifdef INT32_LONG
typedef signed long sint32;
typedef unsigned long uint32;
#endif

#define bswap32(x) (((x<<24)&0xFF000000)|((x<<8)&0x00FF0000)|(((uint32)x>>8)&0x0000FF00)|(((uint32)x>>24)&0x000000FF))
#define bswap16(x) (((x<<8)&0xFF00)|(((uint16)x>>8)&0x00FF))

#ifdef LITTLE_ENDIAN
#define int32_big(x) bswap32(x)
#define int16_big(x) bswap16(x)
#define int32_little(x) (x)
#define int16_little(x) (x)
#else
#define int32_big(x) (x)
#define int16_big(x) (x)
#define int32_little(x) bswap32(x)
#define int16_little(x) bswap16(x)
#endif

#endif
