#ifndef __BULLET_ELEMENT_HPP__
#define __BULLET_ELEMENT_HPP__

#include "TextElement.hpp"

class BulletElement: public TextElement
{
    uint_t childIndentation_;
    
protected:

    uint_t childIndentation() const
    {return childIndentation_;}

public:
    
    
    BulletElement();

    bool breakBefore() const
    {return true;}
    
    void calculateLayout(LayoutContext& mc);
    
    virtual void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
    {
        if (from!=to)
            appendTo.append(_T("\n� "));
    }
    
    void render(RenderingContext& rc);
    
    void wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd);

    ~BulletElement();

};

#endif