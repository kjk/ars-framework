/**
 * Copyright (C) Krzysztof Kowalczyk
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include <uniqueid.h>
#include <sms.h>

#include <Text.hpp>
#include <DeviceInfo.hpp>
#include <UTF8_Processor.hpp>

static status_t getSystemParameter(char*& out, UINT uiAction)
{
	ulong_t size = 64;
    char_t* buffer    = NULL;
    status_t err = ERROR_INSUFFICIENT_BUFFER;
    while (ERROR_INSUFFICIENT_BUFFER == err)
    {
		free(buffer);
		size *= 2; 
		buffer = StrAlloc<char_t>(size);
		
        if (SystemParametersInfo(uiAction, size, buffer, 0))
        {
            err = errNone;
			break;
		}
        else
            err = GetLastError();
    }
	if (errNone != err)
	{
		free(buffer);
		return err;
	}
	out = UTF8_FromNative(buffer);
	free(buffer);
	if (NULL == out)
		return memErrNotEnoughSpace;
	return errNone;
}

static status_t getOEMCompanyId(char*& out)
{
    return getSystemParameter(out, SPI_GETOEMINFO);
}

static status_t getHotSyncName(char*& out)
{   
    return sysErrParamErr;
}

static status_t getPhoneNumber(char*& out)
{
    static HINSTANCE mod = LoadLibrary(TEXT("sms"));
    if (NULL == mod)
        return sysErrParamErr;

    typedef HRESULT (*SmsGetPhoneNumberProc )(SMS_ADDRESS* const);
    static SmsGetPhoneNumberProc proc = (SmsGetPhoneNumberProc)GetProcAddress(mod, TEXT("SmsGetPhoneNumber"));
    if (NULL == proc)
        return sysErrParamErr;
           
    SMS_ADDRESS address;
    ZeroMemory(&address, sizeof(address));
    if (FAILED(proc(&address)))
		return  sysErrParamErr;
		
	out = UTF8_FromNative(address.ptsAddress);
	if (NULL == out)
		return memErrNotEnoughSpace;
	
	return errNone;
}

static status_t getUUID(char*& out)
{ 
    static const ulong_t size = 512;
    union {
        DEVICE_ID devId;  
        char buffer[size];
    };
    ZeroMemory(buffer, size);   
    
    DWORD rSize; 
    if (!KernelIoControl(IOCTL_HAL_GET_DEVICEID, 0, 0, buffer, size, &rSize))
		return GetLastError();
		
	ulong_t sz = devId.dwSize * 2;
	out = StrAlloc<char>(sz);
	if (NULL == out)
		return memErrNotEnoughSpace;
	
	sz = StrHexlify(buffer, devId.dwSize, out, sz + 1);
	assert(sz == devId.dwSize * 2);
    return errNone;
}

// this returns platform e.g. "PocketPC", "Smartphone" etc. plus OS
// version on the format "major.minor" e.g. "4.2". This fully identifies
// a given OS and should work on future devices as well.
// This gives us ability to know what OS is most popular with our users.
// We could be more specific by also using GetVersionEx()
static status_t getPlatform(char*& out)
{
    status_t err = getSystemParameter(out, SPI_GETPLATFORMTYPE);
    if (errNone != err)
    {
        // we couldn't get it dynamically (on emulator there's access denied)
        // so we use names based on compile-time detection

#ifdef WIN32_PLATFORM_PSPC
        out = StringCopy("Pocket PC Static");
#endif

#ifdef WIN32_PLATFORM_WFSP
		out = StringCopy("Smartphone Static");
#endif
		
        if (NULL == out)
            return memErrNotEnoughSpace;  
    }

    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (!GetVersionEx(&osvi))
        return errNone;

    long osMajor = osvi.dwMajorVersion;
    long osMinor = osvi.dwMinorVersion;

    char buffer[16];
    StrPrintF(buffer, " %ld.%ld", osMajor, osMinor);
    out = StrAppend(out, -1, buffer, -1);
    if (NULL == out)
        return memErrNotEnoughSpace;
           
    return errNone;
}

typedef status_t (TokenGetter)(char*&);

static status_t renderDeviceIdentifierToken(char*& out, const char* prefix, TokenGetter* getter)
{
    char* token = NULL;
    status_t err = (*getter)(token);
    if (errNone != err)
        return err;
         
    if (NULL != out)
    {
        out = StrAppend(out, -1, ":", 1);
        if (NULL == out)
            return memErrNotEnoughSpace;
    } 

    out = StrAppend(out, -1, prefix, -1);
    if (NULL == out)
        return memErrNotEnoughSpace;
        
    ulong_t len = Len(token);
    char* coded = StrAlloc<char>(len * 2);
    if (NULL == coded)
        return memErrNotEnoughSpace;
        
    StrHexlify(token, len, coded, len * 2 + 1);
    out = StrAppend(out, -1, coded, len * 2);
    free(coded);
    return errNone;
}

char* deviceInfoToken()
{
    char* out = NULL;
    renderDeviceIdentifierToken(out, "SN", getUUID);
    renderDeviceIdentifierToken(out, "PN", getPhoneNumber);
    renderDeviceIdentifierToken(out, "PL", getPlatform);
    renderDeviceIdentifierToken(out, "OC", getOEMCompanyId);
    return out;
}

#ifndef NDEBUG
void test_DeviceInfoToken()
{
    char* p = deviceInfoToken();
    free(p); 
}
#endif
