/**
 * Copyright (C) Krzysztof Kowalczyk
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include <uniqueid.h>
#include <sms.h>

#include <Text.hpp>
#include <DeviceInfo.hpp>

using ArsLexis::String;
using ArsLexis::status_t;
using ArsLexis::char_t;
using ArsLexis::HexBinEncodeBlob;
using ArsLexis::hexBinEncode;

static status_t getSystemParameter(String& out, UINT uiAction)
{
    int      bufChars = 128;
    int      bufSize;
    char_t * buffer    = NULL;
    BOOL     fOk;

    status_t error = ERROR_INSUFFICIENT_BUFFER;
    while (error == ERROR_INSUFFICIENT_BUFFER)
    {
        bufChars *= 2;
        bufSize = bufChars*sizeof(char_t);
        delete [] buffer;

        buffer = new char_t[bufChars];
        ZeroMemory(buffer,bufSize);

        fOk = SystemParametersInfo(uiAction, bufSize, buffer, 0);
        if (fOk)
        {            
            out.append(buffer);
            error = errNone;
        }
        else
            error = GetLastError();
    }

    delete [] buffer;
    return error;
}

static status_t getOEMCompanyId(String& out)
{
    return getSystemParameter(out,SPI_GETOEMINFO);
}

static status_t getHotSyncName(String& out)
{   
    return sysErrParamErr;
}

static status_t getPhoneNumber(String& out)
{
#ifdef WIN32_PLATFORM_WFSP  // smartphone
    SMS_ADDRESS address;
    ZeroMemory(&address,sizeof(address));
    HRESULT res = SmsGetPhoneNumber(&address);
    if (SUCCEEDED(res))
    {
        out.append(address.ptsAddress);
        return errNone;
    }
#endif
    return sysErrParamErr;
}

#define DEVID_BUF_SIZE 512
static status_t getUUID(String& out)
{ 
    DWORD dwOutBytes; 
    char  devIdBuf[DEVID_BUF_SIZE]; 

    BOOL fOk = ::KernelIoControl(IOCTL_HAL_GET_DEVICEID, 0, 0, devIdBuf, DEVID_BUF_SIZE, &dwOutBytes); 
    if (!fOk)
    {
        // TODO: check if ERROR_INSUFFICIENT_BUFFER == GetLastError() and retry
        // with a bigger buffer. But it really shouldn't be necessary -
        // DEVID_BUF_SIZE should be more than enough
        return sysErrParamErr;
    }

    // TODO: we really ignore the format of DEVICE_ID struct and treat it as a blob.
    // Good enough since it's going to be unique anyway
    String blobEncoded;
    HexBinEncodeBlob( (unsigned char*)devIdBuf, dwOutBytes, blobEncoded);
    out.append(blobEncoded);
    return errNone;
}

// this returns platform e.g. "PocketPC", "Smartphone" etc. plus OS
// version on the format "major.minor" e.g. "4.2". This fully identifies
// a given OS and should work on future devices as well.
// This gives us ability to know what OS is most popular with our users.
// We could be more specific by also using GetVersionEx()
static status_t getPlatform(String& out)
{
    status_t err = getSystemParameter(out,SPI_GETPLATFORMTYPE);
    if (errNone!=err)
    {
        // we couldn't get it dynamically (on emulator there's access denied)
        // so we use names based on compile-time detection

#ifdef WIN32_PLATFORM_PSPC
        out.append(_T("Pocket PC Static"));
#endif

#ifdef WIN32_PLATFORM_WFSP
       out.append(_T("Smartphone Static"));
#endif
       err = errNone;        
    }

    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    BOOL fOk = GetVersionEx(&osvi);
    if (!fOk)
    {
        // GetVersionEx failed. Shouldn't happen but we can safely ignore it
        return errNone;
    }

    long osMajor = (int)osvi.dwMajorVersion;
    long osMinor = (int)osvi.dwMinorVersion;

    char_t buffer[32];
    ZeroMemory(buffer,sizeof(buffer));

    int len = tprintf(buffer, _T(" %ld.%ld"), osMajor, osMinor);
    out.append(buffer);
    return errNone;
}

typedef status_t (TokenGetter)(String&);

static void renderDeviceIdentifierToken(String& out, const char_t* prefix, TokenGetter* getter)
{
    String token;
    status_t error=(*getter)(token);
    if (!error)
    {
        if (!out.empty())
            out+=':';
        out.append(prefix);
        out.append(hexBinEncode(token));
    }
}

String ArsLexis::deviceInfoToken()
{
    String out;
    renderDeviceIdentifierToken(out, _T("SN"), getUUID);
    renderDeviceIdentifierToken(out, _T("PN"), getPhoneNumber);
    renderDeviceIdentifierToken(out, _T("PL"), getPlatform);
    renderDeviceIdentifierToken(out, _T("OC"), getOEMCompanyId);
    return out;
}

