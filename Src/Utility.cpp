#include "Utility.hpp"

namespace ArsLexis 
{

    bool isWhitespace(char chr)
    {
        return chrSpace==chr || chrTab==chr || chrCarriageReturn==chr || chrLineFeed==chr;
    }
    
}

