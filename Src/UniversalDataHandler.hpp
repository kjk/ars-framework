#ifndef __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__

#include <UniversalDataFormat.hpp>
#include <LineBufferedPayloadHandler.hpp>

class Reader;

class DataStoreReader;
class DataStoreWriter;

typedef std::auto_ptr<DataStoreReader> DataStoreReaderPtr;
typedef std::auto_ptr<DataStoreWriter> DataStoreWriterPtr;

extern ArsLexis::status_t readUniversalDataFromReader(Reader& origReader, UniversalDataFormat& out);
extern void readUniversalDataFromStream(const ArsLexis::char_t* streamName, UniversalDataFormat& out);

class UniversalDataHandler: public LineBufferedNarrowProcessor {

    ulong_t                 lineNo_;
    ulong_t                 controlDataLength_;
    
protected:    
    
    status_t handleLine(const char* line, ulong_t len);

    status_t handlePayloadFinish();

public:    

    UniversalDataFormat universalData;

    UniversalDataHandler();

    ~UniversalDataHandler();

    status_t handleIncrement(const char* payload, ulong_t& length, bool finish);
};

#endif
