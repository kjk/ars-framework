#ifndef __HORIZONTAL_LINE_ELEMENT_HPP__
#define __HORIZONTAL_LINE_ELEMENT_HPP__

#include "DefinitionElement.hpp"

class HorizontalLineElement: public DefinitionElement
{
public:

    HorizontalLineElement()
    {}

    bool breakBefore(const RenderingPreferences& preferences) const
    {return true;}

    void calculateLayout(LayoutContext& layoutContext);
    
    void render(RenderingContext& renderingContext);
    
    virtual void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
    {
        if (from!=to)
            appendTo.append(_T("\n----"));
    }
    
    uint_t charIndexAtOffset(LayoutContext& lc, uint_t offset);
    
    void wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd);

    ~HorizontalLineElement();
    
    void setHyperlink(const ArsLexis::String& resource, HyperlinkType type)
    {}

    void setStyle(ElementStyle style)
    {}
    
};

#endif