#include <Text.hpp>

#ifdef __MWERKS__
# pragma pcrelconstdata on
#endif

namespace ArsLexis 
{

#if defined(_WIN32)    
char_t PalmToUnicode[160-128] = 
{
 8364, 129 , 8218,  131, 8222, 8230, 8224, 8225,  710, 8240, 
  352, 8249,  338, 9674,  142,  143,  144, 8216, 8217, 8220, 
 8221, 8226, 8211, 8212, 8776, 8482,  353, 8250,  339,  157, 
  158,  159
};

char_t UnicodeToPalm[24][2] = 
{
  338, 140,  339, 156,  352, 138,  353, 154,  710, 136, 
 8211, 150, 8212, 151, 8216, 145, 8217, 146, 8218, 130, 
 8220, 147, 8221, 148, 8222, 132, 8224, 134, 8225, 135,
 8226, 149, 8230, 133, 8240, 137, 8249, 139, 8250, 155,
 8364, 128, 8482, 153, 8776, 152, 9674, 141
};

struct CharToByte
{ 
    char operator()(char_t in) 
    {   
        if (in<=255)
        {
            // is it common code ?
            return char(in);
        }
        else // it's unicode char
        {
            // Maybe binary search - MS doesn't support 
            // as always standard and bsearch
            for (int i=0; i<24; i++)
            {
                if (UnicodeToPalm[i][0]==in)
                    return (char)UnicodeToPalm[i][1];
            }
            assert(false); //we never shall reach this point
            return 0;
        }
    }
};

struct ByteToChar 
{ 
    char_t operator()(unsigned char in)
    {
        if ((in>=128)&&(in<=159))
            return PalmToUnicode[in-128];
        else
        {
            if ('\t'==in)
            {
                // special case for tab - Palm ignores it, wince displays
                // as a rectangle
                return char_t(' ');
            }
            return char_t(in);
        }
    }
};
#endif

void TextToByteStream(const String& inTxt, NarrowString& outStream)
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

void ByteStreamToText(const NarrowString& inStream, String& outTxt)
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

static ArsLexis::char_t numToHex(int num)
{
    assert( (num>=0) && (num<16) );
    ArsLexis::char_t c = HEX_DIGITS[num];
    assert( '\0' != c );
    return c;
}

// encode binary blob of blobSize size and put the result in the out string
void ArsLexis::HexBinEncodeBlob(unsigned char *blob, int blobSize, ArsLexis::String& out)
{
    out.clear();
    out.reserve(blobSize*2); // 2 chars per each byte

    unsigned char b;
    char_t        hexChar;
    for (int i=0; i<blobSize; i++)
    {
        b = blob[i];
        hexChar = numToHex(b / 16);
        out.append(1,hexChar);
        hexChar = numToHex(b % 16);
        out.append(1,hexChar);
    }
}

ArsLexis::String ArsLexis::hexBinEncode(const String& in)
{
    String out;
    out.reserve(2*in.length());
    String::const_iterator it=in.begin();
    String::const_iterator end=in.end();
    char_t hexChar;
    while (it!=end)
    {
        // at some point this was char_t (i.e. signed char on Palm)
        // and it caused bugs due to b being promoted to unsigned int, which
        // was negative for b values > 127
        unsigned char b=*(it++);
        hexChar = numToHex(b / 16);
        out.append(1,hexChar);
        hexChar = numToHex(b % 16);
        out.append(1,hexChar);
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

void ArsLexis::urlEncode(const ArsLexis::String& in, ArsLexis::String& out)
{
    char_t *hexNum = _T("%  ");

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
            out.append(chr, 1);
        else
        {
            CharToHexString(hexNum+1, chr);
            out.append(hexNum, 3);
        }
    }
}

void ArsLexis::removeNonDigits(const char_t* in, uint_t len, ArsLexis::String& out)
{
    out.resize(0);
    out.reserve(len);

    while (len>0)
    {
        if (isDigit(*in))
            out.append(1, *in);
        ++in;
        --len;
    }
}

// format number num so that they easier to read e.g. turn '10343' into '10.343'
// i.e. insert (locale-dependent) thousand separator in apropriate places
// put the result in buffer buf of length bufSize. Buffer must be big enough
// for the result.
int ArsLexis::formatNumber(long num, char_t* buf, int bufSize)
{
    char thousand=',';
 
#if defined(_PALM_OS)
    char dontMind;
    NumberFormatType nf=static_cast<NumberFormatType>(PrefGetPreference(prefNumberFormat));
    LocGetNumberSeparators(nf, &thousand, &dontMind);
#endif 
   
    char_t buffer[32];
    int len = tprintf(buffer, _T("%ld"), num);
    int lenOut = len + ((len-1)/3);  // a thousand separator every 3 chars
    assert(bufSize>=lenOut+1);
    // copy str in buffer to output buf from the end, adding thousand separator every 3 chars
    char_t *tmp = buffer+len;
    assert( '\0' == *tmp );
    char_t *out = buf+lenOut;
    int toDot = 4; // 3 + 1 for trailing '\0'
    while (true)
    {
        *out = *tmp;
        if (tmp==buffer)
            break;
        --out;
        --tmp;
        --toDot;
        if (0==toDot)
        {
            toDot = 3;
            *out-- = thousand;  // don't put separator if this is the last number
        }
    }
    assert(out==buf);
    return lenOut;
}

// GetNextLine is for iterating over lines in a string.
// given a string str and curPos which is a valid index within str, return
// a substring from curPos until newline or end of string. Removes the newline
// from the string. Updates curPos so that it can be called in sequence.
// Sets fEnd to true if there are no more lines.
// Handles the following kinds of new lines: "\n", "\r", "\n\r", "\r\n"
String GetNextLine(const String& str, String::size_type& curPos, bool& fEnd)
{
    fEnd = false;
    if (curPos==str.length())
    {
        fEnd = true;
        return String();
    }

    String::size_type lineStartPos = curPos;
    String::size_type lineEndPos;
    String::size_type delimPos   = str.find_first_of(_T("\n\r"), lineStartPos);

    if (String::npos == delimPos)
    {
        lineEndPos = str.length()-1;
        curPos = str.length();     
    }
    else
    {
        lineEndPos = delimPos;
        assert ( (_T('\n')==str[lineEndPos]) || (_T('\r')==str[lineEndPos]));

        curPos = delimPos+1;
        while ( (_T('\n')==str[curPos]) || (_T('\r')==str[curPos]))
        {
            curPos++;
        }
    }
    String::size_type lineLen = lineEndPos - lineStartPos;
    return str.substr(lineStartPos, lineLen);
}

// note: caller needs to free memory with delete[]
char_t* StringCopy(const char_t *curStr)
{
    using namespace std;
    int len = tstrlen(curStr);
    char_t *newStr = new char_t[len+1];
    if (NULL==newStr)
        return NULL;

    memcpy(newStr, curStr, len*sizeof(char_t));
    newStr[len] = chrNull;
    return newStr;
}    

// note: caller needs to free memory with delete[]
char_t* StringCopyN(const char_t* str, int strLen)
{
    using namespace std;
    char_t *newStr = new char_t[strLen+1];
    if (NULL==newStr)
        return NULL;
    memcpy(newStr, str, strLen*sizeof(char_t));
    newStr[strLen] = chrNull;
    return newStr;
}

// note: caller needs to free memory with delete[]
char_t* StringCopy(const String& str)
{
    return StringCopyN(str.data(), str.length());
}

// given a list of strings, free memory taken by each string in the list.
void FreeStringsFromCharPtrList(CharPtrList_t& strList)
{
    CharPtrList_t::iterator iter = strList.begin();
    CharPtrList_t::iterator iterEnd = strList.end();

    const char_t *strToDelete;
    do {
        strToDelete = *iter;
        delete [] (char_t*)strToDelete;
        iter++;
    } while (iter!=iterEnd);
}

// given a string txt containing a new-line separated list of strings,
// edd each line of text to strList. Return number of added strings
// Makes copies of the string so client needs to delete those strings
// after use e.g. using FreeStringsFromCharPtrList
int AddLinesToList(const String& txt, CharPtrList_t& strList)
{
    const char_t *wordTxt;
    String::size_type curPos = 0;
    bool fEnd;
    String word;

    int  count = 0;
    while (true)
    {
        word = ArsLexis::GetNextLine(txt, curPos, fEnd);
        if (fEnd)
            break;
        wordTxt = StringCopy(word);
        strList.push_back(wordTxt);
        ++count;
    }
    return count;
}

void strip(String& str)
{
    String::size_type left = 0;
    String::size_type right = str.length();
    while (isSpace(str[left]) && left < right)
        left++;
    while (isSpace(str[right-1]) && left < right)
        right--;
    if (left == right)
        str.clear();
    else
        str.assign(str, left, right-left);
}

/*
std::vector<ArsLexis::String> split(const String& str, const String& spliter)
{
    std::vector<ArsLexis::String> vec;
    String::size_type curPos = 0;
    String::size_type spliterPos = 0;
    while (String::npos != spliterPos)
    {
        spliterPos = str.find(spliter,curPos);
        if (String::npos != spliterPos)
        {
            vec.push_back(String(str,curPos,spliterPos-curPos));
            curPos = spliterPos + spliter.length();
        }    
        else
        {
            vec.push_back(String(str,curPos));
        }
    }
    return vec;
}
*/

std::vector<ArsLexis::String> split(const String& str, const char_t* splitter)
{
    std::vector<ArsLexis::String> vec;
    String::size_type curPos = 0;
    String::size_type spliterPos = 0;
    uint_t slen = tstrlen(splitter);
    while (String::npos != spliterPos)
    {
        spliterPos = str.find(splitter, curPos);
        if (String::npos != spliterPos)
        {
            vec.push_back(String(str, curPos, spliterPos-curPos));
            curPos = spliterPos + slen;
        }    
        else
        {
            vec.push_back(String(str,curPos));
        }
    }
    return vec;
}


// ok, the name sucks
char_t **StringListFromStringList(const StringList_t& strList, int& stringCount)
{

    char_t **result;
    stringCount = strList.size(); 
    if (0==stringCount)
        return NULL;

    result = new char_t *[stringCount];
    if (NULL == result)
        return NULL;

    int curPos = 0;
    StringList_t::const_iterator end(strList.end());
    for (StringList_t::const_iterator it(strList.begin()); it!=end; ++it)
    {
        result[curPos] = StringCopy(*it);
        ++curPos;
    }
    return result;
}
    
// Given a String str returns an array of pointers to strings (a string list).
// Returns the 
char_t **StringListFromString(const String& str, const String& sep, int& stringCount)
{
    char_t **strList = NULL;

    String::size_type curPos;
    String::size_type sepPos;
    int stringCountTmp = 0;
    int strLen;
    int strStart;
    const char_t *origStr = str.c_str();

    // phase == 0 -> we calculate number of strings
    // phase == 1 -> we put copies of string into strList
    for (int phase=0; phase<=1; phase++)
    {
        curPos = 0;
        sepPos = 0;
        if (1==phase)
        {
            assert(0!=stringCountTmp);
            stringCount = stringCountTmp;
            strList = new char_t *[stringCount];
            if (NULL == strList)
                return NULL;
            stringCountTmp = 0;
        }

        while (String::npos != sepPos)
        {
            strStart = curPos;
            sepPos = str.find(sep,curPos);
            if (String::npos != sepPos)
            {
                strLen = sepPos - curPos;
                curPos = sepPos + sep.length();
            }    
            else
            {
                strLen = str.length() - curPos;
            }
            if (1==phase)
            {
                strList[stringCountTmp] = StringCopyN(&origStr[strStart], strLen);
            }
            stringCountTmp += 1;
        }
    }
    return strList;
}

void FreeStringList(char_t* strList[], int strListLen)
{
    for (int i = 0; i < strListLen; ++i)
        delete [] strList[i];
    delete [] strList;
}

   
String join(const std::vector<ArsLexis::String>& vec, const char_t* joiner)
{
    String str;
    for (uint_t i=0; i < vec.size(); i++)
    {
        if (i != 0)
            str.append(joiner);
        str.append(vec[i]);
    }
    return str;
}

void replaceCharInString(char_t* str, char_t orig, char_t replacement)
{
    while (_T('\0')!=*str)
    {
        if (*str==orig)
            *str = replacement;
        ++str;
    }
}

void ReverseStringList(char_t **strList, int strListLen)
{
    char_t *tmp;
    int strListLenHalf = strListLen / 2;
    for (int i=0; i<strListLenHalf; i++)
    {
        tmp = strList[i];
        strList[i] = strList[strListLen-i-1];
        strList[strListLen-i-1] = tmp;
    }
}

void convertFloatStringToUnsignedLong(const ArsLexis::String str, unsigned long& value, unsigned int& digitsAfterComma, ArsLexis::char_t commaSeparator)
{
    digitsAfterComma = 0;
    value = 0;
    bool wasComma = false;
    for (uint_t i=0; i < str.size(); i++)
    {
        if (isDigit(str[i]))
        {
            // is this good method?
            unsigned long digit = str[i] - _T('0');
            assert(0 <= digit && 10 > digit);
            value = 10*value + digit;
            if (wasComma)
                digitsAfterComma++;        
        }
        else if (commaSeparator == str[i])
            //if we want to return errors we can do it when more than one comma apears
            wasComma = true;
    }
}

ArsLexis::String convertUnsignedLongWithCommaToString(unsigned long value, unsigned int comma, ArsLexis::char_t commaSymbol, ArsLexis::char_t kSeparator)
{
    char_t buffer[64];
    tprintf(buffer,_T("%lu"),value);
    String str = buffer;
    //fill with zeros
    while (comma >= str.size())
        str.insert(str.begin(), _T('0'));
    //place comma
    int separator = str.size() - comma - 3;
    while (comma > 0 && _T('0') == str[str.size()-1])
    {
        comma--;
        str.erase(str.end()-1);    
    }        
    if (comma > 0)
        str.insert(str.begin()+str.size()-comma, commaSymbol);
    //use separators
    while (separator > 0)
    {
        str.insert(str.begin()+separator, kSeparator);
        separator -= 3;    
    }
    return str;
}


uint_t fuzzyTimeInterval(ulong_t interval, char_t* buffer)
{
    interval += 30;
    interval /= 60;
    const char_t* pattern;
    if (interval < 14)
    {
        if (1 >= interval)
            pattern = _T("a minute");
        else
            pattern = _T("%ld minutes");
        goto Done;
    }
    interval += 8;
    interval /= 15;
    if (interval < 3)
    {
        if (1 >= interval)
            pattern = _T("a quarter");
        else
            pattern = _T("half an hour");
        goto Done;
    }
    interval += 2;
    interval /= 4;
    if (interval < 8)
    {
        if (1 >= interval)
            pattern = _T("an hour");
        else
            pattern = _T("%ld hours");
        goto Done;
    }
    if (interval < 15)
    {
        pattern = _T("half a day");
        goto Done;
    }
    interval += 12;
    interval /= 24;
    if (interval < 6)
    {
        if (1 >= interval)
            pattern = _T("a day");
        else
            pattern = _T("%ld days");
        goto Done;
    }
    interval += 4;
    interval /= 7;
    if (interval < 4)
    {
        if (1 >= interval)
            pattern = _T("a week");
        else
            pattern = _T("%ld weeks");
        goto Done;
    }
    interval *= 7;
    interval += 11;
    interval /= 30;
    if (interval < 5)
    {
        if (1 >= interval)
            pattern = _T("a month");
        else
            pattern = _T("%ld months");
        goto Done;
    }
    if (interval < 8)
    {
        pattern = _T("half a year");
        goto Done;
    }
    interval += 6;
    interval /= 12;
    if (1 >= interval)
        pattern = _T("a year");
    else
        pattern = _T("%ld years");
Done:
    uint_t res;
    if (_T('%') == *pattern)
        res = tprintf(buffer, pattern, interval);
    else
        res = tprintf(buffer, pattern);
    return res;
}

/***********************************************************************
 strtod.c - Defines a function to turn a numeric string into a double.

 Obtained from http://cold.40-below.com/palm/strtod.html

 This version was formatted and tweaked slightly by Warren Young
    <tangent@cyberport.com>

 Code is in the public domain, per this snippet from an email from
 David Bray <dbray@twcny.rr.com>:

    "You can consider fptest.c and the associated routines to be in
    the public domain."
    
 Code edited 2001-01-14 by Ben Combee <bcombee@metrowerks.com> to
 work with CodeWarrior for Palm OS 7 and 8.
***********************************************************************/

// Routine to convert a string to a double -- 
// Allowed input is in fixed notation ddd.fff
// This does not use MathLib.
//
// Return true if conversion was successful, false otherwise

#define PalmIsDigit(c) ((c) >= _T('0') && (c) <= _T('9'))

bool strToDouble(const char_t* str, double *dbl)
{
    int i, start, length, punctPos;
    double result, sign, fractPart;

    result = fractPart = 0.0;
    length = punctPos = tstrlen(str);

    start = 0;
    sign = 1.0;
    if (str[0] == _T('-')) {
        sign = -1.0;
        start = 1;
    }

    for (i = start; i < length; i++)    // parse the string from left to right converting the integer part
    {
        if (str[i] != _T('.')) {
            if (PalmIsDigit(str[i]))
                result = result * 10.0 + (str[i] - _T('0'));
            else if (_T(',') != str[i]) 
                return false;
        }
        else {
            punctPos = i;
            break;
        }
    }

    if (str[punctPos] == _T('.'))   // parse the string from the end to the '.' converting the fractional part
    {
        for (i = length - 1; i > punctPos; i--) {
            if (PalmIsDigit(str[i]))
                fractPart = fractPart / 10.0 + (str[i] - _T('0'));
            else {
                return false;
            }
        }
        result += fractPart / 10.0;
    }

    *dbl = result * sign;       // correcting the sign
	return true;
}

// compare two version numbers as returned by Latest-Client-Version
// an assumption is that it's of the form X.Y.
// return 0 if version numbers are equal, > 0 if verNumOne is > than verNumTwo
// and < 0 if verNumOne is < verNumTwo
int versionNumberCmp(const char_t *verNumOne, const char_t *verNumTwo)
{
    double verNumOneDbl;
    double verNumTwoDbl;

    strToDouble(verNumOne, &verNumOneDbl);
    strToDouble(verNumTwo, &verNumTwoDbl);

    if (verNumOneDbl==verNumTwoDbl)
        return 0;
    if (verNumOneDbl > verNumTwoDbl)
        return 1;
    return -1;
}

// Andrzej: IMHO this is bad idea to put these functions in Text.h/cpp as they are to closely 
// related to bytecode parser and it's not so obvious what they do in other contexts.
ulong_t readUnaligned32(const char_t* addr)
{
	return
	(
	(((ulong_t)((unsigned char)addr[0])) << 24) | 
	(((ulong_t)((unsigned char)addr[1])) << 16) | 
	(((ulong_t)((unsigned char)addr[2])) <<  8) | 
	((ulong_t)((unsigned char)addr[3])) );
}    
void writeUnaligned32(char_t* addr, ulong_t value)
{
    addr[0] = (char_t)((ulong_t)(value) >> 24);
	addr[1] = (char_t)((ulong_t)(value) >> 16);
	addr[2] = (char_t)((ulong_t)(value) >>  8);
	addr[3] = (char_t)((ulong_t)(value));
}    

} // namespace ArsLexis

