#ifndef __IPEDIA_APPLICATION_HPP__
#define __IPEDIA_APPLICATION_HPP__

#include "ipedia.h"
#include "Application.hpp"
#include "DynamicInputAreas.hpp"
#include "SocketConnection.hpp"
#include "RenderingPreferences.hpp"
#include "iPediaHyperlinkHandler.hpp"
#include "Resolver.hpp"
#include "Logging.hpp"

class iPediaApplication: public ArsLexis::Application 
{
    mutable ArsLexis::RootLogger log_;
    ArsLexis::DIA_Support diaSupport_;
    Boolean diaNotifyRegistered_;
    
    ArsLexis::NetLibrary* netLib_;
    ArsLexis::SocketConnectionManager* connectionManager_;
    
    Err getNetLibrary(ArsLexis::NetLibrary*& netLib);
    
    UInt16 ticksPerSecond_;
    
    iPediaHyperlinkHandler hyperlinkHandler_;
    ArsLexis::Resolver* resolver_;
    
    ArsLexis::String searchResults_;
        
    void detectViewer();
    
protected:

    Err handleSystemNotify(SysNotifyParamType& notify);
    
    Err normalLaunch();

    void waitForEvent(EventType& event);
    
    ArsLexis::Form* createForm(UInt16 formId);

    bool handleApplicationEvent(EventType& event);
    
public:

    static const UInt32 requiredRomVersion=sysMakeROMVersion(3,5,0,sysROMStageDevelopment,0);
    
    const ArsLexis::DIA_Support& getDIASupport() const
    {return diaSupport_;}

    iPediaApplication();
    
    ~iPediaApplication();
    
    Err initialize();
    
    Err getConnectionManager(ArsLexis::SocketConnectionManager*& manager);
    Err getResolver(ArsLexis::Resolver*& resolver);
    
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
        appFirstAvailableEvent
    };
    
    struct DisplayAlertEventData
    {
        UInt16 alertId;
    };
    
    void setSearchResults(const ArsLexis::String& searchResults)
    {searchResults_=searchResults;}
    
    const ArsLexis::String& searchResults() const
    {return searchResults_;}
    
    void sendDisplayAlertEvent(UInt16 alertId);
    
    ArsLexis::Logger& log() const
    {return log_;}
    
private:
    
    Preferences preferences_;
};



#endif