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
#include <DLServer.h>
#include <68K/Hs.h>
#include <Text.hpp>

namespace ArsLexis
{

    Err getDeviceSerialNumber(String& out)
    {
        char* data;
        UInt16 length;
        Err error=SysGetROMToken(0, sysROMTokenSnum, reinterpret_cast<UInt8**>(&data), &length);
        if (!error)
        {
            if (NULL==data || 0xff==*data)
                error=sysErrParamErr;
            else
                out.append(data, length);
        }
        return error;
    }

    Err getOEMCompanyId(String& out)
    {
        UInt32  id;
        char*   idAsChar;
        Err error = FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &id);
        if (!error)
        {
            idAsChar = (char*)&id;
            out.append(idAsChar, sizeof(UInt32));
        }
        return error;
    }

    Err getOEMDeviceId(String& out)
    {
        UInt32  id;
        char *  idAsChar;
        Err error=FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &id);
        if (!error)
        {
            idAsChar = (char*) &id;
            out.append(idAsChar, sizeof(UInt32));
        }
        return error;
    }

    Err getHotSyncName(String& out)
    {
        char  nameBuffer[dlkUserNameBufSize];
        Err error=DlkGetSyncInfo(NULL, NULL, NULL, nameBuffer, NULL, NULL);
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

    Err getPhoneNumber(String& out)
    {
        bool libLoaded=false;
        UInt16 refNum=sysInvalidRefNum ;
        Err error=SysLibFind(phnLibCDMAName, &refNum);
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

    namespace {

        typedef Err (TokenGetter)(String&);

        static void renderDeviceIdentifierToken(String& out, const char* prefix, TokenGetter* getter)
        {
            String token;
            Err error=(*getter)(token);
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
        renderDeviceIdentifierToken(out, "PN", getPhoneNumber);
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
        if (id != 'H101')
            return false;

        return true;
    }

}
