#include <Text.hpp>

#ifdef __MWERKS__
# pragma pcrelconstdata on
#endif

// note: caller needs to free memory with free
char* StringCopyN__(const char* curStr, long len, const char* file, int line)
{
    using namespace std;
    if (-1 == len)
        len = Len(curStr);
    char* newStr = (char*)malloc__(sizeof(*curStr) * (len + 1), file, line);
    if (NULL == newStr)
        return NULL;

    memcpy(newStr, curStr, sizeof(*curStr) * len);
    newStr[len] = 0;
    return newStr;
}

#ifdef _WIN32_WCE

wchar_t* StringCopyN__(const wchar_t* curStr, long len, const char* file, int line)
{
    using namespace std;
    if (-1 == len)
        len = Len(curStr);
    wchar_t* newStr = (wchar_t*)malloc__(sizeof(*curStr) * (len + 1), file, line);
    if (NULL == newStr)
        return NULL;

    memcpy(newStr, curStr, sizeof(*curStr) * len);
    newStr[len] = 0;
    return newStr;
}
#endif

bool equals(const char* s1, long s1len, const char* s2, long s2len)
{
    using namespace std;
    if (-1 == s1len) s1len = Len(s1);
    if (-1 == s2len) s2len = Len(s2);
    if (s1len != s2len)
        return false;
    return 0 == strncmp(s1, s2, s1len);
}

#ifdef _WIN32_WCE
bool equals(const wchar_t* s1, long s1len, const wchar_t* s2, long s2len)
{
    if (-1 == s1len) s1len = Len(s1);
    if (-1 == s2len) s2len = Len(s2);

    if (s1len != s2len)
        return false;

    return 0 == wcsncmp(s1, s2, s1len);
}
#endif

bool equalsIgnoreCase(const char* s1start, const char* s1end, const char* s2start, const char* s2end)
{
    while (s1start != s1end && s2start != s2end)
    {
        if (toLower(*s1start) == toLower(*s2start))
        {
            ++s1start;
            ++s2start;
        }
        else 
            return false;
    }
    return (s1start == s1end && s2start == s2end);
}

