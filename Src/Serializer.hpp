#ifndef ARSLEXIS_SERIALIZER_HPP__
#define ARSLEXIS_SERIALIZER_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    class Reader;
    class Writer;
    class Serializer;
    
    class Serializable {
    
    public:
    
        enum {unusedId = uint_t(-1)};

        virtual void serialize(Serializer& ser) = 0;
        
        virtual uint_t schemaVersion() const;
    
        virtual bool serializeInFromVersion(Serializer& ser, uint_t version);
        
        virtual ~Serializable();
    
    };
    
    class Serializer {
    
        Reader* reader_;
        Writer* writer_;
    
    public:
    
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
        
        Serializer& operator()(bool& value, uint_t id = unusedId);

        Serializer& operator()(signed char& value, uint_t id = unusedId);

        Serializer& operator()(unsigned char& value, uint_t id = unusedId);

        Serializer& operator()(signed short& value, uint_t id = unusedId);

        Serializer& operator()(unsigned short& value, uint_t id = unusedId);

        Serializer& operator()(signed int& value, uint_t id = unusedId);

        Serializer& operator()(unsigned int& value, uint_t id = unusedId);

        Serializer& operator()(signed long& value, uint_t id = unusedId);

        Serializer& operator()(unsigned long& value, uint_t id = unusedId);
        
        Serializer& operator()(String& value, uint_t id = unusedId);
        
        Serializer& operator()(char_t array[], uint_t arraySize, uint_t id = unusedId);
        
        Serializer& operator()(Serializable& value, uint_t id = unusedId);
        
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
            dtString,
            dtSerializable,
            dtIsSequence = 0x8000
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
            
            Record(DataType dt, uint_t i): type(dt), id(i) {}
            
        };
        
        void serializeChunk(void* buffer, uint_t length);
        
        void serializeRecord(Record& record);
        
        void serializeRecordIn(Record& record);
        
        void serializeRecordOut(Record& record);
        
        template<class T, DataType dt>
        Serializer& serializeSimpleType(T& value, uint_t id)
        {
            Record record(dt, id);
            if (directionOutput == direction_)
                record.value = value;
            serializeRecord(record);
            if (directionInput == direction_)
                value = record.value;
            return *this;
        }
        
        Direction direction_;
        
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
    
    };

}

#endif