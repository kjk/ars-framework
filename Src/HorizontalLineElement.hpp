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
    
    void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
    {
        if (from!=to)
            appendTo.append(_T("\n----"));
    }
    
    ~HorizontalLineElement();
    
};

#endif