#include <RichApplication.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

#ifndef NDEBUG
#pragma inline_depth 0
#endif

RichApplication::RichApplication():
    diaNotifyRegistered_(false),
    hasHighDensityFeatures_(highDensityFeaturesPresent()),
    showAlerts_(true)
{}

#ifndef NDEBUG
#pragma inline_depth 2
#endif

RichApplication::~RichApplication() 
{
    if (diaNotifyRegistered_) 
        unregisterNotify(diaSupport_.notifyType());
}


Err RichApplication::initialize()
{
    Err error=Application::initialize();
    if (!error)
    {
        if (diaSupport_ && notifyManagerPresent()) 
        {
            error=registerNotify(diaSupport_.notifyType());
            if (!error)
                diaNotifyRegistered_=true;
        }
    }
    return error;
}

void RichApplication::sendDisplayAlertEvent(uint_t alertId)
{
    sendEvent(appDisplayAlertEvent, DisplayAlertEventData(alertId));
}

void RichApplication::sendDisplayCustomAlertEvent(uint_t alertId, const char_t* text)
{
    customAlerts_.push_back(StringCopy2(text));
    sendEvent(appDisplayCustomAlertEvent, DisplayAlertEventData(alertId));
}

Err RichApplication::handleSystemNotify(SysNotifyParamType& notify)
{
    if (diaSupport_ && diaSupport_.notifyType()==notify.notifyType)
        diaSupport_.handleNotify();
    return errNone;
}

bool RichApplication::handleApplicationEvent(EventType& event)
{
    bool handled=false;
    if (appDisplayAlertEvent==event.eType)
    {
        DisplayAlertEventData& data=reinterpret_cast<DisplayAlertEventData&>(event.data);
        if (showAlerts_)
            FrmAlert(data.alertId);
        else
        {
            Log(eLogDebug, "Alert: ", false);
            LogUlong(eLogDebug, data.alertId, true);
        }
        handled=true;
    }
    else if (appDisplayCustomAlertEvent==event.eType)
    {
        assert(!customAlerts_.empty());            
        DisplayAlertEventData& data=reinterpret_cast<DisplayAlertEventData&>(event.data);
        char_t* text = customAlerts_.front();
        if (showAlerts_)
            FrmCustomAlert(data.alertId, text, "", "");
        else
        {
            Log(eLogDebug, _T("Custom alert: "), false);
            LogUlong(eLogDebug, data.alertId, false);
            Log(eLogDebug, _T("["), false);
            Log(eLogDebug, text, false);                
            Log(eLogDebug, _T("]"), true);
        }
        free(text);
        customAlerts_.erase(customAlerts_.begin());
        handled = true;
    }
    else
        handled = Application::handleApplicationEvent(event);
    return handled;
}

