#ifndef __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__
#define __ARSLEXIS_UNIVERSAL_DATA_HANDLER_HPP__

#include <LineBufferedPayloadHandler.hpp>
#include <UniversalDataFormat.hpp>

namespace ArsLexis {
    class Reader;
}

extern ArsLexis::status_t readUniversalDataFromStream(ArsLexis::Reader& reader, UniversalDataFormat& out);

class UniversalDataHandler: public ArsLexis::LineBufferedPayloadHandler {

    UniversalDataFormat universalData_;
    
    int                 lineNo;
    
protected:    
    
    ArsLexis::status_t handleLine(const ArsLexis::String& line);
    
public:    

    UniversalDataHandler();

    ~UniversalDataHandler();

};

#endif