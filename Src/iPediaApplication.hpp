#ifndef __IPEDIA_APPLICATION_HPP__
#define __IPEDIA_APPLICATION_HPP__

#include "ipedia.h"
#include "Application.hpp"
#include "DynamicInputAreas.hpp"
#include "RenderingPreferences.hpp"
#include "iPediaHyperlinkHandler.hpp"
#include "Logging.hpp"

class LookupManager;

#define serverLocalhost             "192.168.0.1:9000"
#define serverDictPcArslexis    "dict-pc.arslexis.com:9000"

class iPediaApplication: public ArsLexis::Application 
{
    mutable ArsLexis::RootLogger log_;
    ArsLexis::DIA_Support diaSupport_;
    bool diaNotifyRegistered_;
    UInt16 ticksPerSecond_;
    iPediaHyperlinkHandler hyperlinkHandler_;
    LookupManager* lookupManager_;
    ArsLexis::String server_;
    bool stressMode_;
    
    void detectViewer();
    
protected:

    Err handleSystemNotify(SysNotifyParamType& notify);
    
    Err normalLaunch();

    void waitForEvent(EventType& event);
    
    ArsLexis::Form* createForm(UInt16 formId);

    bool handleApplicationEvent(EventType& event);
    
public:

    static const UInt32 requiredRomVersion=sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0);
    static const UInt32 creatorId=appFileCreator;
    
    const ArsLexis::DIA_Support& getDIASupport() const
    {return diaSupport_;}

    iPediaApplication();
    
    ~iPediaApplication();
    
    Err initialize();
    
    LookupManager* getLookupManager(bool create=false);
    const LookupManager* getLookupManager() const
    {return lookupManager_;}
    
    UInt16 ticksPerSecond() const
    {return ticksPerSecond_;}
    
    iPediaHyperlinkHandler& hyperlinkHandler()
    {return hyperlinkHandler_;}
    
    struct Preferences
    {
        RenderingPreferences renderingPreferences;
        
        enum {cookieLength=32};
        ArsLexis::String cookie;
        
        enum {serialNumberLength=32};
        ArsLexis::String serialNumber;
        
        bool serialNumberRegistered;
        
        Preferences():
            serialNumberRegistered(false)
        {}

    };
    
    Preferences& preferences() 
    {return preferences_;}
    
    const Preferences& preferences() const
    {return preferences_;}
    
    const RenderingPreferences& renderingPreferences() const
    {return preferences().renderingPreferences;}
    
    const ArsLexis::DIA_Support& diaSupport() const
    {return diaSupport_;}
    
    enum Event
    {
        appDisplayAlertEvent=appFirstAvailableEvent,
        appLookupStartedEvent,
        appLookupEventFirst=appLookupStartedEvent,
        appLookupProgressEvent,
        appLookupFinishedEvent,
        appLookupEventLast=appLookupFinishedEvent,
        appFirstAvailableEvent
    };
    
    struct DisplayAlertEventData
    {
        UInt16 alertId;
        
        DisplayAlertEventData(UInt16 aid):
            alertId(aid) {}
    };
    
    static void sendDisplayAlertEvent(UInt16 alertId)
    {sendEvent(appDisplayAlertEvent, DisplayAlertEventData(alertId));}
    
    ArsLexis::Logger& log() const
    {return log_;}
    
    void setServer(const ArsLexis::String& server)
    {server_=server;}
    
    const ArsLexis::String& server() const
    {return server_;}
    
    static iPediaApplication& instance()
    {return static_cast<iPediaApplication&>(Application::instance());}
    
    bool inStressMode() const
    {return stressMode_;}
    
    void toggleStressMode(bool enable)
    {stressMode_=enable;}

private:
    
    Preferences preferences_;
};



#endif