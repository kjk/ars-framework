#ifndef __FORMATTED_TEXT_ELEMENT_HPP__
#define __FORMATTED_TEXT_ELEMENT_HPP__

#include "GenericTextElement.hpp"
#include "FontEffects.hpp"

class FormattedTextElement: public GenericTextElement
{
    ArsLexis::FontEffects fontEffects_;
    
protected:

    void applyFormatting(ArsLexis::Graphics& graphics, const RenderingPreferences& preferences);
    
public:

    FormattedTextElement(const ArsLexis::String& text=ArsLexis::String());

    void setEffects(ArsLexis::FontEffects fx)
    {fontEffects_=fx;}
};

#endif