#include <LookupManagerBase.hpp>
#include <Graphics.hpp>

namespace ArsLexis {

    void LookupProgressReportingSupport::showProgress(Graphics& graphics, const Rectangle& bounds) const
    {
        graphics.erase(bounds);
        Rectangle rect(bounds);
        rect.explode(2, 2, -4, -4);
        Graphics::FontSetter setFont(graphics, Font());
        uint_t length=statusText_.length();
        uint_t width=rect.width();
        const char* text=statusText_.c_str();
        graphics.charsInWidth(text, length, width);
        uint_t height=graphics.fontHeight();
        Point p(rect.x(), rect.y()+(rect.height()-height)/2);
        graphics.drawText(text, length, p);
        if (percentProgressDisabled!=percentProgress_)
        {
            assert(percentProgress_<=100);
            p.x+=width+2;
            char_t buffer[8];
            length=tprintf(buffer, _T(" %hd%%"), percentProgress_);
            width=rect.width()-width;
            graphics.charsInWidth(buffer, length, width);
            graphics.drawText(buffer, length, p);
        }
    }

}

