#ifndef __ARSLEXIS_CURRENCIES_HPP__
#define __ARSLEXIS_CURRENCIES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    uint_t getCurrenciesCount();
    
    const char_t* getCurrencySymbol(uint_t pos);

    const char_t* getCurrencyName(uint_t pos);

    const char_t* getCurrencyRegion(uint_t pos);
    
    uint_t getCurrencyIndexByFirstChar(char_t inChar);

    // Return index in range [0, getCurrenciesCount() ) if symbol
    // is found or -1 if not.    
    int getCurrencyIndex(const char_t* symbol);
    
}

#endif //__ARSLEXIS_CURRENCIES