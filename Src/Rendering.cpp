#include "Rendering.hpp"

void LayoutContext::extendHeight(Coord reqHeight, Coord reqBaseLine)
{
    Coord blDiff=reqBaseLine-baseLine;
    if (blDiff>0)
    {
        usedHeight+=blDiff;
        baseLine=reqBaseLine;
    }
    if (reqHeight>usedHeight)
        usedHeight=reqHeight;
}

