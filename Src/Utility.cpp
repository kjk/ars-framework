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

    bool isWhitespace(char chr)
    {
        return chrSpace==chr || chrTab==chr || chrCarriageReturn==chr || chrLineFeed==chr;
    }
    
    void drawCenteredChars(const char* str, Coord left, Coord top, Coord width)
    {
        assert(str!=0);
        UInt16 len=StrLen(str);
        Int16 charsWidth=FntCharsWidth(str, len);
        left+=(width-charsWidth)/2;
        WinDrawChars(str, len, left, top);
    }


}

