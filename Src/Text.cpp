#include <Text.hpp>

#ifdef __MWERKS__
# pragma pcrelconstdata on
#endif

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

static unsigned char Utf16ToChar(char_t in)
{
    if (in <= 255)
    {
        // is it common code ?
        return (unsigned char)in;
    }

    // Maybe binary search - MS doesn't support 
    // as always standard and bsearch
    for (int i=0; i<24; i++)
    {
        if (UnicodeToPalm[i][0]==in)
            return (unsigned char)UnicodeToPalm[i][1];
    }
    assert(false); //we never shall reach this point
    return 0;
}

/*
struct CharToByte
{ 
    unsigned char operator()(char_t in) 
    {   
        return Utf16ToChar(in);
    }
};
*/

static char_t CharToUtf16(unsigned char in)
{
    if ((in >= 128) && (in <= 159))
        return PalmToUnicode[in-128];

    if ('\t' == in)
    {
        // special case for tab - Palm ignores it, wince displays
        // as a rectangle
        return char_t(' ');
    }
    return char_t(in);
}
#endif


char* Utf16ToStr(const char_t *txt, long txtLen)
{
#ifdef _PALM_OS
    return StringCopy2N(txt, txtLen);
#else
    if (-1 == txtLen)
        txtLen = tstrlen(txt);    
    char* res = (char*)malloc(txtLen+1);
    if (NULL == res)
        return NULL;

    for (long i=0; i<txtLen; i++)
    {
        res[i] = Utf16ToChar(txt[i]);
    }
    res[txtLen] = '\0';
    return res;
#endif
}

void ByteStreamToText(const NarrowString& inStream, String& outTxt)
{
#if defined(_WIN32)
    //char_t *out=NULL;
    //int size = MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, inStream.c_str(), -1, out, 0);
    //out=new char_t[size];
    //MultiByteToWideChar(CP_OEMCP, MB_COMPOSITE, inStream.c_str(), -1, out, size);
    //outTxt.assign(out);
    //delete [] out;
    outTxt.reserve(inStream.length());
    std::transform(inStream.begin(), inStream.end(), std::back_inserter(outTxt), Utf16ToChar);
#else
    outTxt.assign(inStream);
#endif
}

// do a primitive conversion of txt in Palm charset to utf-16
char_t *StrToUtf16(const char *txt, long txtLen)
{
#ifdef _PALM_OS
    return StringCopy2N(txt, txtLen);
#else
    if (-1 == txtLen)
        txtLen = strlen(txt);
    char_t * res = (char_t*)malloc((txtLen+1)*sizeof(char_t));
    if (NULL == res)
        return NULL;

    for (long i=0; i<txtLen; i++)
    {
        res[i] = CharToUtf16(txt[i]);
    }
    res[txtLen] = _T('\0');    
    return res;
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

static uint_t charToNumber(const char_t chr)
{
    if ( (chr >= _T('0')) && (chr <= _T('9')) )
        return (uint_t) (chr - _T('0'));

    if ( (chr >= _T('A')) && (chr <= _T('Z')) )
        return (uint_t) 10 + (chr - _T('A'));

    if ( (chr >= _T('a')) && (chr <= _T('z')) )
        return (uint_t) 10 + (chr - _T('a'));

    return 0xffff; // not sure if that's the best interface    
}

#define MAX_BASE 26  // arbitrary but good enough for us

ArsLexis::status_t numericValue(const char_t* begin, const char_t* end, long& result, uint_t base)
{
    ArsLexis::status_t error = errNone;
    bool     negative = false;
    long     res = 0;
    uint_t   num;

    if ( (begin >= end) || (base >  MAX_BASE) )
    {    
        error=sysErrParamErr;
        goto OnError;           
    }

    if (_T('-') == *begin)
    {
        negative = true;
        if (++begin == end)
        {
            error = sysErrParamErr;
            goto OnError;           
        }
    }           

    while (begin != end) 
    {
        num = charToNumber(*begin++);
        if (num >= base)
        {   
            error = sysErrParamErr;
            break;
        }
        else
        {
            res *= base;
            res += num;
        }
    }
    if (!error)
       result = res;
OnError:
    return error;    
}

static ArsLexis::char_t numToHex(int num)
{
    char_t  c;
    assert( (num>=0) && (num<16) );
    if (num<=9)
        c = _T('0') + num;
    else
        c = _T('A') + (num-10);
    return c;
}

static ArsLexis::char_t numToHexSmall(int num)
{
    char_t  c;
    assert( (num>=0) && (num<16) );
    if (num<=9)
        c = _T('0') + num;
    else
        c = _T('a') + (num-10);
    return c;
}

// encode binary blob of blobSize size and put the result in the out string
void HexBinEncodeBlob(unsigned char *blob, int blobSize, ArsLexis::String& out)
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

ArsLexis::String hexBinEncode(const String& in)
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
        unsigned char b = (unsigned char)(*(it++));
        hexChar = numToHex(b / 16);
        out.append(1,hexChar);
        hexChar = numToHex(b % 16);
        out.append(1,hexChar);
    }
    return out;
}

