#include <Debug.hpp>
#include "DynStr.hpp"

using ArsLexis::char_t;

// for interop, get C-compatible string. You can write into this string
// up to its termination 0 - this is guaranteed to belong to the DynStr
// object. However:
// - if you put 0 there, it'll screw up DynStr's view of this string
// - DynStr is binary-safe while C str isn't so what you get here might
//   not be the whole string
char_t *DynStrGetCStr(DynStr *dstr)
{
    return dstr->str;
}

// Return C-compatible, NULL-terminated string from DynStr and give up
// ownership of it (i.e. client will have to release memory for it using free())
// NOTE: the real length might be different than what strlen() will give. If
// caller cares about that, he needs to call DynStrLen() first.
char_t *DynStrReleaseStr(DynStr *dstr)
{
    char_t *toReturn = dstr->str;
    DynStrInit(dstr, 0);
    return toReturn;
}


// same as DynStrGetCStr but reinforces the notion that DynStr can handle
// arbitrary binary data
char *DynStrGetData(DynStr *dstr)
{
    return (char*)dstr->str;
}

// same as DynStrLen but reinforces the notion that DynStr can handle
// arbitrary binary data
UInt32     DynStrGetDataLen(DynStr *dstr)
{
    return dstr->strLen*sizeof(char_t);
}

// get length of the string. Note that since DynStr is binary-safe, this
// might be different (bigger) than StrLen(DynStrGetCStr(dstr))
UInt32 DynStrLen(DynStr *dstr)
{
    return dstr->strLen;
}

/* returns how much data do we have left in the buffer.
   that's how much data we can add to the string without re-allocating it */
static UInt32 DynStrLeft(DynStr *dstr)
{
    if (0 == dstr->bufSize)
        return 0;

    assert( dstr->bufSize - dstr->strLen >= 1 );
    return dstr->bufSize - dstr->strLen - 1;
}

// truncate the string (remove everything after len characters)
// In extreme case len is 0 and we clear the string
// The good thing about this function is that it doesn't
// free the buffer, so it can be used to efficiently re-use
// the string memory without reallocation
void   DynStrTruncate(DynStr *dstr, UInt32 len)
{
    assert(NULL != dstr);
    assert(dstr->strLen >= len);

    if (NULL != dstr->str)
    {
        dstr->str[len] = '\0';
        dstr->strLen = len;
    }
}

/* free memory used by DynStr. Use for structures allocated on stack and
   initialized with DynStrInit */
void DynStrFree(DynStr *dstr)
{
    assert(NULL != dstr);
    if (NULL != dstr->str)
        free(dstr->str);
}

/* free memory used by DynStr as well as the DynStr itself.
   Use on data returned by DynStrNew, DynStrFromCharP* etc. functions. */
void DynStrDelete(DynStr *dstr)
{
    DynStrFree(dstr);
    free(dstr);
}

/* given a pointer to uninitialized DynStr structure,
   initialize it and allocate buffer of size bufSize.
   returns NULL if failed for any reason (currently
   only memory allocation failure) */
DynStr * DynStrInit(DynStr* dstr, UInt32 bufSize)
{
    dstr->reallocIncrement = 0;

    dstr->str = NULL;
    if (0 != bufSize)
    {
        dstr->str = (char_t*)malloc(bufSize);
        assert( NULL != dstr->str );
        if (NULL == dstr->str)
            return NULL;
    }
    dstr->bufSize = bufSize;
    dstr->strLen = 0;
    return dstr;
}

DynStr * DynStrNew__(UInt32 bufSize, const char_t* file, int line)
{
    return DynStrFromCharP__("", bufSize, file, line);
}

void   DynStrSetReallocIncrement(DynStr *dstr, UInt32 increment)
{
    dstr->reallocIncrement = increment;
}

