
#include <FileWriter.hpp>

FileWriter::FileWriter() {}

FileWriter::~FileWriter() {}

status_t FileWriter::flush()
{
    return file_.flush();
}

status_t FileWriter::writeRaw(const void* begin, uint_t length)
{
    assert(isOpen());
    return file_.write(begin, length);
}

status_t FileWriter::open(const char_t* name, ulong_t openMode, ulong_t type, ulong_t creator, uint_t cardNo)
{
    return file_.open(name, openMode, type, creator, cardNo);
}