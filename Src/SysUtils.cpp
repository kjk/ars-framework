#include "SysUtils.hpp"

namespace ArsLexis
{

    Boolean isNotifyManager()
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
    
    UInt32 random(UInt32 range)
    {
        Int16 rand1=SysRandom(0);
        if ( 2 == (rand1 % 3) )
            rand1 = SysRandom(TimGetTicks());

        Int16 rand2 = SysRandom(0);
        if ( 2 == (rand2 % 3) )
            rand2 = SysRandom(TimGetTicks());

        UInt32 result = rand1*sysRandomMax + rand2;
        result = result % range;
        return result;
    }


    String deviceIdToken()
    {
        //! @todo Implement deviceIdToken() (well... copy from iNoah should be enough)
        return "TEST";
    }

}