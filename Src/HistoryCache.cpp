#include "HistoryCache.hpp"
#include <Serializer.hpp>
#include <SysUtils.hpp>
#include <DataStore.hpp>
#include <PopupMenu.hpp>
#include <Text.hpp>

#ifdef __MWERKS__
using std::memcpy;
#endif

#define HISTORY_CACHE_INDEX_STREAM _T("_History Cache Index")

enum {
    serialIdIndexVersion,
    serialIdItemsCount
};

HistoryCache::HistoryCache():
    indexEntries_(NULL),
    indexEntriesCount_(0),
    indexCapacity_(0),
    dataStore(NULL),
    dataStoreOwner_(false)
{
}

HistoryCache::~HistoryCache()
{
    close();    
}

void HistoryCache::close()
{
    if (NULL != indexEntries_)
    {
        if (NULL != dataStore)
            writeIndex();
            
        delete [] indexEntries_;
        indexEntries_ = NULL;
        indexEntriesCount_ = 0;
        indexCapacity_ = 0;
    }
    if (dataStoreOwner_)
        delete dataStore;
    dataStore = NULL;
    dataStoreOwner_ = false;
}

status_t HistoryCache::open(DataStore& ds)
{
    close();
    dataStore = &ds;
    return readIndex();
}

status_t HistoryCache::open(const char_t* dsName)
{
    close();
    dataStore = new_nt DataStore();
    if (NULL == dataStore)
        return memErrNotEnoughSpace;
    dataStoreOwner_ = true;
    status_t err = dataStore->open(dsName, DataStore::createAsNeeded);
    if (errNone != err)
        return err;
    return readIndex();
}

HistoryCache::IndexEntry::IndexEntry()
{
    memzero(this, sizeof(*this));
}

status_t HistoryCache::readIndex()
{
    assert(NULL != dataStore);
    DataStoreReader reader(*dataStore);

    volatile status_t err = reader.open(HISTORY_CACHE_INDEX_STREAM);
    if (errNone != err)
    {
FreshIndex:    
        indexCapacity_ = 0;
        indexEntriesCount_ = 0;
        indexEntries_ = new_nt IndexEntry[maxCacheEntries];
        if (NULL == indexEntries_)
            return memErrNotEnoughSpace;
        indexCapacity_ = maxCacheEntries;
        return errNone;        
    }
    ulong_t cap = maxCacheEntries;
    ulong_t count;
    Serializer serialize(reader);
    ErrTry {
        serialize(count, serialIdIndexVersion);
        serialize(count, serialIdItemsCount);
        if (count > maxCacheEntries)
            cap = count;
        indexEntries_ = new_nt IndexEntry[cap];
        if (NULL == indexEntries_)
            return memErrNotEnoughSpace;
        indexCapacity_ = cap;
        for (ulong_t i = 0; i < count; ++i)
        {
            IndexEntry& entry = indexEntries_[i];
            serialize(entry.url, maxCacheEntryUrlLength + 1);
            serialize(entry.streamName, DataStore::maxStreamNameLength + 1);
            serialize(entry.title, maxCacheEntryTitleLength + 1);
        }
        indexEntriesCount_ = count;
    }
    ErrCatch(ex)
    {
        err = ex;
    } ErrEndCatch
    if (errNone != err)
        goto FreshIndex;
    return err;
}

status_t HistoryCache::writeIndex()
{
    assert(NULL != dataStore);
    DataStoreWriter writer(*dataStore);
    
    volatile status_t err = writer.open(HISTORY_CACHE_INDEX_STREAM);
    if (errNone != err)
        return err;
     
    ulong_t indexVersion = 1;
    Serializer serialize(writer);
    ErrTry {
        serialize(indexVersion, serialIdIndexVersion);      
        serialize(indexEntriesCount_, serialIdItemsCount);
        for (ulong_t i = 0; i < indexEntriesCount_; ++i)
        {
            IndexEntry& entry = indexEntries_[i];
            serialize(entry.url, maxCacheEntryUrlLength + 1);
            serialize(entry.streamName, DataStore::maxStreamNameLength + 1);
            serialize(entry.title, maxCacheEntryTitleLength + 1);
        }
    }
    ErrCatch(ex)
    {
        err = ex;
    } ErrEndCatch
    return err;
}

ulong_t HistoryCache::entryIndex(const char_t* entry) const
{
    for (long i = long(indexEntriesCount_) - 1; i >= 0; --i)
    {   
        if (0 == tstrcmp(entry, indexEntries_[i].url))
            return ulong_t(i);
    }
    return entryNotFound;
}

