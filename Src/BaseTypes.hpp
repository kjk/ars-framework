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
#define ErrTryT __try
#define ErrCatchT(theErr) __except (1) { DWORD theErr=GetExceptionCode();
#define ErrEndCatchT

namespace ArsLexis
{
    struct EventType
    {
        int type;
    };

    typedef DWORD tick_t;

    typedef TCHAR char_t;
    
    typedef SCODE status_t;

    typedef unsigned short LocalID;

# define tstrlen _tcslen
# define tprintf _stprintf
# define ticks   GetTickCount
# define tatoi _wtoi
# define tmalloc malloc
# define tfree free
#else

namespace ArsLexis
{
    
    typedef char char_t;

# define _T(a) a
# define ErrTryT ErrTry
# define ErrCatchT(theErr) ErrCatch(theErr) {
# define ErrEndCatchT ErrEndCatch

# if defined(_PALM_OS)    

    typedef UInt32 tick_t;
    
    // One would ask: why some stupid status_t and not error_t? Because PalmOS Cobalt is coming...
    typedef Err status_t;

#  define ticks TimGetTicks
    
#  define tprintf StrPrintF
#  define tstrlen StrLen
#  define tstrcmp StrCompare
#  define tatoi StrAToI
#  define tmalloc MemPtrNew        
#  define tfree MemPtrFree
# else
        
#  define tprintf sprintf
#  define tstrlen strlen
#  define tstrcmp strcmp
#  define tatoi atoi

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