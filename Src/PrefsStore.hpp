/*
  Copyright (C) Krzysztof Kowalczyk
  Owner: Krzysztof Kowalczyk (krzysztofk@pobox.com)

  Helper class for easy storing preferences.
*/

#ifndef _PREFS_STORE_HPP_
#define _PREFS_STORE_HPP_

#ifndef ARSLEXIS_USE_NEW_FRAMEWORK

#include "ErrBase.h"
#include "PalmOS.h"

#define NON_COPYABLE

#else

#include <ErrBase.h>
#include <Utility.hpp>
#include <PalmOS.h>

#define NON_COPYABLE : private NonCopyable

namespace ArsLexis 
{

#endif // ARSLEXIS_USE_NEW_FRAMEWORK

// tried to set the item with an id of existing item
#define psErrDuplicateId        psErrorClass+1
// didn't find an item with a given id
#define psErrItemNotFound       psErrorClass+2
// preferences database doesn't exist
#define psErrNoPrefDatabase     psErrorClass+3
// the type of an item with a given id is different than requested type
#define psErrItemTypeMismatch   psErrorClass+4
// preferences record is corrupted
#define psErrDatabaseCorrupted  psErrorClass+5

enum PrefItemType {
    pitBool = 1,
    pitInt,
    pitLong,
    pitUInt16,
    pitUInt32,
    pitStr
};

typedef struct _prefItem
{
    int                 uniqueId;
    enum PrefItemType   type;
    union {
        Boolean     boolVal;
        int         intVal;
        long        longVal;
        UInt16      uint16Val;
        UInt32      uint32Val;
        const char *       strVal;
    } value;
} PrefItem;

class PrefsStoreReader NON_COPYABLE
{
private:
    const char *      _dbName;
    UInt32      _dbCreator;
    UInt32      _dbType;
    DmOpenRef   _db;
    MemHandle   _recHandle;
    const unsigned char *  _recData;
    Boolean     _fDbNotFound;

    Err ErrOpenPrefsDatabase();
    Err ErrGetPrefItemWithId(int uniqueId, PrefItem *prefItem);
public:
    PrefsStoreReader(const char *dbName, UInt32 dbCreator, UInt32 dbType);
    Err ErrGetBool(int uniqueId, Boolean *value);
    Err ErrGetInt(int uniqueId, int *value);
    Err ErrGetLong(int uniqueId, long *value);
    Err ErrGetUInt16(int uniqueId, UInt16 *value);
    Err ErrGetUInt32(int uniqueId, UInt32 *value);
    Err ErrGetStr(int uniqueId, const char ** value);
    ~PrefsStoreReader();
};

#define MAX_PREFS_ITEMS   60

class PrefsStoreWriter NON_COPYABLE
{
private:
    const char *      _dbName;
    UInt32      _dbCreator;
    UInt32      _dbType;
    PrefItem    _items[MAX_PREFS_ITEMS];
    int         _itemsCount;

    Err ErrSetItem(PrefItem *item);

public:
    PrefsStoreWriter(const char *dbName, UInt32 dbCreator, UInt32 dbType);
    Err ErrSetBool(int uniqueId, Boolean value);
    Err ErrSetInt(int uniqueId, int value);
    Err ErrSetLong(int uniqueId, long value);
    Err ErrSetUInt16(int uniqueId, UInt16 value);
    Err ErrSetUInt32(int uniqueId, UInt32 value);
    Err ErrSetStr(int uniqueId, const char *value);
    Err ErrSavePreferences();
    ~PrefsStoreWriter();
};

void            serByte    (unsigned char val, char *prefsBlob, long *pCurrBlobSize);
void            serInt     (int val, char *prefsBlob, long *pCurrBlobSize);
void            serLong    (long val, char *prefsBlob, long *pCurrBlobSize);
unsigned char   deserByte  (const unsigned char ** data, long *pBlobSizeLeft);
int             deserInt   (const unsigned char ** data, long *pBlobSizeLeft);
long            deserLong  (const unsigned char **data, long *pBlobSizeLeft);
void            serData    (const char *data, long dataSize, char *prefsBlob, long *pCurrBlobSize);
void            deserData  (unsigned char *valOut, int len, const unsigned char **data, long *pBlobSizeLeft);
void            serString  (const char *str, char *prefsBlob, long *pCurrBlobSize);
char *          deserString(const unsigned char **data, long *pCurrBlobSize);
void            deserStringToBuf(char *buf, int bufSize, const unsigned char **data, long *pCurrBlobSize);

#ifdef ARSLEXIS_USE_NEW_FRAMEWORK
} // namespace ArsLexis
#endif // ARSLEXIS_USE_NEW_FRAMEWORK

#endif
