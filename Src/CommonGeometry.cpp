#include <Geometry.hpp>

namespace ArsLexis
{

    Rectangle& Rectangle::operator &= (const Rectangle& rect)
    {
        Coord_t x1=std::min(x()+width(), rect.x()+rect.width());
        Coord_t y1=std::min(y()+height(), rect.y()+rect.height());
        x()=std::max(x(), rect.x());
        y()=std::max(y(), rect.y());
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

}