DynStr *DynStrFromCharP__(const char_t *str, UInt32 initBufSize, const char_t* file, int line)
{
    UInt32 strLen  = tstrlen(str);
    UInt32 bufSize = (strLen+1)*sizeof(char_t);
    DynStr * dstr;

    if (bufSize < initBufSize)
        bufSize = initBufSize;

    dstr = (DynStr*)malloc__(sizeof(DynStr), file, line);
    if (NULL == dstr)
        return NULL;

    if (NULL == DynStrInit(dstr, bufSize))
    {
        free(dstr);
        return NULL;
    }

    dstr->strLen = strLen;
    memmove( dstr->str, str, strLen + 1);

    return dstr;
}

DynStr *DynStrFromCharP2(const char_t *strOne, const char_t *strTwo)
{
    UInt32 bufSize;
    DynStr * dstr;
    DynStr * dstrNew;
    UInt32 strOneLen, strTwoLen;

    assert(NULL != strOne);
    assert(NULL != strTwo);

    strOneLen = StrLen(strOne);
    strTwoLen = StrLen(strTwo);
    bufSize  = strOneLen + strTwoLen + 1;

    dstr = DynStrFromCharP(strOne, bufSize);
    if (NULL == dstr)
        return NULL;

    dstrNew = DynStrAppendCharPBuf(dstr, strTwo, strTwoLen);
    assert(NULL != dstrNew);
    assert(dstrNew == dstr);
    assert(0==DynStrLeft(dstr));
    return dstr;
}

DynStr *DynStrFromCharP3(const char_t *strOne, const char_t *strTwo, const char_t *strThree)
{
    UInt32 bufSize;
    DynStr * dstr;
    DynStr * dstrNew;
    UInt32 strOneLen, strTwoLen, strThreeLen;

    assert(NULL!=strOne);
    assert(NULL!=strTwo);
    assert(NULL!=strThree);

    strOneLen = tstrlen(strOne);
    strTwoLen = tstrlen(strTwo);
    strThreeLen = tstrlen(strThree);
    bufSize  = strOneLen + strTwoLen + strThreeLen + 1;

    dstr = DynStrFromCharP(strOne, bufSize);
    if (NULL==dstr)
        return NULL;

    dstrNew = DynStrAppendCharPBuf(dstr, strTwo, strTwoLen);
    assert(NULL != dstrNew);
    dstrNew = DynStrAppendCharPBuf(dstr, strThree, strThreeLen);
    assert(NULL != dstrNew);
    assert(dstrNew == dstr);
    assert(0==DynStrLeft(dstr));
    return dstr;
}

// set the value of dstr to str, reallocating if necessary
DynStr * DynStrAssignCharP(DynStr *dstr, const char_t *str)
{
    DynStrTruncate(dstr, 0);
    return DynStrAppendCharP(dstr, str);
}

DynStr * DynStrAppendCharPBuf(DynStr *dstr, const char_t *str, UInt32 strLen)
{
    return DynStrAppendData(dstr, (const char*)str, strLen*sizeof(char_t));
}

DynStr * DynStrAppendCharP(DynStr *dstr, const char_t *str)
{
    UInt32 strLen = tstrlen(str);
    return DynStrAppendData(dstr, (const char*)str, strLen*sizeof(char_t));
}

DynStr * DynStrAppendCharP2(DynStr *dstr, const char_t *str1, const char_t *str2)
{
    if (NULL == DynStrAppendData(dstr, (const char*)str1, tstrlen(str1)*sizeof(char_t)))
        return NULL;
    if (NULL == DynStrAppendData(dstr, (const char*)str2, tstrlen(str2)*sizeof(char_t)))
        return NULL;
    return dstr;
}

DynStr * DynStrAppendCharP3(DynStr *dstr, const char_t *str1, const char_t *str2, const char_t *str3)
{
    if (NULL == DynStrAppendData(dstr, (const char*)str1, tstrlen(str1)*sizeof(char_t)))
        return NULL;
    if (NULL == DynStrAppendData(dstr, (const char*)str2, tstrlen(str2)*sizeof(char_t)))
        return NULL;
    if (NULL == DynStrAppendData(dstr, (const char*)str3, tstrlen(str3)*sizeof(char_t)))
        return NULL;
    return dstr;
}

