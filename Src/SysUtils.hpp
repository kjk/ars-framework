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

    void sendEvent(uint_t event, const void* data=0, uint_t dataSize=0, bool unique=false);
    
    template<class EventData>
    void sendEvent(uint_t event, const EventData& data, bool unique=false)
    {sendEvent(event, &data, sizeof(data), unique);}

    void processReadyUiEvents();

    void localizeNumber(char_t* begin, char_t* end);

}

#endif
