#ifndef __KXML2_XML_READER_MEMORY_HPP__
#define __KXML2_XML_READER_MEMORY_HPP__

#include "KXml2.hpp"
#include "XmlReader.hpp"

// Implementation of XmlReader using a buffer in memory
// For perf reasons doesn't make the copy of the buffer
// passed to the constructor. The caller manages memory
// and must keep the buffer alive for the lifetime of
// XmlReaderMemory
namespace KXml2 {

    class XmlReaderMemory : public XmlReader
    {
        const char_t *  buf_;
        long            bufSize_;

    public:
    
        XmlReaderMemory(const char_t* buf, long bufSize);
        
        int read();
        
        int read(String *dst, int offset, int range);
        
    };
    
}

#endif
