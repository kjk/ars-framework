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
    
    void detectViewer();

protected:

    Err handleSystemNotify(SysNotifyParamType& notify);
    
    Err normalLaunch();

    void waitForEvent(EventType& event);
    
    ArsLexis::Form* createForm(UInt16 formId);
    
    Err initializeForm(ArsLexis::Form& form);
    
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
        ArsLexis::String cookie;
    };
    
    Preferences& preferences() 
    {return preferences_;}
    
    const Preferences& preferences() const
    {return preferences_;}
    
    const RenderingPreferences& renderingPreferences() const
    {return preferences().renderingPreferences;}
    
private:
    
    Preferences preferences_;
};



#endif