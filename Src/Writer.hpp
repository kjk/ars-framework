#ifndef __ARSLEXIS_WRITER_HPP__
#define __ARSLEXIS_WRITER_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

class Writer: private NonCopyable
{
public:

    status_t write(char chr)
    {
        return writeRaw(&chr, sizeof(chr));
    }

    status_t write(const char* buffer, ulong_t length)
    {
        return writeRaw(buffer, sizeof(*buffer)*length);
    }

    status_t write(const char* text)
    {
        using namespace std; 
        return write(text, strlen(text));
    }
    
    status_t write(const NarrowString& text)
    {
        return write(text.data(), text.length());
    }
    
    virtual status_t flush()=0;
    
    virtual status_t writeRaw(const void* buffer, ulong_t length)=0;
    
    virtual ~Writer();

};

#endif