#ifndef __ARSLEXIS_BASE_TYPES_HPP__
#define __ARSLEXIS_BASE_TYPES_HPP__

#include <string>

typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;

#if defined(_WIN32_WCE) || defined(_WIN32)

#include <windows.h>
#include <tchar.h>

namespace ArsLexis
{
    
    typedef DWORD tick_t;

    typedef TCHAR char_t;
    
    typedef SCODE status_t;

# define tstrlen _tcslen
# define tprintf _stprintf
# define ticks	GetTickCount

#else
    
namespace ArsLexis
{
    
    typedef char char_t;

# define _T(a) a

# if defined(_PALM_OS)    
		
    typedef UInt32 tick_t;
    
    typedef Err status_t;

#  define ticks TimGetTicks
    
#  define tprintf StrPrintF
#  define tstrlen StrLen
#  define tstrcmp StrCompare
        
# else
        
#  define tprintf sprintf
#  define tstrlen strlen
#  define tstrcmp strcmp

    typedef long tick_t;
    
    typedef int status_t;
        
# endif
    
#endif

    typedef std::basic_string<char_t> String;

}  

#endif