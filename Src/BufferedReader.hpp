#ifndef ARSLEXIS_BUFFERED_READER_HPP__
#define ARSLEXIS_BUFFERED_READER_HPP__

#include <Reader.hpp>
#include <string>

namespace ArsLexis {

    //! BufferedReader reads data from other Readers in large chunks so that it should
    //! speed up reading slightly (well, not really on Palm, but that's not the reason
    //! it was written for). It also allows to read some data many times using @c mark() 
    //! and @c rewind(). Call @c mark() in place where you want to start reading again. 
    //! When you call @c rewind() reader's read pointer will go back to marked position.
    class BufferedReader: public Reader {
        typedef std::string Buffer_t;
        Buffer_t buffer_;
        Reader& reader_;
        
    public:
    
        enum { defaultChunkSize = 512 };
    
        BufferedReader(Reader& reader, uint_t chunkSize = defaultChunkSize);
        
        ~BufferedReader();
        
        void mark();
        
        void unmark();
        
        void rewind();
        
        status_t readRaw(void* buffer, uint_t& length);
        
    private:

        bool isMarked_;    
        Buffer_t::size_type position_;
        uint_t chunkSize_;
    
    };

}

#endif