DynStr *DynStrAppendDynStr(DynStr *dstr, DynStr *toAppend)
{
    return DynStrAppendCharPBuf(dstr, toAppend->str, toAppend->strLen);
}

// a generic append which appends arbitrary binary data
// to DynStr. Re-allocates the string if neccesary. Note that
// it means that you shouldn't use DynStr passed as an argument
// anymore.
// Return pointer to itself or NULL if we had to re-allocate the buffer
// but failed.
// THE ORIGINAL DynStr - the client has to do it by himself.
DynStr *DynStrAppendData(DynStr *dstr, const char *data, UInt32 dataSize)
{
    char *  curEnd;
    UInt32  newStrLen;
    char *  newStr;
    UInt32  newBufSize;

    if ( dataSize > DynStrLeft(dstr) )
    {
        // need to re-allocate the buffer
        newBufSize = dstr->strLen+1+dataSize;
        if (0 != dstr->reallocIncrement)
        {
            // if reallocIncrement wants us to alloc bigger buffer, do it
            if (dstr->bufSize + dstr->reallocIncrement > newBufSize)
            {
                newBufSize = dstr->bufSize + dstr->reallocIncrement;
            }
        }
        newStr = (char*)malloc(newBufSize);
        assert( NULL != newStr );
        if (NULL == newStr)
            return NULL;
        if (NULL != dstr->str)
        {
            memmove(newStr, dstr->str, DynStrLen(dstr));
            free(dstr->str);
        }
        dstr->str     = newStr;
        dstr->bufSize = newBufSize;
    }

    // here we're sure we have enough space
    assert( dataSize <= DynStrLeft(dstr) );

    if (NULL != dstr->str)
    {
        curEnd = dstr->str + dstr->strLen;
        memmove(curEnd, data, dataSize);
    }
    newStrLen = dstr->strLen + dataSize;
    dstr->str[newStrLen] = '\0';
    dstr->strLen = newStrLen;

    return dstr;
}

// return a C-compatible copy of the string. It might be different
// than real data if the data has embedded 0, which for C means
// end of string
char_t * DynStrCharPCopy(DynStr *dstr)
{
    UInt32 strLen;
    char_t *result;

    if (NULL == dstr->str)
        strLen = 0;
    else
        strLen = tstrlen(dstr->str);
    result = (char_t*)malloc(strLen+1);
    if (NULL == result)
        return NULL;
    if (strLen > 0)
        memmove(result, dstr->str, strLen);
    result[strLen] = '\0';
    return result;
}

// remove part of the string from dstr, starting at position start
// of lenght len.
// e.g. DynStrRemoveStartLen("hello", 1, 2) => "hlo" (removed "el")
// len can be zero to simplify client code
// start has to be within string <0 - strLen-1>
void DynStrRemoveStartLen(DynStr *dstr, UInt32 start, UInt32 len)
{
    Char *  str = dstr->str;
    UInt32  toMove;

    if (0==len)
        return;

    assert(start < dstr->strLen);
    assert(len <= dstr->strLen);

    // TODO: should I really quit? this hides errors in retail
    // on the plus side, it won't currupt memory in shipped software
    if (start >= dstr->strLen)
        return;

    if (len > dstr->strLen)
        return;

    toMove = dstr->strLen - start - len;
    memmove(str+start, str+start+len, toMove);
    dstr->strLen -= len;
    dstr->str[dstr->strLen] = '\0';
}

DynStr * DynStrAppendChar(DynStr *dstr, char_t c)
{
    return DynStrAppendData(dstr, (const char*)&c, sizeof(char_t));
}

#define uriUnescapedChars "-_.!~*'()"

static Boolean isUriUnescapedChar(char_t c)
{
    int i;
    for (i=0; i<sizeof(uriUnescapedChars)-1; i++)
    {
        if (c == uriUnescapedChars[i])
        {
            return true;
        }
    }
    return false;
}

static void CharToHexString(char c, char* buffer)
{
    const char* numbers="0123456789ABCDEF";
    buffer[0]=numbers[c / 16];
    buffer[1]=numbers[c % 16];
}

