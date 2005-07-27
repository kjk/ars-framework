#ifndef __ARSLEXIS_GEOMETRY_HPP__
#define __ARSLEXIS_GEOMETRY_HPP__

#include <NativeGeometry.hpp>

#ifdef _PALM_OS
struct Point {
    Coord_t x, y;    
    NativePoint_t& native() {return *(NativePoint_t*)this;}
    const NativePoint_t& native() const {return *(const NativePoint_t*)this;}
    operator NativePoint_t& () {return native();}
    operator const NativePoint_t& () const {return native();}
    
#endif
#ifdef _WIN32
struct Point: public NativePoint_t {

#endif

    typedef NativePoint_t Native_t;
    
    Point& set(Coord_t xx, Coord_t yy) {x = xx; y = yy; return *this;}

    Point() {set(0, 0);}
    
    Point(Coord_t xx, Coord_t yy) {set(xx, yy);}
    
    Point(const NativePoint_t& np) {set(np.x, np.y);}
    
    Point& operator = (const NativePoint_t& np) {return set(np.x, np.y);}
    
    Point& operator += (const Point& offset)
    {
        x += offset.x;
        y += offset.y;
        return *this;
    }
   
	Point& operator -= (const Point& offset)
	{
		x -= offset.x;
		y -= offset.y;
		return *this;
	} 
    
    Point operator + (const Point& offset) const
    {
        Point result(*this);
        result += offset;
        return result;
    }
    
    bool operator == (const Point& p) const
    {return (x == p.x) && (y == p.y);}
    
    bool operator != (const Point& p) const
    {return (x != p.x) || (y != p.y);}
    
};

inline Point& PointCast(NativePoint_t& p) {return (Point&)p;}
inline const Point& PointCast(const NativePoint_t& p) {return (const Point&)p;}

#ifdef _PALM_OS
struct Rect
{
    Point topLeft_, extent__;
    NativeRectangle_t& native() {return *(NativeRectangle_t*)this;}
    const NativeRectangle_t& native() const {return *(NativeRectangle_t*)this;}
    operator NativeRectangle_t& () {return native();}
    operator const NativeRectangle_t& () const {return native();}
    
#endif
#ifdef _WIN32
struct Rect: public RECT {
#endif

    typedef NativeRectangle_t Native_t;
    
#ifdef _PALM_OS  
    Point& topLeft() {return topLeft_;}
    const Point& topLeft() const {return topLeft_;}
    Point& extent_() {return extent__;}
    const Point& extent_() const {return extent__;}
    Coord_t x() const {return topLeft_.x;}
    Coord_t& x() {return topLeft_.x;}
    Coord_t y() const {return topLeft_.y;}
    Coord_t& y() {return topLeft_.y;}
    Coord_t width() const
    {return extent__.x;}
    Coord_t height() const
    {return extent__.y;}
   
    void setWidth(Coord_t w) {extent__.x = w;}
    void setHeight(Coord_t h) {extent__.y = h;}
    void setExtent(const Point& p) {extent__ = p;} 
    void setExtent(Coord_t w, Coord_t h) {extent__.x = w; extent__.y = h;}
#endif

#ifdef _WIN32
    Point& topLeft() {return *(Point*)&left;}
    Point& bottomRight_() {return *(Point*)&right;}
    const Point& topLeft() const {return *(Point*)&left;}
    const Point& bottomRight_() const {return *(Point*)&right;}
    Coord_t x() const
    {return left;}
    Coord_t& x()
    {return left;}
    Coord_t y() const
    {return top;}
    Coord_t& y()
    {return top;}
    Coord_t width() const {return right - left;}
    Coord_t height() const {return bottom - top;}
    void setWidth(Coord_t w) {right = left + w;}
    void setHeight(Coord_t h) {bottom = top + h;}
    void setExtent(const Point& p) {setWidth(p.x); setHeight(p.y);}
    void setExtent(Coord_t w, Coord_t h) {setWidth(w); setHeight(h);}
#endif

    Coord_t dx() const {return width();}
    Coord_t dy() const {return height();}

    Rect() {set(0, 0, 0, 0);}
    
    Rect(const Point& tl, const Point& ext) {set(tl.x, tl.y, ext.x, ext.y);}
    
    Rect(Coord_t x, Coord_t y, Coord_t w, Coord_t h) {set(x, y, w, h);}
        
    Rect(const NativeRectangle_t& nr);
    
    Rect& operator = (const NativeRectangle_t& nativeRect);         
    
    bool hitTest(const Point& point) const;
    
    struct HitTest
    {
        const Point& point;
        
        HitTest(const Point& p):
            point(p)
        {}
        
        bool operator() (const Rect& rect) const
        {return rect.hitTest(point);}
        
    };
    
    bool empty() const
    {return (width()==0 || height()==0);}
    
    operator bool () const
    {return !empty();}
    
    void clear()
    {setWidth(0); setHeight(0);}
    
    Rect& operator &= (const Rect& rect);

    Rect operator & (const Rect& rect)
    {
        Rect result(*this);
        result &=rect;
        return result;
    }
        
    bool operator && (const Rect& rect) const
    {
        Rect tmp(*this);
        return (tmp &= rect);
    }

    bool operator && (const Point& point) const
    {return hitTest(point);}
    
    Rect& explode(Coord_t deltaLeft, Coord_t deltaTop, Coord_t deltaWidth, Coord_t deltaHeight)
    {
        x() += deltaLeft;
        y() += deltaTop;
        setWidth(width() + deltaWidth);
        setHeight(height() + deltaHeight);
        return *this;
    }

    Rect& operator+= (const Point& offset)
    {
#ifdef _PALM_OS
        topLeft() += offset;
#else
		set(x() + offset.x, y() + offset.y, width(), height());
#endif        
        return *this;
    }
    
    bool operator == (const Rect& rect) const
    {return (topLeft() == rect.topLeft()) && (width() == rect.width()) && (height() == rect.height());}
    
    bool operator != (const Rect& rect) const
    {return !(*this == rect);}

    Rect& set(const Point& tl, const Point& extent) 
    {
        topLeft() = tl;
        setExtent(extent);
        return *this;
    }
    
    Rect& set(Coord_t xx, Coord_t yy, Coord_t width, Coord_t height)
    {   
        x() = xx; y() = yy; setWidth(width); setHeight(height);
        return *this;
    }
    
    Rect& operator += (const Rect& rect); 
    
    void center(Point& point) const;
                  
};

inline Rect& RectCast(NativeRectangle_t& r) {return (Rect&)r;}
inline const Rect& RectCast(const NativeRectangle_t& r) {return (const Rect&)r;}

inline bool operator && (const Point& p, const Rect& r)
{return (r && p);}

inline Rect operator+(const Rect& r, const Point& p)
{
    Rect tmp(r);
    tmp+=p;
    return tmp;
}        

inline Rect operator + (const Point& p, const Rect& r)
{return r + p;}

#endif
