#include "BulletElement.hpp"

using ArsLexis::String;

namespace {
    const ArsLexis::char_t chrBullet=_T('\x95');
}    

BulletElement::BulletElement():
    TextElement(ArsLexis::String(1, chrBullet)),
    childIndentation_(0)
{}

void BulletElement::calculateLayout(LayoutContext& mc)
{
    uint_t widthBefore=indentation()+mc.usedWidth;
    TextElement::calculateLayout(mc);
    childIndentation_=(mc.usedWidth - widthBefore) + 2;
}

BulletElement::~BulletElement()
{}

void BulletElement::render(RenderingContext& rc) 
{
    // Prevent bullets from being displayed in reverse video while selected.
    uint_t selStart = rc.selectionStart;
    uint_t selEnd = rc.selectionEnd;
    rc.selectionStart = rc.selectionEnd = LayoutContext::progressCompleted;
    TextElement::render(rc);
    rc.selectionStart = selStart;
    rc.selectionEnd = selEnd;
}

void BulletElement::wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd)
{
    wordStart = wordEnd = offsetOutsideElement;
}
