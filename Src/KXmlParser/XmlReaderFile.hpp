#ifndef __KXML2_XML_READER_FILE_HPP__
#define __KXML2_XML_READER_FILE_HPP__

#include "KXml2.hpp"
#include "XmlReaderMemory.hpp"
#include <string>
#include <memory>

// Implementation of XmlReader that provides content from a file
namespace KXml2{

    class XmlReaderFile : public XmlReader
    {
        typedef std::auto_ptr<XmlReaderMemory> MemoryReaderPtr;
        MemoryReaderPtr xmlReaderMemory_;
        std::string fileContents_;

    public:
    
        XmlReaderFile(const String& fileName);

        ~XmlReaderFile();

        error_t read(int& ret);

        error_t read(int& ret, String& dst, int offset, int range);
        
    };
    
}

#endif
