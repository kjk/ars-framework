#ifndef __ARSLEXIS_NATIVE_GRAPHICS_HPP__
#define __ARSLEXIS_NATIVE_GRAPHICS_HPP__

namespace ArsLexis 
{

#if defined(__PALMOS_H__)

    typedef FontID NativeFont_t;

    typedef IndexedColorType NativeColor_t;
    
    typedef WinHandle NativeGraphicsHandle_t;
    
    #define USE_DEFAULT_NATIVE_GRAPHICS_HANDLE 1    

#elif defined(_WIN32_WCE)

    typedef COLORREF NativeColor_t;

    typedef HDC NativeGraphicsHandle_t;
    
    // The following #define and struct are only an example how custom draw state container might be done.
    #define USE_NATIVE_GRAPHICS_STATE 1
    struct NativeGraphicsState
    {
    };
    
}    
// What we need to do here is to include header implementing HFONT wrapper for Windows CE. 
// I assume that this wrapper class will be called Font and reside in file Font.hpp
// Font *must* be copyable, so it should be reference-counted internally (we discussed these matters 
// with Marek and it seems it's the easiest way to ensure proper HDC behavior.
#include "Font.hpp"

namespace ArsLexis 
{ 

    typedef Font NativeFont_t;

#else

#error "Define native graphics counterparts in NativeGraphics.hpp before including Graphics.hpp"

#endif

}


#endif