#include <PhoneCall.hpp>

#include <TelephonyMgr.h>
#include <TelephonyMgrTypes.h>

#include <HsExt.h>
#include <HsAppLaunchCmd.h>

#define MAX_PHONE_NUMBER 32

// this is a text that shows up in the "Phone" application
#define CALLER_NAME "InfoMan"

// dial a given number by invoking Treo 600 built-in phone application
// this works on Treo 600 but according to http://news.palmos.com/read/messages?id=162343#162343
// it's better to use HelperServiceClass (works on more devices)
// Phone app accepts phone numbers in variety of forms e.g.
// xxx-yyyy-zzzz, (xxx) yyy-zzzz
Err DialPhoneNumber(const char *phoneNumber)
{
    UInt16      cardNo;
    LocalID     dbID;
    DmSearchStateType SearchState;
    Err err = DmGetNextDatabaseByTypeCreator(true, &SearchState, sysFileTApplication, hsFileCPhone, true, &cardNo, &dbID);
    if (err)
        return err;

    int callerNameLen = sizeof(CALLER_NAME)-1;  // sizeof() includes terminating 0
    int dataLen = sizeof(PhoneAppLaunchCmdDialType)+MAX_PHONE_NUMBER+1+callerNameLen+1;
    char *dataPtr = (char*)MemPtrNew(dataLen);
    if (NULL==dataPtr) 
        return sysErrNoFreeRAM;
    MemSet(dataPtr, dataLen, 0);

    PhoneAppLaunchCmdDialPtr dialInfo;
    dialInfo = (PhoneAppLaunchCmdDialPtr)dataPtr;

    char *number = dataPtr+sizeof(PhoneAppLaunchCmdDialType);
    char *callerName = number+MAX_PHONE_NUMBER+1;

    MemMove(number, phoneNumber, StrLen(phoneNumber));
    MemMove(callerName, CALLER_NAME, callerNameLen);
    dialInfo->number = number;
    dialInfo->name = callerName;
    dialInfo->version = 1;
    dialInfo->dialMethod = PhoneAppDialMethodNormal;
    MemPtrSetOwner(dataPtr, 0); /* owned by system */
    err = SysUIAppSwitch(cardNo, dbID, phoneAppLaunchCmdDial, dataPtr);
    return err;
}


// this doesn't work on Treo
bool FCallPhoneNumber(const char* phoneNumber)
{
    UInt16    gTelRefNum = 0;
    TelAppID  gTelAppID;
    Err       err;
    bool      fOk = false;

    err = SysLibFind(kTelMgrLibName, &gTelRefNum);
    if (errNone != err)
    {
        /* The library is not loaded, so load it. */
        err = SysLibLoad(kTelMgrDatabaseType, kTelMgrDatabaseCreator, &gTelRefNum);
        if (err)
            goto Exit;
    }

    err = TelOpen(gTelRefNum, kTelMgrVersion, &gTelAppID);
    if (errNone != err)
        goto Exit;

    err = TelIsSpcCallNumberSupported(gTelRefNum, gTelAppID, NULL);
    if (errNone != err)
        goto CloseAndExit;

    err = TelSpcCallNumber(gTelRefNum, gTelAppID, phoneNumber, NULL);
    if (errNone != err)
        goto CloseAndExit;

    fOk = true;

CloseAndExit:
    if ( telErrLibStillInUse != TelClose(gTelRefNum, gTelAppID))
        SysLibRemove(gTelRefNum);

Exit:
    return fOk;
}

