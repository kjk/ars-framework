#ifndef __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__

#include <UniversalDataFormat.hpp>
#include <FieldPayloadProtocolConnection.hpp>

namespace ArsLexis {
    class Reader;
    class DataStoreReader;
    class DataStoreWriter;
}

typedef std::auto_ptr<ArsLexis::DataStoreReader> DataStoreReaderPtr;
typedef std::auto_ptr<ArsLexis::DataStoreWriter> DataStoreWriterPtr;

extern void readUniversalDataFromStream(const ArsLexis::char_t* streamName, UniversalDataFormat& out);

//we moved back to FieldPaloadProtocolConnection to add handlePayloadFinish()
class UniversalDataHandler: public ArsLexis::FieldPayloadProtocolConnection::PayloadHandler {

    ArsLexis::String        lineBuffer_;
    ArsLexis::char_t        delimiter_;

    DataStoreWriterPtr      writer_;
    const ArsLexis::char_t* writerStreamName_;
    int                     lineNo_;
    long                    controlDataLength_;
    
protected:    
    
    ArsLexis::status_t handleLine(const ArsLexis::String& line);

    ArsLexis::status_t handlePayloadFinish();

    void setDelimiter(ArsLexis::char_t delimiter)
    {delimiter_=delimiter;}
    
public:    

    UniversalDataFormat universalData;

    UniversalDataHandler();

    UniversalDataHandler(const ArsLexis::char_t* streamName);

    ~UniversalDataHandler();

    ArsLexis::status_t handleIncrement(const ArsLexis::String& payload, ulong_t& length, bool finish);
};

#endif