
#include "XmlReaderMemory.hpp"

using namespace KXml2;

XmlReaderMemory::XmlReaderMemory(const char_t* buf, long bufSize)
{
    buf_ = buf;
    bufSize_ = bufSize;
}


int XmlReaderMemory::read()
{
    return std::char_traits<char_t>::eof();
}

int XmlReaderMemory::read(char_t* dst, int offset, int range)
{
     return 0;
}

