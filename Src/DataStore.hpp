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
    
    public:
        
        enum {
            maxStreamNameLength=32,
            maxStreamsCount=32
        };
        
        explicit DataStore(File& f);
        
        ~DataStore();
        
        status_t open();
        
        status_t removeStream(const String& name);
        
        enum {
            errStoreCorrupted=dsErrorClass,
            errNotFound,
            errAlreadyExists,
            errTooManyStreams,
            errNameTooLong,
        };            
        
    private:
    
        status_t readIndex();
        
        status_t readHeaders();
        
        File& file_;
        
        struct StreamHeader {
            String name;
            uint_t index;
            File::Position firstFragment;
            
            StreamHeader(const String& name, uint_t index, File::Position firstFragment);
            
            StreamHeader();
            
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
            
            FragmentHeader();
            
        };
        
        struct FragmentHeaderLess {
            bool operator()(const FragmentHeader* h1, const FragmentHeader* h2) const
            {return h1->start<h2->start;}
        };
        
        typedef std::set<FragmentHeader*, FragmentHeaderLess> FragmentHeaders_t;
        FragmentHeaders_t fragmentHeaders_;
        
        status_t createStream(const String& name, StreamHeader*& header);
        
        enum { minFragmentLength = sizeof(FragmentHeader) + 128};
        
        status_t createFragment(uint_t ownerIndex, FragmentHeader*& header);
        
        status_t writeFragmentHeader(const FragmentHeader& header);
        
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
        
        status_t readFragment(const FragmentHeader& fragment, uint_t& startOffset, void*& buffer, uint_t& length);
        
        status_t readStream(StreamPosition& position, void* buffer, uint_t& length);
        
    };
    
    class DataStoreReader: public Reader {
        DataStore& store_;
        
    public:
        
        DataStoreReader(DataStore& store);
        
        status_t open(const String& name);
        
    };
    
    class DataStoreWriter: public Writer {
    
        DataStore& store_;
        
    public:
    
        DataStoreWriter(DataStore& store);
        
        status_t open(const String& name);
        
    };
    
}

#endif