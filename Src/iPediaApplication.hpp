#ifndef __IPEDIA_APPLICATION_HPP__
#define __IPEDIA_APPLICATION_HPP__

#include "ipedia.h"
#include "Application.hpp"
#include "DynamicInputAreas.hpp"
#include "SocketConnection.hpp"
#include "RenderingPreferences.hpp"
#include "iPediaHyperlinkHandler.hpp"
#include "Resolver.hpp"

class iPediaApplication: public ArsLexis::Application 
{
    ArsLexis::DIA_Support diaSupport_;
    Boolean diaNotifyRegistered_;
    
    ArsLexis::NetLibrary* netLib_;
    ArsLexis::SocketConnectionManager* connectionManager_;
    
    Err getNetLibrary(ArsLexis::NetLibrary*& netLib);
    
    UInt16 ticksPerSecond_;
    
    iPediaHyperlinkHandler hyperlinkHandler_;
    ArsLexis::Resolver* resolver_;
    
    ArsLexis::String termList_;
        
    void detectViewer();

protected:

    Err handleSystemNotify(SysNotifyParamType& notify);
    
    Err normalLaunch();

    void waitForEvent(EventType& event);
    
    ArsLexis::Form* createForm(UInt16 formId);

    bool handleApplicationEvent(EventType& event);
    
public:

    Err handleLaunchCode(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
        
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
    
    void setTermList(const ArsLexis::String& termList)
    {termList_=termList;}
    
    const ArsLexis::String& termList() const
    {return termList_;}
    
    void sendDisplayAlertEvent(UInt16 alertId);
    
private:
    
    Preferences preferences_;
};



#endif