/**
 * @file DeviceInfo.cpp
 *
 *
 * Copyright (C) 2000-2003 Krzysztof Kowalczyk
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include <DeviceInfo.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

#include <aygshell.h>
#ifndef PPC
#include <tpcshell.h>
#endif
#include <winuser.h>
#include <winbase.h>
#include <sms.h>

namespace ArsLexis
{

    status_t getSystemParameter(String& out, int param)
    {
        status_t error=ERROR_INSUFFICIENT_BUFFER;
        
        int buffSize=128;
        TCHAR *buffer= NULL;

        while(error == ERROR_INSUFFICIENT_BUFFER)
        {
            buffSize*=2;
            delete buffer;
            buffer= new TCHAR[buffSize];
            memset(buffer,0,sizeof(buffSize));
            if (SystemParametersInfo(param, buffSize, buffer, 0))
                error = errNone;
            else
                error = GetLastError();
        }
        
        /*TCHAR            buffer[128];
        memset(&buffer,0,sizeof(buffer));
        if (SystemParametersInfo(SPI_GETOEMINFO, sizeof(buffer), buffer, 0))
            error = errNone;
        else
            error = GetLastError();*/
    
        if (!error)
            out.append(buffer);
        else
            error = sysErrParamErr;
        
        delete buffer;
        return error;                
    }

    status_t getOEMCompanyId(String& out)
    {
        return getSystemParameter(out,SPI_GETOEMINFO);
    }

    status_t getOEMDeviceId(String& out)
    {
        return getSystemParameter(out, SPI_GETPLATFORMTYPE);
    }

    status_t getHotSyncName(String& out)
    {   
        return sysErrParamErr;
    }

    status_t getPhoneNumber(String& out)
    {
        #ifndef PPC
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

    status_t getPlatform(String& out)
    {
        // TODO: make it different for Pocket PC
        #ifdef PPC
            out.append(_T("Pocket PC 2002"));
        #else
            out.append(_T("Smartphone"));
        #endif
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
        renderDeviceIdentifierToken(out, _T("OD"), getOEMDeviceId);
        return out;
    }


    // return true if running on real Treo 600 device
    bool isTreo600()
    {
        return false;
    }

}
