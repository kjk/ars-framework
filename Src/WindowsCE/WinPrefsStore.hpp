#ifndef _PREFS_STORE_H_
#define _PREFS_STORE_H_

#include <ErrBase.h>
#include <BaseTypes.hpp>
#include <Utility.hpp>
#include <map>
#include <list>

namespace ArsLexis 
{
    
    
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
            bool            boolVal;
            int             intVal;
            long            longVal;
            ushort_t        uint16Val;
            ulong_t         uint32Val;
            const char_t *  strVal;
        } value;
    } PrefItem;
    
    class PrefsStoreReader : private NonCopyable
    {
    private:
        std::map< int, PrefItem> items_;
        std::list<char_t*>       strPointers_;

        HANDLE          _handle;
        const char_t *  _dbName;
        ulong_t         _dbCreator;
        ulong_t         _dbType;

        status_t  ErrLoadPreferences();
        status_t  ErrGetPrefItemWithId(int uniqueId, PrefItem *prefItem);
    public:
        PrefsStoreReader(const char_t *dbName, ulong_t dbCreator, ulong_t dbType);
        status_t ErrGetBool(int uniqueId, bool *value);
        status_t ErrGetInt(int uniqueId, int *value);
        status_t ErrGetLong(int uniqueId, long *value);
        status_t ErrGetUInt16(int uniqueId, ushort_t *value);
        status_t ErrGetUInt32(int uniqueId, ulong_t *value);
        status_t ErrGetStr(int uniqueId, const char_t ** value);
        ~PrefsStoreReader();
    };
    
#define MAX_PREFS_ITEMS   60
    
    class PrefsStoreWriter : private NonCopyable
    {
    private:
        std::map< int, PrefItem> items_;
        status_t ErrSetItem(PrefItem *item);
        const char_t * _dbName;
        ulong_t _dbCreator;
        ulong_t _dbType;
        int     _itemsCount;
        HANDLE  _handle;
    public:
        PrefsStoreWriter(const char_t *dbName, ulong_t dbCreator, ulong_t dbType);
        status_t ErrSetBool(int uniqueId, bool value);
        status_t ErrSetInt(int uniqueId, int value);
        status_t ErrSetLong(int uniqueId, long value);
        status_t ErrSetUInt16(int uniqueId, ushort_t value);
        status_t ErrSetUInt32(int uniqueId, ulong_t value);
        status_t ErrSetStr(int uniqueId, const char_t *value);
        status_t ErrSavePreferences();
        ~PrefsStoreWriter();
    };
} // namespace ArsLexis

#endif
