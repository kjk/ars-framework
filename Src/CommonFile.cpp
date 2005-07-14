
#include <File.hpp>


File::File():
    handle_(reinterpret_cast<FileHandle_t>(invalidFileHandle))
{}

File::~File()
{
    if (isOpen())
        close();
}

