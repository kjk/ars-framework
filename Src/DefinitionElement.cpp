#include "DefinitionElement.hpp"

uint_t DefinitionElement::indentation() const
{
    return parent_?(parent_->indentation()+parent_->childIndentation()):0;
}

DefinitionElement::Justification DefinitionElement::justification() const
{
    return (justifyInherit!=justification_?justification_:(0!=parent_?parent_->justification():justifyLeft));
}

DefinitionElement::~DefinitionElement()
{}

void DefinitionElement::performAction(Definition&)
{}
    
