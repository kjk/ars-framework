#ifndef __ARSLEXIS_SYSUTILS_HPP__
#define __ARSLEXIS_SYSUTILS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>

#if defined(_PALM_OS)
# include <PalmOS/PalmSysUtils.hpp>
#elif defined(_WIN32_WCE)
# include <WindowsCE/WinSysUtils.hpp>
#endif

namespace ArsLexis
{
    class Point;
    
    void sendEvent(uint_t event, const void* data = NULL, uint_t dataSize = 0, bool unique = false, const Point* point = NULL);
    
    template<class EventData>
    void sendEvent(uint_t event, const EventData& data, bool unique = false, const Point* point = NULL)
    {sendEvent(event, &data, sizeof(data), unique, point);}

    void processReadyUiEvents();

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
    
}

#endif
