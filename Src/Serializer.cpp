#include <Serializer.hpp>
#include <Writer.hpp>
#include <BufferedReader.hpp>

using namespace ArsLexis;
using namespace std;


bool Serializable::serializeInFromVersion(Serializer& ser, uint_t version) {return false;}

uint_t Serializable::schemaVersion() const {return 1;}

Serializable::~Serializable() {}


Serializer::Serializer(Reader& reader): 
    reader_(new BufferedReader(reader)), 
    writer_(NULL), 
    direction_(directionInput) {
    reader_->mark();
}

Serializer::Serializer(Writer& writer): reader_(NULL), writer_(&writer), direction_(directionOutput) {}

Serializer::Serializer(Reader& reader, Writer& writer, Direction dir): 
    reader_(new BufferedReader(reader)), 
    writer_(&writer), 
    direction_(dir) {
    reader_->mark();
}

Serializer::~Serializer() {delete reader_;}

void Serializer::serializeChunk(void* buffer, uint_t length)
{
    status_t error;
    if (directionInput == direction_)
    {   
        assert(NULL != reader_);
        uint_t read = length;
        error = reader_->readRaw(buffer, read);
        if (read != length)
            ErrThrow(errCorrupted);
    }
    else {
        assert(NULL != writer_);
        error = writer_->writeRaw(buffer, length);
    }
    if (errNone != error)
        ErrThrow(error);
}

void Serializer::serializeRecordIn(Record& record)
{
    Record buffer;
    serializeChunk(&buffer, sizeof(buffer));
    if (buffer.type != record.type || buffer.id != record.id)
        ErrThrow(errCorrupted);
    record = buffer;
}

void Serializer::serializeRecordOut(Record& record)
{
    serializeChunk(&record, sizeof(record));
}

void Serializer::serializeRecord(Record& record)
{
    if (directionInput == direction_)
        serializeRecordIn(record);
    else 
        serializeRecordOut(record);
}

Serializer& Serializer::operator()(bool& value, uint_t id)
{
    return serializeSimpleType<bool, dtBool>(value, id);    
}

Serializer& Serializer::operator()(signed char& value, uint_t id)
{
    return serializeSimpleType<signed char, dtChar>(value, id);    
}

Serializer& Serializer::operator()(unsigned char& value, uint_t id)
{
    return serializeSimpleType<unsigned char, dtUChar>(value, id);    
}

Serializer& Serializer::operator()(signed short& value, uint_t id)
{
    return serializeSimpleType<signed short, dtShort>(value, id);    
}

Serializer& Serializer::operator()(unsigned short& value, uint_t id)
{
    return serializeSimpleType<unsigned short, dtUShort>(value, id);    
}

Serializer& Serializer::operator()(signed int& value, uint_t id)
{
    return serializeSimpleType<signed int, dtInt>(value, id);    
}

Serializer& Serializer::operator()(unsigned int& value, uint_t id)
{
    return serializeSimpleType<unsigned int, dtUInt>(value, id);    
}

Serializer& Serializer::operator()(signed long& value, uint_t id)
{
    return serializeSimpleType<signed long, dtLong>(value, id);    
}

Serializer& Serializer::operator()(unsigned long& value, uint_t id)
{
    return serializeSimpleType<unsigned long, dtULong>(value, id);    
}

Serializer& Serializer::operator()(String& value, uint_t id)
{
    String::size_type length = value.length();
    serializeSimpleType<String::size_type, dtString>(length, id);
    if (directionOutput == direction_)
        serializeChunk(&value[0], sizeof(char_t)*value.length());
    else 
    {
        value.resize(length/sizeof(char_t));
        serializeChunk(&value[0], length);
    }
    return *this;
}

Serializer& Serializer::operator()(char_t array[], uint_t arraySize, uint_t id)
{
    String::size_type length;
    if (directionOutput == direction_)
        length = tstrlen(array);
    serializeSimpleType<String::size_type, dtString>(length, id);
    if (directionOutput == direction_)
        serializeChunk(array, length*sizeof(*array));
    else
    {
        if (arraySize <= length + 1)
            ErrThrow(errBufferTooSmall);
        serializeChunk(array, length*sizeof(*array));
        array[length] = _T('\0');
    }
    return *this;
}

Serializer& Serializer::operator()(Serializable& object, uint_t id)
{
    
    uint_t schemaVersion = object.schemaVersion();
    if (directionInput == direction_)
    {
        uint_t storedSchemaVersion = schemaVersion;
        serializeSimpleType<uint_t, dtSerializable>(storedSchemaVersion, id);
        if (storedSchemaVersion == schemaVersion)
            object.serialize(*this);
        else 
            object.serializeInFromVersion(*this, storedSchemaVersion);
    }
    else
    {
        serializeSimpleType<uint_t, dtSerializable>(schemaVersion, id);
        object.serialize(*this);
    }
    return *this;
}


