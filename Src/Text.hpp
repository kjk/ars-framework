#ifndef __ARSLEXIS_TEXT_HPP__
#define __ARSLEXIS_TEXT_HPP__

#include <BaseTypes.hpp>

namespace ArsLexis
{

    template<class Ch>
    struct C_StringLess
    {
        bool operator ()(const Ch* str1, const Ch* str2) const
        {
            return StrCompare(str1, str2)<0;
        }
    };
    
    bool startsWith(const String& text, const char_t* start, uint_t startOffset=0);
    
    bool startsWith(const String& text, const String& start, uint_t startOffset=0);
    
    bool startsWithIgnoreCase(const String& text, const char_t* start, uint_t startOffset=0);
    
    bool equalsIgnoreCase(const char_t* s1start, const char_t* s1end, const char_t* s2start, const char_t* s2end);
    
    inline bool equalsIgnoreCase(const String& s1, const String& s2)
    {return equalsIgnoreCase(s1.data(), s1.data()+s1.length(), s2.data(), s2.data()+s2.length());}

    status_t numericValue(const char* begin, const char* end, long& result, uint_t base=10);
    
    inline status_t numericValue(const String& text, long& result, uint_t base=10)
    {
        return numericValue(text.data(), text.data()+text.length(), result, base);
    }
    
    String hexBinEncode(const String& in);

    inline void hexBinEncodeInPlace(String& inOut)
    {
        inOut=hexBinEncode(inOut);
    }
    
}

#endif