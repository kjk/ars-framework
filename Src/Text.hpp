#ifndef __ARSLEXIS_TEXT_HPP__
#define __ARSLEXIS_TEXT_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <algorithm>

#if !defined(_WIN32)
#include <cctype>
#endif

#if defined(_MSC_VER)
// disable warning C4800: 'int' : forcing value to bool 'true' or 'false' (performance warning)
// TODO: move it to a more centralized place (like BaseTypes.hpp) ?
# pragma warning( disable : 4800 )
#endif

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
    
    struct CharToByte 
    { 
        char operator()(char_t in) {return char(in);}
    };
    
    struct ByteToChar 
    { 
        char_t operator()(char in) {return char_t(in);}
    };

    inline void TextToByteStream(const String& inTxt, NarrowString& outStream)
    {
#if defined(_WIN32)
        /*Why this doesn't work I have no idea
        char *out=NULL;
        int size = WideCharToMultiByte(CP_OEMCP, WC_SEPCHARS, inTxt.c_str(), -1, out, 0, NULL,NULL);
        out=new char[size];
        WideCharToMultiByte(CP_OEMCP, WC_SEPCHARS, inTxt.c_str(), -1, out, size, NULL,NULL);
        outStream.assign(out);
        delete []out;*/
        outStream.reserve(inTxt.length());
        std::transform(inTxt.begin(), inTxt.end(), std::back_inserter(outStream), CharToByte());
#else
        outStream.assign(inTxt);
#endif
    }

    inline void ByteStreamToText(const NarrowString& inStream, String& outTxt)
    {
#if defined(_WIN32)
        /*Why this doesn't work I have no idea
        char_t *out=NULL;
        int size = MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, inStream.c_str(), -1, out, 0);
        out=new char_t[size];
        MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, inStream.c_str(), -1, out, size);
        outTxt.assign(out);
        delete []out;*/
        outTxt.reserve(inStream.length());
        std::transform(inStream.begin(), inStream.end(), std::back_inserter(outTxt), ByteToChar());
#else
        outTxt.assign(inStream);
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
    
    void urlEncode(String& out, const String& in);
    
    void eraseStart(String& str, String::size_type length);

#if defined(_PALM_OS)
    int formatNumber(long num, char *buf, int bufSize);
#endif // _PALM_OS

}

#endif
