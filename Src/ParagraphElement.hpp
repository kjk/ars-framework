#ifndef __PARAGRAPH_ELEMENT_HPP__
#define __PARAGRAPH_ELEMENT_HPP__

#include "LineBreakElement.hpp"

class ParagraphElement : public LineBreakElement
{

    uint_t childIndentation_;

protected:

    void setChildIndentation(uint_t indent)
    {childIndentation_=indent;}

    uint_t childIndentation() const
    {return childIndentation_;}
    
public:

    bool   fIndented;

    ParagraphElement(bool fIndentedArg=false) : 
        fIndented(fIndentedArg),
        childIndentation_(0)
    {}

    void calculateLayout(LayoutContext& layoutContext)
    {
        if (fIndented)
            setChildIndentation(layoutContext.preferences.standardIndentation());
        LineBreakElement::calculateLayout(layoutContext);

    }

    ~ParagraphElement() {};
    
};

#endif
