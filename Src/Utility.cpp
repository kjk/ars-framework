#include "Utility.hpp"

namespace ArsLexis 
{

    FontID getBoldFont(FontID normal)
    {
        fontID result=normal;
        switch (normal)
        {
            case stdFont:
                result=boldFont;
                break;
            case largeFont:
                result=largeBoldFont;
                break;
        }
        return result;
    }

    
    //! @todo Implement ArsLexis::trim(String&).
    void trim(String& string)
    {
    
    }
    
    Boolean isWhitespace(char chr)
    {
        return chrSpace==chr || chrTab==chr || chrCarriageReturn==chr || chrLineFeed==chr;
    }

}

