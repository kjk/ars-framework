#ifndef __ARSLEXIS_BASE_TYPES_HPP__
#define __ARSLEXIS_BASE_TYPES_HPP__

#if defined(__MWERKS__)
#include <cstdint>
#endif

#include <string>

typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned long ulong_t;

namespace ArsLexis
{
    
#if defined(_WIN32_WCE)

	typedef DWORD tick_t;

    typedef TCHAR char_t;

	#define tstrlen _tcslen
	#define tprintf _stprintf
	#define ticks	GetTickCount
    
#else
    
    typedef char char_t;
    
    #define _T(a) a

    #if defined(_PALM_OS)    
		
		typedef UInt32 tick_t;

		#define ticks TimGetTicks
    
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