/*
  Copyright (C) Krzysztof Kowalczyk
  Owner: Krzysztof Kowalczyk (krzysztofk@pobox.com)
*/

#include "PrefsStore.hpp"

#ifndef ARSLEXIS_USE_NEW_FRAMEWORK

#include "common.h"

#else

#include <Debug.hpp>

#define Assert assert

#ifdef new
#undef new
#endif

#ifndef NDEBUG
#define new_malloc(size) ::operator new ((size), __FILE__, __LINE__)
#else
#define new_malloc(size) ::operator new ((size))
#endif // NDEBUG

#define new_free(ptr) ::operator delete ((ptr))

//! @todo implement IsValidPrefRecord()
#define IsValidPrefRecord(recData) true


namespace {

    //! @todo do something with ErrFindDatabaseByNameTypeCreator()
    static Err ErrFindDatabaseByNameTypeCreator(const char* dbName, UInt32 type, UInt32 creator, LocalID *dbId)
    {
        Err                 err;
        DmSearchStateType   stateInfo;
        UInt16              cardNo = 0;
        char                dbNameBuf[dmDBNameLength];

        Assert(dbName);
        Assert(StrLen(dbName)<dmDBNameLength);
        Assert(dbId);

        err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, type, creator, 0, &cardNo, dbId);
        while (errNone == err)
        {
            MemSet(dbNameBuf, sizeof(dbName), 0);
            DmDatabaseInfo(cardNo, *dbId, dbNameBuf, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL);

            if (0==StrCompare(dbName, dbNameBuf))
                return errNone;
            err = DmGetNextDatabaseByTypeCreator(false, &stateInfo, type, creator, 0, &cardNo, dbId);
        }
        return dmErrCantFind;
    }

}

