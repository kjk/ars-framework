#include <FileReader.hpp>

namespace ArsLexis {

    FileReader::FileReader() {}

    FileReader::~FileReader() {}
    
    status_t FileReader::readRaw(void* buffer, uint_t& length)
    {
        assert(isOpen());
        File::Size read;
        status_t error = file_.read(buffer, length, read);
        if (errNone == error)
            length = read;
        return error;        
    }
    
    status_t FileReader::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
    {
        return file_.open(name, openMode, type, creator, cardNo);
    }

}