#include "FontEffects.hpp"
#include <algorithm>

namespace ArsLexis
{

    FontEffects::FontEffects():
        weight_(weightPlain),
//        typewriter_(false),
        italic_(false),
        small_(false),
        strike_(false),
        superscript_(false),
        subscript_(false),
        underline_(underlineNone)
    {}
    
    FontEffects& FontEffects::operator+=(const FontEffects& eff)
    {
        weight_=std::max(weight(), eff.weight());
        italic_=(italic_ || eff.italic_);
        small_=(small_ || eff.small_);
        strike_=(strike_ || eff.strike_);
        superscript_=(superscript_ || eff.superscript_);
        subscript_=(subscript_ || eff.subscript_);
        underline_=std::max(underline(), eff.underline());
        return *this;            
    }

}