#ifndef __ARSLEXIS_DEVICE_INFO_HPP__
#define __ARSLEXIS_DEVICE_INFO_HPP__

#include <PalmOS.h>
#include "Debug.hpp"
#include "BaseTypes.hpp"

namespace ArsLexis
{

    Err getDeviceSerialNumber(String& out);
    
    Err getOEMCompanyId(String& out);
    
    Err getOEMDeviceId(String& out);
    
    Err getHotSyncName(String& out);

    Err getPhoneNumber(String& out);

    String deviceInfoToken();
    
}

#endif