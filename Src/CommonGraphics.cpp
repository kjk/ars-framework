#include "Graphics.hpp"
#include <cassert>

namespace ArsLexis
{
    
    Graphics::Graphics(const NativeGraphicsHandle_t& handle):
        handle_(handle)
    {
    }

    Graphics::~Graphics()
    {
    }

    typedef NativeColor_t (Graphics::* ColorSetterMethod_t)(NativeColor_t);
    static ColorSetterMethod_t colorSetters[]={&Graphics::setTextColor, &Graphics::setForegroundColor, &Graphics::setBackgroundColor};
    
    NativeColor_t Graphics::setColor(Graphics::ColorChoice choice, NativeColor_t color)
    {
        assert(choice<3);
        ColorSetterMethod_t setter=colorSetters[choice];
        return (this->*setter)(color);
    }
    
}