#include <Debug.hpp>
#include <SysUtils.hpp>
#include <Text.hpp>

using ArsLexis::char_t;

UInt32 romVersion()
{
    UInt32 romVersion,
    error = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    assert(!error);
    return romVersion;
}

UInt16 romVersionMajor()
{
    return sysGetROMVerMajor(romVersion());
}

UInt16 romVersionMinor()
{
    return sysGetROMVerMinor(romVersion());
}


bool ArsLexis::notifyManagerPresent()
{
    UInt32 value;
    Err error=FtrGet(sysFtrCreator,sysFtrNumNotifyMgrVersion, &value);
    return (errNone==error && value);
}

void ArsLexis::getScreenBounds(RectangleType& bounds)
{
    Coord x,y;
    WinGetDisplayExtent(&x, &y);
    assert(x>=0);
    assert(y>=0);
    bounds.topLeft.x=bounds.topLeft.y=0;
    bounds.extent.x=x;
    bounds.extent.y=y;
}

ulong_t ArsLexis::random(ulong_t range)
{
    ulong_t rand1=SysRandom(0);
//        if ( 2 == (rand1 % 3) )
//            rand1 = SysRandom(TimGetTicks());

    ulong_t rand2=SysRandom(0);
//        if ( 2 == (rand2 % 3) )
//            rand2 = SysRandom(TimGetTicks());

//        std::uint32_t result = rand1*sysRandomMax + rand2;
    ulong_t rand3=SysRandom(0);
    ulong_t result = (rand1<<30)|(rand2<<15)|rand3;
    result = result % range;
    return result;
}

// detect a web browser app and return cardNo and dbID of its *.prc.
// returns true if detected some viewer, false if none was found
bool ArsLexis::fDetectViewer(UInt16 *cardNoOut, LocalID *dbIDOut)
{
    DmSearchStateType searchState;

    // NetFront
    Err error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'NF3T', true, cardNoOut, dbIDOut);
    if (!error)
        goto FoundBrowser;
    // xiino
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'PScp', true, cardNoOut, dbIDOut);
    if (!error)
        goto FoundBrowser;
    // Blazer browser on Treo 600
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'BLZ5', true, cardNoOut, dbIDOut);
    if (!error)
        goto FoundBrowser;
    // Blazer browser on Treo 180/270/300
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'BLZ1', true, cardNoOut, dbIDOut);
    if (!error)
        goto FoundBrowser;
    // WebBrowser 2.0
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'NF3P', true, cardNoOut, dbIDOut);
    if (!error)
        goto FoundBrowser;
    // Web Pro (Tungsten T) 
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, 'NOVR', true, cardNoOut, dbIDOut);
    if (!error)
        goto FoundBrowser;
    // Web Broser 1.0 (Palm m505)
    error = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, sysFileCClipper, true, cardNoOut, dbIDOut);
    if (!error)
        goto FoundBrowser;
    return false;
FoundBrowser:
    return true;
}

// given the id of a string resource, return a copy of this resource.
// Client needs to free the memory
// Return NULL in case of an error
char_t *getResource(UInt16 stringId)
{
    char_t  *toReturn = NULL;

    MemHandle handle=DmGet1Resource(strRsc, stringId);
    if (!handle)
        return NULL;

    char* str=(char*)MemHandleLock(handle);
    if (NULL == str)
        goto Exit;

    toReturn = StringCopy2(str, MemHandleSize(handle));
    MemHandleUnlock(handle);

Exit:
    if (handle)
        DmReleaseResource(handle);
    return toReturn;
}

// given the id of a data resource, return a copy of this resource and
// the size of resource in resSizeOut.
// Client needs to free the memory
// Return NULL in case of an error
char_t *getDataResource(UInt16 dataId, UInt32 *resSizeOut)
{
    char_t  *toReturn = NULL;

    MemHandle handle=DmGet1Resource('data', dataId);
    if (!handle)
        return NULL;

    char* str=(char*)MemHandleLock(handle);
    if (NULL == str)
        goto Exit;

    *resSizeOut = MemHandleSize(handle);
    toReturn = StringCopy2(str, *resSizeOut);
    MemHandleUnlock(handle);

Exit:
    if (handle)
        DmReleaseResource(handle);
    return toReturn;
}

