#ifndef __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__

#include <UniversalDataFormat.hpp>
#include <LineBufferedPayloadHandler.hpp>

class Reader;

status_t UDF_ReadFromReader(Reader& origReader, UniversalDataFormat& out);
status_t UDF_ReadFromStream(const char* streamName, UniversalDataFormat& out);

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
