#ifndef _DYNSTR_H_
#define _DYNSTR_H_

#include <BaseTypes.hpp>
using ArsLexis::char_t;

#ifdef WIN32
// warning C4200: nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4200)
#endif

// this is a simple string class that helps efficiently build strings
// compatibility with standard C string is of high priority. We do it by:
// - DynStr can always be read as a C string (always NULL-terminated)
// It can also be treated as a generic, binary buffer since it's binary-safe
// (i.e. you can put 0 in it and it won't screw up the buffer. It will, however
typedef struct DynStrTag {
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
} DynStr;

/* this macro returns a pointer to a string in this DynStr.
   Note that DynStr doesn't have a notion of NULL string,
   it only has empty string */
#define DYNSTR_STR(dstr) (dstr->str)
/* the same as DYNSTR_STR, but enforces the idea, that DynStr can handle
   arbitrary binary data */
#define DYNSTR_DATA(dstr) (dstr->str)
/* length of the data inside DynStr */
#define DYNSTR_LEN(dstr) (dstr->strLen)
/* this macro defines how much data do we have left in the buffer.
   that's how much we can add to the string without re-allocating it */
#define DYNSTR_SIZE_LEFT(dstr) (dstr->bufSize - (dstr->strLen - 1)*sizeof(char_t))

DynStr *   DynStrNew__(UInt32 bufSize, const char_t* file, int line);
DynStr *   DynStrFromCharP__(const char_t *str, UInt32 initBufSize, const char_t* file, int line);

#define DynStrNew(size) DynStrNew__((size), __FILE__, __LINE__)
#define DynStrFromCharP(str, initBufSize) DynStrFromCharP__((str), (initBufSize), __FILE__, __LINE__)

void       DynStrSetReallocIncrement(DynStr *dstr, UInt32 increment);
DynStr *   DynStrFromCharP2(const char_t *strOne, const char_t *strTwo);
DynStr *   DynStrFromCharP3(const char_t *strOne, const char_t *strTwo, const char_t *strThree);
DynStr *   DynStrAssignCharP(DynStr *dstr, const char_t *str);
void       DynStrTruncate(DynStr *dstr, UInt32 len);
char_t *   DynStrGetCStr(DynStr *dstr);
char_t *   DynStrGetData(DynStr *dstr);
UInt32     DynStrLen(DynStr *dstr);
DynStr *   DynStrAppendData(DynStr *dstr, const char *data, UInt32 dataSize);
DynStr *   DynStrAppendCharP(DynStr *dstr, const char_t *str);
DynStr *   DynStrAppendCharPBuf(DynStr *dstr, const char_t *str, UInt32 strLen);
DynStr *   DynStrAppendChar(DynStr *dstr, const char_t c);
void       DynStrDelete(DynStr *dstr);
char_t *   DynStrCharPCopy(DynStr *dstr);
void       DynStrRemoveStartLen(DynStr *dstr, UInt32 start, UInt32 len);
DynStr *   DynStrAppendDynStr(DynStr *dstr, DynStr *toAppend);
DynStr *   DynStrUrlEncode(DynStr *srcUrl);
void       DynStrReplace(DynStr *dstr, char_t orig, char_t replace);

#ifdef DEBUG
void test_DynStrReplace();
#endif

#endif // _DYNSTR_H_
