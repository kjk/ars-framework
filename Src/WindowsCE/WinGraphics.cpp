#include <Debug.hpp>
#include <Text.hpp>
#include "WinGraphics.hpp"

Graphics::Graphics(const NativeGraphicsHandle_t& handle):
    handle_(handle), 
	hwnd_(NULL)
{
	init();
}

Graphics::Graphics(HWND hwnd):
    handle_(::GetDC(hwnd)), 
	hwnd_(hwnd)
{
	init();
}

void Graphics::init()
{
	fontHeight_ = 0;
	fontBaseline_ = 0;

	currentFont_.attach((HFONT)GetStockObject(SYSTEM_FONT), true);
	assert(currentFont_.valid());
	originalFont_ = (HFONT)SelectObject(handle_, currentFont_.handle());
	assert(NULL != originalFont_);
	queryFontMetrics();

#ifdef DEBUG
    statePushCounter_ = 0;
#endif
	initPen();
}

void Graphics::initPen()
{
    HGDIOBJ hgdiobj = GetStockObject(BLACK_PEN);
    GetObject(hgdiobj, sizeof(pen_), &pen_);
    penColor_=pen_.lopnColor;
    SelectObject(handle_,hgdiobj);
}

void Graphics::setFont(const WinFont& font)
{
	assert(font.valid());
	if (!font.valid())
		return;

	if (currentFont_.handle() != font.handle())
	{
		SelectObject(handle_, font.handle());
		queryFontMetrics();
	}
	currentFont_ = font;
}


Graphics::State_t Graphics::pushState()
{
    State_t st;
    st.state = SaveDC(handle_);
    st.fontHeight = fontHeight_;
    st.fontBaseline = fontBaseline_;
	st.font = currentFont_;

#ifdef DEBUG
    statePushCounter_ += 1;
#endif
    return st;
}

void Graphics::popState(const Graphics::State_t& state)
{
    RestoreDC(handle_,state.state);
    fontHeight_ = state.fontHeight;
    fontBaseline_ = state.fontBaseline;
	currentFont_ = state.font;

#ifdef DEBUG
    statePushCounter_ -= 1;
    assert(statePushCounter_>=0);
#endif
}

Graphics::~Graphics()
{
	if (NULL != originalFont_)
		SelectObject(handle_, originalFont_);

#ifdef DEBUG
    assert(0 == statePushCounter_);
#endif

    if(NULL != hwnd_)
        ReleaseDC(hwnd_, handle_);
    else
        DeleteDC(handle_);
}

void Graphics::drawText(const char_t* text, uint_t length, const Point& topLeft, bool inverted)
{
    uint_t len=length;
    uint_t width=0x7fffffff;
    charsInWidth(text, len, width);

    uint_t height=fontHeight();
    uint_t top=topLeft.y;

    NativeColor_t back;
    NativeColor_t fore;
    if (inverted)
    {
        back = GetBkColor(handle_);
        fore = this->setForegroundColor(back);
        SetTextColor(handle_,back);
        SetBkColor(handle_,fore);
    }

    ExtTextOut(handle_, topLeft.x, top, 0, NULL, text, length, NULL);
    
    if (inverted)
    {
        back = GetBkColor(handle_);
        fore = this->setForegroundColor(back);
        SetTextColor(handle_,fore);
        SetBkColor(handle_,fore);
    }

}

void Graphics::erase(const ArsRectangle& rect)
{
    NativeRectangle_t nr=toNative(rect);
    HBRUSH hbr = CreateSolidBrush(GetBkColor(handle_));
    FillRect(handle_, &nr, hbr);
    DeleteObject(hbr);
}

void Graphics::copyArea(const ArsRectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft)
{
    NativeRectangle_t nr=toNative(sourceArea);
    BitBlt(targetSystem.handle_, targetTopLeft.x, targetTopLeft.y,
        sourceArea.width(), sourceArea.height(), 
        handle_,
        nr.left, nr.top, SRCCOPY);
}

void Graphics::drawLine(Coord_t x0, Coord_t y0, Coord_t x1, Coord_t y1)
{
    POINT p[2];
    p[0].x=x0;
    p[0].y=y0;
    p[1].x=x1;
    p[1].y=y1;
    Polyline(handle_, p, 2);
}

