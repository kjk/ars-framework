
#include <FileWriter.hpp>

namespace ArsLexis {

    FileWriter::FileWriter() {}

    FileWriter::~FileWriter() {}
    
    status_t FileWriter::write(char_t chr)
    {
        assert(isOpen());
        return file_.write(&chr, sizeof(chr));
    }

    status_t FileWriter::flush()
    {
        return file_.flush();
    }
    
    status_t FileWriter::write(const char_t* begin, uint_t length)
    {
        assert(isOpen());
        return file_.write(begin, sizeof(char_t)*length);
    }
    
    status_t FileWriter::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
    {
        return file_.open(name, openMode, type, creator, cardNo);
    }

    
}