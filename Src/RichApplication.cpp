#include <RichApplication.hpp>
#include <SysUtils.hpp>



namespace ArsLexis {

#ifndef NDEBUG
#pragma inline_depth 0
#endif

    RichApplication::RichApplication():
        log_(_T("root")),
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
    
    void RichApplication::sendDisplayCustomAlertEvent(uint_t alertId, const ArsLexis::String& text1)
    {
        customAlerts_.push_back(text1);
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
                log().debug()<<"Alert: "<<data.alertId;
            handled=true;
        }
        else if (appDisplayCustomAlertEvent==event.eType)
        {
            assert(!customAlerts_.empty());            
            DisplayAlertEventData& data=reinterpret_cast<DisplayAlertEventData&>(event.data);
            if (showAlerts_)
                FrmCustomAlert(data.alertId, customAlerts_.front().c_str(), "", "");
            else
                log().debug()<<"Custom alert: "<<data.alertId<<'['<<customAlerts_.front()<<']';
            customAlerts_.pop_front();
            handled=true;
        }
        else
            handled=Application::handleApplicationEvent(event);
        return handled;
    }



}