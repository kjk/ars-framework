#include <PalmFont.hpp>

namespace ArsLexis
{

    FontID PalmFont::withEffects() const
    {
        FontID result=fontId_;
        if ((effects_.weight()!=FontEffects::weightPlain) || effects_.italic())
        {
            switch (fontId_)
            {
                case stdFont:
                    result=boldFont;
                    break;
                case largeFont:
                    result=largeBoldFont;
                    break;
            }
        }
        return result;
    }
    
}    