#include <LookupManagerBase.hpp>
#include <Graphics.hpp>
#include <BaseTypes.hpp>
#include <SysUtils.hpp>

namespace ArsLexis {
    
    void LookupProgressReportingSupport::showProgress(Graphics& graphics, const Rectangle& bounds) const
    {
        graphics.erase(bounds);
        Rectangle rect(bounds);
        rect.explode(2, 2, -4, -4);
        Graphics::FontSetter setFont(graphics, Font());
        uint_t length=statusText_.length();
        uint_t width=rect.width();
        const char_t* text=statusText_.c_str();
        graphics.charsInWidth(text, length, width);
        uint_t height=graphics.fontHeight();
        Point p(rect.x(), rect.y()+(rect.height()-height)/2);
        graphics.drawText(text, length, p);
        char_t buffer[16];
        bool drawMore=false;
        if (percentProgressDisabled!=percentProgress_)
        {
            drawMore=true;
            assert(percentProgress_<=100);
            length=tprintf(buffer, _T(" %hu%%"), percentProgress_);
        }
        else if (0!=bytesProgress_)
        {
            drawMore=true;
            const char_t* format=_T(" %lu bytes");
            if (bytesProgress_<1024)
                length=tprintf(buffer, format, bytesProgress_);
            else
            {
                format=_T(" %hu.%hukB");
                uint_t bri=bytesProgress_/1024;
                uint_t brf=((bytesProgress_%1024)+51)/102; // round to 1/10
                if (brf==10)
                {
                    bri++;
                    brf=0;
                }
                length=tprintf(buffer, format, bri, brf);
                localizeNumber(buffer, buffer+length);
            }
        }
        if (drawMore)
        {
            p.x+=width+2;
            width=rect.width()-width;
            graphics.charsInWidth(buffer, length, width);
            graphics.drawText(buffer, length, p);
        }
    }

}

