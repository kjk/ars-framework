#include <FileReader.hpp>

namespace ArsLexis {

    FileReader::FileReader() {}

    FileReader::~FileReader() {}
    
    status_t FileReader::read(int& ret)
    {
        assert(isOpen());
        char chr;
        File::Size read;
        status_t error = file_.read(&chr, sizeof(chr), read);
        if (errNone == error && 0 == read)
            ret = npos;
        else if (errNone == error)
            ret = chr;
        return error;
    }

    status_t FileReader::read(int& ret, String& dst, int offset, int range)
    {
        assert(isOpen());
        File::Size read;
        status_t error = file_.read(&dst[offset], sizeof(char)*range, read);
        if (errNone == error && 0 == read)
            ret = npos;
        else if (errNone == error)
            ret = read;
        return error;        
    }

    status_t FileReader::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
    {
        return file_.open(name, openMode, type, creator, cardNo);
    }

}