#ifndef ARSLEXIS_HM_STYLE_LIST_HPP__
#define ARSLEXIS_HM_STYLE_LIST_HPP__

#include <ExtendedList.hpp>

class HmStyleList: public ExtendedList {

    RGBColorType breakColor_;

protected:

    void drawItemBackground(Graphics& graphics, ArsRectangle& bounds, uint_t item, bool selected);
    
    void drawScrollBar(Graphics& graphics, const ArsRectangle& bounds);
    
public:

    void setBreakColor(const RGBColorType& color) {breakColor_ = color;}
    
    const RGBColorType& breakColor() const {return breakColor_;}

    explicit HmStyleList(Form& form, UInt16 id=frmInvalidObjectId);

    ~HmStyleList();

};

#endif