#include <Debug.hpp>
#include "dStr.hpp"

// for interop, get C-compatible string. You can write into this string
// up to its termination 0 - this is guaranteed to belong to the dStr
// object. However:
// - if you put 0 there, it'll screw up dStr's view of this string
// - dStr is binary-safe while C str isn't so what you get here might
//   not be the whole string
char_t *dStrGetCStr(dStr *dstr)
{
    return DSTR_STR(dstr);
}

// same as dStrGetCStr but reinforces the notion that dStr can handle
// arbitrary binary data
char_t *dStrGetData(dStr *dstr)
{
    return DSTR_DATA(dstr);
}

// get length of the string. Note that since dStr is binary-safe, this
// might be different (bigger) than StrLen(dStrGetCStr(dstr))
UInt32 dStrLen(dStr *dstr)
{
    return dstr->strLen;
}

// truncate the string (remove everything after len characters)
// In extreme case len is 0 and we clear the string
// The good thing about this function is that it doesn't
// free the buffer, so it can be used to efficiently re-use
// the string memory without reallocation
void   dStrTruncate(dStr *dstr, UInt32 len)
{
    assert(NULL != dstr);
    assert(dstr->strLen >= len);

    DSTR_STR(dstr)[len] = '\0';
    dstr->strLen = len;
}

void dStrDelete(dStr *dstr)
{
    assert(NULL != dstr);
    if (NULL == dstr->str)
        MemPtrFree(dstr->str);
    MemPtrFree(dstr);
}

dStr * dStrNew(UInt32 bufSize)
{
    return dStrFromCharP("", bufSize);
}

void   dStrSetReallocIncrement(dStr *dstr, UInt32 increment)
{
    dstr->reallocIncrement = increment;
}

dStr *dStrFromCharP(char_t *str, UInt32 initBufSize)
{
    UInt32 strLen  = StrLen(str);
    UInt32 bufSize = strLen+1;
    dStr * dstr;

    if (bufSize < initBufSize)
        bufSize = initBufSize;

    dstr = (dStr*)MemPtrNew(sizeof(dStr));
    if (NULL==dstr)
        return NULL;

    dstr->reallocIncrement = 0;
    dstr->str = (Char*)MemPtrNew(bufSize);
    if (NULL == dstr->str)
    {
        MemPtrFree(dstr);
        return NULL;
    }

    dstr->bufSize = bufSize;
    dstr->strLen = strLen;
    MemMove( DSTR_STR(dstr), str, strLen + 1);

    return dstr;
}

dStr *dStrFromCharP2(char_t *strOne, char_t *strTwo)
{
    UInt32 bufSize;
    dStr * dstr;
    dStr * dstrNew;
    UInt32 strOneLen, strTwoLen;

    assert(NULL!=strOne);
    assert(NULL!=strTwo);

    strOneLen = StrLen(strOne);
    strTwoLen = StrLen(strTwo);
    bufSize  = strOneLen + strTwoLen + 1;

    dstr = dStrFromCharP(strOne, bufSize);
    if (NULL==dstr)
        return NULL;

    dstr->bufSize = bufSize;
    dstr->strLen = 0;
    DSTR_STR(dstr)[0] = '\0';

    dstrNew = dStrAppend(dstr, strTwo, strTwoLen);
    assert(NULL != dstrNew);
    assert(dstrNew == dstr);
    assert(0==DSTR_LEFT(dstr));
    return dstr;
}

dStr *dStrFromCharP3(char_t *strOne, char_t *strTwo, char_t *strThree)
{
    UInt32 bufSize;
    dStr * dstr;
    dStr * dstrNew;
    UInt32 strOneLen, strTwoLen, strThreeLen;

    assert(NULL!=strOne);
    assert(NULL!=strTwo);
    assert(NULL!=strThree);

    strOneLen = StrLen(strOne);
    strTwoLen = StrLen(strTwo);
    strThreeLen = StrLen(strThree);
    bufSize  = strOneLen + strTwoLen + strThreeLen + 1;

    dstr = dStrFromCharP(strOne, bufSize);
    if (NULL==dstr)
        return NULL;

    dstr->bufSize = bufSize;
    dstr->strLen = 0;
    DSTR_STR(dstr)[0] = '\0';

    dstrNew = dStrAppend(dstr, strTwo, strTwoLen);
    assert(NULL != dstrNew);
    dstrNew = dStrAppend(dstr, strThree, strThreeLen);
    assert(NULL != dstrNew);
    assert(dstrNew == dstr);
    assert(0==DSTR_LEFT(dstr));
    return dstr;
}

dStr * dStrAppendCharP(dStr *dstr, char_t *str)
{
    UInt32 strLen = StrLen(str);
    return dStrAppend(dstr, str, strLen);
}

