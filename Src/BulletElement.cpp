#include "BulletElement.hpp"

using ArsLexis::Graphics;
using ArsLexis::String;

#if defined(_WIN32_WCE)
namespace {
    const TCHAR symbolShiftPunc=_T('*');
    const TCHAR symbolDiamondChr=_T('#');
}
#endif

BulletElement::BulletElement():
    childIndentation_(0)
{}

void BulletElement::applyFormatting(Graphics& graphics, const RenderingPreferences& prefs)
{
    GenericTextElement::applyFormatting(graphics, prefs);
    graphics.setFont(Graphics::Font_t::getSymbolFont());
}

void BulletElement::calculateLayout(LayoutContext& mc)
{
    char bullet=0;
    switch (mc.preferences.bulletType())
    {
        case RenderingPreferences::bulletCircle:
            bullet=symbolShiftPunc;
            break;
        case RenderingPreferences::bulletDiamond:
            bullet=symbolDiamondChr;
            break;
        default:
            assert(false);
    }
    if (text()[0]!=bullet)
    {
        String newBullet;
        newBullet.reserve(2); // Why not 1? Because c_str() will require space for null-terminator anyway.
        newBullet+=bullet;
        swapText(newBullet);
    }
    uint_t widthBefore=indentation()+mc.usedWidth;
    GenericTextElement::calculateLayout(mc);
    childIndentation_=(mc.usedWidth-widthBefore)+mc.preferences.bulletIndentation();
}


