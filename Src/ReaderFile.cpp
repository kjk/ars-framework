#include <ReaderFile.hpp>
#include <cstdio>

using namespace ArsLexis;

ReaderFile::ReaderFile(const String& fileName)
{
    using namespace std;
    
    long    fileSize;
    FILE *  in = fopen(fileName.c_str(), "rb");

    fseek(in, 0L, SEEK_END);
    fileSize = ftell(in);
    fseek(in, 0L, SEEK_SET);

    fileContents_.resize(fileSize);
    fread(&fileContents_[0], fileSize, 1, in);
    fclose(in);

    readerMemory_=MemoryReaderPtr(new ReaderMemory(fileContents_.data(), fileSize));
}

ReaderFile::~ReaderFile()
{}

status_t ReaderFile::read(int& ret)
{
    return readerMemory_->read(ret); 
}

status_t ReaderFile::read(int& ret, String& dst, int offset, int range)
{
    return readerMemory_->read(ret, dst, offset, range);
}

/*
long ReaderFile::getActPosition()
{
    return readerMemory_->getActPosition();
}
*/
