#ifndef __ARSLEXIS_INTERNATIONAL_COUNTRY_CODES_HPP__
#define __ARSLEXIS_INTERNATIONAL_COUNTRY_CODES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    int getCountryCodesCount();
    
    const char_t* getCountryName(int pos);
    
    const char_t* getCountryCode(int pos);
    
    int getCountryIndexByFirstChar(char_t inChar);
    
}

#endif