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
  $Id: os.h,v 1.4 2002/04/25 06:12:53 cperciva Exp $
*/

#ifdef __WATCOMC__
#ifdef __NT__
#define _OS_WINDOWS 1
#endif
#endif

#ifdef _WIN32
#define _OS_WINDOWS 1
#endif

#ifdef _POSIX
#define _OS_POSIX 1
#endif

#ifdef __GNUC__
#define _OS_POSIX 1
#endif

#ifndef _OS_POSIX
#ifndef _OS_WINDOWS
#error "Can't determine operating system"
#endif
#endif
