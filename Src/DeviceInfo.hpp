#ifndef __ARSLEXIS_DEVICE_INFO_HPP__
#define __ARSLEXIS_DEVICE_INFO_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis
{

    status_t getDeviceSerialNumber(String& out);
    
    status_t getOEMCompanyId(String& out);
    
    status_t getOEMDeviceId(String& out);
    
    status_t getHotSyncName(String& out);

    status_t getPhoneNumber(String& out);

    String deviceInfoToken();

    bool isTreo600();    
}

#endif