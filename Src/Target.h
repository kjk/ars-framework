#if __ide_target("Release")
#include "PalmOS_Headers"

#ifndef NDEBUG
#define NDEBUG
#endif

#else
#include "PalmOS_Headers_Debug"
#endif

#define ARSLEXIS_USE_NEW_FRAMEWORK 1
#define _PALM_OS 

#ifndef NDEBUG
//! Some functions depend on this non-standard symbol instead of standard-compliant @c NDEBUG.
#define DEBUG
#endif

