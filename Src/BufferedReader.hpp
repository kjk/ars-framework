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
        
        enum SeekType {
            seekFromBeginning,
            seekFromCurrentPosition,
            seekFromEnd
        };

        typedef long SeekOffset;
        
        //! Seek is relative to marked position. If there's no marked position it fails.        
        status_t  seek(SeekOffset offset, SeekType type=seekFromCurrentPosition);
        
        status_t rewind() { return seek(0, seekFromBeginning);}
        
        status_t readRaw(void* buffer, uint_t& length);
        
        ulong_t position() const {return position_;}
        
    private:

        bool isMarked_;    
        Buffer_t::size_type position_;
        uint_t chunkSize_;
        
        status_t readNonMarked(void* buffer, uint_t& length);
        
        status_t readMarked(void* buffer, uint_t& length);
    
    };

}

#endif
