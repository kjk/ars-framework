#ifndef __ARSLEXIS_GEOMETRY_HPP__
#define __ARSLEXIS_GEOMETRY_HPP__

#include <algorithm>
#include "NativeGeometry.hpp"

namespace ArsLexis
{

    typedef int Coord_t;

    struct Point
    {
        Coord_t x,y;
        
        Point(): x(0), y(0) {}
        
        Point(Coord_t xx, Coord_t yy): x(xx), y(yy) {}
        
        Point(const NativePoint_t& nativePoint);
        
        void toNative(NativePoint_t& nativePoint) const;
        
        Point& operator=(const NativePoint_t& nativePoint);
        
        Point& operator+=(const Point& offset)
        {
            x+=offset.x;
            y+=offset.y;
            return *this;
        }
        
        Point operator + (const Point& offset) const
        {
            Point result(*this);
            result+=offset;
            return result;
        }
        
    };

    struct Rectangle // Unfortunately we can't inherit from RectangleType because typedef struct RectangleType masks its name.
    {
        Point topLeft;
        Point extent;
        
        Rectangle() {}
        
        Rectangle(const Point& tl, const Point& ex): topLeft(tl), extent(ex) {}
        
        Rectangle(Coord_t x, Coord_t y, Coord_t width, Coord_t height):
            topLeft(x, y), extent(width, height) {}
            
        Rectangle(const NativeRectangle_t& nativeRect);
        
        void toNative(NativeRectangle_t& nativeRect) const;   
        
        Rectangle& operator=(const NativeRectangle_t& nativeRect);         
        
        Coord_t x() const
        {return topLeft.x;}
        
        Coord_t& x()
        {return topLeft.x;}
        
        Coord_t y() const
        {return topLeft.y;}
        
        Coord_t& y()
        {return topLeft.y;}
        
        Coord_t width() const
        {return extent.x;}
        
        Coord_t& width()
        {return extent.x;}

        Coord_t height() const
        {return extent.y;}

        Coord_t& height()
        {return extent.y;}

        bool hitTest(const Point& point) const
        {return (point.x>=x() && point.x<x()+width() && point.y>=y() && point.y<y()+height());}
        
        struct HitTest
        {
            const Point& point;
            
            HitTest(const Point& p):
                point(p)
            {}
            
            bool operator() (const Rectangle& rect) const
            {
                return rect.hitTest(point);
            }
            
        };
        
        bool empty() const
        {return (width()>0 && height()>0);}
        
        operator bool () const
        {return !empty();}
        
        void clear()
        {width()=0; height()=0;}
        
        Rectangle& operator &= (const Rectangle& rect)
        {
            x()=std::max(x(), rect.x());
            y()=std::max(y(), rect.y());
            Coord_t x1=std::min(x()+width(), rect.x()+rect.width());
            Coord_t y1=std::min(y()+height(), rect.y()+rect.height());
            if (x1<x() || y1<y())
                clear();
            else
            {
                width()=x1-x();
                height()=y1-y();
            }
            return *this;
        }
        
        Rectangle operator & (const Rectangle& rect)
        {
            Rectangle result(*this);
            result &=rect;
            return result;
        }
            
        bool operator && (const Rectangle& rect) const
        {
            Rectangle tmp(*this);
            tmp &= rect;
            return tmp.operator bool();
        }
        
        bool operator && (const Point& point) const
        {return hitTest(point);}
        
        Rectangle& explode(Coord_t deltaLeft, Coord_t deltaTop, Coord_t deltaWidth, Coord_t deltaHeight)
        {
            x()+=deltaLeft;
            y()+=deltaTop;
            width()+=deltaWidth;
            height()+=deltaHeight;
            return *this;
        }

        Rectangle& operator+= (const Point& offset)
        {
            topLeft+=offset;
            return *this;
        }
              
    };

}


#endif