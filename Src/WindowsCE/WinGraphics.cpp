#include <WinGraphics.hpp>
#include <Wingdi.h>

namespace ArsLexis
{

    Graphics::Graphics(const NativeGraphicsHandle_t& handle, HWND hwnd):
        handle_(handle), hwnd_(hwnd)
    {
        //support_.font.setFontId(FntGetFont());
    }

    Graphics::Font_t Graphics::setFont(const Graphics::Font_t& font)
    {
        Font_t oldOne=font_;
        font_=font;
        //FntSetFont(support_.font.withEffects());
        SelectObject(handle_, font.getHandle());
        return oldOne;
    }

    Graphics::State_t Graphics::pushState()
    {
        StackElement el;
        el.font=font_;
        el.state = SaveDC(handle_);
        fontStack_.push_back(el);
        return el.state;
    }

    void Graphics::popState(const Graphics::State_t& state)
    {
        StackElement el;
        do 
        {
            el = fontStack_.back();
            fontStack_.pop_back();
        }
        while(el.state!=state);
        font_=el.font;
        RestoreDC(handle_,state);
    }
    
    Graphics::~Graphics()
    {
        if(hwnd_)
            ReleaseDC(this->hwnd_,this->handle_);
        else
            DeleteDC(this->handle_);
    }

    void Graphics::drawText(const char_t* text, uint_t length, const Point& topLeft, bool inverted)
    {
        FontEffects fx=font_.effects();

        uint_t len=length;
        uint_t width=0x7fffffff;
        charsInWidth(text, len, width);

        uint_t height=fontHeight();
        uint_t top=topLeft.y;

        
        if (fx.subscript())
            top+=(uint_t)(height*0.4);

        if (fx.isSmall())
            top+=(uint_t)(height*0.1);
        
       
        if (fx.mask()!=0)
            setEffects(fx.weight(), fx.italic(), fx.subscript()||fx.superscript(),fx.isSmall());
        

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

        if (fx.mask()!=0)            
            resetEffects();

        if (fx.strikeOut())
        {
            uint_t baseline=fontBaseline();
            // top=topLeft.y+baseline*0.667;
            top = topLeft.y + ((baseline * 667) / 1000);
            drawLine(topLeft.x, top, topLeft.x+width, top);
        }

        int style;
        switch (fx.underline())
        {
            case ArsLexis::FontEffects::underlineDotted:
                style = PS_DASH;
                break;
            case ArsLexis::FontEffects::underlineSolid:
                style = PS_SOLID;
                break;
        }
        //Still underlining doesn't work
        if (fx.underline()!=ArsLexis::FontEffects::underlineNone)
        {
            LOGPEN pen;
            //TODO: again not effective
            HGDIOBJ hgdiobj = GetCurrentObject(handle_,OBJ_PEN);
            GetObject(hgdiobj, sizeof(pen), &pen);
            pen.lopnStyle = style;
            HPEN newPen=CreatePenIndirect(&pen);
            SelectObject(handle_,newPen);
            uint_t baseline=fontBaseline();
            top=topLeft.y + height - baseline+1;
            drawLine(topLeft.x, top, topLeft.x+width, top);            
            SelectObject(handle_,hgdiobj);
            DeleteObject(newPen);
        }

        //(handle_, topLeft.x , topLeft.y, text, length);
    }
    
    void Graphics::erase(const ArsLexis::Rectangle& rect)
    {
        NativeRectangle_t nr=toNative(rect);
        //TODO: Not effective at all
        HBRUSH hbr=CreateSolidBrush(GetBkColor(handle_));
        FillRect(handle_, &nr, hbr);
        DeleteObject(hbr);
    }
    
    void Graphics::copyArea(const Rectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft)
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
        LOGPEN pen;
        NativeColor_t old;
        //TODO: again not effective
        HGDIOBJ hgdiobj = GetCurrentObject(handle_,OBJ_PEN);
        GetObject(hgdiobj, sizeof(pen), &pen);
        old = pen.lopnColor;
        pen.lopnColor = color;
        HPEN newPen=CreatePenIndirect(&pen);
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
    
    
    uint_t Graphics::fontHeight() const
    {
        /*LOGFONT fnt;
        HGDIOBJ font=GetCurrentObject(handle_, OBJ_FONT);
        GetObject(font, sizeof(fnt), &fnt); 
        //FntLineHeight();
        //FontEffects fx=support_.font.effects();
        //if (fx.superscript() || fx.subscript())
        //    height*=1.333;*/
        
        FontEffects fx=font_.effects();

        TEXTMETRIC ptm;
        GetTextMetrics(handle_, &ptm);
        if(fx.isSmall())
            return (uint_t)(ptm.tmHeight*0.9);
        else
            return ptm.tmHeight;
        //return -fnt.lfHeight;
    }
   
    uint_t Graphics::fontBaseline() const
    {
        //uint_t baseline=FntBaseLine();
        //if (support_.font.effects().superscript())
            //baseline+=(FntLineHeight()*0.333);
        const FontEffects fx=font_.effects();
        TEXTMETRIC ptm;
        GetTextMetrics(handle_, &ptm);
        return ptm.tmDescent;
    }

    // given a string text, determines how many characters of this string can
    // be displayed in a given width. If text is bigger than width, a line
    // break will be on newline, tab or a space.
    // Mimics Palm OS function FntWordWrap()
    uint_t Graphics::wordWrap(const char_t* text, uint_t width)
    {
        int     len;
        SIZE    size;
        int     textLen = tstrlen(text);
        FontEffects fx=font_.effects();

        if (fx.mask()!=0)
            setEffects(fx.weight(), fx.italic(), fx.subscript()||fx.superscript(), fx.isSmall());
        GetTextExtentExPoint(handle_, text, textLen, width, &len, NULL, &size);
        if (fx.mask()!=0)
            resetEffects();

        if (len==textLen)
            return len;

        // the string is too big so try to find a line-breaking spot
        int curPos = len;
        while (curPos>0) // we don't want to break at first character
        {            
            if ((text[curPos]==_T(' '))  ||
                (text[curPos]==_T('\t')) ||
                (text[curPos]==_T('\n'))
               ) 
            {
                return curPos;
            }
            --curPos;
        }
        // we didn't find a line-breaking spot, so return everything that'll fit
        return len;
    }

    uint_t Graphics::textWidth(const char_t* text, uint_t length)
    {
        FontEffects fx=font_.effects();
        SIZE size;
        if (fx.mask()!=0)            
            setEffects(fx.weight(), fx.italic(), fx.subscript()||fx.superscript(), fx.isSmall());
        GetTextExtentPoint(handle_, text, length, &size);
        if (fx.mask()!=0)
            resetEffects();
        return size.cx;
    }
    
    void Graphics::charsInWidth(const char_t* text, uint_t& length, uint_t& width)
    {
        FontEffects fx=font_.effects();
        int   len;
        SIZE  size;
        if (fx.mask()!=0)
            setEffects(fx.weight(), fx.italic(), fx.subscript()||fx.superscript(), fx.isSmall());
        GetTextExtentExPoint(handle_, text, length, width, &len, NULL, &size ); 
        if (fx.mask()!=0)
            resetEffects();

        length = len;
        width = size.cx;
    }

    Graphics::Font_t Graphics::font() const
    {return font_;}
    
    void Graphics::setEffects(FontEffects::Weight weight, bool italic, bool index, bool isSmall)
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
    }
    
    void Graphics::resetEffects()
    {
        SelectObject(handle_, oldFont_);
        DeleteObject(newFont_);
    }

}
