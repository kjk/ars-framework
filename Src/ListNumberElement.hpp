#ifndef __LIST_NUMBER_ELEMENT_HPP__
#define __LIST_NUMBER_ELEMENT_HPP__

#include "GenericTextElement.hpp"

class ListNumberElement: public GenericTextElement
{
    uint_t number_;
    uint_t totalCount_;
    uint_t childIndentation_;

protected:

    uint_t childIndentation() const
    {return childIndentation_;}
    
public:

    ListNumberElement(uint_t number);

    uint_t number() const
    {return number_;}
    
    void setTotalCount(uint_t totalCount)
    {totalCount_=totalCount;}
    
    uint_t totalCount() const
    {return totalCount_;}

    bool breakBefore(const RenderingPreferences& preferences) const
    {return true;}

    void calculateLayout(LayoutContext& mc);

    virtual void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
    {
        if (0==from)
            appendTo.append(1, _T('\n'));
        GenericTextElement::toText(appendTo, from, to);
    }
    
    void wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd);

    ~ListNumberElement();
    
    void render(RenderingContext& rc);
    
};

#endif