#include <SysUtils.hpp>

namespace ArsLexis
{

    bool isNotifyManager()
    {
        UInt32 value=0;
        Err error=FtrGet(sysFtrCreator,sysFtrNumNotifyMgrVersion, &value);
        return (!error && value);
    }
    
    void getScreenBounds(RectangleType& bounds)
    {
        Coord x,y;
        WinGetDisplayExtent(&x, &y);
        assert(x>=0);
        assert(y>=0);
        bounds.topLeft.x=bounds.topLeft.y=0;
        bounds.extent.x=x;
        bounds.extent.y=y;
    }
    
    std::uint32_t random(std::uint32_t range)
    {
        std::uint32_t rand1=SysRandom(0);
//        if ( 2 == (rand1 % 3) )
//            rand1 = SysRandom(TimGetTicks());

        std::uint32_t rand2=SysRandom(0);
//        if ( 2 == (rand2 % 3) )
//            rand2 = SysRandom(TimGetTicks());

//        std::uint32_t result = rand1*sysRandomMax + rand2;
        std::uint32_t rand3=SysRandom(0);
        std::uint32_t result = (rand1<<30)|(rand2<<15)|rand3;
        result = result % range;
        return result;
    }

    Err numericValue(const char* begin, const char* end, std::int32_t& result, uint_t base)
    {
        Err error=errNone;
        bool negative=false;
        Int32 res=0;
        String numbers("0123456789abcdefghijklmnopqrstuvwxyz");
        char buffer[2];
        if (begin>=end || base>numbers.length())
        {    
            error=sysErrParamErr;
            goto OnError;           
        }
        if (*begin=='-')
        {
            negative=true;
            if (++begin==end)
            {
                error=sysErrParamErr;
                goto OnError;           
            }
        }           
        buffer[1]=chrNull;
        while (begin!=end) 
        {
            buffer[0]=*(begin++);
            StrToLower(buffer, buffer); 
            String::size_type num=numbers.find(buffer);
            if (num>=base)
            {   
                error=sysErrParamErr;
                break;
            }
            else
            {
                res*=base;
                res+=num;
            }
        }
        if (!error)
           result=res;
    OnError:
        return error;    
    }

#define HEX_DIGITS "0123456789ABCDEF"

    String hexBinEncode(const String& in)
    {
        String out;
        out.reserve(2*in.length());
        String::const_iterator it=in.begin();
        String::const_iterator end=in.end();
        while (it!=end)
        {
            UInt8 b=*(it++);
            out.append(1, HEX_DIGITS[b/16]);
            out.append(1, HEX_DIGITS[b%16]);
        }
        return out;
    }

    // detect a web browser app and return cardNo and dbID of its *.prc.
    // returns true if detected some viewer, false if none was found
    bool fDetectViewer(UInt16 *cardNoOut, LocalID *dbIDOut)
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


    Err getResource(UInt16 stringId, String& out)
    {
        Err error=errNone;
        MemHandle handle=DmGet1Resource(strRsc, stringId);
        if (handle)
        {
            const char* str=static_cast<const char*>(MemHandleLock(handle));
            if (str)
            {
                out.assign(str);
                MemHandleUnlock(handle);
            }
            else
                error=memErrChunkNotLocked;
            DmReleaseResource(handle);
        }
        else 
            error=sysErrParamErr;
        return error;
    }
    
    Err getResource(UInt16 tableId, UInt16 index, String& out)
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
                error=memErrChunkNotLocked;
            DmReleaseResource(handle);
        }
        else 
            error=sysErrParamErr;
        return error;
    }

    Err WebBrowserCommand(Boolean subLaunch, UInt16 launchFlags, UInt16 command, char *parameterP, UInt32 *resultP)
    {
        UInt16  cardNo;
        LocalID dbID;
        UInt32  result;
        Err     error;

        if (resultP)
            *resultP = errNone;

        if (!fDetectViewer(&cardNo,&dbID))
            return 1;

        if (subLaunch)
        {
            SysAppLaunch(cardNo, dbID, launchFlags, command, parameterP, &result);
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


}