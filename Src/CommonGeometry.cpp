#include <Geometry.hpp>

namespace ArsLexis
{

    Rectangle& Rectangle::operator &= (const Rectangle& rect)
    {
        using namespace std;
        Coord_t x1=min(x()+width(), rect.x()+rect.width());
        Coord_t y1=min(y()+height(), rect.y()+rect.height());
        x()=max(x(), rect.x());
        y()=max(y(), rect.y());
        if (x1<x() || y1<y())
            clear();
        else
        {
            width()=x1-x();
            height()=y1-y();
        }
        return *this;
    }
    
    bool Rectangle::hitTest(const Point& point) const
    {
        return (point.x>=x() && point.x<x()+width() && point.y>=y() && point.y<y()+height());
    }

    Rectangle& Rectangle::operator+= (const Rectangle& rect) 
    {
        using namespace std;
        Coord_t d0 = min(topLeft.x, rect.topLeft.x);
        Coord_t d1 = max(topLeft.x + extent.x, rect.topLeft.x + rect.extent.x);
        topLeft.x = d0;
        extent.x = d1 - d0;
        d0 = min(topLeft.y, rect.topLeft.y);
        d1 = max(topLeft.y + extent.y, rect.topLeft.y + rect.extent.y);
        topLeft.y = d0;
        extent.y = d1 - d0;
        return *this;
    }
    
}

