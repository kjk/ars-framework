#include <DefinitionElement.hpp>
#include <Text.hpp>

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
    if (isHyperlink() && NULL != hyperlink_->hotSpot)
        hyperlink_->hotSpot->element_ = NULL;

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
    hyperlink_->hotSpot = NULL;
}

void DefinitionElement::defineHotSpot(Definition& definition, const Rect& bounds)
{
    assert(isHyperlink());
    if (!hyperlink_->hotSpot)
        definition.addHotSpot(hyperlink_->hotSpot = new Definition::HotSpot(bounds, *this));
    else
        hyperlink_->hotSpot->addRectangle(bounds);
}       

status_t DefinitionElement::setHyperlink(const char* str, long len, HyperlinkType type)
{
	if (-1 == len) len = Len(str);
	char* s = StringCopyN(str, len);
	if (NULL == s)
		return memErrNotEnoughSpace;

    if (!isHyperlink())
	{
        hyperlink_ = new_nt HyperlinkProperties(s, len, type);
		if (NULL == hyperlink_)
		{
			free(s);
			return memErrNotEnoughSpace;
		}
	}
    else
    {
		free(hyperlink_->resource);
        hyperlink_->resource = s;
		hyperlink_->resourceLength = len;
        hyperlink_->type = type;
    }
	return errNone;
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


DefinitionElement::HyperlinkProperties::HyperlinkProperties(char* str, ulong_t len, HyperlinkType t):
	resource(str),
	resourceLength(len),
	hotSpot(NULL),
	type(t)
{}

DefinitionElement::HyperlinkProperties::~HyperlinkProperties()
{
	free(resource);
}