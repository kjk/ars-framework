
#include <Reader.hpp>

status_t Reader::read(int& chr)
{
    char_t c;
    uint_t length=sizeof(c);
    status_t error = readRaw(&c, length);
    if (errNone != error)
        return error;
    if (0 == length)
        chr = npos;
    else
        chr = c;
    return errNone;
}

status_t Reader::read(char_t* buffer, uint_t& length)
{
    uint_t toRead = sizeof(char_t) * length;
    status_t error = readRaw(buffer, toRead);
    if (errNone != error)
        return error;
//        if (toRead % sizeof(char_t) != 0)
//            return DataStore::errStoreCorrupted;
    length = toRead / sizeof(char_t);
    return errNone;    
}

status_t Reader::readLine(bool& eof, String& out, char_t delimiter)
{
    volatile status_t error=errNone;
    ErrTry {
        while (true)
        {
            int chr;
            status_t error=read(chr);
            if (errNone!=error)
                return error;
            if (npos!=chr && delimiter!=chr)
                out.append(1, char_t(chr));
            else 
            {
                if (npos==chr)
                    eof=true;
                break;
            }
        }
    }
    ErrCatch (ex) {
        error=ex;
    }
    ErrEndCatch
    return error;
}

Reader::~Reader()
{}
