#ifndef __GENERICTEXTELEMENT_HPP__
#define __GENERICTEXTELEMENT_HPP__

#include "DefinitionElement.hpp"

class GenericTextElement: public DefinitionElement
{

    ArsLexis::String text_;
    ElementStyle     style_;

    void drawTextWithSelection(Graphics& graphics, uint_t start, uint_t end, uint_t selectionStart, uint_t selectionEnd, const Rectangle& area, bool hyperlink);


protected:

    void calculateOrRender(LayoutContext& layoutContext, uint_t left, uint_t top, Definition* definition=0, bool render=false);
    
    virtual void applyFormatting(Graphics& graphics, const RenderingPreferences& preferences);
    
    void applyHyperlinkDecorations(Graphics& graphics, const RenderingPreferences& preferences);
    
 
public:

    GenericTextElement(const ArsLexis::String& text=ArsLexis::String());
    
    GenericTextElement(const ArsLexis::char_t* text);

    ~GenericTextElement();
    
    void calculateLayout(LayoutContext& mc);
    
    void render(RenderingContext& rc);
    
    void setText(const ArsLexis::String& text)
    {text_=text;}
    
    const ArsLexis::String& text() const
    {return text_;}
    
    void swapText(ArsLexis::String& text)
    {text_.swap(text);}

    void setStyle(ElementStyle style)
    {style_=style;}
    
    ElementStyle style() const
    {return style_;}
    
    bool isTextElement() const
    {return true;}

    virtual void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const;

    uint_t charIndexAtOffset(LayoutContext& lc, uint_t offset);

    void wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd);

};

#endif
