#include "Rendering.hpp"

void LayoutContext::extendHeight(uint_t reqHeight, uint_t reqBaseLine)
{
    int blDiff=reqBaseLine-baseLine;
    if (blDiff>0)
    {
        usedHeight+=blDiff;
        baseLine=reqBaseLine;
    }
    if (reqHeight>usedHeight)
        usedHeight=reqHeight;
}

