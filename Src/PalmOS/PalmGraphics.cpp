#include <PalmOS.h>
#include <Graphics.hpp>
#include <DefinitionStyle.hpp>

static bool useFontScaling()
{
    static bool checked = false;
    static bool useScaling = false;
    if (checked)
        return useScaling;

    checked = true;

    // http://www.palmos.com/dev/support/docs/palmos/CompatibilityApdx.html#997148
    UInt32 val;
    Err error = FtrGet(sysFtrCreator, sysFtrNumWinVersion, &val);
    if ((errNone == error) && (val >= 5))
        useScaling = true;
    else
        return useScaling; // i.e. false

    error = WinScreenGetAttribute(winScreenDensity, &val);
    if (errNone != error || kDensityLow == val)
        useScaling = false;
    
    return useScaling;
}

class ScalingSetter: private NonCopyable {
    UInt32 oldFlags_;
    bool disable_;
    
public:
    
    ScalingSetter(const Graphics& gr)
    : disable_(gr.disableFontScaling_)
    {
        if (disable_)
            oldFlags_ = WinSetScalingMode(kTextScalingOff /* | kTextPaddingOff */);
    }
    
    ~ScalingSetter() 
    {
        if (disable_)
            WinSetScalingMode(oldFlags_);
    }
    
};

Graphics::Font_t Graphics::setFont(const Graphics::Font_t& font)
{
    Font_t oldOne = font_;
    font_ = font;
    FontID id = font_.withEffects();
    bool wantsScaling = 0 != (id & fontScalingDisabled);
    disableFontScaling_ = wantsScaling && useFontScaling();
    if (wantsScaling && !disableFontScaling_)
        id =stdFont;
    ScalingSetter setScaling(*this);
    FntSetFont(FontID(id & ~fontScalingDisabled));
    effectiveLineHeight_ = lineHeight_ = FntLineHeight();
    FontEffects fx=font_.effects();
    if (fx.superscript() || fx.subscript())
    {
        effectiveLineHeight_*=4;
        effectiveLineHeight_/=3;
    }
    effectiveBaseline_=baseline_ = FntBaseLine();
    if (font_.effects().superscript())
        effectiveBaseline_ += (lineHeight_/3);
    return oldOne;
}

class PalmUnderlineSetter
{
    UnderlineModeType originalUnderline_;
public:
    
    explicit PalmUnderlineSetter(UnderlineModeType newMode):
        originalUnderline_(WinSetUnderlineMode(newMode))
    {}
    
    ~PalmUnderlineSetter()
    {WinSetUnderlineMode(originalUnderline_);}
    
};

inline static UnderlineModeType convertUnderlineMode(FontEffects::Underline underline)
{
    UnderlineModeType result=noUnderline;
    switch (underline)
    {
        case FontEffects::underlineDotted:
            result=grayUnderline;
            break;
        case FontEffects::underlineSolid:
            result=solidUnderline;
            break;
    }
    return result;                
}
    
void Graphics::drawText(const char_t* text, uint_t length, const Point& topLeft, bool inverted)
{
    FontEffects fx = font_.effects();    
    PalmUnderlineSetter setUnderline(convertUnderlineMode(fx.underline()));

    ScalingSetter setScaling(*this);

    uint_t height = fontHeight();
    uint_t top=topLeft.y;
    if (fx.subscript())
        top+=(height/3);
    if (inverted)
        WinDrawInvertedChars(text, length, topLeft.x, top);
    else            
        WinDrawChars(text, length, topLeft.x, top);
    if (fx.strikeOut())
    {
        uint_t baseline = fontBaseline();
        top=topLeft.y + (baseline*2)/3;
        uint_t width = FntCharsWidth(text, length);
        Color_t color=setTextColor(0);
        setTextColor(color); // Quite strange method of querying current text color...
        color=setForegroundColor(color);
        WinDrawOperation old;
        if (inverted)
            old=WinSetDrawMode(winInvert);
        drawLine(topLeft.x, top, topLeft.x+width, top);
        if (inverted)
            WinSetDrawMode(old);
        setForegroundColor(color);
    }
  }

