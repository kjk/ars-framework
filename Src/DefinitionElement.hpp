#ifndef __DEFINITIONELEMENT_HPP__
#define __DEFINITIONELEMENT_HPP__

#include "Definition.hpp"

typedef void action_callback(void *data);

class DefinitionElement 
{
    DefinitionElement* parent_;
    
protected:

    DefinitionElement* parent()
    {return parent_;}
    
    DefinitionElement();
    
    virtual uint_t childIndentation() const
    {return 0;}
    
    uint_t indentation() const;
    
    void defineHotSpot(Definition& definition, const Rectangle& bounds);
    
    void invalidateHotSpot();
    
public:
    
    struct HyperlinkProperties
    {
        ArsLexis::String resource;
        Definition::HotSpot* hotSpot;
        HyperlinkType type;
        
        HyperlinkProperties(const ArsLexis::String& res, HyperlinkType t);

    };

private:
    
    // actionCallback_ is being called when hotspot is hit. It takes precedence
    // over hyperlink handler (because we need to mark the text as hyperlink 
    // in order to have a hotspot; this is a bit hackish)
    action_callback * actionCallback_;
    void *            actionCallbackData_;
    HyperlinkProperties* hyperlink_;
    
public:
    
    bool isHyperlink() const
    {return NULL != hyperlink_;}
    
    void setHyperlink(const ArsLexis::String& resource, HyperlinkType type);
    
    const HyperlinkProperties* hyperlinkProperties() const 
    {return hyperlink_;}

    void setActionCallback(action_callback *actionCb, void *data)
    {
        actionCallback_ = actionCb;
        actionCallbackData_ = data;
    }

    const DefinitionElement* parent() const
    {return parent_;}

    virtual bool breakBefore() const
    {return false;}
    
    virtual void calculateLayout(LayoutContext& mc)=0;
    
    virtual void render(RenderingContext& rc)=0;
    
    virtual ~DefinitionElement();
    
    void performAction(Definition&, const Point* point);
    
    void setParent(DefinitionElement* parent)
    {
        assert(parent!=this);
        parent_=parent;
    }
    
    enum Justification {
        justifyInherit, // It means "inherit from parent or use default (left)"
        justifyLeft,
        justifyCenter,
        justifyRight,
        justifyRightLastElementInLine //only one - last element can be justified like this!
    };
    
    virtual bool isTextElement() const
    {return false;}
    
    virtual void toText(ArsLexis::String& appendTo, uint_t from=0, uint_t to=LayoutContext::progressCompleted) const=0;
    
    enum {offsetOutsideElement = uint_t(-1)};
    virtual uint_t charIndexAtOffset(LayoutContext& lc, uint_t offset) = 0;
    
    virtual void wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd) = 0;
    
    friend class Definition::HotSpot;    
    
    void setJustification(Justification j)
    {justification_=j;}
    
    Justification justification() const;

//    virtual void setStyle(ElementStyle style) = 0;

    const DefinitionStyle* getStyle() const;

    enum StyleOwnerFlag
    {
        ownStyleNot,
        ownStyle
    };     
    
    void setStyle(const DefinitionStyle* style, StyleOwnerFlag own=ownStyleNot);
   
private:
    
    DefinitionStyle* definitionStyle_;
    
    StyleOwnerFlag styleOwner_;
    
    Justification justification_;
    
};

#endif