/**
 * @file DeviceInfo.cpp
 *
 * Copyright (C) 2000-2003 Krzysztof Kowalczyk
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include <DeviceInfo.hpp>
#include <SysUtils.hpp>
#include <DLServer.h>
#include <68K/Hs.h>
#include <Text.hpp>

static status_t getPlatform(String& out)
{
    out.append("Palm");
    return errNone;
}

namespace ArsLexis
{

    status_t getDeviceSerialNumber(String& out)
    {
        char* data;
        UInt16 length;
        status_t error=SysGetROMToken(0, sysROMTokenSnum, reinterpret_cast<UInt8**>(&data), &length);
        if (!error)
        {
            if (NULL==data || 0xff==*data)
                error=sysErrParamErr;
            else
                out.append(data, length);
        }
        return error;
    }

    status_t getOEMCompanyId(String& out)
    {
        UInt32  id;
        char*   idAsChar;
        status_t error = FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &id);
        if (!error)
        {
            idAsChar = (char*)&id;
            out.append(idAsChar, sizeof(UInt32));
        }
        return error;
    }

    status_t getOEMDeviceId(String& out)
    {
        UInt32  id;
        char *  idAsChar;
        status_t error=FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &id);
        if (!error)
        {
            idAsChar = (char*) &id;
            out.append(idAsChar, sizeof(UInt32));
        }
        return error;
    }

    status_t getHotSyncName(String& out)
    {
        char  nameBuffer[dlkUserNameBufSize];
        status_t error=DlkGetSyncInfo(NULL, NULL, NULL, nameBuffer, NULL, NULL);
        if (!error)
        {
            uint_t len=StrLen(nameBuffer);
            if (len>0)
                out.append(nameBuffer, len);
            else
                error=sysErrParamErr;
        }
        return error;
    }

    status_t getPhoneNumber(String& out)
    {
        bool libLoaded=false;
        UInt16 refNum=sysInvalidRefNum ;
        status_t error=SysLibFind(phnLibCDMAName, &refNum);
        if (error)
            error=SysLibFind(phnLibGSMName, &refNum);
        if (error)
        {
            error=SysLibLoad(phnLibDbType, phnLibCDMADbCreator, &refNum);
            if (error)
                error=SysLibLoad(phnLibDbType, phnLibGSMDbCreator, &refNum);
            if (!error)
                libLoaded=true;
        }
        if (!error)
        {
            assert(sysInvalidRefNum!=refNum);
            PhnAddressList addressList=NULL;
            error=PhnLibGetOwnNumbers(refNum, &addressList);
            if (!error)
            {
                PhnAddressHandle address=NULL;
                error=PhnLibAPGetNth(refNum, addressList, 1, &address);
                if (!error && address)
                {
                    char* number=PhnLibAPGetField(refNum, address, phnAddrFldPhone);
                    MemHandleFree(address);
                    if (number)
                    {
                        out.append(number);
                        MemPtrFree(number);
                    }
                    else 
                        error=sysErrParamErr;
                }
                MemHandleFree(addressList);
            }
            if (libLoaded)
                SysLibRemove(refNum);
        }
        return error;
    }

    status_t getHsSerialNum(String out)
    {
        char  versionStr[hsVersionStringSize];
    
        if (!isTreo600())
            return sysErrNotAllowed;
    
        MemSet (versionStr, hsVersionStringSize, 0);
        if (HsGetVersionString (hsVerStrSerialNo, versionStr, NULL) != errNone)
            return sysErrNotAllowed;
    
        if (0==versionStr[0])
            return sysErrNotAllowed;
    
        out.append((char*)versionStr);
        return errNone;
    }


    namespace {

        typedef status_t (TokenGetter)(String&);

        static void renderDeviceIdentifierToken(String& out, const char* prefix, TokenGetter* getter)
        {
            String token;
            status_t  error=(*getter)(token);
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
        renderDeviceIdentifierToken(out, "HS", getHotSyncName);
        renderDeviceIdentifierToken(out, "SN", getDeviceSerialNumber);
        RenderDeviceIdentifierToken(out, "HN", getHsSerialNum);
        renderDeviceIdentifierToken(out, "PN", getPhoneNumber);
        renderDeviceIdentifierToken(out, "PL", getPlatform);
        renderDeviceIdentifierToken(out, "OC", getOEMCompanyId);
        renderDeviceIdentifierToken(out, "OD", getOEMDeviceId);
        return out;
    }

    // return true if running on real Treo 600 device
    bool isTreo600()
    {
        UInt32 id;

        FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &id);
        if (id != 'hspr')
            return false;

        FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &id);
        if (('H101'==id) || ('H102'==id))
            return true;

        return false;
    }

}
