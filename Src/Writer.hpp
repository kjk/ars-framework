#ifndef __ARSLEXIS_WRITER_HPP__
#define __ARSLEXIS_WRITER_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

namespace ArsLexis {

    class Writer: private NonCopyable
    {
    public:
    
        status_t write(char_t chr)
        {
            return writeRaw(&chr, sizeof(chr));
        }

        status_t write(const char_t* buffer, uint_t length)
        {
            return writeRaw(buffer, sizeof(*buffer)*length);
        }

        status_t write(const char_t* text)
        {
            using namespace std; 
            return write(text, tstrlen(text));
        }
        
        status_t write(const String& text)
        {
            return write(text.data(), text.length());
        }
        
        virtual status_t flush()=0;
        
        virtual status_t writeRaw(const void* buffer, uint_t length)=0;
        
        virtual ~Writer();
    
    };

}

#endif