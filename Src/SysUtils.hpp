#ifndef __ARSLEXIS_SYSUTILS_HPP__
#define __ARSLEXIS_SYSUTILS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis
{

    
    void getScreenBounds(RectangleType& bounds);

    /** 
     * Generates a random long in the range 0..range-1. SysRandom() returns value
     * between 0..sysRandomMax which is 0x7FFF. We have to construct a long out of
     * that.
     * @note Shamelessly ripped from Noah's noah_pro_2nd_segment.c ;-)
     */
    ulong_t random(ulong_t range);
    
    // detect a web browser app and return cardNo and dbID of its *.prc.
    // returns true if detected some viewer, false if none was found
    bool fDetectViewer(UInt16 *cardNoOut, LocalID *dbIDOut);
   
    Err getResource(UInt16 tableId, UInt16 index, String& out);
    
    Err getResource(UInt16 stringId, String& out);

    Err WebBrowserCommand(Boolean subLaunch, UInt16 launchFlags, UInt16 command, char *parameterP, UInt32 *resultP);
    
    bool highDensityFeaturesPresent();
    
    bool notifyManagerPresent();
    
}

#endif
