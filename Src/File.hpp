#ifndef ARSLEXIS_FILE_HPP__
#define ARSLEXIS_FILE_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Utility.hpp>

namespace ArsLexis {
    
    class File: private NonCopyable {
        
#if defined(_PALM_OS)
        typedef FileHand FileHandle_t;
        enum {invalidFileHandle=0};
#elif defined(_WIN32)
        typedef HANDLE FileHandle_t;
        enum {invalidFileHandle=reinterpret_cast<ulong_t>(INVALID_HANDLE_VALUE)};
#else
# error "Define FileReader::FileHandle_t for your system."
#endif

        FileHandle_t handle_;
        
    public:
    
#if defined(_PALM_OS)
        typedef ulong_t Position;
        typedef long SeekOffset;
#elif defined(_WIN32)
        typedef unsigned __int64 Position;
        typedef __int64 SeekOffset;
#endif
        
        typedef Position Size;
    
        File();
        
        ~File();
        
#if defined(_PALM_OS)

        status_t open(const char_t* name, ulong_t openMode, ulong_t type=0, ulong_t creator=0, uint_t cardNo=0);
        
        DmOpenRef databaseHandle();
        
#elif defined(_WIN32)

        //! @note If all the parameters except fileName will have sensible default values then it'll be possible to use the same call to open() on both platforms.
        status_t open(const char_t* fileName, 
            ulong_t access,
            ulong_t shareMode=FILE_SHARE_READ,
            LPSECURITY_ATTRIBUTES securityAttributes=NULL,
            ulong_t creationDisposition=OPEN_EXISTING,
            ulong_t flagsAndAttributes=FILE_FLAG_SEQUENTIAL_SCAN,
            HANDLE templateFile=NULL);
            
#else
# error "Declare FileReader::open() for your system."
#endif
        
        bool isOpen() const
        {return reinterpret_cast<FileHandle_t>(invalidFileHandle)!=handle_;}

        status_t close();

        status_t size(Size& val) const;
        
        status_t position(Position& val) const;
        
        enum SeekType {
            seekFromBeginning,
            seekFromCurrentPosition,
            seekFromEnd
        };
        
        status_t seek(SeekOffset offset, SeekType type=seekFromCurrentPosition);

        status_t read(void* buffer, Size bytesToRead, Size& bytesRead);
        
        status_t write(const void* buffer, Size bytesToWrite);
        
        status_t flush();
        
        status_t truncate();
        
    };


}

#endif