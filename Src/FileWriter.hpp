#ifndef __ARSLEXIS_FILE_WRITER_HPP__
#define __ARSLEXIS_FILE_WRITER_HPP__

#include <Writer.hpp>

namespace ArsLexis {

    class FileWriter: public Writer 
    {

#if defined(_PALM_OS)
        typedef FileHand FileHandle_t;
        enum {invalidFileHandle=0};
#elif defined(_WIN32)
        typedef HANDLE FileHandle_t;
        enum {invalidFileHandle=INVALID_HANDLE_VALUE};
#else
# error "Define FileWriter::FileHandle_t for your system."
#endif

        FileHandle_t handle_;
        
        status_t close();
        
    public:
    
        FileWriter();
        
#if defined(_PALM_OS)

        status_t open(const char_t* name, ulong_t openMode=fileModeReadWrite, ulong_t type=0, ulong_t creator=0, uint_t cardNo=0);
        
#elif defined(_WIN32)

        //! @todo Transform parameter names into human-readable form. 
        //! @note If all the parameters except fileName will have sensible default values then it'll be possible to use the same call to open() on both platforms.
        status_t open(const char_t* lpFileName,                         // file name
            DWORD dwDesiredAccess,                      // access mode
            DWORD dwShareMode,                          // share mode
            LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
            DWORD dwCreationDisposition,                // how to create
            DWORD dwFlagsAndAttributes,                 // file attributes
            HANDLE hTemplateFile);
            
#else
# error "Declare FileWriter::open() for your system."
#endif
        

        status_t write(char_t chr);

        status_t write(const char_t* begin, const char_t* end);
        
        status_t flush();
        
        bool isOpen() const
        {return reinterpret_cast<FileHandle_t>(invalidFileHandle)!=handle_;}

        ~FileWriter();    

    };

}

#endif