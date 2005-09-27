#ifndef __ARSLEXIS_SYSUTILS_HPP__
#define __ARSLEXIS_SYSUTILS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

#if defined(_PALM_OS)
# include <PalmOS/PalmSysUtils.hpp>
#elif defined(_WIN32_WCE)
# include <WindowsCE/WinSysUtils.hpp>
#endif


/** 
 * Generates a random long in the range 0..range-1. SysRandom() returns value
 * between 0..sysRandomMax which is 0x7FFF. We have to construct a long out of
 * that.
 * @note Shamelessly ripped from Noah's noah_pro_2nd_segment.c ;-)
 */
ulong_t random(ulong_t range);

void localizeNumber(char_t* begin, char_t* end);

inline void localizeNumberStrInPlace(char_t* begin)
{
    localizeNumber(begin, begin + tstrlen(begin));
}

inline void localizeNumber(String& s) {
    char_t* begin = &s[0];
    localizeNumber(begin, begin + s.length());
}

void delocalizeNumber(char_t* begin, char_t* end);

inline void delocalizeNumber(String& s) {
    char_t* begin = &s[0];
    delocalizeNumber(begin, begin + s.length());
}

inline void delocalizeNumberStrInPlace(char_t* begin)
{
    delocalizeNumber(begin, begin + tstrlen(begin));
}

void Alert(uint_t alertId);

#ifdef _WIN32
#define ALERT(palmAlert, winAlert) Alert(winAlert)
#endif

#ifdef _PALM_OS
#define ALERT(palmAlert, winAlert) Alert(palmAlert)
#endif

#endif