NativeColor_t Graphics::setForegroundColor(NativeColor_t color)
{
    HGDIOBJ hgdiobj = GetCurrentObject(handle_,OBJ_PEN);
    NativeColor_t old = pen_.lopnColor;
    pen_.lopnColor = color;
    HPEN newPen = CreatePenIndirect(&pen_);
    SelectObject(handle_,newPen);
    DeleteObject(hgdiobj);
    return old;
}

NativeColor_t Graphics::setBackgroundColor(NativeColor_t color)
{
    return SetBkColor(handle_,color);
}

NativeColor_t Graphics::setTextColor(NativeColor_t color)
{
    return SetTextColor(handle_,color ); 
}

uint_t Graphics::wordWrap(const char_t* text, uint_t availableDx, uint_t& textDx)
{
    int     lenThatFits;
    SIZE    size;
    int     textLen = tstrlen(text);

DoItAgain:
    GetTextExtentExPoint(handle_, text, textLen, availableDx, &lenThatFits, NULL, &size);

    if ( (lenThatFits==textLen) || (0==lenThatFits))
    {
        textDx = size.cx;
        return lenThatFits;
    }

    // the string is too big so try to find a line-breaking place (a whitespace)
    int curPos = lenThatFits-1;
    while (curPos>=0)
    {
        if (isSpace(text[curPos]))
        {
            textLen = curPos+1;
            // this time it should succeed 100%
            goto DoItAgain;
        }
        --curPos;
    }
    
    textDx = textWidth(text, lenThatFits);
    return lenThatFits;
}

// given a string text, determines how many characters of this string can
// be displayed in a given width. If text is bigger than width, a line
// break will be on newline, tab or a space.
// Mimics Palm OS function FntWordWrap()
uint_t Graphics::wordWrap(const char_t* text, uint_t width)
{
    uint_t textDx;
    uint_t lenThatFits = wordWrap(text, width, textDx);
    return lenThatFits;
}

uint_t Graphics::textWidth(const char_t* text, uint_t length)
{
    SIZE size;
    GetTextExtentPoint(handle_, text, length, &size);
    return size.cx;
}

void Graphics::charsInWidth(const char_t* text, uint_t& length, uint_t& width)
{
    int   len;
    SIZE  size;
    GetTextExtentExPoint(handle_, text, length, width, &len, NULL, &size ); 
    length = len;
    width = size.cx;
}

/*void Graphics::setEffects(FontEffects::Weight weight, bool italic, bool index, bool isSmall)
{
    oldFont_ = GetCurrentObject(handle_,OBJ_FONT);
    GetObject(oldFont_, sizeof(fontDescr_), &fontDescr_);
    switch(weight)
    {
    case FontEffects::weightPlain:
        fontDescr_.lfWeight = FW_NORMAL;
        break;
    case FontEffects::weightBold: 
        fontDescr_.lfWeight = FW_BOLD;
        break;
    case FontEffects::weightBlack:
        fontDescr_.lfWeight = FW_EXTRABOLD;
        break;
    }    
    if (italic&&(weight==FontEffects::weightPlain))
        fontDescr_.lfWeight = FW_BOLD;
    if (index)
        fontDescr_.lfHeight = fontDescr_.lfHeight * 3 / 4; // * 0.75;
    if (isSmall)
        fontDescr_.lfHeight = fontDescr_.lfHeight * 9 / 10; // * 0.9
    newFont_ = CreateFontIndirect(&fontDescr_);
    SelectObject(handle_, newFont_);
}*/

/*void Graphics::resetEffects()
{
    /*SelectObject(handle_, oldFont_);
    DeleteObject(newFont_);
}*/

void Graphics::queryFontMetrics()
{
	TEXTMETRIC metrics;
	BOOL res = GetTextMetrics(handle_, &metrics);
	if (!res)
		return;

	fontHeight_ = metrics.tmHeight;
	fontBaseline_ = metrics.tmAscent;
}


void Graphics::applyStyle(const DefinitionStyle* style, bool isHyperlink)
{
}

