#ifndef __ARSLEXIS_TEXT_HPP__
#define __ARSLEXIS_TEXT_HPP__

#include <BaseTypes.hpp>

#if !(defined(_WIN32_WCE) || defined(_WIN32))
#include <cctype>
#endif

#if defined(_MSC_VER)
// disable warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
// TODO: move it to a more centrilsed place (like BaseTypes.hpp) ?
#pragma warning( disable : 4800 )
#endif

namespace ArsLexis
{

    inline char_t toLower(char_t chr)
    {
#if defined(_WIN32_WCE) || defined(_WIN32)
        return static_cast<char_t>(_totlower(chr));
#else
        return std::tolower(chr);
#endif        
    }
    
    inline bool isAlpha(char_t chr)
    {
#if defined(_WIN32_WCE) || defined(_WIN32)
        return _istalpha(chr);
#else
        return std::isalpha(chr);
#endif
    }
    
    inline bool isAlNum(char_t chr)
    {
#if defined(_WIN32_WCE) || defined(_WIN32)
        return _istalnum(chr);
#else
        return std::isalnum(chr);
#endif
    }

    inline bool isDigit(char_t chr)
    {
#if defined(_WIN32_WCE) || defined(_WIN32)
        return _istdigit(chr);
#else
        return std::isdigit(chr);
#endif
    }

    inline bool isSpace(char_t chr)
    {
#if defined(_WIN32_WCE) || defined(_WIN32)
        return _istspace(chr);
#else
        return std::isspace(chr);
#endif
    }

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

    status_t numericValue(const char_t* begin, const char_t* end, long& result, uint_t base=10);
    
    inline status_t numericValue(const String& text, long& result, uint_t base=10)
    {return numericValue(text.data(), text.data()+text.length(), result, base);}
    
    String hexBinEncode(const String& in);

    inline void hexBinEncodeInPlace(String& inOut)
    {inOut=hexBinEncode(inOut);}
    
    void urlEncode(String& out, const String& in);
    
    void eraseStart(String& str, String::size_type length);
   
}

#endif
