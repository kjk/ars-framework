#include <PalmFont.hpp>

namespace ArsLexis
{

    FontID PalmFont::withEffects() const
    {
        FontID mask = FontID(fontId_ & fontScalingDisabled);
        FontID result = FontID(fontId_ & ~fontScalingDisabled);
        if ((effects_.weight() != FontEffects::weightPlain) || effects_.italic())
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
        return FontID(result | mask);
    }
    
}    