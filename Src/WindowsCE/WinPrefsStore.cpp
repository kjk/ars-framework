/*
  Copyright (C) Krzysztof Kowalczyk
  Owner: Krzysztof Kowalczyk (krzysztofk@pobox.com)
*/

#include <windows.h>

#include <WinSysUtils.hpp>
#include <Debug.hpp>

#include "WinPrefsStore.hpp"

namespace ArsLexis 
{

static String GetDbPath(const char_t *dbName)
{
    String dirPath;
    bool fOk = GetSpecialFolderPath(dirPath);
    if (!fOk)
        return _T("");

    String fullPath(dirPath);
    fullPath.append(_T("\\"));

    char_t *tmpDbName = new char_t[tstrlen(dbName)+1];
    wcscpy(tmpDbName, dbName);
    for (unsigned int i=0; i<tstrlen(tmpDbName); i++)
    {
        if(tmpDbName[i]==' ') 
            tmpDbName[i]='_';
    }
    fullPath.append(tmpDbName);
    delete [] tmpDbName;

    return fullPath;
}

static int getPrefItemSize(PrefItem& item)
{
    switch (item.type)
    {
        case pitBool:
            return sizeof(bool);
        case pitInt:
            return sizeof(int);
        case pitLong:
            return sizeof(long);
        case pitUInt16:
            return sizeof(ushort_t);
        case pitUInt32:
            return sizeof(ulong_t);
        case pitStr:
            return tstrlen(item.value.strVal)*2+2;
    }       
    return 0;
}
    
PrefsStoreReader::PrefsStoreReader(const char_t *dbName, ulong_t dbCreator, ulong_t dbType)
    : _dbName(dbName), _dbCreator(dbCreator), _dbType(dbType),_handle(INVALID_HANDLE_VALUE)
{
    String fullPath = GetDbPath(dbName);
    if (fullPath.empty())
        return;
    _handle = CreateFile(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
}
    
PrefsStoreReader::~PrefsStoreReader()
{
    while(!strPointers_.empty())
    {
        delete [] strPointers_.front();
        strPointers_.pop_front();
    }
    if (INVALID_HANDLE_VALUE!=_handle)
        CloseHandle(_handle);
}
    
#define PREFS_STORE_RECORD_ID "aRSp"  // comes from "ArsLexis preferences"
#define FValidPrefsStoreRecord(recData) (0==MemCmp(recData,PREFS_STORE_RECORD_ID,StrLen(PREFS_STORE_RECORD_ID)))

status_t PrefsStoreReader::ErrGetBool(int uniqueId, bool *value)
{
    PrefItem    prefItem;
    status_t err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitBool)
        return psErrItemTypeMismatch;
    *value = prefItem.value.boolVal;
    return errNone;
}
    
status_t PrefsStoreReader::ErrGetInt(int uniqueId, int *value)
{
    PrefItem    prefItem;
    status_t err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitInt)
        return psErrItemTypeMismatch;
    *value = prefItem.value.intVal;
    return errNone;
}
    
status_t PrefsStoreReader::ErrGetLong(int uniqueId, long *value)
{
    PrefItem    prefItem;
    status_t err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitLong)
        return psErrItemTypeMismatch;
    *value = prefItem.value.longVal;
    return errNone;
}
    
status_t PrefsStoreReader::ErrGetUInt16(int uniqueId, ushort_t *value)
{
    PrefItem    prefItem;
    status_t err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitUInt16)
        return psErrItemTypeMismatch;
    *value = prefItem.value.uint16Val;
    return errNone;
}
    
status_t PrefsStoreReader::ErrGetUInt32(int uniqueId, ulong_t *value)
{
    PrefItem    prefItem;
    status_t err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitUInt32)
        return psErrItemTypeMismatch;
    *value = prefItem.value.uint32Val;
    return errNone;
}
    
// the string returned points to data inside the object that is only valid
// while the object is alive. If client wants to use it after that, he must
// make a copy
status_t PrefsStoreReader::ErrGetStr(int uniqueId, const char_t **value)
{
    PrefItem    prefItem;
    status_t err = ErrGetPrefItemWithId(uniqueId, &prefItem);
    if (err)
        return err;
    assert(prefItem.uniqueId == uniqueId);
    if (prefItem.type != pitStr)
        return psErrItemTypeMismatch;
    *value = prefItem.value.strVal;
    return errNone;
}

