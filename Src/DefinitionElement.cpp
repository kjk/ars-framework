#include "DefinitionElement.hpp"

Coord DefinitionElement::indentation() const
{
    return parent_?(parent_->indentation()+parent_->childIndentation()):0;
}

void DefinitionElement::hotSpotClicked(Definition& definition)
{
    
}
