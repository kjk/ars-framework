#include <File.hpp>

using namespace ArsLexis;

File::File():
    handle_(reinterpret_cast<FileHandle_t>(invalidFileHandle))
{}

File::~File()
{
    if (isOpen())
        close();
}

status_t File::close()
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
    status_t error=FileClose(handle_);
    if (errNone!=error)
        FileClearerr(handle_);
    handle_=reinterpret_cast<FileHandle_t>(invalidFileHandle);
    return error;
}

status_t File::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
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

status_t File::read(void* buffer, File::Size bytesToRead, File::Size& bytesRead)
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
    if (FileEOF(handle_))
    {
        bytesRead=0;
        return errNone;
    }
    status_t error;
    long res=FileRead(handle_, buffer, 1, bytesToRead, &error);
    if (errNone == error || fileErrEOF == error)
    {
        bytesRead = res;
        error = errNone;
    }
    FileClearerr(handle_);
    return error;        
}

status_t File::write(const void* buffer, Size bytesToWrite)
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
    if (0==bytesToWrite)
        return errNone;
    status_t error;
    long written = FileWrite(handle_, buffer, bytesToWrite, 1, &error);
    if (errNone!=error)
        FileClearerr(handle_);
    if (0==written && errNone==error)
        error=fileErrIOError;
    return error;
}

status_t File::flush()
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
    status_t error=FileFlush(handle_);
    if (errNone!=error)
        FileClearerr(handle_);
    return error;        
}

status_t File::size(Size& val) const
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
    Int32 size;
    Err error;
    Int32 pos=FileTell(handle_, &size, &error);
    if (errNone!=error)
        FileClearerr(handle_);
    else
        val=size;
    return error;    
}

status_t File::position(Position& val) const
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
    Err error;
    Int32 pos=FileTell(handle_, NULL, &error);
    if (errNone!=error)
        FileClearerr(handle_);
    else
        val=pos;
    return error;    
}

status_t File::truncate()
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
    Position pos;
    status_t error=position(pos);
    if (errNone!=error)
        return error;
    error=FileTruncate(handle_, pos);
    if (errNone!=error)
        FileClearerr(handle_);
    return error;        
}

status_t File::seek(SeekOffset offset, SeekType type)
{
    if (!isOpen())
        return fileErrInvalidDescriptor;
        
    FileOriginEnum origin = fileOriginBeginning;
    if (seekFromCurrentPosition == type)
        origin = fileOriginCurrent;
    else if (seekFromEnd == type)
        origin = fileOriginEnd;
        
    Err error = FileSeek(handle_, offset, origin);
    if (errNone!=error)
        FileClearerr(handle_);
    return error;        
}