const char_t* HistoryCache::entryUrl(ulong_t index) const
{
    assert(index < indexEntriesCount_);
    return indexEntries_[index].url;
}

const char_t* HistoryCache::entryTitle(ulong_t index) const
{
    assert(index < indexEntriesCount_);
    return indexEntries_[index].title;
}

void HistoryCache::setEntryTitle(ulong_t index, const char_t* str)
{
    assert(index < indexEntriesCount_);
    size_t len = tstrlen(str);
    if (len > maxCacheEntryTitleLength)
        len = maxCacheEntryTitleLength;
    
    IndexEntry& entry = indexEntries_[index];
    using namespace std;
    memcpy(entry.title, str, len * sizeof(char_t));
    entry.title[len] = _T('\0');
}

status_t HistoryCache::removeEntry(ulong_t index)
{
    assert(index < indexEntriesCount_);
    assert(NULL != dataStore);
    const char_t* streamName = indexEntries_[index].streamName;
    status_t err = dataStore->removeStream(streamName);
    if (errNone != err)
        return err;
        
    for (ulong_t i = index + 1; i <indexEntriesCount_; ++i)
        indexEntries_[i - 1] = indexEntries_[i];
    --indexEntriesCount_;
    return errNone;
}

status_t HistoryCache::removeEntriesAfter(ulong_t from)
{
    status_t err;
    while (indexEntriesCount_ > from)
    {
        if (errNone != (err = removeEntry(indexEntriesCount_ - 1)))
            return err;
    }
    return errNone;
}

status_t HistoryCache::appendEntry(const char_t* title, ulong_t& index)
{
    status_t err;
    if (indexEntriesCount_ == indexCapacity_)
        if (errNone != (err = removeEntry(0)))
            return err;
    
    ulong_t len = tstrlen(title);
    if (len > maxCacheEntryUrlLength)
        len = maxCacheEntryUrlLength;
    
    IndexEntry& entry = indexEntries_[indexEntriesCount_];
    memcpy(entry.url, title, len * sizeof(char_t));
    entry.url[len] = _T('\0');
    
    tprintf(entry.streamName, _T("_History %lx%lx"), ticks(), random(ULONG_MAX));
    
    DataStoreWriter writer(*dataStore);
    
    if (errNone != (err = writer.open(entry.streamName)))
        return err;
        
    index = indexEntriesCount_++;
    return errNone;
}

status_t HistoryCache::replaceEntries(ulong_t from, const char_t* newEntry)
{
    status_t err = removeEntriesAfter(from);
    if (errNone != err)
        return err;
    return appendEntry(newEntry, from);
}

DataStoreReader* HistoryCache::readerForEntry(ulong_t index)
{
    assert(index < indexEntriesCount_);
    DataStoreReader* reader = new_nt DataStoreReader(*dataStore);
    if (NULL == reader)
        return NULL;
    status_t err = reader->open(indexEntries_[index].streamName);
    if (errNone != err)
    {
        delete reader;
        return NULL;
    }
    return reader;
}

DataStoreWriter* HistoryCache::writerForEntry(ulong_t index)
{
    assert(index < indexEntriesCount_);
    DataStoreWriter* writer = new_nt DataStoreWriter(*dataStore);
    if (NULL == writer)
        return NULL;
    status_t err = writer->open(indexEntries_[index].streamName);
    if (errNone != err)
    {
        delete writer;
        return NULL;
    }
    return writer;
}

#ifdef DEBUG

static void test_HistoryCacheWrite()
{
    HistoryCache cache;
    status_t err = cache.open(_T("UnitTest HistoryCache"));
    assert(errNone == err);
    
    assert(0 == cache.entriesCount());
    char_t buffer[33];
    ulong_t index;
    for (int i = 0; i < 11; ++i)
    {
        tprintf(buffer, _T("test %d"), i);
        cache.appendEntry(buffer, index);
    }
    assert(HistoryCache::maxCacheEntries == cache.entriesCount());
    assert(0 == cache.entryIndex("test 1"));
    
    cache.replaceEntries(0, "test 11");
    assert(0 == cache.entryIndex("test 11"));
}

static void test_HistoryCacheRead()
{
    HistoryCache cache;
    status_t err = cache.open(_T("UnitTest HistoryCache"));
    assert(errNone == err);
    
    assert(1 == cache.entriesCount());
    assert(0 == cache.entryIndex("test 11"));
    cache.removeEntry(0);
    
    assert(0 == cache.entriesCount());
}

void test_HistoryCache()
{
    test_HistoryCacheWrite();
    test_HistoryCacheRead();
    test_HistoryCacheWrite();
    test_HistoryCacheRead();
}   

#endif
