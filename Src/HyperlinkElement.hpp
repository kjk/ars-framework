#ifndef __HYPERLINKELEMENT_HPP__
#define __HYPERLINKELEMENT_HPP__

#include "GenericTextElement.hpp"

class HyperlinkElement: public GenericTextElement
{
    ArsLexis::String resource_;
    Definition::HotSpot* hotSpot_;

public:
    
    HyperlinkElement(const ArsLexis::String& text, const ArsLexis::String& resource):
        GenericTextElement(text),
        resource_(resource),
        hotSpot_(0)
    {}
    
    void invalidateHotSpot();
    
    void defineHotSpot(Definition& definition, const ArsLexis::Rectangle& bounds);
    
    void prepareDrawState();
    
    void elementClicked(Definition& definition);

};

#endif