Err ArsLexis::getResource(UInt16 tableId, UInt16 index, String& out)
{
    Err error=errNone;
    MemHandle handle=DmGet1Resource(strListRscType, tableId);
    if (handle)
    {
        const char* str=static_cast<const char*>(MemHandleLock(handle));
        if (str)
        {
            str+=(StrLen(str)+1); // skip "prefix"
            UInt16 count=(*((UInt8 *)(str)) << 8) | *((UInt8 *)(str+1)); // ugly way of deserializing 2 bytes into UInt16
            if (index<count)
            {
                str+=2;
                while (index)
                {
                    str+=(StrLen(str)+1);   // skip some strings and their terminators
                    --index;
                }
                out.assign(str);
            }
            else
                error=sysErrParamErr;
            MemHandleUnlock(handle);
        }
        else
            error = memErrChunkNotLocked;
        DmReleaseResource(handle);
    }
    else 
        error=sysErrParamErr;
    return error;
}

Err ArsLexis::WebBrowserCommand(Boolean subLaunch, UInt16 launchFlags, UInt16 command, const char *parameterP, UInt32 *resultP)
{
    UInt16  cardNo;
    LocalID dbID;
    UInt32  result;
    Err     error = errNone;

    if (resultP)
        *resultP = errNone;

    if (!fDetectViewer(&cardNo,&dbID))
        return 1;

    if (subLaunch)
    {
        SysAppLaunch(cardNo, dbID, launchFlags, command, const_cast<char*>(parameterP), &result);
        if (resultP) 
            *resultP = result;
    }
    else
    {
        char *newParamP = NULL;
        if (parameterP)
        {
            newParamP=(char*)MemPtrNew( StrLen(parameterP) +1 );
            if (newParamP == NULL) 
                error = memErrNotEnoughSpace;
            else
            {
                StrCopy(newParamP, parameterP);
                MemPtrSetOwner(newParamP, 0); // The OS now owns this memory
            }
        }
        if (error == errNone)
        {
            SysUIAppSwitch(cardNo, dbID, command, newParamP);
        }
    }
    return error;
} 

bool ArsLexis::highDensityFeaturesPresent()
{
    UInt32 version;
    Err error=FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version);
    return (errNone==error && version>=4);
}

void ArsLexis::sendEvent(uint_t e, const void* data, uint_t dataSize, bool unique)
{
    EventType event;
    MemSet(&event, sizeof(event), 0);
    event.eType=static_cast<eventsEnum>(e);
    if (data && dataSize)
    {
        assert(dataSize<=sizeof(event.data));
        MemMove(&event.data, data, dataSize);
    }
    if (unique)
        EvtAddUniqueEventToQueue(&event, 0, false);
    else
        EvtAddEventToQueue(&event);
}

/*
void ArsLexis::processReadyUiEvents()
{
    EventType event;
    Application& app=Application::instance();
    do
    {
        Err error;
        EvtGetEvent(&event, 0);
        if (appStopEvent!=event.eType)
        {
            if (!SysHandleEvent(&event))
                if (!MenuHandleEvent(0, &event, &error))
                    if (!app.handleApplicationEvent(event))
                        FrmDispatchEvent(&event);
        }
    } while (appStopEvent!=event.eType && nilEvent!=event.eType);
    if (appStopEvent==event.eType)
        EvtAddEventToQueue(&event);
}
*/

void ArsLexis::localizeNumber(char_t* begin, char_t* end)
{
    NumberFormatType numFormat = static_cast<NumberFormatType>(PrefGetPreference(prefNumberFormat));
    char th, dec;
    LocGetNumberSeparators(numFormat, &th, &dec);
    while (begin != end)
    {
        if (_T(',') == *begin)
            *begin = th;
        else if (_T('.') == *begin)
            *begin = dec;
        ++begin;            
    }
}

void ArsLexis::delocalizeNumber(char_t* begin, char_t* end)
{
    NumberFormatType numFormat = static_cast<NumberFormatType>(PrefGetPreference(prefNumberFormat));
    char th, dec;
    LocGetNumberSeparators(numFormat, &th, &dec);
    while (begin != end)
    {
        if (th == *begin)
            *begin = _T(',');
        else if (dec == *begin)
            *begin = _T('.');
        ++begin;            
    }
}

