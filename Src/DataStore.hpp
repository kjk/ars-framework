#ifndef ARSLEXIS_DATA_STORE_HPP__
#define ARSLEXIS_DATA_STORE_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>
#include <Reader.hpp>
#include <Writer.hpp>
#include <File.hpp>
#include <set>

class DataStore: private NonCopyable {

public:
    
    explicit DataStore();

    enum {
        maxStreamNameLength=32,
        maxStreamsCount=32
    };
    
    ~DataStore();
    
    enum CreateOption
    {
        createNot,
        createAsNeeded
    };
    
    status_t open(const char_t* fileName);
    
    status_t open(const char_t* fileName, CreateOption c);
    
    status_t create(const char_t* fileName);
    
    status_t removeStream(const char* name);
    
    enum {
        errStoreCorrupted=dsErrorClass,
        errNotFound,
        errAlreadyExists,
        errTooManyStreams,
        errNameTooLong,
    };            
    
    static DataStore* instance();
    
    static status_t initialize(const char_t* fileName);
    
    static void dispose();
    
    const char_t* name() const {return fileName_;}
    
private:

    status_t readIndex();
    
    status_t createIndex();
    
    status_t readHeaders();
    
    char_t* fileName_;
    File file_;
    
    struct StreamHeader {
        NarrowString name;
        uint_t index;
        File::Position firstFragment;
        
        StreamHeader(const char* name, ulong_t nameLength, uint_t index, File::Position firstFragment);
        
    };
    
    struct StreamHeaderLess {
        bool operator()(const StreamHeader* h1, const StreamHeader* h2) const
        {return h1->name < h2->name;}
    };
    
    typedef std::set<StreamHeader*, StreamHeaderLess> StreamHeaders_t;
    StreamHeaders_t streamHeaders_;
    
    status_t readHeadersForOwner(const StreamHeader& streamHeader);
    
    struct FragmentHeader {
        File::Position start;
        uint_t ownerIndex;
        uint_t length;
        File::Position nextFragment;
        
        FragmentHeader(File::Position start, uint_t ownerIndex, uint_t length, File::Position nextFragment);
        
//            FragmentHeader();
        
    };
    
    struct FragmentHeaderLess {
        bool operator()(const FragmentHeader* h1, const FragmentHeader* h2) const
        {return h1->start < h2->start;}
    };
    
    typedef std::set<FragmentHeader*, FragmentHeaderLess> FragmentHeaders_t;
    FragmentHeaders_t fragmentHeaders_;
    
    status_t findStream(const char* name, StreamHeader*& header);
    
    status_t createStream(const char* name, StreamHeader*& header);
    
    enum { minFragmentLength = sizeof(FragmentHeader) + 128};
    
    status_t createFragment(uint_t ownerIndex, FragmentHeader*& header);
    
    status_t writeFragmentHeader(const FragmentHeader& header);
    
    status_t writeStreamHeader(const StreamHeader& header);
    
    ulong_t maxAllowedFragmentLength(FragmentHeader& header) const;
    
    File::Position nextAvailableFragmentStart() const;
    
    status_t truncateFragment(FragmentHeader& fragment, uint_t length);
    
    void removeFragments(File::Position start);
    
    struct StreamPosition {
        
        StreamHeader& stream;
        FragmentHeader* fragment;
        uint_t position;
        
        explicit StreamPosition(StreamHeader& stream);
        
    };
    
    typedef std::auto_ptr<StreamPosition> StreamPositionPtr;
    
    status_t readFragment(const FragmentHeader& fragment, uint_t& startOffset, void*& buffer, uint_t& length);
    
    status_t readStream(StreamPosition& position, void* buffer, ulong_t& length);
    
    status_t writeFragment(FragmentHeader& fragment, uint_t& startOffset, const void*& buffer, uint_t& length);
    
    status_t writeStream(StreamPosition& position, const void* buffer, ulong_t length);
    
    status_t findEof();
    
    friend class DataStoreReader;
    friend class DataStoreWriter;
};

class DataStoreReader: public Reader {
    DataStore& store_;
    DataStore::StreamPositionPtr position_;
    
public:
    
    DataStoreReader(DataStore& store);
    
    ~DataStoreReader();
    
    status_t open(const char* name);
    
    status_t readRaw(void* buffer, ulong_t& length);
    
};

class DataStoreWriter: public Writer {

    DataStore& store_;
    DataStore::StreamPositionPtr position_;
    
public:

    DataStoreWriter(DataStore& store);
    
    ~DataStoreWriter();
    
    status_t open(const char* name, bool dontCreate = false);
    
    status_t writeRaw(const void* buffer, ulong_t length);
    
    status_t flush();
    
};
    
#endif