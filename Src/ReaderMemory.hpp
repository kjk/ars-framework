#ifndef __KXML2_XML_READER_MEMORY_HPP__
#define __KXML2_XML_READER_MEMORY_HPP__

#include <Reader.hpp>

using ArsLexis::String;
using ArsLexis::char_t;

// Implementation of XmlReader using a buffer in memory
// For perf reasons doesn't make the copy of the buffer
// passed to the constructor. The caller manages memory
// and must keep the buffer alive for the lifetime of
// XmlReaderMemory
class ReaderMemory : public Reader
{
    const char_t*  buf_;
    long            bufSize_;
    long            actPos_;

public:

    ReaderMemory(const char_t* buf, long bufSize);
    
    status_t read(int& chr);
    
    status_t read(int& num, String& dst, int offset, int range);
    
//        long getActPosition();
    
};

#endif
