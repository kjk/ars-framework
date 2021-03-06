#ifndef ARSLEXIS_BASE_TYPES_HPP__
#define ARSLEXIS_BASE_TYPES_HPP__

#if defined(_MSC_VER)

namespace std {
    
    typedef __int8 int8_t;
    
    typedef unsigned __int8 uint8_t;
    
    typedef __int16 int16_t;
    
    typedef unsigned __int16 uint16_t;
    
    typedef __int32 int32_t;
    
    typedef unsigned __int32 uint32_t;
}

#else
#  include <cstdint>
#endif

#include <string>
#include <ErrBase.h>

typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;

#ifdef _MSC_VER
# pragma warning (disable : 4786)
#endif

#if defined(_WIN32)

# include <windows.h>
# include <tchar.h>

#ifdef small
#undef small
#endif

#if _MSC_VER == 1200

#pragma warning(disable: 4530)

# define ErrTry __try
# define ErrCatch(theErr) __except (EXCEPTION_EXECUTE_HANDLER) { DWORD theErr=GetExceptionCode();
# define ErrEndCatch }
# define ErrThrow(err) RaiseException((err), 0, 0, NULL); 

#else

# define ErrTry try
# define ErrCatch(theErr) catch (long theErr) 
# define ErrEndCatch 
# define ErrThrow(err) throw(long(err))

#endif

# define ErrReturn(val) return (val)

namespace ArsLexis
{
    typedef DWORD tick_t;

    typedef TCHAR char_t;
    
    typedef SCODE status_t;

    typedef unsigned short LocalID;
    
    typedef std::basic_string<char>   NarrowString;

# define tstrlen   _tcslen
# define tstrcmp   _tcscmp
# define tstrcpy   _tcscpy
# define tstrncmp  _tcsncmp
# define tprintf   _stprintf
# define ticks     GetTickCount
# define memzero(data,size)  memset((data), 0, (size))
# define StrPrintF sprintf

    static inline tick_t ticksPerSecond() { return 1000; } // Win32 uses fixed 1-millisecond ticks.

    
// Using xtoi functions is dangerous as they don't have any way to inform on invalid format. Use ArsLexis::numericValue() instead.
//# define tatoi _wtoi

#else

namespace ArsLexis
{
    
    typedef char char_t;

# define _T(a) a

# if defined(_PALM_OS)    

    typedef UInt32 tick_t;
    
    typedef Err status_t;
    
    typedef std::basic_string<char>   NarrowString;

#  define ticks TimGetTicks

    static inline tick_t ticksPerSecond() {return SysTicksPerSecond();}
    
#  define tprintf StrPrintF
#  define tstrlen StrLen
#  define strlen  StrLen
#  define tstrcmp StrCompare
#  define tstrcpy StrCopy
#  define tstrncmp StrNCompare
#  define memmove  MemMove
// yes, MemSet has different order than memset
#  define memzero(data,size)  MemSet((data), (size), 0)

#define ErrReturn(val) *ErrExceptionList() = (MemPtr)_TryObject.nextP; return (val)

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

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(*arr))

typedef ArsLexis::char_t tchar;

#endif // ARSLEXIS_BASE_TYPES_HPP__