namespace ArsLexis 
{

#endif // ARSLEXIS_USE_NEW_FRAMEWORK

/*
The idea is to provide an API for storing/reading preferences
in a Palm database in a format that is easily upgradeable.
Each item in a preferences database has a type (int, bool, string)
and unique id. That way, when we add new items we want to store in
preferences database we just create a new unique id. The saving part
just needs to be updated to save a new item, reading part must be updated
to ignore the case when a preference item is missing.

We provide separate classes for reading and writing preferences
because they're used in that way.

To save preferences:
* construct PrefsStoreWriter object
* call ErrSet*() to set all preferences items
* call ErrSavePreferences()

To read preferences:
* construct PrefsStoreReader object
* call ErrGet*() to get desired preferences items

Serialization format:
* data is stored as blob in one record in a database whose name, creator and
  type are provided by the caller
* first 4-bytes of a blob is a header (to provide some protection against
  reading stuff we didn't create)
* then we have each item serialized

Serialization of an item:
* 2-byte unique id
* 2-byte type of an item (bool, int, string)
* bool is 1 byte (0/1)
* UInt16 is 2-byte unsigned int
* UInt32 is 4-byte unsigned int
* string is: 4-byte length of the string (including terminating 0) followed
  by string characters (also including terminating 0)
*/

// devnote: could be optimized
void serData(const char *data, long dataSize, char *prefsBlob, long *pCurrBlobSize)
{
    long i;
    for ( i=0; i<dataSize; i++)
        serByte(data[i],prefsBlob,pCurrBlobSize);
}

void serByte(unsigned char val, char *prefsBlob, long *pCurrBlobSize)
{
    Assert( pCurrBlobSize );
    if ( prefsBlob )
        prefsBlob[*pCurrBlobSize] = val;
    *pCurrBlobSize += 1;
}

inline static void serBool(Boolean val, char *prefsBlob, long *pCurrBlobSize)
{
    if (val)
        serByte(1,prefsBlob,pCurrBlobSize);
    else
        serByte(0,prefsBlob,pCurrBlobSize);
}

void serInt(int val, char *prefsBlob, long *pCurrBlobSize)
{
    int high, low;

    high = val / 256;
    low = val % 256;
    serByte( high, prefsBlob, pCurrBlobSize );
    serByte( low, prefsBlob, pCurrBlobSize );
}

static void serUInt16(UInt16 val, char *prefsBlob, long *pCurrBlobSize)
{
    Assert(sizeof(val)==2);
    serData( (char*) &val, sizeof(val), prefsBlob, pCurrBlobSize);
}

static UInt16 deserUInt16(const unsigned char ** data, long *pBlobSizeLeft)
{
    UInt16   val;
    Assert(sizeof(val)==2);
    deserData( (unsigned char*) &val, sizeof(val), data, pBlobSizeLeft);
    return val;
}

static void serUInt32(UInt32 val, char *prefsBlob, long *pCurrBlobSize)
{
    Assert(sizeof(val)==4);
    serData( (char*) &val, sizeof(val), prefsBlob, pCurrBlobSize);
}

static UInt32 deserUInt32(const unsigned char **data, long *pBlobSizeLeft)
{
    UInt32   val;
    Assert(sizeof(val)==4);
    deserData( (unsigned char*) &val, sizeof(val), data, pBlobSizeLeft);
    return val;
}

void serLong(long val, char *prefsBlob, long *pCurrBlobSize)
{
    const unsigned char * valPtr;

    valPtr = (const unsigned char*) &val;
    serByte( valPtr[0], prefsBlob, pCurrBlobSize );
    serByte( valPtr[1], prefsBlob, pCurrBlobSize );
    serByte( valPtr[2], prefsBlob, pCurrBlobSize );
    serByte( valPtr[3], prefsBlob, pCurrBlobSize );
}

static Boolean deserBool(const unsigned char **data, long *pBlobSizeLeft)
{
    unsigned char val = deserByte(data, pBlobSizeLeft);
    Assert( (1==val) || (0==val) );
    if (1==val)
        return true;
    else
        return false;
}

unsigned char deserByte(const unsigned char **data, long *pBlobSizeLeft)
{
    unsigned char val;
    const unsigned char *d = *data;

    Assert( data && *data && pBlobSizeLeft && (*pBlobSizeLeft>=1) );
    val = *d++;
    *data = d;
    *pBlobSizeLeft -= 1;
    return val;
}

static int getInt(const unsigned char *data)
{
    int val;
    val = data[0]*256+data[1];
    return val;
}

int deserInt(const unsigned char **data, long *pBlobSizeLeft)
{
    int val;

    Assert( data && *data && pBlobSizeLeft && (*pBlobSizeLeft>=2) );
    val = getInt( *data );
    *data += 2;
    *pBlobSizeLeft -= 2;
    return val;
}

long deserLong(const unsigned char **data, long *pBlobSizeLeft)
{
    long val;
    unsigned char * valPtr;
    const unsigned char *d = *data;

    valPtr = (unsigned char*) &val;
    valPtr[0] = d[0];
    valPtr[1] = d[1];
    valPtr[2] = d[2];
    valPtr[3] = d[3];
    *data = d+4;
    *pBlobSizeLeft -= 4;
    return val;
}

void deserData(unsigned char *valOut, int len, const unsigned char **data, long *pBlobSizeLeft)
{
    Assert( data && *data && pBlobSizeLeft && (*pBlobSizeLeft>=len) );
    MemMove( valOut, *data, len );
    *data = *data+len;
    *pBlobSizeLeft -= len;
}

void serString(const char *str, char *prefsBlob, long *pCurrBlobSize)
{
    int len = StrLen(str)+1;
    serInt(len, prefsBlob, pCurrBlobSize);
    serData(str, len, prefsBlob, pCurrBlobSize);
}

char *deserString(const unsigned char **data, long *pCurrBlobSize)
{
    char *  str;
    int     strLen;

    strLen = deserInt( data, pCurrBlobSize );
    Assert( 0 == (*data)[strLen-1] );
    str = (char*)new_malloc( strLen );
    if (NULL==str)
        return NULL;
    deserData( (unsigned char*)str, strLen, data, pCurrBlobSize );
    return str;
}

void deserStringToBuf(char *buf, int bufSize, const unsigned char **data, long *pCurrBlobSize)
{
    int     strLen;

    strLen = deserInt( data, pCurrBlobSize );
    Assert( 0 == (*data)[strLen-1] );
    Assert( bufSize >= strLen );
    deserData( (unsigned char*)buf, strLen, data, pCurrBlobSize );
}

static const char *deserStringInPlace(const unsigned char **data, long *pCurrBlobSize)
{
    Assert(*pCurrBlobSize>=2);
    if (*pCurrBlobSize<2)
        return NULL;
    int strLen = deserInt( data, pCurrBlobSize );
    Assert(0 == (*data)[strLen-1]);
    if (0!=(*data)[strLen-1])
        return NULL;      // this means blob corruption
    const char * str = (const char*)*data;
    *data += strLen;
    *pCurrBlobSize -= strLen;
    return str;
}

PrefsStoreReader::PrefsStoreReader(const char *dbName, UInt32 dbCreator, UInt32 dbType)
    : _dbName(dbName), _dbCreator(dbCreator), _dbType(dbType), _db(0),
      _recHandle(NULL), _recData(NULL), _fDbNotFound(false)
{
    Assert(dbName);
    Assert(StrLen(dbName) < dmDBNameLength);
}

PrefsStoreReader::~PrefsStoreReader()
{
    if (_recHandle)
        MemHandleUnlock(_recHandle);
    if (_db)
        DmCloseDatabase(_db);
}

#define PREFS_STORE_RECORD_ID "aRSp"  // comes from "ArsLexis preferences"
#define FValidPrefsStoreRecord(recData) (0==MemCmp(recData,PREFS_STORE_RECORD_ID,StrLen(PREFS_STORE_RECORD_ID)))

// Open preferences database and find a record that contains preferences.
// Return errNone if opened succesfully, otherwise an error:
//   psErrNoPrefDatabase - pref database couldn't be found
// devnote: it scans through all records even though we only store preferences
// in one record because I want to be able to use preferences database used
// in earlier versions of Noah Pro/Thes.
Err PrefsStoreReader::ErrOpenPrefsDatabase()
{
    if (_db)
    {
        Assert(_recHandle);
        Assert(_recData);
        return errNone;
    }

    // we already tried to open the database but couldn't, so don't try again
    if (_fDbNotFound)
        return psErrNoPrefDatabase;

    LocalID dbId;
    Err err = ErrFindDatabaseByNameTypeCreator(_dbName, _dbType, _dbCreator, &dbId);
    if (dmErrCantFind==err)
    {
        err = psErrNoPrefDatabase;
        goto ExitAndMarkNotFound;
    }
    if (err)
        goto ExitAndMarkNotFound;
    Assert(0!=dbId);

    _db = DmOpenDatabase(0, dbId, dmModeReadWrite);
    if (!_db)
    {
        err = DmGetLastErr();
        Assert(err);
        goto ExitAndMarkNotFound;
    }

    UInt16 recsCount = DmNumRecords(_db);
    for (UInt16 recNo = 0; recNo < recsCount; recNo++)
    {
        _recHandle = DmQueryRecord(_db, recNo);
        _recData = (unsigned char*)MemHandleLock(_recHandle);
        if ( (MemHandleSize(_recHandle)>=4) && FValidPrefsStoreRecord(_recData) )
        {
            // we found the record with prefernces so remember _recData and _recHandle
            // those must be freed in destructor
            return errNone;
        }
        MemHandleUnlock(_recHandle);
        _recHandle = NULL;
    }

    DmCloseDatabase(_db);
    _db = 0;
    err = psErrNoPrefDatabase;
ExitAndMarkNotFound:
    _fDbNotFound = true;
    return err;
}

// devnote: not very optimal implementation, we reparse the blob every
// time. We could deserialize things once and store in a buffer or optimize for
// a common pattern: reading in the same sequence as the data was written in which
// case we could remember the current place in the blob and start from there when
// we're called again (and re-start from the beginning if we don't find data)
Err PrefsStoreReader::ErrGetPrefItemWithId(int uniqueId, PrefItem *prefItem)
{
    Assert(uniqueId>=0);
    Assert(prefItem);

    Err err = ErrOpenPrefsDatabase();
    if (err)
        return err;

    Assert(_db);
    Assert(_recHandle);
    Assert(_recData);

    // usually when we Assert() we don't error out on the same condition
    // but in this case, while highly improbably, it's conceivable that some
    // other app created a database with the same name, creator, type and a
    // record that has the same magic header and we don't want to crash
    // in this case
    long recSizeLeft = (long)MemHandleSize(_recHandle);
    Assert(recSizeLeft>=4);
    if (recSizeLeft<4)
        return psErrDatabaseCorrupted;
    const unsigned char *currData = _recData;
    Assert(FValidPrefsStoreRecord(currData));
    // skip the header
    currData += 4;
    recSizeLeft-=4;
    while(recSizeLeft!=0)
    {
        // get unique id and type
        Assert(recSizeLeft>=2);
        if (recSizeLeft<2)
            return psErrDatabaseCorrupted;
        int id = deserInt(&currData,&recSizeLeft);
        Assert(id>=0);
        Assert(recSizeLeft>=2);
        if (recSizeLeft<2)
            return psErrDatabaseCorrupted;
        PrefItemType type = (PrefItemType)deserInt(&currData,&recSizeLeft);
        switch (type)
        {
            case pitBool:
                Assert(recSizeLeft>=1);
                if (recSizeLeft<1)
                    return psErrDatabaseCorrupted;
                prefItem->value.boolVal = deserBool(&currData,&recSizeLeft);
                break;
            case pitInt:
                Assert(recSizeLeft>=sizeof(int));
                if (recSizeLeft<sizeof(int))
                    return psErrDatabaseCorrupted;
                prefItem->value.intVal = deserInt(&currData, &recSizeLeft);
                break;
            case pitLong:
                Assert(recSizeLeft>=sizeof(long));
                if (recSizeLeft<sizeof(long))
                    return psErrDatabaseCorrupted;
                prefItem->value.longVal = deserLong(&currData, &recSizeLeft);
                break;
            case pitUInt16:
                Assert(recSizeLeft>=sizeof(UInt16));
                if (recSizeLeft<sizeof(UInt16))
                    return psErrDatabaseCorrupted;
                prefItem->value.uint16Val = deserUInt16(&currData, &recSizeLeft);
                break;
            case pitUInt32:
                Assert(recSizeLeft>=sizeof(UInt32));
                if (recSizeLeft<sizeof(UInt32))
                    return psErrDatabaseCorrupted;
                prefItem->value.uint32Val = deserUInt32(&currData, &recSizeLeft);
                break;
            case pitStr:
                prefItem->value.strVal = deserStringInPlace(&currData, &recSizeLeft);
                if(NULL==prefItem->value.strVal)
                    return psErrDatabaseCorrupted;
                break;
            default:
                Assert(0);
                return psErrDatabaseCorrupted;
        }
        if (id==uniqueId)
        {
            prefItem->uniqueId=id;
            prefItem->type=type;
            return errNone;
        }
    }
    return psErrItemNotFound;
}

Err PrefsStoreReader::ErrGetBool(int uniqueId, Boolean *value)
{
    PrefItem    prefItem;
    Err err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    Assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitBool)
        return psErrItemTypeMismatch;
    *value = prefItem.value.boolVal;
    return errNone;
}