status_t PrefsStoreReader::ErrLoadPreferences()
{
    if (INVALID_HANDLE_VALUE==_handle)
        return psErrNoPrefDatabase;

    PrefItem prefItem;
    DWORD read=0;
    DWORD size=0;
    while (true)
    {
        ReadFile(_handle, &prefItem.uniqueId, sizeof(prefItem.uniqueId), &read, NULL);
        if (0==read) 
            return errNone;
        if (read!=sizeof(prefItem.uniqueId)) 
            return psErrDatabaseCorrupted;

        ReadFile(_handle, &size, sizeof(size), &read, NULL);
        if (read!=sizeof(size)) 
            return psErrDatabaseCorrupted;

        ReadFile(_handle, &prefItem.type, sizeof(prefItem.type), &read, NULL);
        if (read!=sizeof(prefItem.type)) 
            return psErrDatabaseCorrupted;

        if (pitStr!=prefItem.type)
        {
            ReadFile(_handle, &prefItem.value, size, &read, NULL);
            if (read!=size) 
                return psErrDatabaseCorrupted;
        }
        else
        {
            char_t *str = new char_t[size/sizeof(char_t)+1];
            prefItem.value.strVal = str;
            strPointers_.push_back(str);
            ReadFile(_handle, (void*)prefItem.value.strVal, size, &read, NULL);
            if (read!=size) 
                return psErrDatabaseCorrupted;
        }
        items_.insert(std::pair<int const, PrefItem>(prefItem.uniqueId,prefItem));
    }
    return errNone;
}

status_t PrefsStoreReader::ErrGetPrefItemWithId(int uniqueId, PrefItem *prefItem)
{
    status_t err = errNone;

    if (items_.empty())        
    {
        err = ErrLoadPreferences();
        if (err!=errNone)
            return err;
    }

    std::map< int, PrefItem>::iterator it=items_.find(uniqueId);

    if (it==items_.end())
        return psErrItemNotFound;

    *prefItem=(*it).second;
    return errNone;
}
    
PrefsStoreWriter::PrefsStoreWriter(const char_t *dbName, ulong_t dbCreator, ulong_t dbType)
    : _dbName(dbName), _dbCreator(dbCreator), _dbType(dbType), _itemsCount(0),_handle(INVALID_HANDLE_VALUE)
{
    String fullPath = GetDbPath(dbName);
    if (fullPath.empty())
        return;
    _handle = CreateFile(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL); 
}
    
PrefsStoreWriter::~PrefsStoreWriter()
{
    if (INVALID_HANDLE_VALUE!=_handle)
        CloseHandle(_handle);
}
        
status_t PrefsStoreWriter::ErrSetBool(int uniqueId, bool value)
{
    PrefItem    prefItem;

    prefItem.type = pitBool;
    prefItem.uniqueId = uniqueId;
    prefItem.value.boolVal = value;
    
    return ErrSetItem( &prefItem );
}

status_t PrefsStoreWriter::ErrSetInt(int uniqueId, int value)
{
    PrefItem    prefItem;

    prefItem.type = pitInt;
    prefItem.uniqueId = uniqueId;
    prefItem.value.intVal = value;
 
    return ErrSetItem( &prefItem );
}

status_t PrefsStoreWriter::ErrSetLong(int uniqueId, long value)
{
    PrefItem    prefItem;

    prefItem.type = pitLong;
    prefItem.uniqueId = uniqueId;
    prefItem.value.longVal = value;
 
    return ErrSetItem( &prefItem );
}

status_t PrefsStoreWriter::ErrSetUInt16(int uniqueId, ushort_t value)
{
    PrefItem    prefItem;

    prefItem.type = pitUInt16;
    prefItem.uniqueId = uniqueId;
    prefItem.value.uint16Val = value;
 
    return ErrSetItem( &prefItem );
}

status_t PrefsStoreWriter::ErrSetUInt32(int uniqueId, ulong_t value)
{
    PrefItem    prefItem;

    prefItem.type = pitUInt32;
    prefItem.uniqueId = uniqueId;
    prefItem.value.uint32Val = value;

    return ErrSetItem( &prefItem );        
}

// value must point to a valid location during ErrSavePrefernces() since
// for perf reasons we don't make a copy of the string
status_t PrefsStoreWriter::ErrSetStr(int uniqueId, const char_t *value)
{
    PrefItem    prefItem;

    prefItem.type = pitStr;
    prefItem.uniqueId = uniqueId;
    prefItem.value.strVal = value;
 
    return ErrSetItem( &prefItem );
}

status_t PrefsStoreWriter::ErrSetItem(PrefItem *item)
{
    if(items_.find(item->uniqueId)!=items_.end())
        return psErrDuplicateId;
    
    items_.insert(std::pair<int const, PrefItem>(item->uniqueId,*item));
    return errNone;
}

status_t PrefsStoreWriter::ErrSavePreferences()
{
    if (_handle==INVALID_HANDLE_VALUE)
        return psErrNoPrefDatabase;

    DWORD written;
    for (std::map< int, PrefItem>::iterator it=items_.begin();
        it!=items_.end();it++)
    {
        PrefItem prefItem=(*it).second;
        int size=getPrefItemSize(prefItem);
        WriteFile(_handle, &prefItem.uniqueId, sizeof(prefItem.uniqueId),&written, NULL);
        WriteFile(_handle, &size, sizeof(size),&written, NULL);
        WriteFile(_handle, &prefItem.type, sizeof(int),&written, NULL);
        if(prefItem.type != pitStr)
            WriteFile(_handle, &prefItem.value, size,&written, NULL);
        else
            WriteFile(_handle, prefItem.value.strVal, size,&written, NULL);
    }
    return errNone;
}

} // namespace ArsLexis
