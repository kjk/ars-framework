#ifndef __ARSLEXIS_GRAPHICS_HPP__
#define __ARSLEXIS_GRAPHICS_HPP__

#if defined(_PALM_OS)
# include <PalmOS/PalmGraphics.hpp>
#elif defined(_WIN32_WCE)
# include <WindowsCE/WinGraphics.hpp>
#else
# error "Define Graphics system for your build target."
#endif

typedef Graphics::Font_t Font;
typedef Graphics::Color_t Color;

#endif