Err PrefsStoreReader::ErrGetInt(int uniqueId, int *value)
{
    PrefItem    prefItem;
    Err err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    Assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitInt)
        return psErrItemTypeMismatch;
    *value = prefItem.value.intVal;
    return errNone;
}

Err PrefsStoreReader::ErrGetLong(int uniqueId, long *value)
{
    PrefItem    prefItem;
    Err err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    Assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitLong)
        return psErrItemTypeMismatch;
    *value = prefItem.value.longVal;
    return errNone;
}

Err PrefsStoreReader::ErrGetUInt16(int uniqueId, UInt16 *value)
{
    PrefItem    prefItem;
    Err err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    Assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitUInt16)
        return psErrItemTypeMismatch;
    *value = prefItem.value.uint16Val;
    return errNone;
}

Err PrefsStoreReader::ErrGetUInt32(int uniqueId, UInt32 *value)
{
    PrefItem    prefItem;
    Err err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    Assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitUInt32)
        return psErrItemTypeMismatch;
    *value = prefItem.value.uint32Val;
    return errNone;
}

// the string returned points to data inside the object that is only valid
// while the object is alive. If client wants to use it after that, he must
// make a copy
Err PrefsStoreReader::ErrGetStr(int uniqueId, const char **value)
{
    PrefItem    prefItem;
    Err err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    Assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitStr)
        return psErrItemTypeMismatch;
    *value = prefItem.value.strVal;
    return errNone;
}

