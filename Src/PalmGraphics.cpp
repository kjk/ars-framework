#include <PalmOS.h>
#include "Graphics.hpp"

namespace ArsLexis
{
    Graphics::Graphics(const NativeGraphicsHandle_t& handle):
        handle_(handle)
    {
        support_.font.setFontId(FntGetFont());
    }

    Graphics::Font_t Graphics::setFont(const Graphics::Font_t& font)
    {
        Font_t oldOne=support_.font;
        support_.font=font;
        FntSetFont(support_.font.withEffects());
        return oldOne;
    }

    Graphics::State_t Graphics::pushState()
    {
        WinPushDrawState();
        return support_.font;
    }

    void Graphics::popState(const Graphics::State_t& state)
    {
        setFont(state);
        WinPopDrawState();
    }

}