#ifndef __IPEDIA_APPLICATION_HPP__
#define __IPEDIA_APPLICATION_HPP__

#include "ipedia.h"
#include "Application.hpp"
#include "DynamicInputAreas.hpp"
#include "NetLibrary.hpp"
#include "SocketConnection.hpp"
#include "RenderingPreferences.hpp"

class iPediaApplication: public ArsLexis::Application 
{
    ArsLexis::DIA_Support diaSupport_;
    Boolean diaNotifyRegistered_;
    
    ArsLexis::NetLibrary* netLib_;
    ArsLexis::SocketConnectionManager* connectionManager_;
    
    Err getNetLibrary(ArsLexis::NetLibrary*& netLib);
    
    UInt16 ticksPerSecond_;
    RenderingPreferences renderingPrefs_;

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
    
    UInt16 ticksPerSecond() const
    {return ticksPerSecond_;}
    
    const RenderingPreferences& renderingPreferences() const
    {return renderingPrefs_;}
    
};



#endif