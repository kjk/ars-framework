#include <DataStore.hpp>
#include <File.hpp>

using namespace ArsLexis;

namespace {

    struct StreamIndexEntry {
        bool used;
        char_t name[DataStore::maxStreamNameLength];
        File::Position firstFragment;
        
        StreamIndexEntry();
        
    };
    
    static const File::Position invalidFragmentStart=0;
    
    StreamIndexEntry::StreamIndexEntry():
        used(false),
        firstFragment(invalidFragmentStart)
    {
        MemSet(name, sizeof(name), 0);
    }
    
    struct FragmentHeaderEntry {
        uint_t ownerIndex;
        uint_t length;
        File::Position nextFragment;
        
        FragmentHeaderEntry();
    };
    
    FragmentHeaderEntry::FragmentHeaderEntry():
        ownerIndex(0),
        length(0),
        nextFragment(invalidFragmentStart)
    {}
    
}

DataStore::~DataStore()
{
    std::for_each(streamHeaders_.begin(), streamHeaders_.end(), ObjectDeleter<StreamHeader>());
    std::for_each(fragmentHeaders_.begin(), fragmentHeaders_.end(), ObjectDeleter<FragmentHeader>());
}

status_t DataStore::readIndex()
{
    assert(streamHeaders_.empty());
    status_t error = file_.seek(0, File::seekFromBeginning);
    if (errNone != error)
        return error;
    File::Size size;
    if (errNone != (error = file_.size(size)))
        return error;
    if (size < sizeof(StreamIndexEntry) * maxStreamsCount)
        return errStoreCorrupted;
    for (uint_t i=0; i<maxStreamsCount; ++i) 
    {
        StreamIndexEntry entry;
        if (errNone != (error = file_.read(&entry, sizeof(entry), size)))
            return error;
        if (sizeof(entry) != size)
            return errStoreCorrupted;
        if (entry.used)
        {
            uint_t nameLength = std::find(entry.name, entry.name + maxStreamNameLength, _T('\0')) - entry.name;
            String name(entry.name, nameLength);
            streamHeaders_.insert(new StreamHeader(name, i, entry.firstFragment));
        }
    }
    return errNone;
}

status_t DataStore::readHeaders()
{
    StreamHeaders_t::const_iterator end = streamHeaders_.end();
    for (StreamHeaders_t::const_iterator it = streamHeaders_.begin(); it != end; ++it)
    {
        status_t error = readHeadersForOwner(it);
        if (errNone != error)
            return error;
    }
    return errNone;
}

status_t DataStore::readHeadersForOwner(const DataStore::StreamHeaders_t::const_iterator& streamHeader)
{
    File::Position nextFragment = (*streamHeader)->firstFragment;
    while (invalidFragmentStart != nextFragment) 
    {
        FragmentHeaderEntry entry;
        status_t error=file_.seek(nextFragment, File::seekFromBeginning);
        if (errNone != error)
            return error;
        File::Size size;
        if (errNone != (error = file_.read(&entry, sizeof(entry), size)))
            return error;
        if (sizeof(entry) != size)
            return errStoreCorrupted;
        fragmentHeaders_.insert(new FragmentHeader(nextFragment, (*streamHeader)->index, entry.length, entry.nextFragment));
        nextFragment=entry.nextFragment;
    }
    return errNone;
}
