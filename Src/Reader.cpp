
#include <Reader.hpp>

status_t Reader::read(int& chr)
{
    char c;
    ulong_t length = 1;
    status_t error = readRaw(&c, length);
    if (errNone != error)
        return error;
    if (0 == length)
        chr = npos;
    else
        chr = c;
    return errNone;
}

status_t Reader::read(char* buffer, ulong_t& length)
{
    return readRaw(buffer, length);
}

status_t Reader::readLine(bool& eof, NarrowString& out, char delimiter)
{
    volatile status_t error=errNone;
    ErrTry {
        while (true)
        {
            int chr;
            status_t error = read(chr);
            if (errNone != error)
                return error;
            if (npos != chr && delimiter != chr)
                out.append(1, char(chr));
            else 
            {
                if (npos == chr)
                    eof = true;
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
