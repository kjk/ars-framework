#include "FontEffects.hpp"
#include <algorithm>

namespace ArsLexis
{

    FontEffects::FontEffects()
    {
        effects_.all=0;
    }
    
    FontEffects& FontEffects::operator+=(const FontEffects& eff)
    {
        effects_.fx.weight=std::max(weight(), eff.weight());
        effects_.fx.italic=(italic() || eff.italic());
        effects_.fx.isSsmall=(isSmall() || eff.isSmall());
        effects_.fx.strike=(strikeOut() || eff.strikeOut());
        effects_.fx.superscript=(superscript() || eff.superscript());
        effects_.fx.subscript=(subscript() || eff.subscript());
        effects_.fx.underline=std::max(underline(), eff.underline());
        return *this;            
    }

}