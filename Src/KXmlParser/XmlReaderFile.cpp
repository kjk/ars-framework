#include "XmlReaderFile.hpp"
#include <cstdio>

using namespace KXml2;

XmlReaderFile::XmlReaderFile(const String& fileName)
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

    xmlReaderMemory_ = MemoryReaderPtr(new XmlReaderMemory(fileContents_.data(), fileSize));
}

XmlReaderFile::~XmlReaderFile()
{}

error_t XmlReaderFile::read(int& ret)
{
    return xmlReaderMemory_->read(ret); 
}

error_t XmlReaderFile::read(int& ret, String& dst, int offset, int range)
{
    return xmlReaderMemory_->read(ret, dst, offset, range);
}

