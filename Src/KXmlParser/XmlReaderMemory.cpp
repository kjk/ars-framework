
#include "XmlReaderMemory.hpp"

using namespace KXml2;

XmlReaderMemory::XmlReaderMemory(const char_t* buf, long bufSize)
{
    buf_ = buf;
    bufSize_ = bufSize;
}


error_t XmlReaderMemory::read(int& ret)
{
    ret = -1;
    return eNoError;
}

error_t XmlReaderMemory::read(int& ret, String& dst, int offset, int range)
{
     ret = -1;
     return eNoError;
}

