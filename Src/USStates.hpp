#ifndef __ARSLEXIS_USSTATES_HPP__
#define __ARSLEXIS_USSTATES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    int getStatesCount();

    char_t *getStateName(int pos);

    char_t *getStateSymbol(int pos);

    int getIndexByFirstChar(ArsLexis::char_t inChar);
    
}

#endif