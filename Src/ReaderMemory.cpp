
#include <ReaderMemory.hpp>

using ArsLexis::String;
using ArsLexis::char_t;

ReaderMemory::ReaderMemory(const char_t* buf, long bufSize):
    buf_(buf),
    bufSize_(bufSize),
    actPos_(0)
{}

status_t ReaderMemory::read(int& ret)
{
    if(actPos_ >= bufSize_)
        ret = npos;
    else
        ret = buf_[actPos_++];
    return errNone;
}

status_t ReaderMemory::read(int& ret, String& dst, int offset, int range)
{
     long i = 0;
     if((bufSize_ - actPos_) < range)
         range = bufSize_-actPos_;
     for(i = 0; i < range; i++)
         dst[offset+i] = buf_[actPos_++];
     if(i > 0)
        ret = range;
     else
        ret = npos;
     return errNone;
}

/*
long ReaderMemory::getActPosition()
{
    return actPos_;    
}
*/

