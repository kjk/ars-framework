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
        virtual status_t write(const String& str, String::size_type startOffset=0, String::size_type length=String::npos);
    
        virtual status_t flush()=0;
        
        virtual ~Writer();
    
    };

}

#endif