// url-encode a srcUrl DynStr and return a newly allocated DynStr. Caller has to
// free the result. Return NULL if couldn't allocatte new string
DynStr *DynStrUrlEncode(DynStr *toEncode)
{
    DynStr  * encodedUrl;
    DynStr  * result;
    int     i, len;
    char_t    c;
    char_t    buffer[3];

    // allocate a new DynStr of 2x size (a safe margin for escaped characaters,
    // so that we don't have to re-allocate memory every time we
    // encode a character)
    encodedUrl = DynStrNew(2*DynStrLen(toEncode));
    if (NULL == encodedUrl)
        return NULL;

    len = DynStrLen(toEncode);
    for (i=0; i<len; i++)
    {
        c = DYNSTR_STR(toEncode)[i];
        if ( ' ' == c )
            result = DynStrAppendChar(encodedUrl, '+');
        else if ('\0'==c || (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || isUriUnescapedChar(c))
        {
            result = DynStrAppendChar(encodedUrl, c);
        }
        else
        {
            buffer[0] = '%';
            CharToHexString(c, &(buffer[1]) );
            result = DynStrAppendCharPBuf(encodedUrl, buffer, 3);
        }
        if (NULL == result)
        {
            // re-allocation of encodedUrl failed
            DynStrDelete(encodedUrl);
            return NULL;
        }
        assert(result == encodedUrl);
    }
    return encodedUrl;
}

void DynStrReplace(DynStr *dstr, char_t orig, char_t replace)
{
    UInt32 i;
    UInt32 len = DynStrLen(dstr);
    char_t * txt = DynStrGetCStr(dstr);
    for (i=0; i<len; i++)
    {
        if (orig==txt[i])
            txt[i] = replace;
    }
}


/*
DynStr* DynStrResize(DynStr* dstr, UInt32 newLen)
{
    
    if (newLen + 1 > dstr->bufSize)
    {
        UInt32 l = newLen + 1;
        if (dstr->bufSize + dstr->reallocIncrement > l)
            l = dstr->bufSize + dstr->reallocIncrement;
            
        using namespace std;
        char_t* d = (char_t*)realloc(dstr->str, l * sizeof(char_t));
        if (NULL == d)
            return NULL;

        dstr->bufSize = l;
        dstr->str = d;
    }
    dstr->strLen = newLen;
    dstr->str[newLen] = _T('\0');
    return dstr;
}
 */

void ReplaceCDynStrP(CDynStr** target, CDynStr* newValue)
{   
    assert(NULL != target);
    delete *target;
    *target = newValue;
}

void ReplaceCharP(char_t** target, char_t* newValue)
{
    assert(NULL != target);
    if (NULL != *target)
        free(*target);
    *target = newValue;
}

// free the string pointed out by str
void FreeCharP(char_t ** str)
{
    ReplaceCharP(str, NULL);
}


#ifdef DEBUG
static void test_DynStrReplace()
{
    DynStr *dstr = DynStrFromCharP(_T("hello"), 0);
    DynStrReplace(dstr, _T('h'), _T('p'));
    assert( 0 == tstrcmp(_T("pello"), dstr->str) );
    DynStrReplace(dstr, _T('z'), _T('k'));
    assert( 0 == tstrcmp(_T("pello"), dstr->str) );
    DynStrDelete(dstr);
}

static void test_DynStrAppendToNull()
{
    DynStr dstr;
    DynStrInit(&dstr,0);
    DynStrAppendCharP(&dstr, _T("Hello"));
    DynStrFree(&dstr);
}

static void test_DynStrRemoveStartLen()
{
    char    *str = NULL;

    DynStr *dstr = DynStrFromCharP(_T("hello"), 32);
    assert( 5 == DynStrLen(dstr) );
    DynStrRemoveStartLen(dstr, 0, 1);
    // should be "ello" now
    assert( 4 == DynStrLen(dstr) );
    str = DynStrGetCStr(dstr);
    assert( 0 == tstrcmp(_T("ello"), str) );
    DynStrRemoveStartLen(dstr, 3, 1);
    // should be "ell" now
    assert( 3 == DynStrLen(dstr) );
    str = DynStrGetCStr(dstr);
    assert( 0 == tstrcmp(_T("ell"), str) );
    DynStrAppendCharP(dstr, _T("blast"));
    // should be "ellblast" now
    assert( 8 == DynStrLen(dstr) );
    str = DynStrGetCStr(dstr);
    assert( 0 == tstrcmp(_T("ellblast"), str) );
    DynStrRemoveStartLen(dstr, 2, 4);
    // should be "elst" now
    assert( 4 == DynStrLen(dstr) );
    str = DynStrGetCStr(dstr);
    assert( 0 == tstrcmp(_T("elst"), str) );
    DynStrDelete(dstr);
}

static void test_DynStrTruncate()
{
    char_t *str = NULL;
    DynStr  dstr;

    DynStrInit(&dstr, 0);
    assert(0 == DynStrLen(&dstr));
    DynStrTruncate(&dstr,0);
    assert(0 == DynStrLen(&dstr));
    DynStrAppendCharP(&dstr, _T("hello"));
    assert(5 == DynStrLen(&dstr));
    DynStrTruncate(&dstr, 3);
    assert(3 == DynStrLen(&dstr));
    str = DynStrGetCStr(&dstr);
    assert( 0 == tstrcmp(_T("hel"), str) );

    DynStrTruncate(&dstr, 2);
    assert(2 == DynStrLen(&dstr));
    str = DynStrGetCStr(&dstr);
    assert( 0 == tstrcmp(_T("he"), str) );

    DynStrTruncate(&dstr, 0);
    assert(0 == DynStrLen(&dstr));
    str = DynStrGetCStr(&dstr);
    assert( 0 == tstrcmp(_T(""), str) );

    DynStrFree(&dstr);    

}

static void test_DynStrFromCharP()
{
    DynStr * dstr;
    char_t * str;

    dstr = DynStrFromCharP(_T("hello"), 0);
    assert( 5 == DynStrLen(dstr) );
    str = DynStrGetCStr(dstr);
    assert( 0 == tstrcmp(_T("hello"), str));
    DynStrDelete(dstr);

    dstr = DynStrFromCharP2(_T("from"), _T("hell"));
    assert( 8 == DynStrLen(dstr) );
    str = DynStrGetCStr(dstr);
    assert( 0 == tstrcmp(_T("fromhell"), str));
    DynStrDelete(dstr);

    dstr = DynStrFromCharP3(_T("me"), _T(""), _T("llow"));
    assert( 6 == DynStrLen(dstr) );
    str = DynStrGetCStr(dstr);
    assert( 0 == tstrcmp(_T("mellow"), str));
    DynStrDelete(dstr);
}

static void test_DynStrRelease()
{
    char_t * str;
    DynStr * dstr = DynStrFromCharP(_T("rusty"), 20);
    str = DynStrReleaseStr(dstr);
    assert( 0 == tstrcmp(_T("rusty"), str));
    assert( 0 == DynStrLen(dstr) );
    assert( NULL == DynStrGetCStr(dstr) );
    DynStrAppendCharP(dstr, _T("moo"));
    assert( 3 == DynStrLen(dstr) );
    assert( 0 == tstrcmp(_T("moo"), DynStrGetCStr(dstr)));
    DynStrAppendCharP2(dstr, _T(""), _T("r"));
    assert( 4 == DynStrLen(dstr) );
    assert( 0 == tstrcmp(_T("moor"), DynStrGetCStr(dstr)));
    assert( 0 == tstrcmp(_T("rusty"), str));
    DynStrDelete(dstr);
    assert( 0 == tstrcmp(_T("rusty"), str));
    free(str);
}

void test_DynStrAll()
{
    test_DynStrRelease();
    test_DynStrFromCharP();
    test_DynStrReplace();
    test_DynStrAppendToNull();
    test_DynStrRemoveStartLen();
    test_DynStrTruncate();
}

#endif