void Graphics::charsInWidth(const char_t* text, uint_t& length, uint_t& width)
{

    ScalingSetter setScaling(*this);

    Int16 w=width;
//        Int16 len=length;
//        Boolean dontMind;
//        FntCharsInWidth(text, &w, &len, &dontMind);
//        length=len;
    length = FntWidthToOffset(text, length, width, NULL, &w);
    width=w;

}

uint_t Graphics::wordWrap(const char_t* text, uint_t availableDx, uint_t& textDx)
{
    ScalingSetter setScaling(*this);

    int charsThatFit = FntWordWrap(text, availableDx);

    textDx = textWidth(text, charsThatFit);
    return charsThatFit;
}

void Graphics::erase(const ArsRectangle& rect)
{
    RectangleType nr=toNative(rect);
    WinEraseRectangle(&nr, 0);
}

void Graphics::copyArea(const ArsRectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft)
{
    RectangleType nr=toNative(sourceArea);
    WinCopyRectangle(handle_, targetSystem.handle_, &nr, targetTopLeft.x, targetTopLeft.y, winPaint);
}

Graphics::Graphics(const Handle_t& handle):
    handle_(handle),
    lineHeight_(0),
    effectiveLineHeight_(0),
    baseline_(0),
    effectiveBaseline_(0),
    disableFontScaling_(false)
{
    setFont(FntGetFont());
}

void Graphics::setClipping(const ArsRectangle& rect)
{
    RectangleType nr=toNative(rect);
    WinSetClip(&nr);        
}

Graphics::ClipRectangleSetter::ClipRectangleSetter(Graphics& graphics, const ArsRectangle& rectangle):
    graphics_(graphics)
{
    WinGetClip(&original_);
    graphics_.setClipping(rectangle);
}

Graphics::ClipRectangleSetter::~ClipRectangleSetter()
{
    WinSetClip(&original_);        
}

void Graphics::drawBitmap(uint_t bitmapId, const Point& topLeft)
{
    MemHandle handle=DmGet1Resource(bitmapRsc, bitmapId);
    if (handle) 
    {
        BitmapType* bmp=static_cast<BitmapType*>(MemHandleLock(handle));
        if (bmp) 
        {
            WinDrawBitmap(bmp, topLeft.x, topLeft.y);
            MemHandleUnlock(handle);
        }
        DmReleaseResource(handle);
    }
}

void Graphics::invertRectangle(const ArsRectangle& rect) 
{
    RectangleType r;
    rect.toNative(r);
    WinInvertRectangle(&r, 0);
}

uint_t Graphics::textWidth(const char_t* text, uint_t length)
{
    ScalingSetter setScaling(*this);
    return FntCharsWidth(text, length);
}
    

uint_t Graphics::wordWrap(const char_t* text, uint_t width)
{
    ScalingSetter setScaling(*this);
    return FntWordWrap(text, width);
}

void Graphics::applyStyle(const DefinitionStyle* style, bool isHyperlink)
{
    const DefinitionStyle* def = StyleGetStaticStyle(styleIndexDefault);
    DefinitionStyle s = *def;

    if (isHyperlink)
        s |= *StyleGetStaticStyle(styleIndexHyperlink);

    if (NULL != style && def != style)
        s |= *style;

    FontEffects fx;
    fx.setItalic(s.yes == s.italic);
    fx.setSmall(s.yes == s.small);
    fx.setStrikeOut(s.yes == s.strike);
    fx.setSubscript(s.yes == s.subscript);
    fx.setSuperscript(s.yes == s.superscript);

    if (grayUnderline == s.underline)
        fx.setUnderline(fx.underlineDotted);
    else if (solidUnderline == s.underline || colorUnderline == s.underline)
        fx.setUnderline(fx.underlineSolid);
        
    if (s.yes == s.bold)
        fx.setWeight(fx.weightBold);

    PalmFont font(s.fontId);
    font.setEffects(fx);
    
    setFont(font);
    WinSetForeColorRGB(&s.foregroundColor, NULL);
    WinSetTextColorRGB(&s.foregroundColor, NULL);
    WinSetBackColorRGB(&s.backgroundColor, NULL);
}