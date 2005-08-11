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

// "PL" tag
static status_t getPlatform(char*& out)
{
    out = StringCopy("PalmOS");
    if (NULL == out)
        return memErrNotEnoughSpace;
    return errNone;
}

// "HN" tag - handspring serial number, seems to be unique (at least on Treo 600)
static status_t getHsSerialNum(char*& out)
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

    out = StringCopy(versionStr);
    if (NULL == out)
        return memErrNotEnoughSpace;
    return errNone;
}

// "SN" tag, doesn't seem to be available (at least not on Treo600). On simulator
// it's "SERIAL NUMBER"
static status_t getDeviceSerialNumber(char*& out)
{
    unsigned char *  data=NULL;
    UInt16           length=0;

    status_t error=SysGetROMToken(0, sysROMTokenSnum, reinterpret_cast<UInt8**>(&data), &length);
    if (errNone != error)
        return error;

    if (NULL == data || 0xff == *data || 0x00 == *data || 0 == length)
        return sysErrParamErr;

    out = StringCopy((const char*)data);
    if (NULL == out)
        return memErrNotEnoughSpace;
    return errNone;
}

// "OC" tag
static status_t getOEMCompanyId(char*& out)
{
    UInt32  id;
    status_t error = FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &id);
    if (errNone != error)
        return error;
        
    const char* idAsChar = (const char*)&id;
    out = StringCopyN(idAsChar, sizeof(UInt32));
    if (NULL == out)
        return memErrNotEnoughSpace;

    return errNone;
}

// "OD" tag
static status_t getOEMDeviceId(char*& out)
{
    UInt32  id;
    status_t error=FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &id);
    if (errNone != error)
        return error;
            
    const char* idAsChar = (const char*)&id;
    out = StringCopyN(idAsChar, sizeof(UInt32));
    if (NULL == out)
        return memErrNotEnoughSpace;

    return errNone;
}

// "HN" tag
static status_t getHotSyncName(char*& out)
{
    char  nameBuffer[dlkUserNameBufSize];
    status_t error=DlkGetSyncInfo(NULL, NULL, NULL, nameBuffer, NULL, NULL);
    if (errNone != error)
        return error;
        
    uint_t len=StrLen(nameBuffer);
    if (len == 0)
        return sysErrParamErr;
        
    out = StringCopyN(nameBuffer, len);
    if (NULL == out)
        return memErrNotEnoughSpace;
        
    return errNone;
}

// "IM" tag for IMEI number, should be unique but apparently the phone
// needs to be connected to network in order to return this
// see http://www.mail-archive.com/palm-dev-forum@news.palmos.com/msg76403.html
static status_t getIMEI(char*& out)
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

    char* emei = NULL;

    if (PhnLibCardInfo(refNum, 0, 0, NULL, &emei))
    {
        // "00000000" is an invalid IMEI which can happen, according to
        // http://www.mail-archive.com/palm-dev-forum@news.palmos.com/msg76416.html
        // (it's also what simulator returns)
        if (emei && *emei && (0 != StrCompare(emei, "00000000")))
        {
            out = StringCopy(emei);
            if (NULL == out)
                error = memErrNotEnoughSpace;
        }
        else
            error = sysErrParamErr;
        free(emei);
    }
    else
        error = sysErrParamErr;

    if (libLoaded)
        SysLibRemove(refNum);

    return error;
}


// "PN" tag, should be unique
static status_t getPhoneNumber(char*& out)
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
        assert(sysInvalidRefNum != refNum);
        PhnAddressList addressList = NULL;
        error = PhnLibGetOwnNumbers(refNum, &addressList);
        if (!error)
        {
            PhnAddressHandle address = NULL;
            error = PhnLibAPGetNth(refNum, addressList, 1, &address);
            if (!error && address)
            {
                char* number = PhnLibAPGetField(refNum, address, phnAddrFldPhone);
                MemHandleFree(address);
                // "00000000000" (eleven '0') is phone number returned by sim
                if (number && *number && 0 != StrCompare("00000000000", number))
                    out = number;
                else 
                {
                    free(number);
                    error = sysErrParamErr;
                }
            }
            MemHandleFree(addressList);
        }
        if (libLoaded)
            SysLibRemove(refNum);
    }
    return error;
}

char* deviceInfoToken()
{
    char* out;
    DeviceInfoTokenRender(out, "SN", getDeviceSerialNumber);
    DeviceInfoTokenRender(out, "HS", getHotSyncName);
    DeviceInfoTokenRender(out, "HN", getHsSerialNum);
    DeviceInfoTokenRender(out, "PN", getPhoneNumber);
    DeviceInfoTokenRender(out, "PL", getPlatform);
    DeviceInfoTokenRender(out, "OC", getOEMCompanyId);
    DeviceInfoTokenRender(out, "OD", getOEMDeviceId);
    DeviceInfoTokenRender(out, "IM", getIMEI);    
    return out;
}

#pragma segment Segment1

// return true if running on Treo 600 device
// TODO: should be renamed to isTreo()
bool isTreo600()
{
    UInt32 id;

    FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &id);
    if (id != 'hspr')
        return false;

    FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &id);
    // H101 is Treo 600
    // H201 is Treo 600 simulator - I need this to test 
    // H102 is Treo 650
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

