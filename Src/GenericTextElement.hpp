#ifndef __GENERICTEXTELEMENT_HPP__
#define __GENERICTEXTELEMENT_HPP__

#include "DefinitionElement.hpp"

class GenericTextElement: public DefinitionElement
{

    ArsLexis::String text_;
    FontID fontId_;
    UnderlineModeType underlineMode_;

protected:

    void calculateOrRender(LayoutContext& layoutContext, Coord left, Coord top, Boolean render=false);

public:

    GenericTextElement(const ArsLexis::String& text, FontID fontId=stdFont, UnderlineModeType underlineMode=noUnderline):
        text_(text),
        fontId_(fontId),
        underlineMode_(underlineMode)
    {}

    ~GenericTextElement();
    
    void calculateLayout(LayoutContext& mc);
    
    void render(RenderContext& rc);
    
};

#endif
