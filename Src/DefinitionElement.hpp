#ifndef __DEFINITIONELEMENT_HPP__
#define __DEFINITIONELEMENT_HPP__

#include "Definition.hpp"

class DefinitionElement 
{
    DefinitionElement* parent_;
    
protected:

    DefinitionElement* parent()
    {return parent_;}
    
    DefinitionElement():
        parent_(0),
        justification_(justifyInherit)
    {}
    
    virtual uint_t childIndentation() const
    {return 0;}
    
    uint_t indentation() const;
    
    virtual void invalidateHotSpot()
    {}

public:
    
    const DefinitionElement* parent() const
    {return parent_;}

    virtual bool breakBefore(const RenderingPreferences&) const
    {return false;}
    
    virtual void calculateLayout(LayoutContext& mc)=0;
    
    virtual void render(RenderingContext& rc)=0;
    
    virtual ~DefinitionElement();
    
    virtual void performAction(Definition&);
    
    void setParent(DefinitionElement* parent)
    {
        assert(parent!=this);
        parent_=parent;
    }
    
    enum Justification {
        justifyInherit, // It means "inherit from parent or use default (left)"
        justifyLeft,
        justifyCenter,
        justifyRight
    };
    
    virtual bool isTextElement() const
    {return false;}
    
    virtual void toText(ArsLexis::String& appendTo, uint_t from=0, uint_t to=LayoutContext::progressCompleted) const=0;
    
    friend class Definition::HotSpot;    
    
    void setJustification(Justification j)
    {justification_=j;}
    
    Justification justification() const;

private:
    
    Justification justification_;
    
};

#endif