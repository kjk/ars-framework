#include "BulletElement.hpp"

BulletElement::BulletElement():
    childIndentation_(0)
{}

void BulletElement::applyFormatting(const RenderingPreferences& prefs)
{
    GenericTextElement::applyFormatting(prefs);
    FntSetFont(symbolFont);
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
        ArsLexis::String newBullet;
        newBullet.reserve(3); // Why not 2? Because c_str() will require space for null-terminator anyway.
        newBullet+=bullet;
        newBullet+=symbolShiftNone;
        swapText(newBullet);
    }
    uint_t widthBefore=indentation()+mc.usedWidth;
    GenericTextElement::calculateLayout(mc);
    childIndentation_=mc.usedWidth-widthBefore;
}


