#ifndef __GENERICTEXTELEMENT_HPP__
#define __GENERICTEXTELEMENT_HPP__

#include "DefinitionElement.hpp"

class GenericTextElement: public DefinitionElement
{

    ArsLexis::String text_;
    ElementStyle style_;

public:
    
    struct HyperlinkProperties
    {
        ArsLexis::String resource;
        Definition::HotSpot* hotSpot;
        HyperlinkType type;
        
        HyperlinkProperties(const ArsLexis::String& res, HyperlinkType t);

    };

private:
    
    HyperlinkProperties* hyperlink_;

    void defineHotSpot(Definition& definition, const ArsLexis::Rectangle& bounds);
    
protected:

    void calculateOrRender(LayoutContext& layoutContext, uint_t left, uint_t top, Definition* definition=0, bool render=false);
    
    virtual void applyFormatting(ArsLexis::Graphics& graphics, const RenderingPreferences& preferences);
    
    void applyHyperlinkDecorations(ArsLexis::Graphics& graphics, const RenderingPreferences& preferences);
    
public:

    GenericTextElement(const ArsLexis::String& text=ArsLexis::String());

    ~GenericTextElement();
    
    void calculateLayout(LayoutContext& mc);
    
    void render(RenderingContext& rc);
    
    bool isHyperlink() const
    {return hyperlink_!=0;}
    
    void setHyperlink(const ArsLexis::String& resource, HyperlinkType type);
    
    const HyperlinkProperties* hyperlinkProperties() const 
    {return hyperlink_;}

    HyperlinkProperties* hyperlinkProperties()
    {return hyperlink_;}
    
    void setText(const ArsLexis::String& text)
    {text_=text;}
    
    const ArsLexis::String& text() const
    {return text_;}
    
    void swapText(ArsLexis::String& text)
    {text_.swap(text);}

    void invalidateHotSpot();
    
    void setStyle(ElementStyle style)
    {style_=style;}
    
    ElementStyle style() const
    {return style_;}
    
    void performAction(Definition& definition);
    
    bool isTextElement() const
    {return true;}

    void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const;
    
};

#endif
