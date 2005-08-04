#include <HistoryCache.hpp>
#include <Serializer.hpp>
#include <SysUtils.hpp>
#include <DataStore.hpp>
#include <Text.hpp>
#include <Logging.hpp>

#ifdef __MWERKS__
using std::memcpy;
#endif

//#define DEBUG_URLS

#define HISTORY_CACHE_INDEX_STREAM "_History Cache Index"

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

    status_t err = reader.open(HISTORY_CACHE_INDEX_STREAM);
    if (errNone != err)
    {
FreshIndex:    
        indexCapacity_ = 0;
        indexEntriesCount_ = 0;
        delete [] indexEntries_;
        indexEntries_ = new_nt IndexEntry[maxCacheEntries];
        if (NULL == indexEntries_)
            return memErrNotEnoughSpace;
        indexCapacity_ = maxCacheEntries;
        return errNone;        
    }
    Serializer serialize(reader);
    err = serializeIndexIn(serialize);
    if (errNone != err)
    {
        LogStrUlong(eLogError, _T("HistoryCache::readIndex(): serializeIndexIn() returned error: "), err);
        goto FreshIndex;
    }
    return err;
}

status_t HistoryCache::serializeIndexIn(Serializer& serialize)
{
    ErrTry {
        ulong_t cap = maxCacheEntries;
        ulong_t count;
        serialize(count, serialIdIndexVersion);
        serialize(count, serialIdItemsCount);
        if (count > maxCacheEntries)
            cap = count;
        indexEntries_ = new_nt IndexEntry[cap];
        if (NULL == indexEntries_)
            ErrReturn(memErrNotEnoughSpace);
        indexCapacity_ = cap;
        for (ulong_t i = 0; i < count; ++i)
        {
            IndexEntry& entry = indexEntries_[i];
            serialize.narrowBuffer(entry.url, maxCacheEntryUrlLength + 1);
#ifdef DEBUG_URLS
            assert('s' == *entry.url);
#endif         
            serialize.narrowBuffer(entry.streamName, DataStore::maxStreamNameLength + 1);
            serialize.textBuffer(entry.title, maxCacheEntryTitleLength + 1);
            serialize(entry.onlyLink);
        }
        indexEntriesCount_ = count;
    }
    ErrCatch(ex) {
        return ex;
    } ErrEndCatch
    return errNone;
}

status_t HistoryCache::writeIndex()
{
    assert(NULL != dataStore);
    DataStoreWriter writer(*dataStore);
    
    status_t err = writer.open(HISTORY_CACHE_INDEX_STREAM);
    if (errNone != err)
        return err;
     
    Serializer serialize(writer);
    return serializeIndexOut(serialize);
}

status_t HistoryCache::serializeIndexOut(Serializer& serialize)
{
    ErrTry {
        ulong_t indexVersion = 1;
        serialize(indexVersion, serialIdIndexVersion);      
        serialize(indexEntriesCount_, serialIdItemsCount);
        for (ulong_t i = 0; i < indexEntriesCount_; ++i)
        {
            IndexEntry& entry = indexEntries_[i];
#ifdef DEBUG_URLS
            assert('s' == *entry.url);
#endif         
            serialize.narrowOut(entry.url);
#ifdef DEBUG_URLS
            assert('s' == *entry.url);
#endif         
            serialize.narrowOut(entry.streamName);
            serialize.textOut(entry.title);
            serialize(entry.onlyLink);
        }
    }
    ErrCatch(ex) {
		return ex;
    } ErrEndCatch
    return errNone;    
}

ulong_t HistoryCache::entryIndex(const char* entry) const
{
    for (long i = long(indexEntriesCount_) - 1; i >= 0; --i)
    {   
        if (StrEquals(entry, indexEntries_[i].url))
            return ulong_t(i);
    }
    return entryNotFound;
}

const char* HistoryCache::entryUrl(ulong_t index) const
{
    assert(index < indexEntriesCount_);
    return indexEntries_[index].url;
}

const char_t* HistoryCache::entryTitle(ulong_t index) const
{
    assert(index < indexEntriesCount_);
    return indexEntries_[index].title;
}

bool HistoryCache::entryIsOnlyLink(ulong_t index) const
{
    assert(index < indexEntriesCount_);
    return indexEntries_[index].onlyLink;
}

void HistoryCache::setEntryTitle(ulong_t index, const char_t* str)
{
    assert(index < indexEntriesCount_);
    size_t len = Len(str);
    if (len > maxCacheEntryTitleLength)
        len = maxCacheEntryTitleLength;
    
    IndexEntry& entry = indexEntries_[index];
    memmove(entry.title, str, len * sizeof(char_t));
    entry.title[len] = _T('\0');
#ifdef DEBUG_URLS
    assert('s' == *entry.url);
#endif         
}

