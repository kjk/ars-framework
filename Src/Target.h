#ifndef _IPEDIA_TARGET_H_
#define _IPEDIA_TARGET_H_

#if __ide_target("Release")
#include <PalmOS_Headers>

#ifndef NDEBUG
#define NDEBUG
#endif

#else
#include <PalmOS_Headers_Debug>
#endif

#define INTERNAL_BUILD
#define ARSLEXIS_USE_NEW_FRAMEWORK 1
#define _PALM_OS  1

#ifndef NDEBUG
//! Some functions depend on this non-standard symbol instead of standard-compliant @c NDEBUG.
#define DEBUG
#endif

#else

#error "You must not include file Target.h directly. You should use it as your prefix file."

#endif // _IPEDIA_TARGET_H_