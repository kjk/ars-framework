#ifndef __ARSLEXIS_INTERNATIONAL_COUNTRY_CODES_HPP__
#define __ARSLEXIS_INTERNATIONAL_COUNTRY_CODES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    int getInternationalCountryCodesCount();
    
    void getInternationalCountryName(int pos,ArsLexis::String& nameOut);
    
    void getInternationalCountryCode(int pos, ArsLexis::String& symbolOut);
    
    int getInternationalCountryIndexByFirstChar(ArsLexis::char_t inChar);
    
}

#endif