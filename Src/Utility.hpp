#ifndef __ARSLEXIS_UTILITY_HPP__
#define __ARSLEXIS_UTILITY_HPP__

#include "Debug.hpp"
#include "BaseTypes.hpp"

namespace ArsLexis
{

    template<typename T>
    struct ObjectDeleter
    {
        void operator() (T* object) const 
        {
            delete object;
        }
    };

    struct C_StringLess
    {
        bool operator ()(const char* str1, const char* str2) const
        {return StrCompare(str1, str2)<0;}
    };
    
    bool startsWith(const String& text, const char_t* start, uint_t startOffset=0);
    
    bool startsWith(const String& text, const String& start, uint_t startOffset=0);
    
    bool startsWithIgnoreCase(const String& text, const char_t* start, uint_t startOffset=0);
    
}

#endif