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
/*
  $Id: aes.h,v 1.3 2002/04/25 06:13:53 cperciva Exp $
*/

#include "ptypes.h"

#define aes_128 10
#define aes_256 14

#define aes_keyexpsize128 (aes_128+1)*16
#define aes_keyexpsize256 (aes_256+1)*16

extern void aes_keyexpand128(uint8 * key,uint8 * keyexp);
extern void aes_keyexpand256(uint8 * key,uint8 * keyexp);
extern void aes_key2dec(uint8 * src,uint8 *dest,uint8 rounds);
extern void aes_blockenc(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds);
extern void aes_blockdec(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds);
extern void aes_cbcenc(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds,uint32 blocks);
extern void aes_cbcdec(uint8 * src,uint8 * dest,
                        uint8 * keyexp,uint8 rounds,uint32 blocks);