dStr *dStrAppendStr(dStr *dstr, dStr *toAppend)
{
    return dStrAppend(dstr, DSTR_STR(toAppend), dStrLen(toAppend));
}

// a generic append which appends arbitrary binary data
// to dStr. Re-allocates the string if neccesary. Note that
// it means that you shouldn't use dStr passed as an argument
// anymore.
// Return pointer to itself or NULL if we had to re-allocate the buffer
// but failed.
// THE ORIGINAL dStr - the client has to do it by himself.
dStr *dStrAppend(dStr *dstr, char_t *buf, UInt32 bufSize)
{
    UInt32  newRequiredSize;
    char_t *  curEnd;
    UInt32  newStrLen;
    char_t *  newStr;
    UInt32  newLen;

    if ( bufSize > DSTR_LEFT(dstr) )
    {
        // need to re-allocate the buffer
        newLen = dstr->strLen+1+bufSize;
        if (0 != dstr->reallocIncrement)
        {
            // if reallocIncrement wants us to alloc bigger buffer, do it
            if (dstr->bufSize + dstr->reallocIncrement > newLen)
            {
                newLen = dstr->bufSize + dstr->reallocIncrement;
            }
        }
        newStr = (Char*)MemPtrNew(dstr->strLen+1+bufSize);
        if (NULL == newStr)
            return NULL;
        MemMove(newStr, DSTR_STR(dstr), dStrLen(dstr));
        MemPtrFree(DSTR_STR(dstr));
        dstr->str = newStr;
    }

    // here we're sure we have enough space
    assert( bufSize <= DSTR_LEFT(dstr) );

    // TODO: it might overflow UInt32. but what the hell. If the caller
    // wants more than 4 GB in total, then he has problems anyway
    newRequiredSize = dstr->strLen + 1 + bufSize; // 1 for terminating 0

    curEnd = DSTR_STR(dstr) + dstr->strLen;
    MemMove(curEnd, buf, bufSize);
    newStrLen = dstr->strLen + bufSize;
    DSTR_STR(dstr)[newStrLen] = '\0';
    dstr->strLen = newStrLen;

    return dstr;
}

// return a C-compatible copy of the string. It might be different
// than real data if the data has embedded 0, which for C means
// end of string
char_t * dStrCharPCopy(dStr *dstr)
{
    UInt32 strLen = StrLen(DSTR_STR(dstr));
    char_t *result = (Char*)MemPtrNew(strLen+1);
    if (NULL == result)
        return NULL;
    MemMove(result, DSTR_STR(dstr), strLen+1);
    return result;
}

// remove part of the string from dstr, starting at position start
// of lenght len.
// e.g. dStrRemoveStartLen("hello", 1, 2) => "hlo" (removed "el")
// len can be zero to simplify client code
// start has to be withint string
void dStrRemoveStartLen(dStr *dstr, UInt32 start, UInt32 len)
{
    char_t *  str = DSTR_STR(dstr);
    UInt32  toMove;

    if (0==len)
        return;

    assert(start<dStrLen(dstr));

    // TODO: should I really quit? this hides errors in retail
    // on the plus side, it won't currupt memory in shipped software
    if (start<dStrLen(dstr))
        return;

    toMove = dStrLen(dstr) - start - len;
    MemMove(str+start, str+start+len, toMove);
}

dStr * dStrAppendChar(dStr *dstr, char_t c)
{
    return dStrAppend(dstr, &c, 1);
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

// url-encode a srcUrl dStr and return a newly allocated dStr. Caller has to
// free the result. Return NULL if couldn't allocatte new string
dStr *dStrUrlEncode(dStr *toEncode)
{
    dStr  * encodedUrl;
    dStr  * result;
    int     i, len;
    char_t    c;
    char_t    buffer[3];

    // allocate a new dStr of 2x size (a safe margin for escaped characaters,
    // so that we don't have to re-allocate memory every time we
    // encode a character)
    encodedUrl = dStrNew(2*dStrLen(toEncode));
    if (NULL == encodedUrl)
        return NULL;

    len = dStrLen(toEncode);
    for (i=0; i<len; i++)
    {
        c = DSTR_STR(toEncode)[i];
        if ( ' ' == c )
            result = dStrAppendChar(encodedUrl, '+');
        else if ('\0'==c || (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || isUriUnescapedChar(c))
        {
            result = dStrAppendChar(encodedUrl, c);
        }
        else
        {
            buffer[0] = '%';
            CharToHexString(c, &(buffer[1]) );
            result = dStrAppend(encodedUrl, buffer, 3);
        }
        if (NULL == result)
        {
            // re-allocation of encodedUrl failed
            dStrDelete(encodedUrl);
            return NULL;
        }
        assert(result == encodedUrl);
    }
    return encodedUrl;
}

