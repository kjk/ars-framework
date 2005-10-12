#include <Serializer.hpp>
#include <Writer.hpp>
#include <BufferedReader.hpp>
#include <Text.hpp>
#include <UTF8_Processor.hpp>

#define SERIALIZER_MAGIC 'serl'

using namespace std;

bool Serializable::serializeInFromVersion(Serializer& ser, ulong_t version) {return false;}

ulong_t Serializable::schemaVersion() const {return 1;}

Serializable::~Serializable() {}


Serializer::Serializer(Reader& reader): 
reader_(new BufferedReader(reader)), 
writer_(NULL), 
direction_(directionInput),
isIndexed_(false),
skipLastRecord_(false),
version_(0),
buffer_(NULL)
{
    reader_->mark();
}

Serializer::Serializer(Writer& writer): reader_(NULL), writer_(&writer), direction_(directionOutput), isIndexed_(false), skipLastRecord_(false), version_(currentVersion), buffer_(NULL) 
{
    writeVersion(); 
}

Serializer::Serializer(Reader& reader, Writer& writer, Direction dir): 
reader_(new BufferedReader(reader)), 
writer_(&writer), 
direction_(dir),
isIndexed_(false),
skipLastRecord_(false) ,
version_(currentVersion),
buffer_(NULL)
{
    reader_->mark();
    if (directionOutput == direction_)
        writeVersion(); 
}

