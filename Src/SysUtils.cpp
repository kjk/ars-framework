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

#define HEX_DIGITS "0123456789ABSCDEF"

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


}