#ifndef __KXML2_XML_READER_FILE_HPP__
#define __KXML2_XML_READER_FILE_HPP__

#include "XmlReaderMemory.hpp"
#include "KXml2.hpp"

// Implementation of XmlReader that provides content from a file
namespace KXml2{
    class XmlReaderFile : public XmlReader
    {
        private:
            XmlReaderMemory * xmlReaderMemory_;
            char *            fileContent_;

        public:
            XmlReaderFile(const String fileName);
            ~XmlReaderFile();

            int read();
            int read(char_t *dst, int offset, int range);
    };
}

#endif
