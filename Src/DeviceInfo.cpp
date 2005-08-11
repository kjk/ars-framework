#include <DeviceInfo.hpp>
#include <Text.hpp>

status_t DeviceInfoTokenRender(char*& out, const char* prefix, DeviceInfoProvider provider)
{
    char* token = NULL;
    status_t err = (*provider)(token);
    if (errNone != err)
        return err;
         
    if (NULL != out)
    {
        out = StrAppend(out, -1, ":", 1);
        if (NULL == out)
        {
            free(token);
            return memErrNotEnoughSpace;
        }
    } 

    out = StrAppend(out, -1, prefix, -1);
    if (NULL == out)
    {  
        free(token); 
        return memErrNotEnoughSpace;
    }
        
    ulong_t len = Len(token);
    char* coded = StrAlloc<char>(len * 2);
    if (NULL == coded)
    {
        free(token);  
        return memErrNotEnoughSpace;
    }
        
    StrHexlify(token, len, coded, len * 2 + 1);
    free(token);
     
    out = StrAppend(out, -1, coded, len * 2);
    free(coded);
    return errNone;
}

