#ifndef __ARSLEXIS_NATIVE_GEOMETRY_HPP__
#define __ARSLEXIS_NATIVE_GEOMETRY_HPP__

#if defined(_WIN32_WCE)
# include <windows.h>
#endif

#if defined(_PALM_OS)

#include <Rect.h>

typedef RectangleType NativeRectangle_t;
typedef PointType NativePoint_t;
typedef Coord Coord_t;

#elif defined(_WIN32_WCE)

typedef RECT NativeRectangle_t;
typedef POINT NativePoint_t;
typedef LONG Coord_t;

#else

#error "Define native geometry counterparts in NativeGeometry.hpp before including Geometry.hpp"

#endif

#endif