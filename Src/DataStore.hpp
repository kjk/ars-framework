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
        
    private:
    
        status_t readIndex();
        
        status_t readHeaders();
        
        File& file_;
        
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
        
        status_t readHeadersForOwner(const StreamHeaders_t::const_iterator& streamHeader);
        
        struct FragmentHeader {
            File::Position start;
            uint_t ownerIndex;
            uint_t length;
            File::Position nextFragment;
            
            FragmentHeader(File::Position start, uint_t ownerIndex, uint_t length, File::Position nextFragment);
            
        };
        
        struct FragmentHeaderLess {
            bool operator()(const FragmentHeader* h1, const FragmentHeader* h2) const
            {return h1->start<h2->start;}
        };
        
        typedef std::set<FragmentHeader*, FragmentHeaderLess> FragmentHeaders_t;
        FragmentHeaders_t fragmentHeaders_;
        
        enum {
            errStoreCorrupted=dsErrorClass,
            errStreamNotFound,
            
        };            
        
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