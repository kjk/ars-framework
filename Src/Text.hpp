#ifndef __ARSLEXIS_TEXT_HPP__
#define __ARSLEXIS_TEXT_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <algorithm>
#include <list>
#include <vector>

#if !defined(_WIN32)
#include <cctype>
#endif

#if defined(_MSC_VER)
// disable warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
// TODO: move it to a more centralized place (like BaseTypes.hpp) ?
# pragma warning( disable : 4800 )
#endif

typedef std::list<const ArsLexis::char_t *> CharPtrList_t;
typedef std::list<ArsLexis::String> StringList_t;

namespace ArsLexis
{

    inline char_t toLower(char_t chr)
    {
#if defined(_WIN32)
        return static_cast<char_t>(_totlower(chr));
#else
        return std::tolower(chr);
#endif        
    }
    
    inline bool isAlpha(char_t chr)
    {
#if defined(_WIN32)
        return _istalpha(chr);
#else
        return std::isalpha(chr);
#endif
    }
    
    inline bool isAlNum(char_t chr)
    {
#if defined(_WIN32)
        return _istalnum(chr);
#else
        return std::isalnum(chr);
#endif
    }

    inline bool isDigit(char_t chr)
    {
#if defined(_WIN32)
        return _istdigit(chr);
#else
        return std::isdigit(chr);
#endif
    }

    inline bool isSpace(char_t chr)
    {
#if defined(_WIN32)
        return _istspace(chr);
#else
        return std::isspace(chr);
#endif
    }

    void TextToByteStream(const String& inTxt, NarrowString& outStream);

    void ByteStreamToText(const NarrowString& inStream, String& outTxt);

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
    
    inline bool equalsIgnoreCase(const String& s1, const char_t* s2)
    {
        using namespace std;
        return equalsIgnoreCase(s1.data(), s1.data()+s1.length(), s2, s2+tstrlen(s2));
    }

    inline bool equalsIgnoreCase(const char_t* s2, const String& s1)
    {
        using namespace std;
        return equalsIgnoreCase(s1.data(), s1.data()+s1.length(), s2, s2+tstrlen(s2));
    }

    inline bool equalsIgnoreCase(const char_t* s1, const char_t* s2)
    {
        using namespace std;
        return equalsIgnoreCase(s1, s1+tstrlen(s1), s2, s2+tstrlen(s2));
    }
    
    inline bool equalsIgnoreCase(const String& s1, const String& s2)
    {return equalsIgnoreCase(s1.data(), s1.data()+s1.length(), s2.data(), s2.data()+s2.length());}

    status_t numericValue(const char_t* begin, const char_t* end, long& result, uint_t base=10);
    
    inline status_t numericValue(const String& text, long& result, uint_t base=10)
    {return numericValue(text.data(), text.data()+text.length(), result, base);}
    
    String hexBinEncode(const String& in);

    inline void hexBinEncodeInPlace(String& inOut)
    {inOut=hexBinEncode(inOut);}
    
    void urlEncode(const String& in, String& out);

    void removeNonDigits(const char_t* in, uint_t inLength, String& out);

    inline void removeNonDigits(const String& in, String& out)
    {removeNonDigits(in.data(), in.length(), out);}
    
    inline void removeNonDigits(const char_t* in, String& out)
    {removeNonDigits(in, tstrlen(in), out);}

    int formatNumber(long num, char_t *buf, int bufSize);

    void HexBinEncodeBlob(unsigned char *blob, int blobSize, String& out);

    String GetNextLine(const ArsLexis::String& str, String::size_type& curPos, bool& fEnd);

    char_t *StringCopy(const char_t *str);

    char_t *StringCopy(const String& str);

    char_t *StringCopyN(const char_t *str, int strLen);

    void FreeStringsFromCharPtrList(CharPtrList_t& strList);

    int AddLinesToList(const String& txt, CharPtrList_t& strList);

    char_t **StringListFromString(const String& str, const String& sep, int& stringCount);

    char_t **StringListFromStringList(const StringList_t& strList, int& stringCount);

    void strip(String& str);
    
    std::vector<ArsLexis::String> split(const String& str, const String& splitter = _T(" "));
    
    String join(const std::vector<ArsLexis::String>& vec, const String& joiner = _T(" "));

    void replaceCharInString(char_t *str, char_t orig, char_t replacement);

}

/*
These below are redundant - MSL has implementation of both memmove and memcpy.    
#ifdef _PALM_OS
void memmove(char *dst, char *src, int len);
#define memcpy memmove
#endif
*/

#endif
