#include "Utility.hpp"

namespace ArsLexis 
{

    fontID getBoldFont(fontID normal)
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

}