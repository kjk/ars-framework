#ifndef __ARSLEXIS_FILE_WRITER_HPP__
#define __ARSLEXIS_FILE_WRITER_HPP__

#include <Writer.hpp>
#include <File.hpp>

class FileWriter: public Writer 
{

    File file_;
    
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
    

    status_t writeRaw(const void* begin, uint_t length);
    
    status_t flush();
    
    bool isOpen() const
    {return file_.isOpen();}

    ~FileWriter();    

};

#endif