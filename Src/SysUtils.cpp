#include "SysUtils.hpp"

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
        std::int32_t rand1=SysRandom(0);
        if ( 2 == (rand1 % 3) )
            rand1 = SysRandom(TimGetTicks());

        std::int32_t rand2=SysRandom(0);
        if ( 2 == (rand2 % 3) )
            rand2 = SysRandom(TimGetTicks());

        std::uint32_t result = rand1*sysRandomMax + rand2;
        result = result % range;
        return result;
    }


    String deviceIdToken()
    {
        //! @todo Implement deviceIdToken() (well... copy from iNoah should be enough)
        return "TEST";
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

}