#ifndef __LIST_NUMBER_ELEMENT_HPP__
#define __LIST_NUMBER_ELEMENT_HPP__

#include "GenericTextElement.hpp"

class ListNumberElement: public GenericTextElement
{
    UInt16 number_;
    UInt16 totalCount_;
    Coord childIndentation_;

protected:

    Coord childIndentation() const
    {return childIndentation_;}
    
public:

    ListNumberElement(UInt16 number);

    UInt16 number() const
    {return number_;}
    
    void setTotalCount(UInt16 totalCount)
    {totalCount_=totalCount;}
    
    UInt16 totalCount() const
    {return totalCount_;}

    Boolean requiresNewLine(const RenderingPreferences& preferences) const
    {return true;}

    void calculateLayout(LayoutContext& mc);
};

#endif