#include <FontEffects.hpp>
#include <algorithm>
#include <Utility.hpp>

namespace ArsLexis
{

    namespace {
        static StaticAssert<sizeof(FontEffects)==sizeof(uint_t)> sizeof_FontEffects_equals_sizeof_uint;
    }

    FontEffects& FontEffects::operator+=(const FontEffects& eff)
    {
        effects_.fx.weight=std::max(weight(), eff.weight());
        effects_.fx.italic=(italic() || eff.italic());
        effects_.fx.isSmall=(isSmall() || eff.isSmall());
        effects_.fx.strike=(strikeOut() || eff.strikeOut());
        effects_.fx.superscript=(superscript() || eff.superscript());
        effects_.fx.subscript=(subscript() || eff.subscript());
        effects_.fx.underline=std::max(underline(), eff.underline());
        return *this;            
    }

}