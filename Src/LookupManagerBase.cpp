#include <LookupManagerBase.hpp>
#include <Graphics.hpp>
#include <BaseTypes.hpp>
#include <SysUtils.hpp>

namespace ArsLexis {

    LookupProgressReporter::~LookupProgressReporter()
    {}
    
    DefaultLookupProgressReporter::~DefaultLookupProgressReporter()
    {}
    
    void DefaultLookupProgressReporter::showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const Rectangle& bounds, bool clearBkg)
    {
        if (clearBkg) 
            graphics.erase(bounds);

        Rectangle rect(bounds);
        rect.explode(2, 2, -4, -4);
        Graphics::FontSetter setFont(graphics, Font());
        const char_t* text = support.statusText;
        uint_t length = tstrlen(text);
        uint_t width = rect.width();
        graphics.charsInWidth(text, length, width);
        uint_t height = graphics.fontHeight();
        Point p(rect.x(), rect.y()+(rect.height()-height)/2);
        graphics.drawText(text, length, p);
        char_t buffer[32];
        bool drawMore=false;
        if (support.percentProgressDisabled!=support.percentProgress())
        {
            drawMore=true;
            assert(support.percentProgress()<=100);
            length=tprintf(buffer, _T(" %hu%%"), support.percentProgress());
        }
        else if (0!=support.bytesProgress())
        {
            drawMore=true;
            const char_t* format=_T(" %lu bytes");
            if (support.bytesProgress()<1024)
                length=tprintf(buffer, format, support.bytesProgress());
            else
            {
                format=_T(" %hu.%hukB");
                uint_t bri=support.bytesProgress()/1024;
                uint_t brf=((support.bytesProgress()%1024)+51)/102; // round to 1/10
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
            p.x = p.x + width + 2;
            width = rect.width()-width;
            graphics.charsInWidth(buffer, length, width);
            graphics.drawText(buffer, length, p);
        }
    }

    LookupProgressReportingSupport::LookupProgressReportingSupport():
        percentProgress_(percentProgressDisabled),
        bytesProgress_(0),
        progressReporter_(new DefaultLookupProgressReporter())
    {}

    LookupProgressReportingSupport::~LookupProgressReportingSupport()
    {}

}

