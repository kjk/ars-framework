#include <IncrementalProcessor.hpp>
#include <Reader.hpp>
#include <Text.hpp>

BinaryIncrementalProcessor::~BinaryIncrementalProcessor()
{}

TextIncrementalProcessor::~TextIncrementalProcessor()
{}

status_t FeedHandlerFromReader(BinaryIncrementalProcessor& handler, Reader& reader)
{
    char* str = NULL;
    ulong_t strLen = 0;
     
    status_t err = errNone;
    const ulong_t chunk = 8192;
    char* buffer = StrAlloc<char>(chunk);
    if (NULL == buffer)
        return memErrNotEnoughSpace;
        
    ulong_t length = chunk;
    while (true)
    {
        if (errNone != (err = reader.read(buffer, length)))
            goto Error;
        
        if (NULL == (str = StrAppend(str, strLen, buffer, length)))
        {
            err = memErrNotEnoughSpace;
            goto Error;
        }
        strLen += length;
        
        ulong_t incLen = strLen;
        if (errNone != (err = handler.handleIncrement(str, incLen, chunk != length)))
            return err;
        
        assert(incLen <= strLen);
        StrErase(str, strLen, 0, incLen);
        strLen -= incLen;
        if (chunk != length)
            break;
    }
Error:
    free(buffer);
    free(str); 
    return err;
}
