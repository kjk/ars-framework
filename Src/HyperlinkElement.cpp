#include "HyperlinkElement.hpp"
#include "ipedia_Rsc.h"

using ArsLexis::String;

void HyperlinkElement::invalidateHotSpot()
{
    assert(hotSpot_);
    hotSpot_=0;
}

void HyperlinkElement::defineHotSpot(Definition& definition, const ArsLexis::Rectangle& bounds)
{
    if (!hotSpot_)
    {
        hotSpot_=new Definition::HotSpot(bounds, *this);
        definition.addHotSpot(hotSpot_);
    }   
    else
        hotSpot_->addRectangle(bounds);
}       

void HyperlinkElement::prepareDrawState()
{
    WinSetUnderlineMode(grayUnderline);
}

void HyperlinkElement::elementClicked(Definition& definition)
{
}
