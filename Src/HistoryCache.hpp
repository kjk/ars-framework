#ifndef ARSLEXIS_HISTORY_CACHE_HPP__
#define ARSLEXIS_HISTORY_CACHE_HPP__

#include <DataStore.hpp>
#include <Utility.hpp>

class HistoryCache: private NonCopyable 
{
public:

    enum {
        maxCacheEntries = 10, 
        maxCacheEntryNameLength = 255
    };
    
private:
    
    typedef char_t CacheEntryNameBuffer_t[maxCacheEntryNameLength + 1];
    typedef char_t StreamNameBuffer_t[DataStore::maxStreamNameLength + 1];

    struct IndexEntry 
    {
        CacheEntryNameBuffer_t cacheEntryName;
        StreamNameBuffer_t streamName;
    };
    
    IndexEntry* indexEntries_;    
    ulong_t indexEntriesCount_;
    ulong_t indexCapacity_;
    
    bool dataStoreOwner_;
    
    status_t readIndex();
    
    status_t writeIndex();
    
public:
    
    DataStore* dataStore;

    HistoryCache();
    
    ~HistoryCache();

    status_t open(DataStore& ds);
    status_t open(const char_t* dataStoreName);
    
    ulong_t entriesCount() const {return indexEntriesCount_;}
    
    enum {entryNotFound = ulong_t(-1)};
    
    ulong_t entryIndex(const char_t* entry) const;
    
    const char_t* entryTitle(ulong_t index) const;
    
    status_t removeEntry(ulong_t index);
    
    status_t appendEntry(const char_t* title, ulong_t& index);
    
    status_t replaceEntries(ulong_t from, const char_t* newEntry);
    
    DataStoreReader* readerForEntry(ulong_t index);
    
    DataStoreWriter* writerForEntry(ulong_t index);
    
    void close();
    
};

#ifdef DEBUG
void test_HistoryCache();
#endif

#endif


