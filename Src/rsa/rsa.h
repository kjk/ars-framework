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
  $Id: rsa.h,v 1.2 2002/04/25 06:13:52 cperciva Exp $
*/

#include "ptypes.h"

extern int rsa_encrypt(uint8 * ptext,uint32 datalen,uint8 * ctext,
                        uint8 * pub,uint32 len,uint8 * T1);
extern int rsa_decrypt(uint8 * ctext,uint8 * ptext,
                        uint8 * pub,uint8 * priv,uint32 len,uint8 * T1);
extern int rsa_sign(uint8 * ptext,uint32 datalen,uint8 * ctext,
                        uint8 * pub,uint8 * priv,uint32 len,uint8 * T1);
extern uint32 rsa_verify(uint8 * ctext,uint8 * ptext,
                        uint8 * pub,uint32 len,uint8 * T1);
extern void rsa_keygen(uint8 * r,uint8 * pub,uint8 * priv,uint32 len,uint8 * T1);