PrefsStoreWriter::PrefsStoreWriter(const char *dbName, UInt32 dbCreator, UInt32 dbType)
    : _dbName(dbName), _dbCreator(dbCreator), _dbType(dbType), _itemsCount(0)
{
    Assert(dbName);
    Assert(StrLen(dbName) < dmDBNameLength);
}

PrefsStoreWriter::~PrefsStoreWriter()
{
}

static PrefItem * FindPrefItemById(PrefItem *items, int itemsCount, int uniqueId)
{
    for(int i=0; i<itemsCount; i++)
    {
        if (items[i].uniqueId==uniqueId)
            return &(items[i]);
    }
    return NULL;
}

Err PrefsStoreWriter::ErrSetItem(PrefItem *item)
{
    Assert(item->uniqueId>=0);

    if ( NULL != FindPrefItemById((PrefItem*)_items, _itemsCount, item->uniqueId) )
    {
        Assert(0); // we assert because we never want this to happen
        return psErrDuplicateId;
    }

    // TODO: make it grow dynamically in the future
    if (_itemsCount>=MAX_PREFS_ITEMS)
        return memErrNotEnoughSpace;

    _items[_itemsCount++] = *item;
    return errNone;
}

Err PrefsStoreWriter::ErrSetBool(int uniqueId, Boolean value)
{
    PrefItem    prefItem;

    prefItem.type = pitBool;
    prefItem.uniqueId = uniqueId;
    prefItem.value.boolVal = value;

    return ErrSetItem( &prefItem );
}

