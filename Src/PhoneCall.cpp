#include <PhoneCall.hpp>

//#include <PhoneLib.h>

namespace ArsLexis
{
/*
    static Err LoadPhoneLibrary(UInt16* libRefP, Boolean* libLoadedP)
    {
        Err error = 0;
        // Routine is pointless without this parameter
        if (!libRefP)
            return memErrInvalidParam;
        // Get the Phone library
        error = SysLibFind("GSM Library", libRefP);
        if (error)
        {
        
        assert( 0 == sysErrLibNotFound);
        assert( 0 == sysErrNoFreeRAM);
        assert( 0 == sysErrNoFreeLibSlots);        
            // It's not already here - have to load it ourselves
            error = SysLibLoad('libr', 'GSM!', libRefP);
            if (error)
                return error;
            if (libLoadedP)
                *libLoadedP = true;
        }
        return error;
    }

*/
    bool call10DigitsNumber(const ArsLexis::String& callNumber)
    {
/*        UInt16 libRefP = 0;
        Boolean libLoadedP = true;

        Err error = LoadPhoneLibrary(&libRefP, &libLoadedP);
        if (error)
            return false;
    
        PhnLibPlayDTMF(libRefP,(char*)callNumber.c_str());
    
        PhnLibClose (libRefP);
        return true;
*/        
        return false;
    }
}