#ifdef _WIN32_WCE
bool equalsIgnoreCase(const wchar_t* s1start, const wchar_t* s1end, const wchar_t* s2start, const wchar_t* s2end)
{
    while (s1start != s1end && s2start != s2end)
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
#endif

bool startsWith(const char* text, long len, const char* prefix, long plen)
{
    if (-1 == len) len = Len(text);
    if (-1 == plen) plen = Len(prefix);
    
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

#ifdef _WIN32_WCE
bool startsWith(const wchar_t* text, long len, const wchar_t* prefix, long plen)
{
    if (-1 == len) len =Len(text);
    if (-1 == plen) plen = Len(prefix);
    
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
#endif

bool startsWithIgnoreCase(const char* text, long len, const char* prefix, long plen)
{
    if (-1 == len) len = Len(text);
    if (-1 == plen) plen = Len(prefix);
    
    if (plen > len)
        return false;
    
    while (0 != plen)
    {
        if (toLower(*text++) != toLower(*prefix++))
            return false;
        --plen;
    }   
    return true;
}

#ifdef _WIN32_WCE
bool startsWithIgnoreCase(const wchar_t* text, long len, const wchar_t* prefix, long plen)
{
    if (-1 == len) len =Len(text);
    if (-1 == plen) plen = Len(prefix);
    
    if (plen > len)
        return false;
    
    while (0 != plen)
    {
        if (toLower(*text++) != toLower(*prefix++))
            return false;
        --plen;
    }   
    return true;
}
#endif

#define MAX_BASE 26  // arbitrary but good enough for us

static uint_t charToNumber(uint_t chr)
{
    if ( (chr >= '0') && (chr <= '9') )
        return uint_t(chr - '0');

    if ( (chr >= 'A') && (chr <= 'Z') )
        return uint_t(10 + (chr - 'A'));

    if ( (chr >= 'a') && (chr <= 'z') )
        return uint_t(10 + (chr - 'a'));

    return uint_t(-1);
}

status_t numericValue(const char* begin, const char* end, long& result, uint_t base)
{
    status_t error = errNone;
    bool     negative = false;
    long     res = 0;
    uint_t   num;

    if ( (begin >= end) || (base > MAX_BASE) )
    {    
        error=sysErrParamErr;
        goto OnError;           
    }

    if ('-' == *begin)
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

#ifdef _WIN32_WCE
status_t numericValue(const wchar_t* begin, const wchar_t* end, long& result, uint_t base)
{
    status_t error = errNone;
    bool     negative = false;
    long     res = 0;
    uint_t   num;

    if ( (begin >= end) || (base >  MAX_BASE) )
    {    
        error=sysErrParamErr;
        goto OnError;           
    }

    if (L'-' == *begin)
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
#endif



long StrFind(const char* str, long len, char chr)
{
    if (NULL == str)
        return -1;
    if (-1 == len) len = Len(str);

    for (long i = 0; i < len; ++i)
        if (str[i] == chr)
            return i;    
    return -1;
}

#ifdef _WIN32_WCE
long StrFind(const wchar_t* str, long len, wchar_t chr)
{
    if (NULL == str)
        return -1;
    if (-1 == len) len = Len(str);

    for (long i = 0; i < len; ++i)
        if (str[i] == chr)
            return i;    
    return -1;
}
#endif

long StrFind(const char* str, long len, const char* sub, long slen)
{
    if (NULL == str || NULL == sub)
        return -1;
    if (-1 == len)
        len = Len(str);
    if (-1 == slen)
        slen = Len(sub);
        
    if (slen > len)
        return -1;
    
    long checks = (len - slen) + 1;
    for (long i = 0; i < checks; ++i)
        if (StrEquals(str + i, slen, sub, slen))
            return i;
    return -1;
}

#ifdef _WIN32_WCE
long StrFind(const wchar_t* str, long len, const wchar_t* sub, long slen)
{
    if (NULL == str || NULL == sub)
        return -1;
    if (-1 == len)
        len = Len(str);
    if (-1 == slen)
        slen = Len(sub);
        
    if (slen > len)
        return -1;
    
    long checks = (len - slen) + 1;
    for (long i = 0; i < checks; ++i)
        if (StrEquals(str + i, slen, sub, slen))
            return i;
    return -1;
}
#endif

void strip(const char*& start, ulong_t& length)
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

#ifdef _WIN32_WCE
void strip(const wchar_t*& start, ulong_t& length)
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
#endif

/*
bool startsWith(const String& text, const char_t* start, uint_t startOffset)
{
    while (startOffset < text.length() && *start)
    {
        if (*start == text[startOffset])
        {
            ++start;
            ++startOffset;
        }
        else
            return false;
    }
    return 0 == *start;
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
*/

static char numToHex(unsigned char num)
{
    char c;
    assert(num < 16);
    if (num <= 9)
        c = '0' + num;
    else
        c = 'A' + (num - 10);
    return c;
}

static char numToHexSmall(unsigned char num)
{
    char  c;
    assert(num < 16);
    if (num <= 9)
        c = '0' + num;
    else
        c = 'a' + (num-10);
    return c;
}

// encode binary blob of blobSize size and put the result in the out string
void HexBinEncodeBlob(const char* blob, ulong_t blobSize, NarrowString& out)
{
    out.clear();
    out.reserve(blobSize * 2); // 2 chars per each byte

    unsigned char b;
    char        hexChar;
    for (ulong_t i = 0; i < blobSize; i++)
    {
        b = blob[i];
        hexChar = numToHex(b / 16);
        out.append(1, hexChar);
        hexChar = numToHex(b % 16);
        out.append(1, hexChar);
    }
}

inline static void CharToHexString(char* buffer, char chr)
{
    buffer[0] = numToHex(chr / 16);
    buffer[1] = numToHex(chr % 16);
}

void urlEncode(const NarrowString& in, NarrowString& out)
{
	char hexNum[4] = {'%', '\0', '\0', '\0'};

    const char* begin = in.data();
    const char* end = begin + in.length();
    out.resize(0);
    out.reserve(in.length());

    while (begin != end)
    {        
        char chr=*begin++;
        if ((chr >= 'a' && chr <= 'z') || 
            (chr >= 'A' && chr <= 'Z') || 
            (chr >= '0' && chr <= '9') || 
		-1 != StrFind("-_.!~*\'()", -1,  chr))
            out.append(1, chr);
        else
        {
            CharToHexString(hexNum + 1, chr);
            out.append(hexNum, 3);
        }
    }
}

// remove non-digit characters in-place.
void removeNonDigitsInPlace(char_t* txt)
{
    char_t* curReadPos = txt;
    char_t* curWritePos = txt;
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

void removeNonDigits(const char_t* in, uint_t len, String& out)
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
/*
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
*/

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
*/

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
        result[curPos] = StringCopyN(it->data(), it->length());
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

/*
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
*/

void FreeStringList(char_t* strList[], int strListLen)
{
    for (int i = 0; i < strListLen; ++i)
        free(strList[i]);
    delete [] strList;
}

/*   
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
*/
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
/*
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
*/


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
 Defines a function to turn a numeric string into a double.

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
            if (isDigit(str[i]))
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
            if (isDigit(str[i]))
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

    if (verNumOneDbl == verNumTwoDbl)
        return 0;

    if (verNumOneDbl > verNumTwoDbl)
        return 1;
    return -1;
}


/*
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
 */

/*
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
 */

/*
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
 */

ulong_t StrHexlify(const char* in, long inLength, char* out, ulong_t outLength)
{
	if (-1 == inLength) inLength = Len(in);
    assert(ulong_t(inLength * 2) <= outLength);

    for (long pos = 0; pos < inLength; pos++)
    {
        out[2 * pos + 1] = numToHexSmall(in[pos] & 0x0f);
        out[2 * pos]   = numToHexSmall((in[pos] & 0xf0) >> 4);
    }
    //if given outLength is greater than needed - terminate with zero
    if (outLength > ulong_t(inLength * 2))
        out[inLength * 2] = '\0';
    return inLength * 2;
}

char_t* StrUnhexlify(const char_t* in, long inLen)
{
    if (-1 == inLen)
        inLen = tstrlen(in);
    if ((inLen % 2) != 0)
        return NULL;
    
    char_t* buffer = (char_t*)malloc(inLen / 2 + 1);
    if (NULL == buffer)
        return NULL;
    
    char_t* out = buffer;
    while (inLen != 0)
    {
        long val;
        status_t err = numericValue(in, in + 2, val, 16);
        if (errNone != err)
            goto Error;
        
        if (val < 0 || val > 255)
            goto Error;
        
        char c = (unsigned char)val;
        *out++ = c;
        in += 2;
        inLen -= 2;
    }
    *out = _T('\0');
    return buffer;
Error:
    free(buffer);
    return NULL;    
}




#ifdef _WIN32 // overloads for working with narrow-strings also







#endif


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

long StrArrFindPrefix(char_t** array, ulong_t length, char_t nextChar, const char_t* str, long len)
{
    if (-1 == len)
        len = tstrlen(str);

    for (ulong_t i = 0; i < length; ++i)
    {
        ulong_t l = tstrlen(array[i]);
        if (l < length)
            continue;
            
        if (!StrStartsWith(array[i], l, str, len))
            continue;
            
        if (array[i][len] == nextChar)
            return i;
    }
    return -1;
}

/*
long StrArrFind(char_t** array, ulong_t length, const char_t* str, long len)
{
    if (-1 == len)
        len = tstrlen(str);
        
    for (ulong_t i = 0; i < length; ++i)
        if (StrEquals(array[i], str, len))
            return i;
    return -1;
}
 */
 
long StrArrFind(char_t** array, ulong_t length, const char_t* str, long len)
{
    return StrArrFindPrefix(array, length, _T('\0'), str, len);
}    


void* MemAppend(void* target, ulong_t tlen, const void* source, ulong_t slen, ulong_t termlen)
{
	using namespace std;
	void* newLoc = realloc(target, tlen + slen + termlen);
	if (NULL == newLoc)
	{
		free(target);
		return NULL;
	}
	void* t = ((char*)newLoc) + tlen;
	memmove(t, source, slen);
	return newLoc;
}

void MemErase(void* target, ulong_t tlen, ulong_t efrom, ulong_t elen)
{
	assert(efrom + elen <= tlen);
	void* t = ((char*)target) + efrom;
	const void* s = ((const char*)target) + efrom + elen;
	ulong_t len = tlen - efrom - elen;
	memmove(t, s, len);
}

char* StrAppend(char* target, long tlen, const char* source, long slen)
{
	if (-1 == tlen) tlen = Len(target);
	if (-1 == slen) slen = Len(source);
	target = (char*)MemAppend(target, tlen, source, slen, sizeof(*target));
	if (NULL == target)
		return NULL;
	target[tlen + slen] = '\0';
	return target;
}

void StrErase(char* target, long tlen, ulong_t efrom, ulong_t elen)
{
	if (-1 == tlen) tlen = Len(target);
	MemErase(target, sizeof(*target) * tlen, sizeof(*target) * efrom, sizeof(*target) * elen);
	if (efrom + elen < ulong_t(tlen))
		target[tlen - efrom - elen] = '\0';
}

#ifdef _WIN32_WCE
wchar_t* StrAppend(wchar_t* target, long tlen, const wchar_t* source, long slen)
{
	if (-1 == tlen) tlen = Len(target);
	if (-1 == slen) slen = Len(source);
	target = (wchar_t*)MemAppend(target, sizeof(*target) * tlen, source, sizeof(*target) * slen, sizeof(*target));
	if (NULL == target)
		return NULL;
	target[tlen + slen] = L'\0';
	return target;

}
void StrErase(wchar_t* target, long tlen, ulong_t efrom, ulong_t elen)
{
	if (-1 == tlen) tlen = Len(target);
	MemErase(target, sizeof(*target) * tlen, sizeof(*target) * efrom, sizeof(*target) * elen);
	if (efrom + elen < ulong_t(tlen))
		target[tlen - efrom - elen] = L'\0';
}
#endif

template<class Ch>
void strip(std::basic_string<Ch>& str)
{
	const Ch* d = str.data();
	ulong_t l = str.length();
	strip(d, l);
	str.erase(0, d - str.data());
	str.resize(l);
}

template<class Ch>
status_t StringAppend(std::basic_string<Ch>& out, const Ch* str, long len)
{
    status_t err = errNone;
	if (-1 == len) len = Len(str);
	ErrTry {
		out.append(str, len);
	} 
	ErrCatch(ex) {
		err = ex;
	} ErrEndCatch
	return err;
}

template status_t StringAppend<char>(std::string& out, const char* str, long len);
template void strip<char>(std::string& str);

#ifdef _WIN32_WCE
template status_t StringAppend<wchar_t>(std::wstring& out, const wchar_t* str, long len);
template void strip<wchar_t>(std::wstring& str);
#endif



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
	/*
    assert(true == StrEmpty(NULL));
    assert(true == StrEmpty(_T("")));
    assert(true == StrEmpty(_T(" ")));
    assert(true == StrEmpty(_T("          ")));
    assert(false == StrEmpty(_T(" a")));
	*/
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
    assert(12 == StrHexlify("?text?", 6, buffer, 20));
    assert(buffer[12] == '\0');
    assert(0 == strcmp(buffer,"3f746578743f"));
}

void test_TextUnitTestAll()
{
    test_removeNonDigitsInPlace();
    test_StrFind();
    test_StrEmpty();
    test_HexCode();
    test_startsWith();
}

#endif