Err PrefsStoreWriter::ErrSetInt(int uniqueId, int value)
{
    PrefItem    prefItem;

    prefItem.type = pitInt;
    prefItem.uniqueId = uniqueId;
    prefItem.value.intVal = value;

    return ErrSetItem( &prefItem );
}

Err PrefsStoreWriter::ErrSetLong(int uniqueId, long value)
{
    PrefItem    prefItem;

    prefItem.type = pitLong;
    prefItem.uniqueId = uniqueId;
    prefItem.value.longVal = value;

    return ErrSetItem( &prefItem );
}

Err PrefsStoreWriter::ErrSetUInt16(int uniqueId, UInt16 value)
{
    PrefItem    prefItem;

    prefItem.type = pitUInt16;
    prefItem.uniqueId = uniqueId;
    prefItem.value.uint16Val = value;

    return ErrSetItem( &prefItem );
}

Err PrefsStoreWriter::ErrSetUInt32(int uniqueId, UInt32 value)
{
    PrefItem    prefItem;

    prefItem.type = pitUInt32;
    prefItem.uniqueId = uniqueId;
    prefItem.value.uint32Val = value;

    return ErrSetItem( &prefItem );
}

// value must point to a valid location during ErrSavePrefernces() since
// for perf reasons we don't make a copy of the string
Err PrefsStoreWriter::ErrSetStr(int uniqueId, const char *value)
{
    PrefItem    prefItem;

    prefItem.type = pitStr;
    prefItem.uniqueId = uniqueId;
    prefItem.value.strVal = value;

    return ErrSetItem( &prefItem );
}

// Create a blob containing serialized preferences.
// Devnote: caller needs to free memory returned.
// TODO: move ser* (serData etc.) functions from common.c to here
// after changing prefs in all apps to use PrefsStore
static void* SerializeItems(PrefItem *items, int itemsCount, long *pBlobSize)
{
    Assert(items);
    Assert(itemsCount>=0);
    Assert(pBlobSize);

    char *  prefsBlob = NULL;
    long    blobSize, blobSizePhaseOne;
    /* phase one: calculate the size of the blob */
    /* phase two: create the blob */
    for(int phase=1; phase<=2; phase++)
    {
        blobSize = 0;
        Assert( 4 == StrLen(PREFS_STORE_RECORD_ID) );

        serData( (char*)PREFS_STORE_RECORD_ID, StrLen(PREFS_STORE_RECORD_ID), prefsBlob, &blobSize );
        for(int item=0; item<itemsCount; item++)
        {
            Assert( items[item].uniqueId >= 0 );
            serInt( items[item].uniqueId, prefsBlob, &blobSize);
            serInt( (int)items[item].type, prefsBlob, &blobSize);
            switch( items[item].type )
            {
                case pitBool:
                    serBool(items[item].value.boolVal, prefsBlob, &blobSize);
                    break;
                case pitInt:
                    serInt(items[item].value.intVal, prefsBlob, &blobSize);
                    break;
                case pitLong:
                    serLong(items[item].value.longVal, prefsBlob, &blobSize);
                    break;
                case pitUInt16:
                    serUInt16(items[item].value.uint16Val, prefsBlob, &blobSize);
                    break;
                case pitUInt32:
                    serUInt32(items[item].value.uint32Val, prefsBlob, &blobSize);
                    break;
                case pitStr:
                    serString(items[item].value.strVal, prefsBlob, &blobSize);
                    break;
                default:
                    Assert(0);
                    break;
            }            
        }

        if ( 1 == phase )
        {
            Assert( blobSize > 0 );
            blobSizePhaseOne = blobSize;
            prefsBlob = (char*)new_malloc( blobSize );
            if (NULL == prefsBlob)
                return NULL;
        }
    }
    Assert( blobSize == blobSizePhaseOne );
    Assert( blobSize >= 4 );

    *pBlobSize = blobSize;
    Assert( prefsBlob );
    return prefsBlob;
}

