#ifndef __ARSLEXIS_DEVICE_INFO_HPP__
#define __ARSLEXIS_DEVICE_INFO_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

#if defined(_PALM_OS)
bool isTreo600();
#endif

bool underSimulator();


typedef status_t (*DeviceInfoProvider)(char*&);

status_t DeviceInfoTokenRender(char*& out, const char* prefix, DeviceInfoProvider provider);

char* deviceInfoToken();

#ifndef NDEBUG
void test_DeviceInfoToken();
#endif

#endif
