#include <PalmOS.h>
#include <Geometry.hpp>

Point::Point(const NativePoint_t& nativePoint):
    x(nativePoint.x),
    y(nativePoint.y)
{}

void Point::toNative(NativePoint_t& nativePoint) const
{
    nativePoint.x=x;
    nativePoint.y=y;
}

Point& Point::operator=(const NativePoint_t& nativePoint)
{
    x=nativePoint.x;
    y=nativePoint.y;
    return *this;
}

ArsRectangle& ArsRectangle::operator=(const NativeRectangle_t& nativeRect)
{
    topLeft=nativeRect.topLeft;
    extent=nativeRect.extent;
    return *this;        
}

ArsRectangle::ArsRectangle(const NativeRectangle_t& nativeRect)
{
    operator=(nativeRect);
}
    
void ArsRectangle::toNative(NativeRectangle_t& nativeRect) const
{
    nativeRect.topLeft.x=x();
    nativeRect.topLeft.y=y();
    nativeRect.extent.x=width();
    nativeRect.extent.y=height();
}
