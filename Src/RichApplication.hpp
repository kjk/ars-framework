#ifndef __ARSLEXIS_RICH_APPLICATION_HPP__
#define __ARSLEXIS_RICH_APPLICATION_HPP__

#include <Application.hpp>
#include <DynamicInputAreas.hpp>
#include <Logging.hpp>


namespace ArsLexis {

    class RichApplication: public Application {

        mutable ArsLexis::RootLogger log_;
        ArsLexis::DIA_Support diaSupport_;
        uint_t ticksPerSecond_;
        
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
        
        uint_t ticksPerSecond() const
        {return ticksPerSecond_;}
        
        const ArsLexis::DIA_Support& diaSupport() const
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
        
        ArsLexis::RootLogger& log() const
        {return log_;}
    
        bool hasHighDensityFeatures() const
        {return hasHighDensityFeatures_;}
        
        Err initialize();
        
        void toggleShowAlerts(bool enable)
        {showAlerts_=enable;}
        
    };

}

#endif