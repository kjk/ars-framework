#ifndef __PARAGRAPH_ELEMENT_HPP__
#define __PARAGRAPH_ELEMENT_HPP__

#include "LineBreakElement.hpp"

class ParagraphElement: public LineBreakElement
{

    uint_t childIndentation_;

protected:

    void setChildIndentation(uint_t indent)
    {childIndentation_=indent;}

    uint_t childIndentation() const
    {return childIndentation_;}
    
public:
    
    ParagraphElement():
        childIndentation_(0)
    {}

};

class IndentedParagraphElement: public ParagraphElement
{
public:
    
    void calculateLayout(LayoutContext& layoutContext);

};    

#endif