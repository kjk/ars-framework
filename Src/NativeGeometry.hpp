#ifndef __ARSLEXIS_NATIVE_GEOMETRY_HPP__
#define __ARSLEXIS_NATIVE_GEOMETRY_HPP__

namespace ArsLexis 
{

#if defined(__PALMOS_H__)

    typedef RectangleType NativeRectangle_t;
    typedef PointType NativePoint_t;
    
#elif defined(_WIN32_WCE)

    typedef RECT NativeRectangle_t;
    typedef POINT NativePoint_t;

#else

#error "Define native geometry counterparts in NativeGeometry.hpp before including Geometry.hpp"

#endif

    template<class NativeType, class Wrapper> 
    static inline NativeType toNative(const Wrapper& wrapper)
    {
        NativeType result;
        wrapper.toNative(result);
        return result;
    }

}

#endif