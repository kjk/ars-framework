
#include "XmlReaderMemory.hpp"

using namespace KXml2;

XmlReaderMemory::XmlReaderMemory(const char_t* buf, long bufSize)
{
    buf_ = buf;
    bufSize_ = bufSize;
    actPos_ = 0;
}


error_t XmlReaderMemory::read(int& ret)
{
    if(actPos_ >= bufSize_)
        ret = -1;
    else
        ret = buf_[actPos_++];
    return eNoError;
}

error_t XmlReaderMemory::read(int& ret, String& dst, int offset, int range)
{
     long i = 0;
     if((bufSize_ - actPos_) < range)
         range = bufSize_-actPos_;
     for(i = 0; i < range; i++)
         dst[offset+i] = buf_[actPos_++];
     if(i > 0)
        ret = range;
     else
        ret = -1;
     return eNoError;
}

long XmlReaderMemory::getActPosition()
{
    return actPos_;    
}

