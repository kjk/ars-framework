#ifndef __ARSLEXIS_GEOMETRY_HPP__
#define __ARSLEXIS_GEOMETRY_HPP__

#include <algorithm>
#include <NativeGeometry.hpp>

namespace ArsLexis
{

    typedef int Coord_t;

    struct Point
    {
        typedef NativePoint_t Native_t;
    
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
        
        bool operator==(const Point& p) const
        {return (x==p.x) && (y==p.y);}
        
        bool operator!=(const Point& p) const
        {return (x!=p.x) || (y!=p.y);}
        
    };

    struct Rectangle
    {
        typedef NativeRectangle_t Native_t;
        
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

        Coord_t dx() const
        {return extent.x;}

        Coord_t& dx()
        {return extent.x;}

        Coord_t height() const
        {return extent.y;}

        Coord_t& height()
        {return extent.y;}

        Coord_t dy() const
        {return extent.y;}

        Coord_t& dy()
        {return extent.y;}

//        bool hitTest(const Point& point) const
//        {return (point.x>=x() && point.x<x()+width() && point.y>=y() && point.y<y()+height());}
        bool hitTest(const Point& point) const;
        
        struct HitTest
        {
            const Point& point;
            
            HitTest(const Point& p):
                point(p)
            {}
            
            bool operator() (const Rectangle& rect) const
            {return rect.hitTest(point);}
            
        };
        
        bool empty() const
        {return (width()==0 || height()==0);}
        
        operator bool () const
        {return !empty();}
        
        void clear()
        {width()=0; height()=0;}
        
        Rectangle& operator &= (const Rectangle& rect);

        Rectangle operator & (const Rectangle& rect)
        {
            Rectangle result(*this);
            result &=rect;
            return result;
        }
            
        bool operator && (const Rectangle& rect) const
        {
            Rectangle tmp(*this);
            return (tmp &= rect);
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
        
        bool operator==(const Rectangle& rect) const
        {return (topLeft==rect.topLeft) && (extent==rect.extent);}
        
        bool operator!=(const Rectangle& rect) const
        {return (topLeft!=rect.topLeft) || (extent!=rect.extent);}

        Rectangle& assign(const Point& tl, const Point& ext) 
        {
            topLeft = tl;
            extent = ext;
            return *this;
        }
        
        Rectangle& assign(Coord_t x, Coord_t y, Coord_t width, Coord_t height)
        {   
            topLeft.x = x;
            topLeft.y = y;
            extent.x = width;
            extent.y = height;
            return *this;
        }
        
        Rectangle& operator+= (const Rectangle& rect); 
        
        void center(Point& point) const;
                      
    };
    
    inline bool operator && (const Point& p, const Rectangle& r)
    {return (r && p);}
    
    inline Rectangle operator+(const Rectangle& r, const Point& p)
    {
        Rectangle tmp(r);
        tmp+=p;
        return tmp;
    }        

    inline Rectangle operator+(const Point& p, const Rectangle& r)
    {return r+p;}
    
}


#endif
