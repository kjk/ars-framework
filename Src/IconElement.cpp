#include "IconElement.hpp"

void IconElement::calculateLayout(LayoutContext& layoutContext)
{
    layoutContext.extendHeight(height_+2*spacePx_, height_+2*spacePx_);
    layoutContext.markElementCompleted(width_+2*spacePx_);
}

void IconElement::render(RenderingContext& renderingContext)
{
    uint_t indent=indentation();
    if (renderingContext.usedWidth<indent)
        renderingContext.usedWidth=indent;
    
    if (frmInvalidObjectId != bitmapId_)
    {
        MemHandle handle = DmGet1Resource(bitmapRsc, bitmapId_);
        if (handle) 
        {
            BitmapType* bmp = static_cast<BitmapType*>(MemHandleLock(handle));
            if (bmp) 
            {
                WinDrawBitmap(bmp, renderingContext.usedWidth+spacePx_, renderingContext.top+spacePx_);
                if (isHyperlink())
                {
                    ArsRectangle imgArea(renderingContext.usedWidth, renderingContext.top, width_+2*spacePx_, height_+2*spacePx_);
                    defineHotSpot(renderingContext.definition, imgArea);
                }
                
                MemHandleUnlock(handle);
            }
            DmReleaseResource(handle);
        }
    }
    renderingContext.markElementCompleted(width_+2*spacePx_);
}
    
IconElement::~IconElement()
{
    if (imageTypeInternalById == imageType_)
        return;
    else
        assert(false);
}

IconElement::IconElement(uint_t bitmapId)
{
    imageType_ = imageTypeInternalById;
    bitmapId_ = bitmapId;
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
                spacePx_ = 0;
            }
            DmReleaseResource(handle);
        }
    }
}

uint_t IconElement::charIndexAtOffset(LayoutContext& lc, uint_t offset) {
    return 0;
}

void IconElement::wordAtIndex(LayoutContext& lc, uint_t index, uint_t& wordStart, uint_t& wordEnd)
{
    wordStart = wordEnd = offsetOutsideElement;
}
