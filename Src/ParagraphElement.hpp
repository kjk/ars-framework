#ifndef __PARAGRAPH_ELEMENT_HPP__
#define __PARAGRAPH_ELEMENT_HPP__

#include "LineBreakElement.hpp"

class ParagraphElement: public LineBreakElement
{

    Coord childIndentation_;

public:
    
    ParagraphElement():
        childIndentation_(0)
    {}

    Coord childIndentation() const
    {return childIndentation_;}
    
    void setChildIndentation(Coord indent)
    {childIndentation_=indent;}

};

class IndentedParagraphElement: public ParagraphElement
{
public:
    
    void calculateLayout(LayoutContext& layoutContext);

};    

#endif