#ifndef __ICON_ELEMENT_HPP__
#define __ICON_ELEMENT_HPP__

#include "DefinitionElement.hpp"

class IconElement: public DefinitionElement
{
    enum ImageType
    {
        imageTypeInternalById
    };

    ImageType imageType_;
    uint_t width_;
    uint_t height_;
    
    uint_t spacePx_;
    
    // for internal by id
    uint_t bitmapId_;

public:

    IconElement(uint_t bitmapId);

    bool breakBefore() const
    {return false;}

    void setSpace(uint_t newSpace)
    {
        spacePx_ = newSpace;
    }

    void calculateLayout(LayoutContext& layoutContext);
    
    void render(RenderingContext& renderingContext);
    
    virtual void toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
    {
        if (from!=to)
            appendTo.append(_T("[x]"));
    }
    
    uint_t charIndexAtOffset(LayoutContext& lc, uint_t offset);
    
    void wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd);

    ~IconElement();
    
};

#endif