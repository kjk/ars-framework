#ifndef __GENERICTEXTELEMENT_HPP__
#define __GENERICTEXTELEMENT_HPP__

#include "DefinitionElement.hpp"

class GenericTextElement: public DefinitionElement
{

    ArsLexis::String text_;

protected:

    void calculateOrRender(LayoutContext& layoutContext, Coord left, Coord top, Definition* definition=0, Boolean render=false);
    
    virtual void defineHotSpot(Definition& definition, const ArsLexis::Rectangle& bounds)
    {}
    
    virtual void prepareDrawState()
    {}

public:

    GenericTextElement(const ArsLexis::String& text):
        text_(text)
    {}

    ~GenericTextElement();
    
    void calculateLayout(LayoutContext& mc);
    
    void render(RenderingContext& rc);
    
};

#endif
