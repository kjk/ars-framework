/**
 * @file DeviceInfo.cpp
 *
 * Copyright (C) 2000-2003 Krzysztof Kowalczyk
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include <DLServer.h>
#include <68K/Hs.h>
#include <Debug.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>
#include <DeviceInfo.hpp>

using ArsLexis::status_t;
using ArsLexis::String;
using ArsLexis::hexBinEncode;

// "PL" tag
static status_t getPlatform(String& out)
{
    out.append("Palm");
    return errNone;
}

// "HN" tag - handspring serial number, seems to be unique (at least on Treo 600)
static status_t getHsSerialNum(String& out)
{
    char  versionStr[hsVersionStringSize];

    // TODO: needs a better way to figure out if I can call this function
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

// "SN" tag, doesn't seem to be available (at least not on Treo600). On simulator
// it's "SERIAL NUMBER"
static status_t getDeviceSerialNumber(String& out)
{
    unsigned char *  data=NULL;
    UInt16           length=0;

    status_t error=SysGetROMToken(0, sysROMTokenSnum, reinterpret_cast<UInt8**>(&data), &length);
    if (error)
        return error;

    if (NULL==data || 0xff==*data || 0x00==*data || 0==length)
        return sysErrParamErr;

    out.append((char*)data, length);
    assert(errNone==error);
    return error;
}

// "OC" tag
static status_t getOEMCompanyId(String& out)
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

// "OD" tag
static status_t getOEMDeviceId(String& out)
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

// "HN" tag
static status_t getHotSyncName(String& out)
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

// "IM" tag for IMEI number, should be unique but apparently the phone
// needs to be connected to network in order to return this
// see http://www.mail-archive.com/palm-dev-forum@news.palmos.com/msg76403.html
static status_t getIMEI(String& out)
{
    bool     libLoaded=false;
    UInt16   refNum=sysInvalidRefNum ;

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

    if (error)
        return error;

    assert(sysInvalidRefNum!=refNum);

    char *emei=NULL;

    if (PhnLibCardInfo(refNum,0,0,NULL,&emei))
    {
        // "00000000" is an invalid IMEI which can happen, according to
        // http://www.mail-archive.com/palm-dev-forum@news.palmos.com/msg76416.html
        // (it's also what simulator returns)
        if (emei && *emei && (0!=StrCompare(emei,"00000000")))
        {
            out.append(emei);
            MemPtrFree (emei);
        }
        else
            error = sysErrParamErr;
    }
    else
        error = sysErrParamErr;

    if (libLoaded)
        SysLibRemove(refNum);

    return error;
}


// "PN" tag, should be unique
static status_t getPhoneNumber(String& out)
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
                // "00000000000" (eleven '0') is phone number returned by sim
                if (number && *number && 0!=StrCompare("00000000000",number))
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

typedef status_t (TokenGetter)(String&);

static void renderDeviceIdentifierToken(String& out, const char* prefix, TokenGetter* getter)
{
    String    token;
    status_t  error=(*getter)(token);
    if (error)
        return;

    if (!out.empty())
        out+=':';
    out.append(prefix);
    String tokenEncoded = hexBinEncode(token);
    out.append(hexBinEncode(token));
}

ArsLexis::String deviceInfoToken()
{
    ArsLexis::String out;
    renderDeviceIdentifierToken(out, "SN", getDeviceSerialNumber);
    renderDeviceIdentifierToken(out, "HS", getHotSyncName);
    renderDeviceIdentifierToken(out, "HN", getHsSerialNum);
    renderDeviceIdentifierToken(out, "PN", getPhoneNumber);
    renderDeviceIdentifierToken(out, "PL", getPlatform);
    renderDeviceIdentifierToken(out, "OC", getOEMCompanyId);
    renderDeviceIdentifierToken(out, "OD", getOEMDeviceId);
    renderDeviceIdentifierToken(out, "IM", getIMEI);    
    return out;
}

// return true if running on Treo 600 device
// TODO: should be renamed to isTreo()
bool isTreo600()
{
    UInt32 id;

    FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &id);
    if (id != 'hspr')
        return false;

    FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &id);
    // H201 is Treo 600 simulator - I need this to test 
    // H101 is Treo 600
    // H102 is ???
    // H202 is Treo 650 simulator
    if (('H101' == id) || ('H102' == id) || ('H201' == id) || ('H202' == id))
        return true;

    return false;
}

bool underSimulator() 
{
    UInt32 processorId;
    FtrGet(sysFtrCreator, sysFtrNumProcessorID, &processorId);
    return (sysFtrNumProcessorx86 == (sysFtrNumProcessorMask & processorId));
}

