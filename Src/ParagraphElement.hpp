#ifndef __PARAGRAPH_ELEMENT_HPP__
#define __PARAGRAPH_ELEMENT_HPP__

#include "LineBreakElement.hpp"

class ParagraphElement: public LineBreakElement
{

    Coord childIndentation_;

protected:

    Coord childIndentation() const
    {return childIndentation_;}
    
public:
    
    ParagraphElement():
        childIndentation_(0)
    {}

    void setChildIndentation(Coord indent)
    {childIndentation_=indent;}

};

#endif