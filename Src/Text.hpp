#ifndef __ARSLEXIS_TEXT_HPP__
#define __ARSLEXIS_TEXT_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <algorithm>
#include <list>
#include <vector>

typedef std::list<ArsLexis::String> StringList_t;

#if !defined(_WIN32) || _MSC_VER != 1200
#include <cctype>
#endif

# if defined(_PALM_OS)    
extern void printDouble(double x, char* s);
extern void printDoubleRound(double x, char* s, double roundFactor, int numDigits, int precLimit = -1, bool insertThousandSeparator = false);
#endif

inline char toLower(char chr) {using namespace std; return tolower(chr);}
inline char toUpper(char chr) {using namespace std; return toupper(chr);}
inline bool isAlpha(char chr) {using namespace std; return 0 != isalpha(chr);}
inline bool isAlNum(char chr) {using namespace std; return 0 != isalnum(chr);}
inline bool isDigit(char chr) {using namespace std; return 0 != isdigit(chr);}
inline bool isSpace(char chr) {using namespace std; return 0 != isspace(chr);}
inline ulong_t Len(const char* str) {using namespace std; return strlen(str);}

char* StringCopyN__(const char* curStr, long len, const char* file, int line);

bool equals(const char* s1, long s1len, const char* s2, long s2len);
bool equalsIgnoreCase(const char* s1start, const char* s1end, const char* s2start, const char* s2end);
bool startsWith(const char* text, long len, const char* prefix, long plen);
bool startsWithIgnoreCase(const char* text, long len, const char* prefix, long plen);
status_t numericValue(const char* begin, const char* end, long& result, uint_t base = 10);
long StrFind(const char* str, long len, char chr);
long StrFind(const char* str, long len, const char* sub, long slen);
void strip(const char*& start, ulong_t& length);

void* MemAppend(void* target, ulong_t tlen, const void* source, ulong_t slen, ulong_t termlen);
void MemErase(void* target, ulong_t tlen, ulong_t efrom, ulong_t elen);

char* StrAppend(char* target, long tlen, const char* source, long slen);
void StrErase(char* target, long tlen, ulong_t efrom, ulong_t elen);


#ifdef _WIN32_WCE

inline wchar_t toLower(wchar_t chr) {using namespace std; return towlower(chr);}
inline wchar_t toUpper(wchar_t chr) {using namespace std; return towupper(chr);}
inline bool isAlpha(wchar_t chr) {using namespace std; return 0 != iswalpha(chr);}
inline bool isAlNum(wchar_t chr) {using namespace std; return 0 != iswalnum(chr);}
inline bool isDigit(wchar_t chr) {using namespace std; return 0 != iswdigit(chr);}
inline bool isSpace(wchar_t chr) {using namespace std; return 0 != iswspace(chr);}
inline ulong_t Len(const wchar_t* str) {using namespace std; return wcslen(str);}

wchar_t* StringCopyN__(const wchar_t* curStr, long len, const char* file, int line);

bool equals(const wchar_t* s1, long s1len, const wchar_t* s2, long s2len);
bool equalsIgnoreCase(const wchar_t* s1start, const wchar_t* s1end, const wchar_t* s2start, const wchar_t* s2end);
bool startsWith(const wchar_t* text, long len, const wchar_t* prefix, long plen);
bool startsWithIgnoreCase(const wchar_t* text, long len, const wchar_t* prefix, long plen);
status_t numericValue(const wchar_t* begin, const wchar_t* end, long& result, uint_t base = 10);
long StrFind(const wchar_t* str, long len, wchar_t chr);
long StrFind(const wchar_t* str, long len, const wchar_t* sub, long slen);
void strip(const wchar_t*& start, ulong_t& length);


wchar_t* StrAppend(wchar_t* target, long tlen, const wchar_t* source, long slen);
void StrErase(wchar_t* target, long tlen, ulong_t efrom, ulong_t elen);

