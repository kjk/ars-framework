#ifndef __KXML2_XML_READER_MEMORY_HPP__
#define __KXML2_XML_READER_MEMORY_HPP__

#include "XmlReader.hpp"
#include "KXml2.hpp"

// Implementation of XmlReader using a buffer in memory
namespace KXml2{
    class XmlReaderMemory : public XmlReader
    {
        private:
            const char_t *  buf_;
            long            bufSize_;
            
        public:
            XmlReaderMemory(const char_t *buf, long bufSize);

            int read();
            int read(char_t *dst, int offset, int range);
    };
}

#endif
