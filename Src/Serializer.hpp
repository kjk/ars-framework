#ifndef ARSLEXIS_SERIALIZER_HPP__
#define ARSLEXIS_SERIALIZER_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <map>

class Reader;
class Writer;
class Serializer;
class BufferedReader;

struct DynStrTag;

class Serializable {

public:

    enum {unusedIdVer0 = ulong_t(0xffff), unusedId = ulong_t(-1)};

    virtual void serialize(Serializer& ser) = 0;
    
    virtual ulong_t schemaVersion() const;

    virtual bool serializeInFromVersion(Serializer& ser, ulong_t version);
    
    virtual ~Serializable();

};

class Serializer {

	ulong_t version_;
	
    BufferedReader* reader_;
    Writer* writer_;
    bool isIndexed_;
    bool skipLastRecord_;
    
    typedef std::map<std::uint32_t, ulong_t> RecordIndex_t;
    RecordIndex_t recordIndex_;
    
    bool indexNextRecord();

    void loadIndex();
    
    void assureIndexed() { if (!isIndexed_) loadIndex();}
   
   void readVersion();
   
   bool oldVersion() const {return 0 == version_;} 
   
   enum {unusedIdVer0 = Serializable::unusedIdVer0};

public:

	enum {currentVersion = 1}; // version 0 is what is used in PalmOS InfoMan up to 1.3

    enum Direction {
        directionInput,
        directionOutput
    };

    explicit Serializer(Reader& reader);

    explicit Serializer(Writer& reader);

    Serializer(Reader& reader, Writer& writer, Direction dir);

    ~Serializer();

    Direction direction() const {return direction_;}
    
    void setDirection(Direction dir) {direction_ = dir;} 

    enum Error {
        errClass = serializerErrorClass,
        errCorrupted = errClass,
        errNotEnoughSpace,
        errBufferTooSmall,
    };
    
    enum {unusedId = Serializable::unusedId};
    
    Serializer& operator()(bool& value, ulong_t id = unusedId);

    Serializer& operator()(signed char& value, ulong_t id = unusedId);

    Serializer& operator()(unsigned char& value, ulong_t id = unusedId);

    Serializer& operator()(signed short& value, ulong_t id = unusedId);

    Serializer& operator()(unsigned short& value, ulong_t id = unusedId);

    Serializer& operator()(signed int& value, ulong_t id = unusedId);

    Serializer& operator()(unsigned int& value, ulong_t id = unusedId);

    Serializer& operator()(signed long& value, ulong_t id = unusedId);

    Serializer& operator()(unsigned long& value, ulong_t id = unusedId);
   
    
/*    
   
	Serializer& operator()(NarrowString& value, uint_t id = unusedId);
    
    Serializer& operator()(char array[], uint_t arraySize, uint_t id = unusedId);
   
    
    Serializer& operator()(DynStrTag& value, uint_t id = unusedId);
    
  
    
    Serializer& operator()(String& value, uint_t id = unusedId);
    
    Serializer& operator()(char_t array[], uint_t arraySize, uint_t id = unusedId);
    
    Serializer& operator()(DynStrTag& value, uint_t id = unusedId);
    
 */    

	// Serializes passed data as a blob, without any transformation
	Serializer& binary(NarrowString& value, ulong_t id = unusedId);
	Serializer& binary(void* array, ulong_t size, ulong_t = unusedId);
	
	// Converts passed text to UTF-8 encoding prior to serialization
	Serializer& text(String& value, ulong_t id = unusedId);
	Serializer& text(char_t* array, ulong_t size, ulong_t = unusedId); 
    
    Serializer& operator()(Serializable& value, ulong_t id = unusedId);
    
private:

    enum DataType {
        dtBool, 
        dtChar,
        dtUChar, 
        dtShort,
        dtUShort,
        dtInt,
        dtUInt,
        dtLong,
        dtULong,
        dtStringVer0,
        dtSerializable,
        dtBlob,
        dtText
    };
    
    struct Record {
    
        union {
            DataType type;
            std::uint32_t fill_;
        };
        
        std::uint32_t id;
        
        union {
            std::uint32_t value;
            std::uint32_t stringLength;
            std::uint32_t objectVersion;
        };
        
        Record() {}
        
        Record(DataType dt, ulong_t i): type(dt), id(i) {}

    };
    
    void serializeChunk(void* buffer, ulong_t length);
    
    void serializeRecord(Record& record);
    
    void serializeRecordIn(Record& record);
    
    void serializeRecordOut(Record& record);
    
    template<class T, DataType dt>
    Serializer& serializeSimpleType(T& value, ulong_t id)
    {
        Record record(dt, id);
        record.value = value;
        serializeRecord(record);
        if (directionInput == direction_)
            value = record.value;
        return *this;
    }
    
    Direction direction_;

/*    
    void serializeValue(bool& value);
    
    void serializeValue(signed char& value);

    void serializeValue(unsigned char& value);

    void serializeValue(signed short& value);

    void serializeValue(unsigned short& value);

    void serializeValue(signed int& value);

    void serializeValue(unsigned int& value);

    void serializeValue(signed long& value);

    void serializeValue(unsigned long& value);
    
    void serializeValue(String& value);
    
    void serializeValue(Serializable& value);
    
    void serializeValue(DynStr& value);

*/
};

#endif