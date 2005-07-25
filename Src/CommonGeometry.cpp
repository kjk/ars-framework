#include <Geometry.hpp>
#include <Utility.hpp>

static StaticAssert<sizeof(Point) == sizeof(NativePoint_t)> point_has_proper_size;
static StaticAssert<sizeof(Rect) == sizeof(NativeRectangle_t)> rect_has_proper_size0;

Rect& Rect::operator &= (const Rect& rect)
{
    using namespace std;
    Coord_t x1=min(x() + width(), rect.x() + rect.width());
    Coord_t y1=min(y() + height(), rect.y() + rect.height());
    x()=max(x(), rect.x());
    y()=max(y(), rect.y());
    if (x1 < x() || y1 < y())
        clear();
    else
    {
        setWidth(x1 - x());
        setHeight(y1 - y());
    }
    return *this;
}

bool Rect::hitTest(const Point& point) const
{
    return (point.x >= x() && point.x < x() + width() && point.y >= y() && point.y < y() + height());
}

Rect& Rect::operator+= (const Rect& rect) 
{
    using namespace std;
    Coord_t d0 = min(x(), rect.x());
    Coord_t d1 = max(x() + width(), rect.x() + rect.width());
    x() = d0;
    setWidth(d1 - d0);
    d0 = min(y(), rect.y());
    d1 = max(y() + height(), rect.y() + rect.height());
    y() = d0;
    setHeight(d1 - d0);
    return *this;
}


void Rect::center(Point& p) const
{
	p.x = x() + width() / 2; 
	p.y = y() + height() / 2;
}
