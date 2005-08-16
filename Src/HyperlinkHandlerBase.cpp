#include <HyperlinkHandlerBase.hpp>
#include <DefinitionElement.hpp>

#if defined(_PALM_OS)
#pragma segment Segment1
#endif

HyperlinkHandlerBase::~HyperlinkHandlerBase()
{}

void HyperlinkHandlerBase::handleHyperlink(Definition& definition, DefinitionElement& hyperlinkElement, const Point* point)
{
    assert(hyperlinkElement.isHyperlink());
    const DefinitionElement::HyperlinkProperties& props = *hyperlinkElement.hyperlinkProperties();
	if (NULL != props.resource && 0 != props.resourceLength)  
		handleHyperlink(props.resource, props.resourceLength, point);
}


void Definition::HyperlinkHandler::handleHyperlink(const char* link, ulong_t len, const Point*)
{
}

