#ifndef __FORMATTED_TEXT_ELEMENT_HPP__
#define __FORMATTED_TEXT_ELEMENT_HPP__

#include "GenericTextElement.hpp"
#include "FormattingProperties.hpp"

class FormattedTextElement: public GenericTextElement
{
    FormattingProperties formatting_;
    
protected:

    void applyFormatting(const RenderingPreferences& preferences);
    
public:

    FormattedTextElement(const ArsLexis::String& text=ArsLexis::String());

    FormattingProperties& formatting()
    {return formatting_;}
    
    const FormattingProperties& formatting() const
    {return formatting_;}
        
};

#endif