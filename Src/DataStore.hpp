#ifndef ARSLEXIS_DATA_STORE_HPP__
#define ARSLEXIS_DATA_STORE_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>
#include <Reader.hpp>
#include <Writer.hpp>
#include <File.hpp>
#include <set>

namespace ArsLexis {

    class DataStore: private NonCopyable {
    
        explicit DataStore(const char_t* fileName);
    
    public:
        
        enum {
            maxStreamNameLength=32,
            maxStreamsCount=32
        };
        
        ~DataStore();
        
        status_t open();
        
        status_t create();
        
        status_t removeStream(const String& name);
        
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
        
    private:
    
        status_t readIndex();
        
        status_t createIndex();
        
        status_t readHeaders();
        
        String fileName_;
        File file_;
        
        struct StreamHeader {
            String name;
            uint_t index;
            File::Position firstFragment;
            
            StreamHeader(const String& name, uint_t index, File::Position firstFragment);
            
        };
        
        struct StreamHeaderLess {
            bool operator()(const StreamHeader* h1, const StreamHeader* h2) const
            {return h1->name<h2->name;}
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
            {return h1->start<h2->start;}
        };
        
        typedef std::set<FragmentHeader*, FragmentHeaderLess> FragmentHeaders_t;
        FragmentHeaders_t fragmentHeaders_;
        
        status_t findStream(const String& name, StreamHeader*& header);
        
        status_t createStream(const String& name, StreamHeader*& header);
        
        enum { minFragmentLength = sizeof(FragmentHeader) + 128};
        
        status_t createFragment(uint_t ownerIndex, FragmentHeader*& header);
        
        status_t writeFragmentHeader(const FragmentHeader& header);
        
        status_t writeStreamHeader(const StreamHeader& header);
        
        uint_t maxAllowedFragmentLength(FragmentHeader& header) const;
        
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
        
        status_t readStream(StreamPosition& position, void* buffer, uint_t& length);
        
        status_t writeFragment(FragmentHeader& fragment, uint_t& startOffset, const void*& buffer, uint_t& length);
        
        status_t writeStream(StreamPosition& position, const void* buffer, uint_t length);
        
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
        
        status_t open(const String& name);
        
        status_t read(int& chr);
        
        status_t read(char_t* buffer, uint_t& length);
        
        status_t readRaw(void* buffer, uint_t& length);
        
    };
    
    class DataStoreWriter: public Writer {
    
        DataStore& store_;
        DataStore::StreamPositionPtr position_;
        
    public:
    
        DataStoreWriter(DataStore& store);
        
        ~DataStoreWriter();
        
        status_t open(const String& name, bool dontCreate = false);
        
        status_t write(char_t chr);

        status_t write(const char_t* buffer, uint_t length);
        
        status_t writeRaw(const void* buffer, uint_t length);
        
        status_t flush();
        
    };
    
}

#endif