#ifndef __ARSLEXIS_BASE_TYPES_HPP__
#define __ARSLEXIS_BASE_TYPES_HPP__

#if defined(__PALMOS_H__)    
#include <cstdint>
#endif
#include <string>

typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;

namespace ArsLexis
{
    
#if defined(_WIN32_WCE)

    typedef TCHAR char_t;

	#define tstrlen _tcslen
	#define tprintf _stprintf
    
#else
    
    typedef char char_t;
    
    #define _T(a) a

    #if defined(__PALMOS_H__)    
    
        #define tprintf StrPrintF
        #define tstrlen StrLen
        #define tstrcmp StrCompare
        
    #else
        
        #define tprintf sprintf
        #define tstrlen strlen
        #define tstrcmp strcmp
        
    #endif
    
#endif

    typedef std::basic_string<char_t> String;

}  

#endif