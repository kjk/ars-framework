#ifndef __DEFINITIONELEMENT_HPP__
#define __DEFINITIONELEMENT_HPP__

#include "Definition.hpp"

class DefinitionElement 
{
public:

    virtual void render(Definition::RenderingContext& context)=0;

    virtual Boolean requiresNewLine() const
    {return false;}

    virtual ~DefinitionElement()
    {}
};

#endif