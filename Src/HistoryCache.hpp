#ifndef ARSLEXIS_HISTORY_CACHE_HPP__
#define ARSLEXIS_HISTORY_CACHE_HPP__

#include <DataStore.hpp>
#include <Utility.hpp>

class PopupMenuModel;

class HistoryCache: private NonCopyable 
{
public:

    enum {
        maxCacheEntries = 10, 
        maxCacheEntryUrlLength = 255,
        maxCacheEntryTitleLength = 63
    };
    
private:
    
    typedef char_t CacheEntryUrlBuffer_t[maxCacheEntryUrlLength + 1];
    typedef char_t CacheEntryTitleBuffer_t[maxCacheEntryTitleLength + 1];
    typedef char_t StreamNameBuffer_t[DataStore::maxStreamNameLength + 1];

    struct IndexEntry 
    {
        CacheEntryUrlBuffer_t url;
        StreamNameBuffer_t streamName;
        CacheEntryTitleBuffer_t title;
        
        IndexEntry();
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
    
    const char_t* entryUrl(ulong_t index) const;
    
    const char_t* entryTitle(ulong_t index) const;
    
    void setEntryTitle(ulong_t index, const char_t* title);
    
    status_t removeEntry(ulong_t index);
    
    status_t removeEntriesAfter(ulong_t startIndex);
    
    status_t appendEntry(const char_t* url, ulong_t& index);
    
    status_t replaceEntries(ulong_t from, const char_t* newUrl);
    
    DataStoreReader* readerForEntry(ulong_t index);
    
    DataStoreWriter* writerForEntry(ulong_t index);
    
    void close();
   
};

#ifdef DEBUG
void test_HistoryCache();
#endif

#endif


