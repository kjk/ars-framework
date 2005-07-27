#ifndef __ARSLEXIS_SYSUTILS_HPP__
#define __ARSLEXIS_SYSUTILS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

#if defined(_PALM_OS)
# include <PalmOS/PalmSysUtils.hpp>
#elif defined(_WIN32_WCE)
# include <WindowsCE/WinSysUtils.hpp>
#endif

void localizeNumber(char_t* begin, char_t* end);

inline void localizeNumberStrInPlace(char_t *begin)
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

void Alert(uint_t alertId);

#endif
