#ifndef __ARSLEXIS_RICH_APPLICATION_HPP__
#define __ARSLEXIS_RICH_APPLICATION_HPP__

#include <Application.hpp>
#include <DynamicInputAreas.hpp>
#include <Logging.hpp>

class RichApplication: public Application {

    DIA_Support diaSupport_;
    
    typedef std::list<ArsLexis::String> CustomAlerts_t;
    CustomAlerts_t customAlerts_;

    bool diaNotifyRegistered_:1;
    bool hasHighDensityFeatures_:1;
    bool showAlerts_:1;

protected:

    Err handleSystemNotify(SysNotifyParamType& notify);
    
    bool handleApplicationEvent(EventType& event);

    RichApplication();

public:

    ~RichApplication();
    
    const DIA_Support& diaSupport() const
    {return diaSupport_;}

    enum Event {
        appDisplayAlertEvent=appFirstAvailableEvent,
        appDisplayCustomAlertEvent,
        appFirstAvailableEvent
    };        
    
    struct DisplayAlertEventData
    {
        uint_t alertId;
        DisplayAlertEventData(uint_t aid):
            alertId(aid) {}
    };

    static void sendDisplayAlertEvent(uint_t alertId);
    
    void sendDisplayCustomAlertEvent(uint_t alertId, const ArsLexis::String& text1);

    bool hasHighDensityFeatures() const
    {return hasHighDensityFeatures_;}
    
    Err initialize();
    
    void toggleShowAlerts(bool enable)
    {showAlerts_=enable;}
    
    static RichApplication& instance() {return static_cast<RichApplication&>(Application::instance());}
    
};

#endif