#include <DataStore.hpp>
#include <File.hpp>
#include <bitset>

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
        status_t error = readHeadersForOwner(*(*it));
        if (errNone != error)
            return error;
    }
    return errNone;
}

status_t DataStore::readHeadersForOwner(const DataStore::StreamHeader& streamHeader)
{
    File::Position nextFragment = streamHeader.firstFragment;
    while (invalidFragmentStart != nextFragment) 
    {
        FragmentHeaderEntry entry;
        status_t error = file_.seek(nextFragment, File::seekFromBeginning);
        if (errNone != error)
            return error;
        File::Size size;
        if (errNone != (error = file_.read(&entry, sizeof(entry), size)))
            return error;
        if (sizeof(entry) != size)
            return errStoreCorrupted;
        fragmentHeaders_.insert(new FragmentHeader(nextFragment, streamHeader.index, entry.length, entry.nextFragment));
        nextFragment=entry.nextFragment;
    }
    return errNone;
}

status_t DataStore::removeStream(const String& name)
{
    StreamHeaders_t::iterator it = streamHeaders_.find(&StreamHeader(name, 0, 0));
    if (streamHeaders_.end() == it)
        return errNotFound;
    File::Position pos = (*it)->index*sizeof(StreamIndexEntry);
    status_t error = file_.seek(pos, File::seekFromBeginning);
    if (errNone != error)
        return error;
    error=file_.write(&StreamIndexEntry(), sizeof(StreamIndexEntry));
    if (errNone != error)
        return error;
    removeFragments((*it)->firstFragment);
    delete *it;
    streamHeaders_.erase(it);
    return errNone;
}

status_t DataStore::createStream(const String& name, StreamHeader*& header)
{
    if (maxStreamNameLength < name.length())
        return errNameTooLong;
    if (maxStreamsCount == streamHeaders_.size())
        return errTooManyStreams;
    StreamHeaders_t::const_iterator end = streamHeaders_.end();
    if (end != streamHeaders_.find(&StreamHeader(name, 0, 0)))
        return errAlreadyExists;
    typedef std::bitset<maxStreamsCount> StreamsUsage_t;
    StreamsUsage_t usage;
    for (StreamHeaders_t::const_iterator it = streamHeaders_.begin(); it != end; ++it)
        usage.set((*it)->index);
    uint_t index;
    for (index = 0; index < maxStreamsCount; ++index)
        if (!usage.test(index))
            break;
    assert(index < maxStreamsCount);
    status_t error = file_.seek(index*sizeof(StreamIndexEntry), File::seekFromBeginning);
    StreamIndexEntry indexEntry;
    indexEntry.used = true;
    MemMove(indexEntry.name, name.data(), name.length()*sizeof(char_t));
    error = file_.write(&indexEntry, sizeof(indexEntry));
    if (errNone != error)
        return error;
    header = *streamHeaders_.insert(new StreamHeader(name, index, invalidFragmentStart)).first;
    return errNone;
}

status_t DataStore::writeFragmentHeader(const DataStore::FragmentHeader& header)
{
    status_t error = file_.seek(header.start, File::seekFromBeginning);
    if (errNone != error)
        return error;
    error = file_.write(&header, sizeof(FragmentHeader));    return error;
}

File::Position DataStore::nextAvailableFragmentStart() const
{
    File::Position start = maxStreamsCount*sizeof(StreamIndexEntry);
    FragmentHeaders_t::const_iterator end=fragmentHeaders_.end();
    for (FragmentHeaders_t::const_iterator it = fragmentHeaders_.begin(); it != end; ++it)
    {
        if ((*it)->start - start >= minFragmentLength)
            return start;
        start = (*it)->start + (*it)->length;
    }
    return start;
}

uint_t DataStore::maxAllowedFragmentLength(FragmentHeader& header) const
{
    FragmentHeaders_t::const_iterator it = fragmentHeaders_.find(&header);
    assert(fragmentHeaders_.end() != it);
    ++it;
    if (fragmentHeaders_.end() == it)
        return uint_t(-1);
    else
        return (*it)->start - header.start;
}

status_t DataStore::createFragment(uint_t ownerIndex, FragmentHeader*& header)
{
    File::Position start = nextAvailableFragmentStart();
    std::auto_ptr<FragmentHeader> fh(new FragmentHeader(start, ownerIndex, 0, invalidFragmentStart));
    status_t error = writeFragmentHeader(*fh);
    if (errNone != error)
        return error;
    header=*fragmentHeaders_.insert(fh.release()).first;
    return errNone;
}

status_t DataStore::truncateFragment(FragmentHeader& fragment, uint_t length)
{
    removeFragments(fragment.nextFragment);
    fragment.length = length;
    return writeFragmentHeader(fragment);
}

void DataStore::removeFragments(File::Position start)
{
    while (invalidFragmentStart != start) 
    {
        FragmentHeaders_t::iterator it = fragmentHeaders_.find(&FragmentHeader(start, 0, 0, 0));
        assert(fragmentHeaders_.end() != it);
        start = (*it)->nextFragment;
        delete *it;
        fragmentHeaders_.erase(it);
    }
}

DataStore::StreamPosition::StreamPosition(DataStore::StreamHeader& s):
    stream(s),
    fragment(NULL),
    position(0)
{}

status_t DataStore::readFragment(const FragmentHeader& fragment, uint_t& startOffset, void*& buffer, uint_t& length)
{
    status_t error = file_.seek(fragment.start, File::seekFromBeginning);
    if (errNone != error)
        return error;
        
    File::Size toRead = std::min(length, fragment.length-startOffset);
    File::Size read = toRead;
    error=file_.read(buffer, toRead, read);
    if (errNone != error)
        return error;
    
    if (read != toRead)
        return errStoreCorrupted;
        
    length = read;
    buffer = (static_cast<char*>(buffer)+read);
    startOffset+=read;
    return errNone;
}

status_t DataStore::readStream(StreamPosition& position, void* buffer, uint_t& length)
{
    if (0 == length)
        return errNone;

    if (NULL == position.fragment)
    {
        FragmentHeaders_t::iterator it = fragmentHeaders_.find(&FragmentHeader(position.stream.firstFragment, 0, 0, 0));
        assert(fragmentHeaders_.end() != it);
        position.fragment = *it;
    }
    uint_t read = 0;
    uint_t left = length;
    while (left > 0)
    {
        uint_t len = left;
        status_t error=readFragment(*position.fragment, position.position, buffer, len);
        if (errNone != error)
            return error;
        if (0 == len)
            break;
        read+=len;
        left-=len;
        if (position.fragment->length == position.position) 
        {
            if (invalidFragmentStart == position.fragment->nextFragment)
                break;
            position.position = 0;
            FragmentHeaders_t::iterator it = fragmentHeaders_.find(&FragmentHeader(position.fragment->nextFragment, 0, 0, 0));
            assert(fragmentHeaders_.end() != it);
            position.fragment = *it; 
        }
    }
    length=read;
    return errNone;
}
