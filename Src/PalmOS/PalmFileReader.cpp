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

    status_t FileReader::read(char_t* buffer, uint_t& length)
    {
        assert(isOpen());
        File::Size read;
        status_t error = file_.read(buffer, sizeof(char_t)*length, read);
        if (errNone == error)
            length = read;
        return error;        
    }

    status_t FileReader::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
    {
        return file_.open(name, openMode, type, creator, cardNo);
    }

}