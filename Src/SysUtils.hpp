#ifndef __ARSLEXIS_SYSUTILS_HPP__
#define __ARSLEXIS_SYSUTILS_HPP__

#include "Debug.hpp"
#include "BaseTypes.hpp"

namespace ArsLexis
{

    bool isNotifyManager();
    
    void getScreenBounds(RectangleType& bounds);
    

    /** 
     * Generates a random long in the range 0..range-1. SysRandom() returns value
     * between 0..sysRandomMax which is 0x7FFF. We have to construct a long out of
     * that.
     * @note Shamelessly ripped from Noah's noah_pro_2nd_segment.c ;-)
     */
    std::uint32_t random(std::uint32_t range);
    
    Err numericValue(const char* begin, const char* end, std::int32_t& result, uint_t base=10);
    
    String hexBinEncode(const String& in);

    inline void hexBinEncodeInPlace(String& inOut)
    {
        inOut=hexBinEncode(inOut);
    }
    
}

#endif