#ifndef __ARSLEXIS_DEVICE_INFO_HPP__
#define __ARSLEXIS_DEVICE_INFO_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <PalmOS.h>

namespace ArsLexis
{

    Err getDeviceSerialNumber(String& out);
    
    Err getOEMCompanyId(String& out);
    
    Err getOEMDeviceId(String& out);
    
    Err getHotSyncName(String& out);

    Err getPhoneNumber(String& out);

    String deviceInfoToken();

    bool isTreo600();    
}

#endif