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
        
        Coord& x()
        {return topLeft().x;}
        
        Coord y() const
        {return topLeft().y;}
        
        Coord& y()
        {return topLeft().y;}
        
        Coord width() const
        {return extent().x;}
        
        Coord& width()
        {return extent().x;}

        Coord height() const
        {return extent().y;}

        Coord& height()
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
        
        Rectangle& operator=(const RectangleType* rect)
        {
            RctCopyRectangle(rect, &data);
            return *this;
        }
        
        Boolean intersectsWith(const Rectangle& rect) const
        {
            Rectangle tmp;
            RctGetIntersection(&data, &rect.data, &tmp.data);
            return (tmp.width() && tmp.height());
        }
        
        Boolean operator && (const Rectangle& rect) const
        {return intersectsWith(rect);}
        
        Boolean operator && (const PointType& point) const
        {return hitTest(point);}
        
        void explode(Coord deltaLeft, Coord deltaTop, Coord deltaWidth, Coord deltaHeight)
        {
            x()+=deltaLeft;
            y()+=deltaTop;
            width()+=deltaWidth;
            height()+=deltaHeight;
        }
                
    };
    
    struct Point
    {
        PointType data;
        
        Point()
        {data.x=data.y=0;}
        
        Point(Coord x, Coord y)
        {data.x=x; data.y=y;}
        
        operator const PointType* () const
        {return &data;}
        
        operator PointType* ()
        {return &data;}
        
    };
    
}


#endif