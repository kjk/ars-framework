#ifndef __GENERICTEXTELEMENT_HPP__
#define __GENERICTEXTELEMENT_HPP__

#include "DefinitionElement.hpp"

class GenericTextElement: public DefinitionElement
{

    ArsLexis::String text_;
    ElementStyle style_;
    
    struct HyperlinkProperties
    {
        ArsLexis::String resource;
        Definition::HotSpot* hotSpot;
        HyperlinkType type;
        
        HyperlinkProperties(const ArsLexis::String& res, HyperlinkType t);

    };
    
    HyperlinkProperties* hyperlink_;

    void defineHotSpot(Definition& definition, const ArsLexis::Rectangle& bounds);
    
protected:

    void calculateOrRender(LayoutContext& layoutContext, Coord left, Coord top, Definition* definition=0, Boolean render=false);
    
    virtual void applyFormatting(const RenderingPreferences& preferences);
    
    void applyHyperlinkDecorations(const RenderingPreferences& preferences);
    
public:

    GenericTextElement(const ArsLexis::String& text=ArsLexis::String());

    ~GenericTextElement();
    
    void calculateLayout(LayoutContext& mc);
    
    void render(RenderingContext& rc);
    
    bool isHyperlink() const
    {return hyperlink_!=0;}
    
    void setHyperlink(const ArsLexis::String& resource, HyperlinkType type);
    
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
    
    void hotSpotClicked(Definition& definition);
    
};

#endif
