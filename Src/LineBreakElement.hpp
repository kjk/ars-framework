#ifndef __LINE_BREAK_ELEMENT_HPP__
#define __LINE_BREAK_ELEMENT_HPP__

#include "DefinitionElement.hpp"

class LineBreakElement: public DefinitionElement
{
    void calculateOrRender(LayoutContext& layoutContext, bool render=false);

    // those scale the dy (height) of the line break element. To obtain final
    // dy we multiply by mult_ and divide by div_ the original dy.
    // i.e. mult_=1 and div_=2 => dyFinal = dyOrig / 2
    int mult_;
    int div_;

public:

    LineBreakElement() : DefinitionElement(), mult_(1), div_(1)
    {}

    LineBreakElement(int mult, int div) : DefinitionElement(), mult_(mult), div_(div)
    {}

    void setSize(int mult, int div);

    bool breakBefore(const RenderingPreferences& preferences) const
    {return true;}

    void calculateLayout(LayoutContext& mc)
    {calculateOrRender(mc);}
    
    void render(RenderingContext& rc)
    {calculateOrRender(rc, true);}

    virtual void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
    {
        if (from!=to)
            appendTo.append(1, _T('\n'));
    }

    uint_t charIndexAtOffset(LayoutContext& lc, uint_t offset);
    
    void wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd);
    
    ~LineBreakElement();
    
};

#endif
