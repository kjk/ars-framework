#ifndef __LINE_BREAK_ELEMENT_HPP__
#define __LINE_BREAK_ELEMENT_HPP__

#include "DefinitionElement.hpp"

class LineBreakElement: public DefinitionElement
{
    void calculateOrRender(LayoutContext& layoutContext, Boolean render=false);

public:
    
    Boolean requiresNewLine(const RenderingPreferences& preferences) const
    {return true;}

    void calculateLayout(LayoutContext& mc)
    {calculateOrRender(mc);}
    
    void render(RenderingContext& rc)
    {calculateOrRender(rc, true);}

};

#endif