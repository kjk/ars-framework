#include "DefinitionElement.hpp"

uint_t DefinitionElement::indentation() const
{
    return parent_?(parent_->indentation()+parent_->childIndentation()):0;
}
