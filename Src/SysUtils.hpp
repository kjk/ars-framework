#ifndef __ARSLEXIS_SYSUTILS_HPP__
#define __ARSLEXIS_SYSUTILS_HPP__

#include "Debug.hpp"

namespace ArsLexis
{

    Boolean isNotifyManager();
    
    void getScreenBounds(RectangleType& bounds);
    

    /** 
     * Generates a random long in the range 0..range-1. SysRandom() returns value
     * between 0..sysRandomMax which is 0x7FFF. We have to construct a long out of
     * that.
     * @note Shamelessly ripped from Noah's noah_pro_2nd_segment.c ;-)
     */
    UInt32 random(UInt32 range);
    
    String deviceIdToken();
    
}

#endif