
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

int XmlReaderMemory::read(String *dst, int offset, int range)
{
     return 0;
}

