#ifndef __ARSLEXIS_BASE_TYPES_HPP__
#define __ARSLEXIS_BASE_TYPES_HPP__

#include <string>
#include <ErrBase.h>

typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;

#ifdef _MSC_VER
# pragma warning (disable : 4786)
#endif

#if defined(_WIN32_WCE) || defined(_WIN32)

#include <windows.h>
#include <tchar.h>

namespace ArsLexis
{
    struct EventType
    {
        int type;
    };

    typedef DWORD tick_t;

    typedef TCHAR char_t;
    
    typedef SCODE status_t;

# define tstrlen _tcslen
# define tprintf _stprintf
# define ticks   GetTickCount

#else

namespace ArsLexis
{
    
    typedef char char_t;

# define _T(a) a

# if defined(_PALM_OS)    

    typedef UInt32 tick_t;
    
    // One would ask: why some stupid status_t and not error_t? Because PalmOS Cobalt is coming...
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

#if !defined(chrNull)
# define chrNull _T('\0')
#endif

#endif // __ARSLEXIS_BASE_TYPES_HPP__