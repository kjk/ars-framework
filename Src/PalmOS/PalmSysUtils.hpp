#ifndef __ARSLEXIS_PALM_SYSUTILS_HPP__
#define __ARSLEXIS_PALM_SYSUTILS_HPP__

#include <BaseTypes.hpp>

UInt32 romVersion();
UInt16 romVersionMajor();
UInt16 romVersionMinor();

using ArsLexis::char_t;
char_t *getResource(UInt16 stringId);
char_t *getDataResource(UInt16 dataId, UInt32 *resSizeOut);

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
    
    Err WebBrowserCommand(Boolean subLaunch, UInt16 launchFlags, UInt16 command, const char *parameterP, UInt32 *resultP);
    
    bool highDensityFeaturesPresent();
    
    bool notifyManagerPresent();
    
}

#endif
