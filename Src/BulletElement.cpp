#include "BulletElement.hpp"

using ArsLexis::Graphics;
using ArsLexis::String;

namespace {
    const ArsLexis::char_t chrBullet=_T('\x95');
}    

BulletElement::BulletElement():
    GenericTextElement(ArsLexis::String(1, chrBullet)),
    childIndentation_(0)
{}

void BulletElement::calculateLayout(LayoutContext& mc)
{
    uint_t widthBefore=indentation()+mc.usedWidth;
    GenericTextElement::calculateLayout(mc);
    childIndentation_=(mc.usedWidth-widthBefore)+mc.preferences.bulletIndentation();
}

BulletElement::~BulletElement()
{}