// Save preferences previously set via ErrSet*() calls to a database.
// If something goes wrong, returns an error
// Possible errors:
//   memErrNotEnoughSpace - not enough memory to allocate needed structures
//   errors from Dm*() calls
Err PrefsStoreWriter::ErrSavePreferences()
{
    Err     err = errNone;
    long    blobSize;
    void *  prefsBlob = SerializeItems(_items, _itemsCount, &blobSize);
    if ( NULL == prefsBlob ) 
        return memErrNotEnoughSpace;

    DmOpenRef db = DmOpenDatabaseByTypeCreator(_dbType, _dbCreator, dmModeReadWrite);
    if (!db)
    {
        err = DmCreateDatabase(0, _dbName, _dbCreator, _dbType, false);
        if ( err)
            return err;

        db = DmOpenDatabaseByTypeCreator(_dbType, _dbCreator, dmModeReadWrite);
        if (!db)
            return DmGetLastErr();
    }

    // set backup bit on the database. code adapted from DataStore.cpp
    // DataStore::open()
    if (errNone == err)
    {
        LocalID localId;
        UInt16 cardNo;
        UInt16 attribs;
        err = DmOpenDatabaseInfo(db, &localId, NULL, NULL, &cardNo, NULL);
        if (errNone != err)
            goto Continue;
        err = DmDatabaseInfo(cardNo, localId, NULL, &attribs, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        if (errNone != err)
            goto Continue;
        if (0 != attribs & dmHdrAttrBackup)
            goto Continue;
        attribs |= dmHdrAttrBackup;
        err = DmSetDatabaseInfo(cardNo, localId, NULL, &attribs, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
Continue:
        err = errNone;
    }

    UInt16    recNo = 0;
    UInt16    recsCount = DmNumRecords(db);
    MemHandle recHandle;
    Boolean   fRecordBusy = false;
    Boolean   fRecFound = false;
    void *    recData;
    long      recSize;
    while (recNo < recsCount)
    {
        recHandle = DmGetRecord(db, recNo);
        fRecordBusy = true;
        recData = MemHandleLock(recHandle);
        recSize = MemHandleSize(recHandle);
        if (IsValidPrefRecord(recData))
        {
            fRecFound = true;
            break;
        }
        MemPtrUnlock(recData);
        DmReleaseRecord(db, recNo, true);
        fRecordBusy = false;
        ++recNo;
    }

    if (fRecFound && blobSize>recSize)
    {
        /* need to resize the record */
        MemPtrUnlock(recData);
        DmReleaseRecord(db,recNo,true);
        fRecordBusy = false;
        recHandle = DmResizeRecord(db, recNo, blobSize);
        if ( NULL == recHandle )
            return DmGetLastErr();
        recData = MemHandleLock(recHandle);
        Assert( MemHandleSize(recHandle) == blobSize );        
    }

    if (!fRecFound)
    {
        recNo = 0;
        recHandle = DmNewRecord(db, &recNo, blobSize);
        if (!recHandle)
        {
            err = DmGetLastErr();
            goto CloseDbExit;
        }
        recData = MemHandleLock(recHandle);
        fRecordBusy = true;
    }

    err = DmWrite(recData, 0, prefsBlob, blobSize);
    MemPtrUnlock(recData);
    if (fRecordBusy)
        DmReleaseRecord(db, recNo, true);
CloseDbExit:
    // if had error before - preserve that error
    // otherwise return error code from DmCloseDatabase()
    if (err)
        DmCloseDatabase(db);
    else
        err = DmCloseDatabase(db);
    new_free( prefsBlob );
    return err;
}

#ifdef ARSLEXIS_USE_NEW_FRAMEWORK
} // namespace ArsLexis
#endif // ARSLEXIS_USE_NEW_FRAMEWORK

