#ifndef __ARSLEXIS_CURRENCIES_HPP__
#define __ARSLEXIS_CURRENCIES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

#define CURRENCY_COUNT 137

namespace ArsLexis {

    class Currency
    {
        ArsLexis::String abbrev_;
        ArsLexis::String name_;
        ArsLexis::String countries_; //Maybe list in the feature, for a while comma separated list of countries

    public:
        Currency(ArsLexis::String name, ArsLexis::String abbrev, ArsLexis::String countries):
            abbrev_(abbrev),
            name_(name),
            countries_(countries)
        {}
        void getName(ArsLexis::String& nameOut) {nameOut = name_;}
        void getAbbrev(ArsLexis::String& abbrevOut) {abbrevOut = abbrev_;}
        void getCountries(ArsLexis::String& countriesOut) {countriesOut = countries_;}
    };
    
    uint_t getCurrenciesCount();
    ArsLexis::Currency getCurrency(int pos);
}

#endif //__ARSLEXIS_CURRENCIES