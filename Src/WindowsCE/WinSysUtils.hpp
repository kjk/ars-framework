#ifndef __ARSLEXIS_PALM_WINSYSUTILS_HPP__
#define __ARSLEXIS_PALM_WINSYSUTILS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

namespace ArsLexis
{

    
    /*void getScreenBounds(RectangleType& bounds);*/
       
    ulong_t random(ulong_t range);
    
    // detect a web browser app and return cardNo and dbID of its *.prc.
    // returns true if detected some viewer, false if none was found
    // bool fDetectViewer(UInt16 *cardNoOut, LocalID *dbIDOut);
   
    // Err getResource(UInt16 tableId, UInt16 index, String& out);
    
    // Err getResource(UInt16 stringId, String& out);

    // Err WebBrowserCommand(Boolean subLaunch, UInt16 launchFlags, UInt16 command, char *parameterP, UInt32 *resultP);
    
    // bool highDensityFeaturesPresent();
    
    // bool notifyManagerPresent();
    void sendEvent(uint_t event, short wph, short wpl, int lp);
    struct EventData {LPARAM lParam; WPARAM wParam;};
}

#endif