inline static void CharToHexString(ArsLexis::char_t* buffer, ArsLexis::char_t chr)
{
    buffer[0] = numToHex(chr / 16);
    buffer[1] = numToHex(chr % 16);
}

void urlEncode(const ArsLexis::String& in, ArsLexis::String& out)
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

// remove non-digit characters in-place.
void removeNonDigitsInPlace(char_t *txt)
{
    char_t * curReadPos = txt;
    char_t * curWritePos = txt;
    while (*curReadPos)
    {
        if (isDigit(*curReadPos))
        {
            if (curReadPos != curWritePos)
            {
                assert( curWritePos < curReadPos);
                *curWritePos++ = *curReadPos++;
            }
            else
            {
                ++curWritePos;
                ++curReadPos;
            }
        }
        else
            ++curReadPos;
    }

    // terminating 0
    if (curReadPos != curWritePos)
    {
        assert( curWritePos < curReadPos);
        *curWritePos = *curReadPos;
    }
}

void removeNonDigits(const char_t* in, uint_t len, ArsLexis::String& out)
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
int formatNumber(long num, char_t* buf, int bufSize)
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
        word = GetNextLine(txt, curPos, fEnd);
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

char_t** StringVectorToCStrArray(const std::vector<String>& vec)
{
    ulong_t len = vec.size();
    typedef char_t* cstr;
    cstr* arr = new_nt cstr[len];
    if (NULL == arr)
        return NULL;
    
    memzero(arr, sizeof(char_t*) * len);
    for (ulong_t i = 0; i < len; ++i)
        if (NULL == (arr[i] = StringCopyN(vec[i].data(), vec[i].length())))
            goto NoMemory;
            
    return arr;
NoMemory:
    FreeStringList(arr, len);
    return NULL;        
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

// note: caller needs to free memory with free
char_t* StringCopy2__(const char_t *curStr, int len, const char* file, int line)
{
    using namespace std;
    if (-1 == len)
        len = tstrlen(curStr);
    char_t *newStr = (char_t*)malloc__(sizeof(char_t) * (len + 1), file, line);
    if (NULL == newStr)
        return NULL;

    memcpy(newStr, curStr, sizeof(char_t) * len);
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

// note: caller needs to free memory with free
char* CharCopyN__(const char* curStr, int len, const char* file, int line)
{
    using namespace std;
    if (-1 == len)
        len = strlen(curStr);
    char *newStr = (char*)malloc__(sizeof(char) * (len + 1), file, line);
    if (NULL == newStr)
        return NULL;

    memcpy(newStr, curStr, sizeof(char) * len);
    newStr[len] = '\0';
    return newStr;
}


// strip (i.e. remove whitespace from the beginning and the end of the string)
// str in place.
void StrStrip(char_t *str)
{
    assert( NULL != str);
    ulong_t finalLen = tstrlen(str);
    if (0 == finalLen)
        return;

    while (isSpace(str[finalLen-1]))
    {
        --finalLen;
    }
    str[finalLen] = _T('\0');

    if (0 == finalLen)
        return;

    char_t *strStart = str;
    while ( (finalLen > 0) && (isSpace(*strStart)))
    {
        strStart++;
        finalLen -= 1;
    }

    assert( 0 != finalLen);
    if (strStart == str)
        return;

    memmove(str, strStart, finalLen+1);
    return;
}

void strip(const char_t*& start, ulong_t& length)
{
    assert(NULL != start);
    while (isSpace(start[0]) && length > 0)
    {
        length--;
        start++;
    }
    while (length > 0)
    {
        if (isSpace(start[length-1]))
            length--;
        else
            break;
    }
}

// return true if this is an empty string (NULL or consits of white-space
// characters only
bool StrEmpty(const char_t *str)
{
    if (NULL == str)
        return true;
    while (*str)
    {
        if (_T(' ') != *str)
            return false;
        ++str;
    }
    return true;
}

long StrFind(const char_t* str, long len, char_t chr)
{
    if (NULL == str)
        return -1;
    if (-1 == len)
        len = tstrlen(str);

    for (long i = 0; i < len; ++i)
        if (str[i] == chr)
            return i;    
    return -1;
}

long StrFind(const char_t* str, long len, const char_t* sub, long slen)
{
    if (NULL == str || NULL == sub)
        return -1;
    if (-1 == len)
        len = tstrlen(str);
    if (-1 == slen)
        slen = tstrlen(sub);
        
    if (slen > len)
        return -1;
    
    long checks = (len - slen) + 1;
    for (long i = 0; i < checks; ++i)
        if (StrEquals(str + i, slen, sub, slen))
            return i;
    return -1;
}


long bufferToHexCode(const char* in, long inLength, char* out, long outLength)
{
    assert(inLength*2 <= outLength);
    long pos = 0;
    for (pos = 0; pos < inLength; pos++)
    {
        out[2*pos+1] = (char)numToHexSmall(in[pos] & 0x0f);
        out[2*pos]   = (char)numToHexSmall((in[pos] & 0xf0) >> 4);
    }
    //if given outLength is greater than needed - terminate with zero
    if (outLength > inLength*2)
        out[inLength*2] = '\0';
    return inLength*2;
}

bool startsWith(const char_t* text, ulong_t len, const char_t* prefix, ulong_t plen)
{
    if (ulong_t(-1) == len)
        len =tstrlen(text);
    if (ulong_t(-1) == plen)
        plen = tstrlen(prefix);
    
    if (plen > len)
        return false;
    
    while (0 != plen)
    {
        if (*text++ != *prefix++)
            return false;
        --plen;
    }   
    return true;
}

bool equals(const char_t* s1, ulong_t s1len, const char_t* s2, ulong_t s2len)
{
    if (ulong_t(-1) == s1len)
        s1len = tstrlen(s1);
    if (ulong_t(-1) == s2len)
        s2len = tstrlen(s2);
    if (s1len != s2len)
        return false;
    return 0 == tstrncmp(s1, s2, s1len);
}

char_t** StrArrCreate(ulong_t size)
{
    if (0 == size)
        return NULL;
    size *= sizeof(char_t*);
    char_t** arr = (char_t**)malloc(size);
    if (NULL == arr)
        return NULL;
    memzero(arr, size);
    return arr;
}

void StrArrFree(char_t**& array, ulong_t size)
{
    if (NULL == array)
    {
        assert(0 == size);
        return;
    }
    for (ulong_t i = 0; i < size; ++i)
    {
        char_t* p = array[i];
        if (NULL != p)
            free(p);
    }
    free(array);
    array = NULL;
}

char_t** StrArrResize(char_t**& array, ulong_t currSize, ulong_t newSize)
{
    using namespace std;
    
    newSize *= sizeof(char_t*);
    char_t** arr = (char_t**)realloc(array, newSize);
    if (NULL == arr)
        return NULL;
    
    array = arr;
    if (newSize <= currSize)
        return arr;
    
    memzero(&arr[currSize], newSize - (sizeof(char_t*) * currSize));
    return arr;
}


char_t** StrArrAppendStr(char_t**& array, ulong_t& length, char_t* str)
{
    if (NULL == StrArrResize(array, length, length + 1))
        return NULL;
        
    array[length++] = str;
    return array;
}

char_t** StrArrAppendStrCopy(char_t**& array, ulong_t& length, const char_t* str)
{
    char_t* copy = StringCopy2(str);
    if (NULL == copy)
        return NULL;
    
    if (NULL == StrArrAppendStr(array, length, copy))
    {
        free(copy);
        return NULL;
    }
    return array;
}

void StrArrEraseStr(char_t** array, ulong_t& length, ulong_t index)
{
    assert(index < length);
    char_t* p = array[index];
    if (NULL != p)
        free(p);
    memmove(&array[index], &array[index + 1], sizeof(char_t*) * (length - index - 1));
    array[length - 1] = NULL;
    StrArrResize(array, length, length - 1);    
}

char_t** StrArrInsertStr(char_t**& array, ulong_t& length, ulong_t index, char_t* str)
{
    assert(index <= length);
    if (NULL == StrArrResize(array, length, length + 1))
        return NULL;
    
    memmove(&array[index + 1], &array[index], sizeof(char_t*) * (length - index));
    ++length;
    array[index] = str;
    return array;
}

char_t** StrArrInsertStrCopy(char_t**& array, ulong_t& length, ulong_t index, const char_t* str)
{
    char_t* copy = StringCopy2(str);
    if (NULL == copy)
        return NULL;

    if (NULL == StrArrInsertStr(array, length, index, copy))
    {
        free(copy);
        return NULL;
    }
    return array;
}

long StrArrFind(char_t** array, ulong_t length, const char_t* str, long len)
{
    if (-1 == len)
        len = tstrlen(str);
        
    for (ulong_t i = 0; i < length; ++i)
        if (StrEquals(array[i], str, len))
            return i;
    return -1;
}

#ifdef DEBUG

static void test_startsWith()
{
    assert(startsWith(_T("prefix0123456787"), -1, _T("prefix"), -1));
    assert(!startsWith(_T("prefix0123456787"), 4, _T("prefix"), -1));
    assert(!startsWith(_T("preFix0123456787"), -1, _T("prefix"), -1));
}

static void test_removeNonDigitsInPlace()
{
    char_t buf[32];
    char_t *bufStart = &(buf[0]);

    tstrcpy(bufStart, _T(""));
    removeNonDigitsInPlace(bufStart);
    assert(0 == tstrlen(bufStart) );

    tstrcpy(bufStart, _T(" 0 a 33"));
    removeNonDigitsInPlace(bufStart);
    assert(3 == tstrlen(bufStart));
    assert(0 == tstrcmp(bufStart, _T("033")));

    tstrcpy(bufStart, _T("5"));
    removeNonDigitsInPlace(bufStart);
    assert(1 == tstrlen(bufStart));
    assert(0 == tstrcmp(bufStart, _T("5")));

    tstrcpy(bufStart, _T("9 8a7e6g5e4w3w2q1q0gla "));
    removeNonDigitsInPlace(bufStart);
    assert(10 == tstrlen(bufStart));
    assert(0 == tstrcmp(bufStart, _T("9876543210")));
}

static void test_StrEmpty()
{
    assert(true == StrEmpty(NULL));
    assert(true == StrEmpty(_T("")));
    assert(true == StrEmpty(_T(" ")));
    assert(true == StrEmpty(_T("          ")));
    assert(false == StrEmpty(_T(" a")));
}

static void test_StrFind()
{
    assert(-1 == StrFind(NULL, -1, _T(' ')));
    assert(-1 == StrFind(_T("abcd"), -1, _T('e')));
    assert( 1 == StrFind(_T("abcd"), -1, _T('b')));
    assert(-1 == StrFind(_T("abcd"), 3, _T('d')));
    assert( 0 == StrFind(_T("hello"), -1, _T('h')) );
    assert( 4 == StrFind(_T("hello"), -1, _T('o')) );
}

static void test_HexCode()
{
    using namespace std;
    char buffer[20];
    assert(12 == bufferToHexCode("?text?",6,buffer,20));
    buffer[12] = '\0';
    assert(0 == strcmp(buffer,"3f746578743f"));
}

void test_TextUnitTestAll()
{
    test_removeNonDigitsInPlace();
    test_StrFind();
    test_StrEmpty();
    test_StrFind();
    test_HexCode();
    test_startsWith();
}

#endif

