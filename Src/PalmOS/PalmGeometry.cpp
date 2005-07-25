#include <PalmOS.h>
#include <Geometry.hpp>

Rect& Rect::operator=(const NativeRectangle_t& nativeRect)
{
    topLeft_ = nativeRect.topLeft;
    extent__ = nativeRect.extent;
    return *this;        
}

Rect::Rect(const NativeRectangle_t& nativeRect)
{
    operator=(nativeRect);
}
