#ifndef __ARSLEXIS_WRITER_HPP__
#define __ARSLEXIS_WRITER_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

namespace ArsLexis {

    class Writer: private NonCopyable
    {
    public:
    
        virtual status_t write(char_t chr)=0;

        /**
         * I provide a default implementation, but it's based on write(char_t), so please override it whenever possible.
         */        
        virtual status_t write(const char_t* begin, uint_t length);
    
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
        
        virtual ~Writer();
    
    };

}

#endif