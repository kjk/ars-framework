#ifndef __DEFINITIONELEMENT_HPP__
#define __DEFINITIONELEMENT_HPP__

#include "Definition.hpp"

class DefinitionElement 
{
protected:

    DefinitionElement* parent_;
    
    DefinitionElement():
        parent_(0)
    {}
    
    virtual uint_t childIndentation() const
    {return 0;}
    
    uint_t indentation() const;
    
    virtual void invalidateHotSpot()
    {}

public:
    
    virtual bool breakBefore(const RenderingPreferences&) const
    {return false;}
    
    virtual void calculateLayout(LayoutContext& mc)=0;
    
    virtual void render(RenderingContext& rc)=0;
    
    virtual ~DefinitionElement()
    {}
    
    virtual void performAction(Definition&)
    {}
    
    void setParent(DefinitionElement* parent)
    {
        assert(parent!=this);
        parent_=parent;
    }
    
    virtual bool isTextElement() const
    {return false;}
    
    virtual void toText(ArsLexis::String& appendTo, uint_t from=0, uint_t to=LayoutContext::progressCompleted) const=0;
    
    friend class Definition::HotSpot;    
};

#endif