#ifndef __ARSLEXIS_USSTATES_HPP__
#define __ARSLEXIS_USSTATES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis {

    int getStatesCount();
    
    void getStateName(int pos,ArsLexis::String& nameOut);
    
    void getStateSymbol(int pos, ArsLexis::String& symbolOut);
    
}

#endif