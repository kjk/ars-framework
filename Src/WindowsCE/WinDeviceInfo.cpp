/**
 * Copyright (C) Krzysztof Kowalczyk
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include <DeviceInfo.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

#include <aygshell.h>
#ifdef WIN32_PLATFORM_WFSP
#include <tpcshell.h>
#endif
#include <winuser.h>
#include <winbase.h>
#include <sms.h>

namespace ArsLexis
{

status_t getSystemParameter(String& out, UINT uiAction)
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
        memset(buffer,0,bufSize);

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

status_t getOEMCompanyId(String& out)
{
    return getSystemParameter(out,SPI_GETOEMINFO);
}

status_t getHotSyncName(String& out)
{   
    return sysErrParamErr;
}

status_t getPhoneNumber(String& out)
{
#ifdef WIN32_PLATFORM_WFSP  // smartphone
    SMS_ADDRESS address;
    memset(&address,0,sizeof(SMS_ADDRESS));
    HRESULT res = SmsGetPhoneNumber(&address);
    if (SUCCEEDED(res))
    {
        out.append(address.ptsAddress);
        return errNone;
    }
#endif
    return sysErrParamErr;
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
    memset(&buffer,0,sizeof(buffer));

    int len = tprintf(buffer, _T(" %ld.%ld"), osMajor, osMinor);
    out.append(buffer);
    return errNone;
}

namespace {

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

}

String deviceInfoToken()
{
    String out;
    renderDeviceIdentifierToken(out, _T("PN"), getPhoneNumber);
    renderDeviceIdentifierToken(out, _T("PL"), getPlatform);
    renderDeviceIdentifierToken(out, _T("OC"), getOEMCompanyId);
    return out;
}

}
