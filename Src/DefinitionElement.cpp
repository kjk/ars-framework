#include "DefinitionElement.hpp"

DefinitionElement::DefinitionElement():
    parent_(0),
    justification_(justifyInherit),
    hyperlink_(NULL),
    actionCallback_(NULL),
    actionCallbackData_(NULL),
    styleOwner_(ownStyleNot),
    definitionStyle_(NULL)
{}


uint_t DefinitionElement::indentation() const
{
    return parent_?(parent_->indentation()+parent_->childIndentation()):0;
}

DefinitionElement::Justification DefinitionElement::justification() const
{
    return (justifyInherit!=justification_?justification_:(0!=parent_?parent_->justification():justifyLeft));
}

DefinitionElement::~DefinitionElement()
{
    delete hyperlink_;
    if (ownStyle == styleOwner_)
        delete definitionStyle_;
}

void DefinitionElement::performAction(Definition& definition, const Point* point)
{
    assert(isHyperlink());
    // actionCallback_ takes precedence over hyperlink handler and we don't
    // call hyperlink handler if actionCallback_ is present
    if (actionCallback_)
    {
        (*actionCallback_)(actionCallbackData_);
    }
    else
    {
        Definition::HyperlinkHandler* handler = definition.hyperlinkHandler();
        if (handler) 
            handler->handleHyperlink(definition, *this, point);
    }

}
    
    
void DefinitionElement::invalidateHotSpot()
{
    assert(isHyperlink());
    hyperlink_->hotSpot=0;
}

void DefinitionElement::defineHotSpot(Definition& definition, const ArsRectangle& bounds)
{
    assert(isHyperlink());
    if (!hyperlink_->hotSpot)
        definition.addHotSpot(hyperlink_->hotSpot = new Definition::HotSpot(bounds, *this));
    else
        hyperlink_->hotSpot->addRectangle(bounds);
}       

void DefinitionElement::setHyperlink(const ArsLexis::String& resource, HyperlinkType type)
{
    if (!isHyperlink())
        hyperlink_=new HyperlinkProperties(resource, type);
    else
    {
        hyperlink_->resource=resource;
        hyperlink_->type=type;
    }
}

void DefinitionElement::setHyperlink(const char_t* str, HyperlinkType type)
{
    if (!isHyperlink())
        hyperlink_ = new HyperlinkProperties(str, type);
    else
    {
        hyperlink_->resource = str;
        hyperlink_->type = type;
    }
}

void DefinitionElement::setStyle(const DefinitionStyle* style, StyleOwnerFlag own)
{
    if (ownStyle == styleOwner_)
        delete definitionStyle_;
    definitionStyle_ = (DefinitionStyle*) style;
    styleOwner_ = own;
}    

const DefinitionStyle* DefinitionElement::getStyle() const
{
    return definitionStyle_;
}    
