#ifndef __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__

#include <LineBufferedPayloadHandler.hpp>
#include <UniversalDataFormat.hpp>

namespace ArsLexis {
    class Reader;
    class DataStoreReader;
    class DataStoreWriter;
}

typedef std::auto_ptr<ArsLexis::DataStoreReader> DataStoreReaderPtr;
typedef std::auto_ptr<ArsLexis::DataStoreWriter> DataStoreWriterPtr;

extern void readUniversalDataFromStream(const ArsLexis::char_t* streamName, UniversalDataFormat& out);

class UniversalDataHandler: public ArsLexis::LineBufferedPayloadHandler {

    DataStoreWriterPtr      writer_;
    const ArsLexis::char_t* writerStreamName_;
    bool                    fIsWriterAvailable_;
    
    int                     lineNo;
    
protected:    
    
    ArsLexis::status_t handleLine(const ArsLexis::String& line);
    
public:    

    UniversalDataFormat universalData_;

    UniversalDataHandler();

    UniversalDataHandler(const ArsLexis::char_t* streamName);

    ~UniversalDataHandler();

};

#endif