#endif // _WIN32_WCE

//bool startsWith(const String& text, const char_t* start, uint_t startOffset = 0);
//bool startsWith(const String& text, const String& start, uint_t startOffset = 0);
//bool startsWithIgnoreCase(const String& text, const char_t* start, uint_t startOffset = 0);

template<class Ch> inline bool startsWith(const Ch* text, const Ch* prefix) 
{return startsWith(text, -1, prefix, -1);}

template<class Ch> inline bool startsWith(const std::basic_string<Ch>& text, const Ch* prefix, ulong_t startOffset = 0)
{return startsWith(text.data() + startOffset, text.length() - startOffset, prefix, -1);}

template<class Ch> inline bool startsWith(const std::basic_string<Ch>& text, const std::basic_string<Ch>& prefix, ulong_t startOffset = 0)
{return startsWith(text.data() + startOffset, text.length() - startOffset, prefix.data(), prefix.length());}

template<class Ch> inline bool startsWithIgnoreCase(const Ch* text, const Ch* prefix) 
{return startsWithIgnoreCase(text, -1, prefix, -1);}

template<class Ch> inline bool startsWithIgnoreCase(const std::basic_string<Ch>& text, const Ch* prefix, ulong_t startOffset = 0)
{return startsWithIgnoreCase(text.data() + startOffset, text.length() - startOffset, prefix, -1);}

template<class Ch> inline bool startsWithIgnoreCase(const std::basic_string<Ch>& text, const std::basic_string<Ch>& prefix, ulong_t startOffset = 0)
{return startsWithIgnoreCase(text.data() + startOffset, text.length() - startOffset, prefix.data(), prefix.length());}

template<class Ch> inline bool equals(const Ch* s1, const Ch* s2, long s2len) 
{return equals(s1, -1, s2, s2len);}

template<class Ch> inline bool equals(const Ch* s1, long s1len, const Ch* s2) 
{return equals(s1, s1len, s2, -1);}

template<class Ch> inline bool equals(const Ch* s1, const Ch* s2) 
{return equals(s1, -1, s2, -1);}

template<class Ch> inline bool equalsIgnoreCase(const std::basic_string<Ch>& s1, const Ch* s2) 
{return equalsIgnoreCase(s1.data(), s1.data() + s1.length(), s2, s2 + Len(s2));}

template<class Ch> inline bool equalsIgnoreCase(const Ch* s2, const std::basic_string<Ch>& s1) 
{return equalsIgnoreCase(s1.data(), s1.data() + s1.length(), s2, s2 + Len(s2));}

template<class Ch> inline bool equalsIgnoreCase(const Ch* s1, const Ch* s2) 
{return equalsIgnoreCase(s1, s1 + Len(s1), s2, s2 + Len(s2));}

template<class Ch> inline bool equalsIgnoreCase(const std::basic_string<Ch>& s1, const std::basic_string<Ch>& s2)
{return equalsIgnoreCase(s1.data(), s1.data() + s1.length(), s2.data(), s2.data() + s2.length());}

template<class Ch> inline status_t numericValue(const std::basic_string<Ch>& text, long& result, uint_t base=10)
{return numericValue(text.data(), text.data() + text.length(), result, base);}

void urlEncode(const NarrowString& in, NarrowString& out);

void removeNonDigitsInPlace(char_t *txt);

void removeNonDigits(const char_t* in, uint_t inLength, String& out);

inline void removeNonDigits(const String& in, String& out)
{removeNonDigits(in.data(), in.length(), out);}

inline void removeNonDigits(const char_t* in, String& out)
{removeNonDigits(in, tstrlen(in), out);}

int formatNumber(long num, char_t *buf, int bufSize);

void HexBinEncodeBlob(const char* blob, ulong_t blobSize, NarrowString& out);

inline void HexBinEncode(const NarrowString& in, NarrowString& out) {HexBinEncodeBlob(in.data(), in.size(), out);}

