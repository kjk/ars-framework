#ifndef __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__

#include <UniversalDataFormat.hpp>
#include <LineBufferedPayloadHandler.hpp>

namespace ArsLexis {
    class Reader;
    class DataStoreReader;
    class DataStoreWriter;
}

typedef std::auto_ptr<ArsLexis::DataStoreReader> DataStoreReaderPtr;
typedef std::auto_ptr<ArsLexis::DataStoreWriter> DataStoreWriterPtr;

extern void readUniversalDataFromStream(const ArsLexis::char_t* streamName, UniversalDataFormat& out);

class UniversalDataHandler: public ArsLexis::LineBufferedPayloadHandler {

    int                     lineNo_;
    long                    controlDataLength_;
    
protected:    
    
    ArsLexis::status_t handleLine(const ArsLexis::String& line);

    ArsLexis::status_t handlePayloadFinish();

public:    

    UniversalDataFormat universalData;

    UniversalDataHandler();

    ~UniversalDataHandler();

    ArsLexis::status_t handleIncrement(const ArsLexis::String& payload, ulong_t& length, bool finish);
};

#endif