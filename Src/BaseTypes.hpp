#ifndef __ARSLEXIS_BASE_TYPES_HPP__
#define __ARSLEXIS_BASE_TYPES_HPP__

#include <cstdint>
#include <string>

typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;

namespace ArsLexis
{
    
#if defined(_WIN32_WCE)

    typedef TCHAR char_t;
    
#else
    
    typedef char char_t;
    
    #define _T(a) a

    #if defined(__PALMOS_H__)    
    
        #define tprintf StrPrintF
        
    #else
        
        #define tprintf sprintf
        
    #endif
    
#endif

    typedef std::basic_string<char_t> String;

}  

#endif