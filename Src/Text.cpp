#include <Text.hpp>

#ifdef __MWERKS__
# pragma pcrelconstdata on
//# pragma inline_bottom_up on
#endif

namespace ArsLexis 
{

    bool startsWith(const String& text, const char_t* start, uint_t startOffset)
    {
        while (startOffset<text.length() && *start)
        {
            if (*start==text[startOffset])
            {
                ++start;
                ++startOffset;
            }
            else
                return false;
        }
        return 0==*start;
    }
    
    bool startsWith(const String& text, const String& start, uint_t startOffset)
    {
        uint_t pos=0;
        while (startOffset<text.length() && pos<start.length())
        {
            if (text[startOffset]==start[pos])
            {
                ++startOffset;
                ++pos;
            }
            else
                return false;
        }
        return pos==start.length();
    }
    
    bool startsWithIgnoreCase(const String& text, const char_t* start, uint_t startOffset)
    {
        while (startOffset<text.length() && *start)
        {
            if (toLower(*start)==toLower(text[startOffset]))
            {
                ++start;
                ++startOffset;
            }
            else
                return false;
        }
        return 0==*start;
    }

    bool equalsIgnoreCase(const char_t* s1start, const char_t* s1end, const char_t* s2start, const char_t* s2end)
    {
        while (s1start!=s1end && s2start!=s2end)
        {
            if (toLower(*s1start)==toLower(*s2start))
            {
                ++s1start;
                ++s2start;
            }
            else 
                return false;
        }
        return (s1start==s1end && s2start==s2end);
    }
    
}

ArsLexis::status_t ArsLexis::numericValue(const char_t* begin, const char_t* end, long& result, uint_t base)
{
    ArsLexis::status_t error=errNone;
    bool     negative=false;
    long     res=0;
    String   numbers(_T("0123456789abcdefghijklmnopqrstuvwxyz"));
    char_t   buffer[2];

    if (begin>=end || base>numbers.length())
    {    
        error=sysErrParamErr;
        goto OnError;           
    }
    if (*begin==_T('-'))
    {
        negative=true;
        if (++begin==end)
        {
            error=sysErrParamErr;
            goto OnError;           
        }
    }           
    buffer[1]=chrNull;
    while (begin!=end) 
    {
        // TODO: will it work with unicode on WINCE?
        buffer[0]=toLower(*(begin++));
        String::size_type num=numbers.find(buffer);
        if (num>=base)
        {   
            error=sysErrParamErr;
            break;
        }
        else
        {
            res*=base;
            res+=num;
        }
    }
    if (!error)
       result=res;
OnError:
    return error;    
}

#define HEX_DIGITS _T("0123456789ABCDEF")

ArsLexis::String ArsLexis::hexBinEncode(const String& in)
{
    String out;
    out.reserve(2*in.length());
    String::const_iterator it=in.begin();
    String::const_iterator end=in.end();
    while (it!=end)
    {
        char_t b=*(it++);
        out.append(1, HEX_DIGITS[(int)b/16]);
        out.append(1, HEX_DIGITS[(int)b%16]);
    }
    return out;
}

namespace {

    inline static void CharToHexString(ArsLexis::char_t* buffer, ArsLexis::char_t chr)
    {
        buffer[0]=HEX_DIGITS[chr/16];
        buffer[1]=HEX_DIGITS[chr%16];
    }

}

void ArsLexis::urlEncode(ArsLexis::String& out, const ArsLexis::String& in)
{
    const char_t* begin=in.data();
    const char_t* end=begin+in.length();
    out.resize(0);
    out.reserve(in.length());
    while (begin!=end)
    {        
        char_t chr=*begin++;
        if ((chr>=_T('a') && chr<=_T('z')) || 
            (chr>=_T('A') && chr<=_T('Z')) || 
            (chr>=_T('0') && chr<=_T('9')) || 
            _T('-')==chr || _T('_')==chr || _T('.')==chr || _T('!')==chr || 
            _T('~')==chr || _T('*')==chr || _T('\'')==chr || _T('(')==chr || _T(')')==chr)
            out.append(1, chr);
        else
        {
            char_t buffer[3];
            *buffer=_T('%');
            CharToHexString(buffer+1, chr);
            out.append(buffer, 3);
        }
    }
}
