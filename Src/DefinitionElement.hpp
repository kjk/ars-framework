#ifndef __DEFINITIONELEMENT_HPP__
#define __DEFINITIONELEMENT_HPP__

#include "Definition.hpp"

class DefinitionElement 
{
public:
    
    virtual Boolean requiresNewLine(const RenderingPreferences& preferences) const
    {return false;}
    
    virtual void calculateLayout(LayoutContext& mc)=0;
    
    virtual void render(RenderingContext& rc)=0;
    
    virtual void invalidateHotSpot()
    {}

    virtual ~DefinitionElement()
    {}
    
    virtual void elementClicked(Definition& definition)
    {}
};

#endif