#ifndef _DSTR_H_
#define _DSTR_H_

#include <BaseTypes.hpp>
using ArsLexis::char_t;

#ifdef WIN32
// warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4200)
#endif

// this is a simple string class that helps efficiently build strings
// compatibility with standard C string is of high priority. We do it by:
// - dStr can always be read as a C string (always NULL-terminated)
// It can also be treated as a generic, binary buffer since it's binary-safe
// (i.e. you can put 0 in it and it won't screw up the buffer. It will, however
typedef struct dStr {
    // how much space do we have in total for this string. We can use
    // only bufSize-1 since the last byte is always 0 for NULL termination
    UInt32   bufSize;
    // real length of the string. does not include terminating NULL
    UInt32   strLen;
    // by how much increase the buffer when a re-allocation is needed.
    // if 0, add only as much as necessary. This can be used to optimize
    // appending i.e. if you expect to append a lot of small strings, set
    // this to minimize the number of re-allocations due to appending
    UInt32   reallocIncrement;
    // that's where the data begins
    char_t     *str;
} dStr;

/* this macro returns a pointer to a string in this dStr.
   Note that dStr doesn't have a notion of NULL string,
   it only has empty string */
#define DSTR_STR(dstr) (dstr->str)
/* the same as DSTR_STR, but enforces the idea, that dStr can handle
   arbitrary binary data */
#define DSTR_DATA(dstr) (dstr->str)
/* length of the data inside dStr */
#define DSTR_LEN(dstr) (dstr->strLen)
/* this macro defines how much data do we have left in the buffer.
   that's how much we can add to the string without re-allocating it */
#define DSTR_LEFT(dstr) (dstr->bufSize - dstr->strLen - 1)

dStr *   dStrNew(UInt32 bufSize);
void     dStrSetReallocIncrement(dStr *dstr, UInt32 increment);
dStr *   dStrFromCharP(char_t *str, UInt32 initBufSize);
dStr *   dStrFromCharP2(char_t *strOne, char_t *strTwo);
dStr *   dStrFromCharP3(char_t *strOne, char_t *strTwo, char_t *strThree);
void     dStrTruncate(dStr *dstr, UInt32 len);
char_t * dStrGetCStr(dStr *dstr);
char_t * dStrGetData(dStr *dstr);
UInt32   dStrLen(dStr *dstr);
dStr *   dStrAppend(dStr *dstr, char_t *data, UInt32 dataSize);
dStr *   dStrAppendCharP(dStr *dstr, char_t *str);
dStr *   dStrAppendChar(dStr *dstr, char_t c);
void     dStrDelete(dStr *dstr);
char_t * dStrCharPCopy(dStr *dstr);
void     dStrRemoveStartLen(dStr *dstr, UInt32 start, UInt32 len);
dStr *   dStrAppendStr(dStr *dstr, dStr *toAppend);
dStr *   dStrUrlEncode(dStr *srcUrl);

#endif // _DSTR_H_
