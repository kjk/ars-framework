#include <FileReader.hpp>

namespace ArsLexis {

    FileReader::FileReader():
        handle_(reinterpret_cast<FileHandle_t>(invalidFileHandle))
    {}

    FileReader::~FileReader()
    {
        if (isOpen())
            close();
    }
    
    status_t FileReader::close()
    {
        assert(isOpen());
        status_t error=FileClose(handle_);
        if (errNone!=error)
            FileClearerr(handle_);
        handle_=reinterpret_cast<FileHandle_t>(invalidFileHandle);
        return error;
    }
    
    status_t FileReader::read(int& ret)
    {
        assert(isOpen());
        if (FileEOF(handle_))
        {
            ret=npos;
            return errNone;
        }
        status_t error;
        char chr;
        FileRead(handle_, &chr, sizeof(chr), 1, &error);
        if (fileErrEOF==error)
        {
            ret=npos;
            error=errNone;
        }
        else if (errNone==error)
            ret=chr;
        else 
            FileClearerr(handle_);
        return error;
    }

    status_t FileReader::read(int& ret, String& dst, int offset, int range)
    {
        assert(isOpen());
        if (FileEOF(handle_))
        {
            ret=npos;
            return errNone;
        }
        status_t error;
        long res=FileRead(handle_, &dst[offset], sizeof(char), range, &error);
        if (errNone==error || fileErrEOF==error)
        {
            ret=res;
            error=errNone;
        }
        FileClearerr(handle_);
        return error;        
    }

    status_t FileReader::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
    {
        if (isOpen())
            close();
        status_t error;
        handle_=FileOpen(cardNo, name, type, creator, openMode, &error);
        if (errNone!=error)
        {
            FileClearerr(handle_);
            handle_=reinterpret_cast<FileHandle_t>(invalidFileHandle);
        }
        return error;
    }

}