#ifndef __IPEDIA_APPLICATION_HPP__
#define __IPEDIA_APPLICATION_HPP__

#include "ipedia.h"
#include "Application.hpp"
#include "DynamicInputAreas.hpp"
#include "NetLibrary.hpp"
#include "SocketConnection.hpp"

class iPediaApplication: public ArsLexis::Application 
{
    ArsLexis::DIA_Support diaSupport_;
    Boolean diaNotifyRegistered_;
    
    typedef std::auto_ptr<ArsLexis::NetLibrary> NetLibPtr;
    NetLibPtr netLib_;
    typedef std::auto_ptr<ArsLexis::SocketConnectionManager> ConnectionManagerPtr;
    ConnectionManagerPtr connectionManager_;
    
    Err getNetLib(ArsLexis::NetLibrary*& netLib);
    
    UInt16 ticksPerSecond_;

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
    
};



#endif