Serializer::~Serializer() 
{
    delete reader_;
    free(buffer_); 
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

    if (oldVersion() && unusedIdVer0 == record.id)
        record.id = unusedId;     

    if (unusedId != record.id)
        recordIndex_[record.id] = reader_->position() - length;

    if (dtStringVer0 == record.type || dtBlob == record.type || dtText == record.type || dtDouble == record.type)
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

void Serializer::readVersion()
{
    uint32_t magic = 0;
    serializeChunk(&magic, sizeof(magic));
    if (SERIALIZER_MAGIC != magic)
    {
        version_ = 0;
        reader_->rewind();
    }
    else
    {
        uint32_t version = 0;
        serializeChunk(&version, sizeof(version));
        version_ = version;
    }
}

void Serializer::writeVersion()
{
    uint32_t magic = SERIALIZER_MAGIC;
    uint32_t version = currentVersion;
    serializeChunk(&magic, sizeof(magic));
    serializeChunk(&version, sizeof(version));
}


void Serializer::loadIndex() 
{
    assert(!isIndexed_);
    isIndexed_ = true;
    bool goOn = true;
    readVersion(); 
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
    if (oldVersion() && unusedIdVer0 == buffer.id)
        buffer.id = unusedId;

    if (buffer.id != record.id)
        ErrThrow(errCorrupted);

    if (buffer.type != record.type)
        if (!(dtStringVer0 == buffer.type && (dtBlob == record.type || dtText == record.type)))
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

Serializer& Serializer::operator()(bool& value, ulong_t id)
{
    return serializeSimpleType(dtBool, value, id);    
}

Serializer& Serializer::operator()(signed char& value, ulong_t id)
{
    return serializeSimpleType(dtChar, value, id);    
}

Serializer& Serializer::operator()(unsigned char& value, ulong_t id)
{
    return serializeSimpleType(dtUChar, value, id);    
}

Serializer& Serializer::operator()(signed short& value, ulong_t id)
{
    return serializeSimpleType(dtShort, value, id);    
}

Serializer& Serializer::operator()(unsigned short& value, ulong_t id)
{
    return serializeSimpleType(dtUShort, value, id);    
}

Serializer& Serializer::operator()(signed int& value, ulong_t id)
{
    return serializeSimpleType(dtInt, value, id);    
}

Serializer& Serializer::operator()(unsigned int& value, ulong_t id)
{
    return serializeSimpleType(dtUInt, value, id);    
}

Serializer& Serializer::operator()(signed long& value, ulong_t id)
{
    return serializeSimpleType(dtLong, value, id);    
}

Serializer& Serializer::operator()(unsigned long& value, ulong_t id)
{
    return serializeSimpleType(dtULong, value, id);    
}

Serializer& Serializer::operator()(double& value, ulong_t id)
{
    ulong_t length = sizeof(double);
    serializeSimpleType(dtDouble, length, id);

    if (directionInput == direction() && sizeof(double) != length)
        ErrThrow(errCorrupted);

    if (directionOutput == direction() || !skipLastRecord_) 
        serializeChunk(&value, sizeof(double)); 
    return *this;
}


Serializer& Serializer::narrowBuffer(char* array, ulong_t size, ulong_t id)
{
    ulong_t length = size;
    serializeSimpleType(dtBlob, length, id);
    if (directionOutput == direction_)
        serializeChunk(array, length);
    else if (!skipLastRecord_)
    {
        if (size < length)
            ErrThrow(errBufferTooSmall);
        serializeChunk(array, length);
        if (length < size)
            ((char*)array)[length] = '\0';
    } 
    return *this;
}


Serializer& Serializer::narrow(NarrowString& value, ulong_t id)
{
    ulong_t length = value.length();
    serializeSimpleType(dtBlob, length, id);
    if (directionOutput == direction_)
        serializeChunk((char*)value.data(), length);
    else if (!skipLastRecord_)
    {
        value.resize(length); 
        serializeChunk(&value[0], length);
    }
    return *this;
}

Serializer& Serializer::narrowOut(const char* str, long len, ulong_t id)
{
    if (directionOutput != direction_)
        return *this;

    StrLenFix(str, len);
    ulong_t length = len;
    serializeSimpleType(dtBlob, length, id);
    serializeChunk((char*)str, length);
    return *this; 
}

Serializer& Serializer::narrowIn(char*& str, ulong_t* len, ulong_t id)
{
    if (directionInput != direction_)
        return *this;

    ulong_t length = 0;
    serializeSimpleType(dtBlob, length, id);
    if (skipLastRecord_)
        return *this;

    free(buffer_);
    buffer_ = NULL;

    char* p = (char*)malloc(length + 1);
    if (NULL == p)
        ErrThrow(memErrNotEnoughSpace);

    buffer_ = p;
    serializeChunk(p, length);
    p[length] = '\0';

    if (NULL != len)
        *len = length;

    free(str);
    str = p;
    buffer_ = NULL;	
    return *this;
}

Serializer& Serializer::narrow(char*& str, ulong_t* len, ulong_t id)
{
    if (directionOutput == direction_)
    {
        long length = -1;
        if (NULL != len) length = *len;
        return narrowOut(str, length, id); 
    }
    else
        return narrowIn(str, len, id);
}

Serializer& Serializer::textOut(const char_t* str, long len, ulong_t id)
{
    if (directionOutput != direction_)
        return *this;

    ulong_t length;
    free(buffer_);
    buffer_ = UTF8_FromNative(str, len, &length);
    if (NULL == buffer_)
        ErrThrow(memErrNotEnoughSpace);

    serializeSimpleType(dtText, length, id);
    serializeChunk(buffer_, length);

    free(buffer_);
    buffer_ = NULL;
    return *this;
}

Serializer& Serializer::textIn(char_t*& str, ulong_t* len, ulong_t id)
{
    if (directionInput != direction_)
        return *this;

    ulong_t length = 0;
    serializeSimpleType(dtText, length, id);
    if (skipLastRecord_)
        return *this;

    free(buffer_);
    buffer_ = NULL;

    char* p = (char*)malloc(length);
    if (NULL == p)
        ErrThrow(memErrNotEnoughSpace);

    buffer_ = p;
    serializeChunk(buffer_, length);

    ulong_t slen;
    char_t* s = UTF8_ToNative(p, length, &slen);

    free(buffer_);
    buffer_ = NULL;

    if (NULL == s)
        ErrThrow(memErrNotEnoughSpace);

    free(str);
    str = s;
    if (NULL != len)
        *len = slen;
    return *this;
}

Serializer& Serializer::text(char_t*& str, ulong_t* len, ulong_t id)
{
    if (directionOutput == direction_)
    {
        long length = -1;
        if (NULL != len) length = *len;
        return textOut(str, length, id); 
    }
    else
        return textIn(str, len, id);
}

Serializer& Serializer::text(String& value, ulong_t id)
{
    if (directionOutput == direction_)
        return textOut((char_t*)value.data(), value.length(), id);

    ulong_t length = 0;
    serializeSimpleType(dtText, length, id);
    if (skipLastRecord_)
        return *this;

    free(buffer_);
    buffer_ = NULL;

    char* p = (char*)malloc(length);
    if (NULL == p)
        ErrThrow(memErrNotEnoughSpace);

    buffer_ = p;
    serializeChunk(buffer_, length);

    ulong_t slen;
    char_t* s = UTF8_ToNative(p, length, &slen);

    free(buffer_);
    buffer_ = NULL;

    if (NULL == s)
        ErrThrow(memErrNotEnoughSpace);

    buffer_ = s;
    value.assign(s, slen);

    free(buffer_);
    buffer_ = NULL;

    return *this;
}

Serializer& Serializer::textBuffer(char_t* array, ulong_t size, ulong_t id)
{
    if (directionOutput == direction_)
        return textOut(array, size, id);

    ulong_t length = 0;
    serializeSimpleType(dtText, length, id);
    if (skipLastRecord_)
        return *this;

    free(buffer_);
    buffer_ = NULL;

    char* p = (char*)malloc(length);
    if (NULL == p)
        ErrThrow(memErrNotEnoughSpace);

    buffer_ = p;
    serializeChunk(buffer_, length);

    ulong_t slen;
    char_t* s = UTF8_ToNative(p, length, &slen);

    free(buffer_);
    buffer_ = NULL;

    if (NULL == s)
        ErrThrow(memErrNotEnoughSpace);

    buffer_ = s;
    if (slen > size)
        ErrThrow(errBufferTooSmall);

    memmove(array, s, slen * sizeof(char_t));
    if (slen < size)
        array[slen] = _T('\0');

    free(buffer_);
    buffer_ = NULL;

    return *this;
}

Serializer& Serializer::operator()(Serializable& object, ulong_t id)
{

    ulong_t schemaVersion = object.schemaVersion();
    if (directionInput == direction_)
    {
        ulong_t storedSchemaVersion = schemaVersion;
        serializeSimpleType(dtSerializable, storedSchemaVersion, id);
        if (skipLastRecord_)
            return *this;
        if (storedSchemaVersion == schemaVersion)
            object.serialize(*this);
        else 
            object.serializeInFromVersion(*this, storedSchemaVersion);
    }
    else
    {
        serializeSimpleType(dtSerializable, schemaVersion, id);
        object.serialize(*this);
    }
    return *this;
}


