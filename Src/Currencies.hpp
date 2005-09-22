#ifndef __ARSLEXIS_CURRENCIES_HPP__
#define __ARSLEXIS_CURRENCIES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

uint_t CurrencyCount();

const char_t* CurrencySymbol(uint_t pos);

const char_t* CurrencyName(uint_t pos);

const char_t* CurrencyRegion(uint_t pos);

uint_t CurrencyIndexByFirstChar(char_t inChar);

// Return index in range [0, getCurrenciesCount() ) if symbol
// is found or -1 if not.    
int CurrencyIndex(const char_t* symbol);

#endif //__ARSLEXIS_CURRENCIES