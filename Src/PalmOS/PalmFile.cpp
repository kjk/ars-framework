#include <File.hpp>
#include <Application.hpp>

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

uint_t File::findCardNumber(const char* name, ulong_t openMode, ulong_t type, ulong_t creator) const 
{
    if (0 == creator)
        creator = Application::creator();
    if (0 == type)
    {
        type = sysFileTFileStream;
        if (0 != openMode & fileModeTemporary)
            type = sysFileTTemp;
    }
    DmSearchStateType searchState;
    LocalID localId;
    UInt16 cardNo;
    bool firstSearch = true;
    Err error;
    char nameBuffer[32];
    while (true)
    {
        error = DmGetNextDatabaseByTypeCreator(firstSearch, &searchState, type, creator, true, &cardNo, &localId);
        if (errNone != error)
            return 0;
        firstSearch = false;
        error = DmDatabaseInfo(cardNo, localId, nameBuffer, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        if (errNone != error)
            return 0;
        if (0 == StrCompareAscii(name, nameBuffer))
            return  cardNo;
    }
}


status_t File::open(const char_t* name, ulong_t openMode, uint_t cardNo, ulong_t type, ulong_t creator)
{
    if (isOpen())
        close();
    status_t error;
    if (anyCard == cardNo)
        cardNo = findCardNumber(name, openMode, type, creator);
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

DmOpenRef File::databaseHandle()
{
    if (!isOpen())
        return 0;
    DmOpenRef ref;
    Int32 size = sizeof(ref);
    Err error = FileControl(fileOpGetOpenDbRef, handle_, &ref, &size);
    if (errNone != error)
        return 0;
    assert(sizeof(ref) == size);
    return ref; 
}
