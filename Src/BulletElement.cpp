#include "BulletElement.hpp"

using ArsLexis::Graphics;
using ArsLexis::String;

BulletElement::BulletElement():
    childIndentation_(0)
{}

void BulletElement::applyFormatting(Graphics& graphics, const RenderingPreferences& prefs)
{
    GenericTextElement::applyFormatting(graphics, prefs);
    graphics.setFont(symbolFont);
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
        newBullet.reserve(3); // Why not 2? Because c_str() will require space for null-terminator anyway.
        newBullet+=bullet;
        newBullet+=symbolShiftNone;
        swapText(newBullet);
    }
    uint_t widthBefore=indentation()+mc.usedWidth;
    GenericTextElement::calculateLayout(mc);
    childIndentation_=mc.usedWidth-widthBefore;
}


