#include "IconElement.hpp"

#ifndef _PALM_OS
#error IconElement currently works only on Palm OS
#endif 


void IconElement::calculateOrRender(LayoutContext& layoutContext, uint_t left, uint_t top, Definition* definition, bool render)
{
    uint_t totalHeight = height_ + 2 * margin_;
    uint_t baseLine = (totalHeight * 2) / 3;
    layoutContext.extendHeight(totalHeight, baseLine);
    
    uint_t indent=indentation();
    layoutContext.usedWidth=std::max(layoutContext.usedWidth, indent);
    
    if (render)
    {
        left += layoutContext.usedWidth;
        top += (layoutContext.baseLine-baseLine);
    }
    
    uint_t totalWidth = width_ + 2 * margin_;
    
    if (layoutContext.screenWidth - indent < totalWidth)
        layoutContext.markElementCompleted(layoutContext.availableWidth());

    if (layoutContext.availableWidth() < totalWidth)
    {
        layoutContext.usedWidth = layoutContext.screenWidth;
        return;
    }

    layoutContext.markElementCompleted(totalWidth);
   
    if (render && frmInvalidObjectId != bitmapId_)
    {
        MemHandle handle = DmGet1Resource(bitmapRsc, bitmapId_);
        if (handle) 
        {
            BitmapType* bmp = static_cast<BitmapType*>(MemHandleLock(handle));
            if (bmp) 
            {
                if (isHyperlink())
                {
                    ArsRectangle rect(left, top, totalWidth, totalHeight);
                    defineHotSpot(*definition, rect);
                    if (layoutContext.selectionIsHyperlink)
                    {
                        if (0 == layoutContext.selectionStart
                            && layoutContext.progressCompleted == layoutContext.selectionEnd)
                            layoutContext.graphics.invertRectangle(rect);
                        else
                            layoutContext.graphics.erase(rect);
                        
                    }
                }
                WinDrawBitmap(bmp, left + margin_, top + margin_);
                MemHandleUnlock(handle);
            }
            DmReleaseResource(handle);
        }
    }
}

IconElement::~IconElement()
{
}

IconElement::IconElement(uint_t bitmapId):
    imageType_(imageTypeInternalById),
    bitmapId_(bitmapId),
    margin_(0),
    width_(0),
    height_(0)
{
    if (frmInvalidObjectId != bitmapId_)
    {
        MemHandle handle = DmGet1Resource(bitmapRsc, bitmapId_);
        if (handle) 
        {
            BitmapType* bmp = static_cast<BitmapType*>(MemHandleLock(handle));
            if (bmp) 
            {
                Coord bmpW, bmpH;
                UInt16 rowSize;
                BmpGetDimensions(bmp, &bmpW, &bmpH, &rowSize);
                width_ = bmpW;
                height_ = bmpH;
            }
            DmReleaseResource(handle);
        }
    }
}

void IconElement::toText(ArsLexis::String& appendTo, uint_t from, uint_t to) const
{
    if (from!=to)
        appendTo.append(_T("[x]"));
}


uint_t IconElement::charIndexAtOffset(LayoutContext& lc, uint_t offset) 
{
    return 0;
}

void IconElement::wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd)
{
    wordStart = wordEnd = offsetOutsideElement;
}

void IconElement::calculateLayout(LayoutContext& layoutContext)
{
    calculateOrRender(layoutContext, 0, 0, NULL, false);
}

void IconElement::render(RenderingContext& renderContext)
{
    calculateOrRender(renderContext, renderContext.left, renderContext.top, &renderContext.definition, true);
}

