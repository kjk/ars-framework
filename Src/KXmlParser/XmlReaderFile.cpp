#include <stdio.h>
#include "XmlReaderFile.hpp"

using namespace KXml2;

XmlReaderFile::XmlReaderFile(const String fileName)
{
    long    fileSize;
    FILE *  in = fopen(fileName.c_str(), "rb");

    fseek(in,0L,SEEK_END);
    fileSize = ftell(in);
    fseek(in,0L,SEEK_SET);

    fileContent_ = new char[fileSize];
    fread(fileContent_,fileSize,1,in);
    fclose(in);

    xmlReaderMemory_ = new XmlReaderMemory(fileContent_,fileSize);
}

XmlReaderFile::~XmlReaderFile()
{
    delete fileContent_;
}

int XmlReaderFile::read()
{
    return xmlReaderMemory_->read(); 
}

int XmlReaderFile::read(char_t *dst, int offset, int range)
{
    return xmlReaderMemory_->read(dst,offset,range);
}

