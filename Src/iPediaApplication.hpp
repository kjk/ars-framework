#ifndef __IPEDIA_APPLICATION_HPP__
#define __IPEDIA_APPLICATION_HPP__

#include "ipedia.h"
#include <Application.hpp>
#include <DynamicInputAreas.hpp>
#include <Logging.hpp>
#include "RenderingPreferences.hpp"
#include "iPediaHyperlinkHandler.hpp"

class LookupManager;
class LookupHistory;

#define serverLocalhost             "192.168.0.1:9000"
#define serverDictPcArslexis    "dict-pc.arslexis.com:9000"

class iPediaApplication: public ArsLexis::Application 
{
    mutable ArsLexis::RootLogger log_;
    ArsLexis::DIA_Support diaSupport_;
    UInt16 ticksPerSecond_;
    iPediaHyperlinkHandler hyperlinkHandler_;
    LookupHistory* history_;
    LookupManager* lookupManager_;
    ArsLexis::String server_;
    
    typedef std::list<ArsLexis::String> CustomAlerts_t;
    CustomAlerts_t customAlerts_;

    void detectViewer();
    
    void loadPreferences();

    void savePreferences();
    
protected:

    Err handleSystemNotify(SysNotifyParamType& notify);
    
    Err normalLaunch();

    void waitForEvent(EventType& event);
    
    ArsLexis::Form* createForm(UInt16 formId);

    bool handleApplicationEvent(EventType& event);
    
public:

    static const UInt32 requiredRomVersion=sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0);
    static const UInt32 creatorId=appFileCreator;
    static const UInt16 notEnoughMemoryAlertId=notEnoughMemoryAlert;
    static const UInt16 romIncompatibleAlertId=romIncompatibleAlert;
    
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
    
    struct Preferences
    {
        RenderingPreferences renderingPreferences;
        
        enum {cookieLength=32};
        ArsLexis::String cookie;
        
        enum {serialNumberLength=32};
        ArsLexis::String serialNumber;
        
        bool serialNumberRegistered;
        bool checkArticleCountAtStartup;
        
        enum {articleCountNotChecked=-1L};
        
        long articleCount;
        
        Preferences():
            serialNumberRegistered(false),
            checkArticleCountAtStartup(true),
            articleCount(articleCountNotChecked)            
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
        appDisplayCustomAlertEvent,
        appLookupStartedEvent,
        appLookupEventFirst=appLookupStartedEvent,
        appLookupProgressEvent,
        appLookupFinishedEvent,
        appGetArticlesCountEvent,
        appLookupEventLast=appGetArticlesCountEvent,
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
    
    void sendDisplayCustomAlertEvent(UInt16 alertId, const ArsLexis::String& text1);
    
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
    
    const LookupHistory& history() const
    {
        assert(0!=history_);
        return *history_;
    }
    
    bool hasHighDensityFeatures() const
    {return hasHighDensityFeatures_;}

    iPediaHyperlinkHandler& hyperlinkHandler()
    {return hyperlinkHandler_;}    
    
private:
    
    Preferences preferences_;

    bool diaNotifyRegistered_:1;
    bool stressMode_:1;
    bool hasHighDensityFeatures_:1;
    
};



#endif