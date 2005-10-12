#include <LookupManagerBase.hpp>
#include <Graphics.hpp>
#include <BaseTypes.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

LookupProgressReporter::~LookupProgressReporter()
{}

DefaultLookupProgressReporter::~DefaultLookupProgressReporter()
{}

void DefaultLookupProgressReporter::showProgress(const LookupProgressReportingSupport& support, Graphics& graphics, const Rect& bounds, bool clearBkg)
{
    if (clearBkg) 
        graphics.erase(bounds);

    Rect rect(bounds);
    rect.explode(2, 2, -4, -4);
#ifndef _WIN32
    Graphics::FontSetter setFont(graphics, Font());
#endif

#ifdef _WIN32        
    Graphics::ColorSetter colorTxt(graphics, Graphics::colorText, RGB(0,0,0));
#endif
    const char_t* text = support.statusText;
    ulong_t length = Len(text);
    uint_t width = rect.width();
    graphics.charsInWidth(text, length, width);
    uint_t fontDy = graphics.fontHeight();
    uint_t rectDy = rect.height();
    uint_t fontYOffsetCentered;
    if (rectDy > fontDy)
        fontYOffsetCentered = (rect.height() - fontDy)/2;
    else
        fontYOffsetCentered = 0;

    Point p(rect.x(), rect.y() + fontYOffsetCentered);
    graphics.drawText(text, length, p);
    char_t buffer[32];
    bool drawMore=false;
    if (support.percentProgressDisabled != support.percentProgress())
    {
        drawMore=true;
        assert(support.percentProgress() <= 100);
        length = tprintf(buffer, _T(" %hu%%"), support.percentProgress());
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
            localizeNumber(buffer, buffer + length);
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
progressReporter_(new_nt DefaultLookupProgressReporter()),
statusText(_T("")) 
{}

LookupProgressReportingSupport::~LookupProgressReportingSupport()
{}

void LookupProgressReportingSupport::showProgress(Graphics& graphics, const Rect& bounds, bool clearBkg)
{
    if (NULL != progressReporter_.get())
        progressReporter_->showProgress(*this, graphics, bounds, clearBkg);
}

LookupManagerBase::LookupManagerBase(uint_t firstEvent): 
lookupInProgress_(false), 
lookupStartedEvent_(firstEvent) 
{}

void LookupManagerBase::handleLookupEvent(const Event& event)
{
    ulong_t id = ExtEventGetID(event);
    if (lookupStartedEvent_ == id)
        lookupInProgress_ = true;
    else if (lookupStartedEvent_ + 2 == id)
    {
        lookupInProgress_ = false;
        setStatusText(_T(""));
        setPercentProgress(percentProgressDisabled);
        setBytesProgress(0);
    }
}

void LookupManagerBase::abortConnections()
{
    lookupInProgress_ = false;
    connectionManager().abortConnections();
}

