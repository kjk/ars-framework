#ifndef __ARSLEXIS_FILE_READER_HPP__
#define __ARSLEXIS_FILE_READER_HPP__

#include <Reader.hpp>
#include <File.hpp>

// Implementation of XmlReader that provides content from a file
class FileReader: public Reader
{
    
    File file_;
    
public:

    FileReader();

    ~FileReader();
    
#if defined(_PALM_OS)

    status_t open(const char_t* name, ulong_t openMode=fileModeReadOnly, ulong_t type=0, ulong_t creator=0, uint_t cardNo=0);
    
#elif defined(_WIN32)

    //! @note If all the parameters except fileName will have sensible default values then it'll be possible to use the same call to open() on both platforms.
    status_t open(const char_t* fileName, 
        ulong_t access=GENERIC_READ,
        ulong_t shareMode=FILE_SHARE_READ,
        LPSECURITY_ATTRIBUTES securityAttributes=NULL,
        ulong_t creationDisposition=OPEN_EXISTING,
        ulong_t flagsAndAttributes=FILE_FLAG_SEQUENTIAL_SCAN,
        HANDLE templateFile=NULL);
        
#else
# error "Declare FileReader::open() for your system."
#endif
    
    bool isOpen() const
    {return file_.isOpen();}

    status_t readRaw(void* buffer, ulong_t& length);
};

#endif