status_t HistoryCache::removeEntry(ulong_t index)
{
    assert(index < indexEntriesCount_);
    assert(NULL != dataStore);
    const char* streamName = indexEntries_[index].streamName;
    status_t err = dataStore->removeStream(streamName);
    if (errNone != err)
        return err;
    
    memmove(indexEntries_ + index, indexEntries_ + index + 1, (indexEntriesCount_ - (index + 1)) * sizeof(IndexEntry));
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

status_t HistoryCache::appendEntry(const char* url, ulong_t& index)
{
    assert(0 != Len(url));

    status_t err;
    if (indexEntriesCount_ == indexCapacity_)
        if (errNone != (err = removeEntry(0UL)))
            return err;
    
    ulong_t len = Len(url);
    if (len > maxCacheEntryUrlLength)
        len = maxCacheEntryUrlLength;
    
    IndexEntry& entry = indexEntries_[indexEntriesCount_];
    memmove(entry.url, url, len);
    entry.url[len] = '\0';
	StrPrintF(entry.streamName, "_History %lx%lx", ticks(), random(ULONG_MAX));

#ifdef DEBUG_URLS
    assert('s' == *entry.url);
#endif         
    
    DataStoreWriter writer(*dataStore);
    
    if (errNone != (err = writer.open(entry.streamName)))
        return err;
        
    index = indexEntriesCount_++;
    return errNone;
}

status_t HistoryCache::appendLink(const char* url, const char_t* title)
{
    assert(0 != Len(url));

    ulong_t index;
    status_t err = appendEntry(url, index);
    if (errNone != err)
        return err;
    
    setEntryTitle(index, title);
    indexEntries_[index].onlyLink = true;

#ifdef DEBUG_URLS
    assert('s' == *indexEntries_[index].url);
#endif         

    return errNone;
}

status_t HistoryCache::insertLink(ulong_t index, const char* url, const char_t* title)
{
    assert(0 != Len(url));
    
    status_t err = appendLink(url, title);
    if (errNone != err)
        return err;
     
    IndexEntry* tmp = new_nt IndexEntry();
    if (NULL == tmp)
        return memErrNotEnoughSpace;
    
    ulong_t count = indexEntriesCount_ - (index + 1);
    memmove(tmp, indexEntries_ + (indexEntriesCount_ - 1), sizeof(IndexEntry));
    memmove(indexEntries_ + (index + 1), indexEntries_ + index, count * sizeof(IndexEntry));
    memmove(indexEntries_ + index, tmp, sizeof(IndexEntry));
    
    delete tmp;

#ifdef DEBUG_URLS
    assert('s' == *indexEntries_[index].url);
#endif         

    return errNone;    
}


status_t HistoryCache::replaceEntries(ulong_t from, const char* newEntryUrl)
{
    status_t err = removeEntriesAfter(from);
    if (errNone != err)
        return err;
    return appendEntry(newEntryUrl, from);
}

DataStoreReader* HistoryCache::readerForEntry(ulong_t index)
{
    assert(index < indexEntriesCount_);
    assert(!indexEntries_[index].onlyLink);
    
    DataStoreReader* reader = new_nt DataStoreReader(*dataStore);
    if (NULL == reader)
        return NULL;
    status_t err = reader->open(indexEntries_[index].streamName);
    if (errNone != err)
    {
        delete reader;
        return NULL;
    }

#ifdef DEBUG_URLS
    assert('s' == *indexEntries_[index].url);
#endif         

    return reader;
}

DataStoreWriter* HistoryCache::writerForEntry(ulong_t index)
{
    assert(index < indexEntriesCount_);
    assert(!indexEntries_[index].onlyLink);

    DataStoreWriter* writer = new_nt DataStoreWriter(*dataStore);
    if (NULL == writer)
        return NULL;
    status_t err = writer->open(indexEntries_[index].streamName);
    if (errNone != err)
    {
        delete writer;
        return NULL;
    }

#ifdef DEBUG_URLS
    assert('s' == *indexEntries_[index].url);
#endif         

    return writer;
}

status_t HistoryCache::moveEntryToEnd(ulong_t& index)
{   
    assert(index < indexEntriesCount_);
    if (index == indexEntriesCount_ - 1)
        return errNone;
        
    // IndexEntry is quite big, so better not to use stack to allocate it.
    IndexEntry* tmp = new_nt IndexEntry;
    if (NULL == tmp)
        return memErrNotEnoughSpace;
    
    memmove(tmp, indexEntries_ + index, sizeof(*tmp));
    memmove(indexEntries_ + index, indexEntries_ + (index + 1), (indexEntriesCount_ - (index + 1)) * sizeof(*tmp));
    memmove(indexEntries_ + (indexEntriesCount_ - 1), tmp, sizeof(*tmp));
    
    delete tmp;
    
    index = (indexEntriesCount_ - 1);

#ifdef DEBUG_URLS
    assert('s' == *indexEntries_[index].url);
#endif         

    return errNone;
}

status_t HistoryCache::removeEntry(const char* url)
{
    ulong_t index = entryIndex(url);
    if (entryNotFound == index)
        return errNone;
    return removeEntry(index);
}


#ifndef NDEBUG

#ifdef _WIN32
static const char_t* unitTestCacheName = _T("UnitTest HistoryCache.dat");
#endif
#ifdef _PALM_OS
static const char_t* unitTestCacheName = _T("UnitTest HistoryCache");
#endif
static void test_HistoryCacheWrite()
{
    HistoryCache cache;
    status_t err = cache.open(unitTestCacheName);
    assert(errNone == err);
    
    assert(0 == cache.entriesCount());
    char buffer[33];
    ulong_t index;
    for (int i = 0; i < 11; ++i)
    {
        StrPrintF(buffer, "test %d", i);
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
    status_t err = cache.open(unitTestCacheName);
    assert(errNone == err);
    
    assert(1 == cache.entriesCount());
    assert(0 == cache.entryIndex("test 11"));
    cache.removeEntry(0UL);
    
    assert(0 == cache.entriesCount());
}

void test_HistoryCache()
{
#ifndef DEBUG_URLS
    test_HistoryCacheWrite();
    test_HistoryCacheRead();
    test_HistoryCacheWrite();
    test_HistoryCacheRead();
#endif
}   

#endif
