#ifndef __ARSLEXIS_GEOMETRY_HPP__
#define __ARSLEXIS_GEOMETRY_HPP__

#include <PalmOS.h>

namespace ArsLexis
{

    struct Rectangle // Unfortunately we can't inherit from RectangleType because typedef struct RectangleType masks its name.
    {
        RectangleType data;
        
        Rectangle()
        {RctSetRectangle(&data, 0, 0, 0, 0);}
        
        Rectangle(const RectangleType& rect)
        {RctCopyRectangle(&rect, &data);}
        
        Rectangle(const PointType& tl, const PointType& ex)
        {RctSetRectangle(&data, tl.x, tl.y, ex.x, ex.y);}
        
        Rectangle(Coord x, Coord y, Coord dx, Coord dy)
        {RctSetRectangle(&data, x, y, dx, dy);}
        
        Boolean hitTest(const PointType& point) const
        {return RctPtInRectangle(point.x, point.y, &data);}
        
        operator const RectangleType* () const
        {return &data;}
        
        operator RectangleType* ()
        {return &data;}
        
        const PointType& topLeft() const
        {return data.topLeft;}
        
        PointType& topLeft()
        {return data.topLeft;}
        
        const PointType extent() const
        {return data.extent;}
        
        PointType& extent()
        {return data.extent;}
        
        Coord x() const
        {return topLeft().x;}
        
        Coord y() const
        {return topLeft().y;}
        
        Coord width() const
        {return extent().x;}
        
        Coord height() const
        {return extent().y;}

        struct HitTest
        {
            const PointType& point;
            
            HitTest(const PointType& p):
                point(p)
            {}
            
            bool operator() (const Rectangle& rect) const
            {
                return rect.hitTest(point);
            }
            
        };
        
        Rectangle& operator=(const RectangleType& rect)
        {
            RctCopyRectangle(&rect, &data);
            return *this;
        }
                
    };
    
}


#endif