#ifndef __KXML2_XML_READER_FILE_HPP__
#define __KXML2_XML_READER_FILE_HPP__

#include <ReaderMemory.hpp>
#include <string>
#include <memory>

// Implementation of XmlReader that provides content from a file
namespace ArsLexis {

    class ReaderFile : public Reader
    {
        typedef std::auto_ptr<ReaderMemory> MemoryReaderPtr;
        MemoryReaderPtr readerMemory_;
        std::string fileContents_;

    public:
    
        explicit ReaderFile(const String& fileName);

        ~ReaderFile();

        status_t read(int& ret);

        status_t read(int& ret, String& dst, int offset, int range);

//        long getActPosition();
        
    };
    
}

#endif
