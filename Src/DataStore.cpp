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

DataStore::StreamHeader::StreamHeader(const String& n, uint_t i, File::Position f):
    name(n),
    index(i),
    firstFragment(f)
{}  

DataStore::FragmentHeader::FragmentHeader(File::Position s, uint_t o, uint_t l, File::Position n):
    start(s),
    ownerIndex(o),
    length(l),
    nextFragment(n)
{}    
  
DataStore::DataStore(File& f): file_(f) {}

DataStore::~DataStore()
{
    std::for_each(streamHeaders_.begin(), streamHeaders_.end(), ObjectDeleter<StreamHeader>());
    std::for_each(fragmentHeaders_.begin(), fragmentHeaders_.end(), ObjectDeleter<FragmentHeader>());
}

status_t DataStore::createIndex()
{
    std::for_each(streamHeaders_.begin(), streamHeaders_.end(), ObjectDeleter<StreamHeader>());
    streamHeaders_.clear();
    std::for_each(fragmentHeaders_.begin(), fragmentHeaders_.end(), ObjectDeleter<FragmentHeader>());
    fragmentHeaders_.clear();
    status_t error = file_.seek(0, File::seekFromBeginning);
    if (errNone != error)
        return error;
    StreamIndexEntry entry;
    for (uint_t i=0; i<maxStreamsCount; ++i) 
    {
        error = file_.write(&entry, sizeof(entry));
        if (errNone != error)
            return error;
    }
    error = findEof();
    return error;
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

status_t DataStore::open()
{
    status_t error = readIndex();
    if (errNone != error)
        return error;
    error = readHeaders();
    return error;
}

status_t DataStore::create()
{
    status_t error = createIndex();
    return error;
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
    {
        status_t error = removeStream(name);
        if (errNone != error)
            return error;
    }
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
    if (errNone != error)
        return error;
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
    FragmentHeaderEntry entry;
    entry.ownerIndex = header.ownerIndex;
    entry.length = header.length;
    entry.nextFragment = header.nextFragment;
    error = file_.write(&entry, sizeof(entry));
    return error;
}

status_t DataStore::writeStreamHeader(const DataStore::StreamHeader& header)
{
    status_t error = file_.seek(header.index*sizeof(StreamIndexEntry), File::seekFromBeginning);
    if (errNone != error)
        return error;
    StreamIndexEntry indexEntry;
    indexEntry.used = true;
    MemMove(indexEntry.name, header.name.data(), header.name.length()*sizeof(char_t));
    indexEntry.firstFragment=header.firstFragment;
    error = file_.write(&indexEntry, sizeof(indexEntry));
    if (errNone != error)
        return error;
    return errNone;
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
        return (*it)->start - header.start - sizeof(FragmentHeaderEntry);
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
    status_t error = file_.seek(fragment.start+sizeof(FragmentHeaderEntry)+startOffset, File::seekFromBeginning);
    if (errNone != error)
        return error;
        
    File::Size toRead = std::min<File::Size>(length, fragment.length-sizeof(FragmentHeaderEntry)-startOffset);
    File::Size read;
    error = file_.read(buffer, toRead, read);
    if (errNone != error)
        return error;
    
    if (read != toRead)
        return errStoreCorrupted;
        
    length = read;
    buffer = (static_cast<char*>(buffer)+length);
    startOffset+=length;
    return errNone;
}

status_t DataStore::writeFragment(FragmentHeader& fragment, uint_t& startOffset, const void*& buffer, uint_t& length)
{
    status_t error = file_.seek(fragment.start+sizeof(FragmentHeaderEntry)+startOffset, File::seekFromBeginning);
    if (errNone != error)
        return error;
    
    uint_t spaceLeft = maxAllowedFragmentLength(fragment)-startOffset;
    File::Size toWrite=std::min(length, spaceLeft);
    error = file_.write(buffer, toWrite);
    if (errNone != error)
        return error;

    fragment.length = startOffset+sizeof(FragmentHeaderEntry)+toWrite;
    if (invalidFragmentStart != fragment.nextFragment)
    {
        removeFragments(fragment.nextFragment);
        fragment.nextFragment = invalidFragmentStart;
    }
        
    error=writeFragmentHeader(fragment);
    if (errNone != error)
        return error;
    
    length = toWrite;
    buffer = (static_cast<const char*>(buffer)+length);
    startOffset+=length;
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
        if (position.fragment->length == position.position+sizeof(FragmentHeaderEntry)) 
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

status_t DataStore::writeStream(StreamPosition& position, const void* buffer, uint_t length)
{
    if (0 == length)
        return errNone;
        
    if (NULL == position.fragment)
    {
        if (invalidFragmentStart != position.stream.firstFragment)
            removeFragments(position.stream.firstFragment);
        status_t error=createFragment(position.stream.index, position.fragment);
        if (errNone != error)
            return error;            
        assert(NULL != position.fragment);
        position.stream.firstFragment=position.fragment->start;        
        error = writeStreamHeader(position.stream);
        if (errNone != error)
            return error;
    }
    while (length > 0)
    {
        uint_t len = length;
        status_t error = writeFragment(*position.fragment, position.position, buffer, len);
        if (errNone != error)
            return error;
        length -= len;
        if (length > 0)
        {
            FragmentHeader* prevFragment = position.fragment;
            error = createFragment(position.stream.index, position.fragment);
            if (errNone != error)
                return error;
            prevFragment->nextFragment = position.fragment->start;
            error = writeFragmentHeader(*prevFragment);
            if (errNone != error)
                return error;
            position.position = 0;
        }
    }
    return errNone;        
}

status_t DataStore::findStream(const String& name, StreamHeader*& header)
{
    StreamHeaders_t::iterator it = streamHeaders_.find(&StreamHeader(name, 0, 0));
    if (streamHeaders_.end() == it)
        return errNotFound;
    header = *it;
    return errNone;
}

status_t DataStore::findEof()
{
    File::Position pos = sizeof(StreamIndexEntry) * maxStreamsCount;
    if (!fragmentHeaders_.empty())
    {
        FragmentHeaders_t::const_iterator it = fragmentHeaders_.end();
        const FragmentHeader* last = *--it;
        pos = last->start + last->length;
    }
    status_t error = file_.seek(pos, File::seekFromBeginning);
    if (errNone != error)
        return error;
    error = file_.truncate();
    if (errNone != error)
        return error;
    return errNone;
}

DataStoreReader::DataStoreReader(DataStore& store): store_(store) {}    

DataStoreReader::~DataStoreReader() {}

status_t DataStoreReader::open(const String& name)
{
    DataStore::StreamHeader* header = NULL;
    status_t error = store_.findStream(name, header);
    if (errNone != error)
        return error;
    assert(NULL != header);
    position_.reset(new DataStore::StreamPosition(*header));
    return errNone;
}

status_t DataStoreReader::readRaw(void* buffer, uint_t& length)
{
    assert(NULL != position_.get());
    return store_.readStream(*position_, buffer, length);
}

status_t DataStoreReader::read(int& chr)
{
    char_t c;
    uint_t length=sizeof(c);
    status_t error = readRaw(&c, length);
    if (errNone != error)
        return error;
    if (0 == length)
        chr = npos;
    else
        chr = c;
    return errNone;
}

status_t DataStoreReader::read(char_t* buffer, uint_t& length)
{
    uint_t toRead = sizeof(char_t) * length;
    status_t error = readRaw(buffer, toRead);
    if (errNone != error)
        return error;
    if (toRead % sizeof(char_t) != 0)
        return DataStore::errStoreCorrupted;
    length = toRead / sizeof(char_t);
    return errNone;    
}

DataStoreWriter::DataStoreWriter(DataStore& store): store_(store) {}

DataStoreWriter::~DataStoreWriter() 
{
    store_.findEof();
}

status_t DataStoreWriter::open(const String& name, bool dontCreate)
{
    DataStore::StreamHeader* header = NULL;
    status_t error = store_.findStream(name, header);
    if (DataStore::errNotFound == error && !dontCreate)
        error = store_.createStream(name, header);
    if (errNone != error)
        return error;
    assert(NULL != header);
    position_.reset(new DataStore::StreamPosition(*header));
    return errNone;
}

status_t DataStoreWriter::writeRaw(const void* buffer, uint_t length)
{
    assert(NULL != position_.get());
    return store_.writeStream(*position_, buffer, length);
}

status_t DataStoreWriter::write(char_t chr)
{
    return writeRaw(&chr, sizeof(chr));
}

status_t DataStoreWriter::write(const char_t* buffer, uint_t length)
{
    return writeRaw(buffer, sizeof(char_t)*length);
}

status_t DataStoreWriter::flush()
{
    return errNone;
}
