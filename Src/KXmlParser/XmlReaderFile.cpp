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

int XmlReaderFile::read()
{
    return xmlReaderMemory_->read(); 
}

int XmlReaderFile::read(char_t* dst, int offset, int range)
{
    return xmlReaderMemory_->read(dst, offset, range);
}

