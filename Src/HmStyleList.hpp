#ifndef ARSLEXIS_HM_STYLE_LIST_HPP__
#define ARSLEXIS_HM_STYLE_LIST_HPP__

#include <ExtendedList.hpp>

namespace ArsLexis {

    class HmStyleList: public ExtendedList {
    
        RGBColorType breakColor_;

    protected:
    
        void drawItemBackground(Graphics& graphics, Rectangle& bounds, uint_t item, bool selected);
        
        void drawScrollBar(Graphics& graphics, const Rectangle& bounds);
        
    public:
    
        void setBreakColor(const RGBColorType& color) {breakColor_ = color;}
        
        const RGBColorType& greakColor() const {return breakColor_;}
    
        explicit HmStyleList(Form& form, UInt16 id=frmInvalidObjectId);
    
        ~HmStyleList();
    
    
    };

}

#endif