/*
String GetNextLine(const ArsLexis::String& str, String::size_type& curPos, bool& fEnd);

char_t* StringCopy(const String& str);

void FreeStringsFromCharPtrList(CharPtrList_t& strList);

int AddLinesToList(const String& txt, CharPtrList_t& strList);

*/

char_t **StringListFromString(const String& str, const String& sep, int& stringCount);

char_t **StringListFromStringList(const StringList_t& strList, int& stringCount);

/*
char_t** StringVectorToCStrArray(const std::vector<String>& vec);

std::vector<ArsLexis::String> split(const String& str, const char_t* splitter = _T(" "));

String join(const std::vector<ArsLexis::String>& vec, const char_t* joiner = _T(" "));
*/

void strip(String& str);


void replaceCharInString(char_t *str, char_t orig, char_t replacement);

void ReverseStringList(char_t **strList, int strListLen);

void FreeStringList(char_t* strList[], int strListLen);
/*

void convertFloatStringToUnsignedLong(const String& str, unsigned long& value, unsigned int& digitsAfterComma, ArsLexis::char_t commaSeparator = _T('.'));

ArsLexis::String convertUnsignedLongWithCommaToString(unsigned long value, unsigned int comma = 0, ArsLexis::char_t commaSymbol = _T('.'), ArsLexis::char_t kSeparator = _T(','));
*/


uint_t fuzzyTimeInterval(ulong_t seconds, char_t* buffer);

bool strToDouble(const char* str, double *dbl);

int versionNumberCmp(const char_t *verNumOne, const char_t *verNumTwo);    

/*
char *Utf16ToStr(const char_t *txt, long txtLen);

char_t *StrToUtf16(const char *txt, long txtLen = -1);
*/

// bool StrEmpty(const char_t *str);

/**
 * outLength must be inLength*2 ("?text?"->"3f746578743f")
 * return used length of out buffer (2*inLength)
 */
ulong_t StrHexlify(const char* in, long inLength, char* out, ulong_t outLength);

char_t* StrUnhexlify(const char_t* in, long inLen = -1);

char_t** StrArrCreate(ulong_t size);

void StrArrFree(char_t**& array, ulong_t size);

char_t** StrArrResize(char_t**& array, ulong_t currSize, ulong_t newSize);

char_t** StrArrAppendStr(char_t**& array, ulong_t& length, char_t* str);

char_t** StrArrAppendStrCopy(char_t**& array, ulong_t& length, const char_t* str);

void StrArrEraseStr(char_t** array, ulong_t& length, ulong_t index);

char_t** StrArrInsertStr(char_t**& array, ulong_t& length, ulong_t index, char_t* str);

char_t** StrArrInsertStrCopy(char_t**& array, ulong_t& length, ulong_t index, const char_t* str);

long StrArrFind(char_t** array, ulong_t length, const char_t* str, long len = -1);

long StrArrFindPrefix(char_t** array, ulong_t length, char_t nextChar, const char_t* str, long len = -1);





#define StringCopy2(str) StringCopyN__(str, -1, __FILE__, __LINE__)
#define StringCopy2N(str, len) StringCopyN__(str, len, __FILE__, __LINE__)

#define StringCopy(str) StringCopyN__(str, -1, __FILE__, __LINE__)
#define StringCopyN(str, len) StringCopyN__(str, len, __FILE__, __LINE__)

#define StrStartsWith startsWith
#define StrEquals equals

/*
template<class Ch, class Str>
status_t StringAppend(std::basic_string<Ch>& out, const Str& app)
{
	ErrTry {
		out.append(app);
	} 
	ErrCatch(ex) {
		return ex;
	} ErrEndCatch
	return errNone;
}
 */

template<class Ch>
status_t StringAppend(std::basic_string<Ch>& out, const Ch* str, long len = -1)
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

#ifdef DEBUG
void test_TextUnitTestAll();
#endif

#endif
