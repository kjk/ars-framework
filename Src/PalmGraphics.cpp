#include <PalmOS.h>
#include "Graphics.hpp"

namespace ArsLexis
{

    Graphics::Font_t Graphics::makeBold(Graphics::Font_t normal)
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