
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
    ErrTry {
        while (true)
        {
            int chr;
            status_t error = read(chr);
            if (errNone != error)
                ErrReturn(error);
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
        return ex;
    }
    ErrEndCatch
    return errNone;
}

Reader::~Reader()
{}
