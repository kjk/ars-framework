#ifndef __IPEDIA_APPLICATION_HPP__
#define __IPEDIA_APPLICATION_HPP__

#include "ipedia.h"
#include "Application.hpp"
#include "DynamicInputAreas.hpp"

class iPediaApplication: public ArsLexis::Application 
{
    ArsLexis::DIA_Support diaSupport_;
    Boolean diaNotifyRegistered_;
        
public:

    const ArsLexis::DIA_Support& getDIASupport() const throw()
    {return diaSupport_;}

    iPediaApplication(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
    
    ~iPediaApplication() throw();
    
    void handleSystemNotify(SysNotifyParamType& notify);
    
    void run();
};



#endif