#ifndef __ARSLEXIS_UTILITY_HPP__
#define __ARSLEXIS_UTILITY_HPP__

#include "Debug.hpp"

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
    
    bool isWhitespace(char chr);
    
    void drawCenteredChars(const char* str, Coord left, Coord top, Coord width);
    
}

#endif