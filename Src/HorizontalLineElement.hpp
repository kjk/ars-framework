#ifndef __HORIZONTAL_LINE_ELEMENT_HPP__
#define __HORIZONTAL_LINE_ELEMENT_HPP__

#include "DefinitionElement.hpp"

class HorizontalLineElement: public DefinitionElement
{
public:

    HorizontalLineElement()
    {}

    Boolean requiresNewLine(const RenderingPreferences& preferences) const
    {return true;}

    void calculateLayout(LayoutContext& layoutContext);
    
    void render(RenderingContext& renderingContext);
    
};

#endif