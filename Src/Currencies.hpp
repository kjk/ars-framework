#ifndef __ARSLEXIS_CURRENCIES_HPP__
#define __ARSLEXIS_CURRENCIES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    enum {
        currenciesCount = 141
    };

    class Currency
    {
        ArsLexis::String abbrev_;
        ArsLexis::String name_;
        ArsLexis::String countries_; //Maybe list in the feature, for a while comma separated list of countries

    public:
    
        Currency(const String& name, const String& abbrev, const String& countries):
            abbrev_(abbrev),
            name_(name),
            countries_(countries)
        {}
        
        void getName(String& nameOut) {nameOut = name_;}
        void getAbbrev(String& abbrevOut) {abbrevOut = abbrev_;}
        void getCountries(String& countriesOut) {countriesOut = countries_;}
        
    };
    
    uint_t getCurrenciesCount();
    
    Currency getCurrency(int pos);
    
    int getCurrencyIndexByFirstChar(char_t inChar);
    
}

#endif //__ARSLEXIS_CURRENCIES