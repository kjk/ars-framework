#include "SysUtils.hpp"

namespace ArsLexis
{

    Boolean isNotifyManager()
    {
        UInt32 value=0;
        Err error=FtrGet(sysFtrCreator,sysFtrNumNotifyMgrVersion, &value);
        return (!error && value);
    }

}