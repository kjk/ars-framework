
#include <FileWriter.hpp>

#if defined(__MWERKS__)
# pragma inline_depth(100)
# pragma inline_bottom_up on
#endif

namespace ArsLexis {

    FileWriter::FileWriter():
        handle_(reinterpret_cast<FileHandle_t>(invalidFileHandle))
    {}

    FileWriter::~FileWriter()
    {
        if (isOpen())
            close();
    }
    
    status_t FileWriter::close()
    {
        flush();
        assert(isOpen());
        status_t error=FileClose(handle_);
        if (errNone!=error)
            FileClearerr(handle_);
        handle_=reinterpret_cast<FileHandle_t>(invalidFileHandle);
        return error;
    }
    
    status_t FileWriter::write(char_t chr)
    {
        assert(isOpen());
        status_t error;
        long written=FileWrite(handle_, &chr, sizeof(chr), 1, &error);
        if (errNone!=error)
            FileClearerr(handle_);
        if (0==written && errNone==error)
            error=fileErrIOError;
        return error;
    }

    status_t FileWriter::flush()
    {
        assert(isOpen());
        status_t error=FileFlush(handle_);
        if (errNone!=error)
            FileClearerr(handle_);
        return error;        
    }
    
    status_t FileWriter::write(const String& str, String::size_type startOffset, String::size_type length)
    {
        assert(isOpen());
        const char_t* data=str.data()+startOffset;
        if (str.npos==length)
            length=str.length()-startOffset;
        if (0==length)
            return errNone;
        status_t error;
        long written=FileWrite(handle_, data, length, 1, &error);
        if (errNone!=error)
            FileClearerr(handle_);
        if (0==written && errNone==error)
            error=fileErrIOError;
        return error;
    }
    
    status_t FileWriter::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
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