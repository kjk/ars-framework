#include <Serializer.hpp>
#include <Writer.hpp>
#include <BufferedReader.hpp>
#include <DynStr.hpp>

using namespace std;

bool Serializable::serializeInFromVersion(Serializer& ser, uint_t version) {return false;}

uint_t Serializable::schemaVersion() const {return 1;}

Serializable::~Serializable() {}


Serializer::Serializer(Reader& reader): 
    reader_(new BufferedReader(reader)), 
    writer_(NULL), 
    direction_(directionInput),
    isIndexed_(false),
    skipLastRecord_(false) 
{
    reader_->mark();
}

Serializer::Serializer(Writer& writer): reader_(NULL), writer_(&writer), direction_(directionOutput), isIndexed_(false), skipLastRecord_(false) {}

Serializer::Serializer(Reader& reader, Writer& writer, Direction dir): 
    reader_(new BufferedReader(reader)), 
    writer_(&writer), 
    direction_(dir),
    isIndexed_(false),
    skipLastRecord_(false) 
{
    reader_->mark();
}

Serializer::~Serializer() 
{
    delete reader_;
}

void Serializer::serializeChunk(void* buffer, ulong_t length)
{
    status_t error;
    if (directionInput == direction_)
    {   
        assert(NULL != reader_);
        ulong_t read = length;
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

bool Serializer::indexNextRecord()
{
    Record record;
    ulong_t length = sizeof(record);
    status_t error = reader_->readRaw(&record, length);
    if (errNone != error)
        ErrThrow(error);
    if (0 == length)
        return false;
    if (sizeof(record) != length)
        ErrThrow(errCorrupted);
    if (unusedId != record.id)
        recordIndex_[record.id] = reader_->position() - length;
        
    if (dtString == record.type) // String is currently only data type that writes something after Record. We have to skip its data...
    { 
        length = record.stringLength;
        void* buffer = malloc(length);
        if (NULL == buffer)
            ErrThrow(memErrNotEnoughSpace);
            
        error = reader_->readRaw(buffer, length);
        free(buffer);
        
        if (errNone != error)
            ErrThrow(errCorrupted);

    }
    return true;
}

void Serializer::loadIndex() 
{
    assert(!isIndexed_);
    isIndexed_ = true;
    bool goOn = true;
    while (goOn)
        goOn = indexNextRecord();
}

void Serializer::serializeRecordIn(Record& record)
{
    Record buffer;
    assureIndexed();
    skipLastRecord_ = false;
    if (unusedId != record.id)
    {
        RecordIndex_t::const_iterator it = recordIndex_.find(record.id);
        if (recordIndex_.end() == it) // Don't try loading missing records with explicit ids. Skip them assuming that they are initialized to some reasonable defaults.
        {
            skipLastRecord_ = true;
            return;  
        }
        if (reader_->position() != it->second)
        {
            status_t error = reader_->seek(it->second, BufferedReader::seekFromBeginning);
            assert(errNone == error); // This is correct assumption as we must have already read entire stream once to know record's position.
        }
    }
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
        serializeChunk(&value[0], sizeof(char_t) * length);
    else if (!skipLastRecord_)
    {
        value.resize(length);
        serializeChunk(&value[0], sizeof(char_t) * length);
    }
    return *this;
}

Serializer& Serializer::operator()(DynStr& value, uint_t id)
{
    ulong_t len = DynStrLen(&value);
    serializeSimpleType<ulong_t, dtString>(len, id);
    char_t* data;
    if (directionOutput == direction_)
    {
        data = DynStrGetData(&value);
        serializeChunk(data, sizeof(char_t) * len);
    }
    else if (!skipLastRecord_)
    {
        if (0 == len)
        {
            data = DynStrReleaseStr(&value);
            if (NULL != data)
                free(data);
        }
        else
        {
            data = (char_t*)malloc((len + 1) * sizeof(char_t));
            if (NULL == data)
                ErrThrow(memErrNotEnoughSpace);
            
            memzero(data, (len + 1) * sizeof(char_t));
            serializeChunk(data, len * sizeof(char_t));
            
            DynStrAttachCharPBuf(&value, data, len, len + 1);
        }
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
        serializeChunk(array, length * sizeof(*array));
    else if (!skipLastRecord_)
    {
        if (arraySize < length + 1)
            ErrThrow(errBufferTooSmall);
        serializeChunk(array, length * sizeof(*array));
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
        if (skipLastRecord_)
            return *this;
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


