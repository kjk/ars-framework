#include "Geometry.hpp"

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
    topLeft=Point(nativeRect.left,nativeRect.top);
    extent=Point(nativeRect.right-nativeRect.left,
        nativeRect.bottom-nativeRect.top);
    return *this;        
}

ArsRectangle::ArsRectangle(const NativeRectangle_t& nativeRect)
{
    operator=(nativeRect);
}

void ArsRectangle::toNative(NativeRectangle_t& nativeRect) const
{
    nativeRect.top=y();
    nativeRect.left=x();
    nativeRect.right=nativeRect.left+width();
    nativeRect.bottom=nativeRect.top+height();
}

