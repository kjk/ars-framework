#ifndef __ARSLEXIS_PALM_GRAPHICS_HPP__
#define __ARSLEXIS_PALM_GRAPHICS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Geometry.hpp>
#include <Utility.hpp>
#include <PalmFont.hpp>

class DefinitionStyle;

class Graphics: private NonCopyable
{
    WinHandle handle_;        
    PalmFont font_;
    uint_t lineHeight_;
    uint_t effectiveLineHeight_;
    uint_t baseline_;
    uint_t effectiveBaseline_;
    bool disableFontScaling_;
    
public:

    typedef WinHandle Handle_t;
    typedef IndexedColorType Color_t;
    typedef PalmFont Font_t;
    typedef PalmFont State_t;

    explicit Graphics(const Handle_t& handle=0);
    
    /**
     * Fills specified rectangle with current background color.
     * @param rect @c ArsRectangle to erase.
     */        
    void erase(const ArsRectangle& rect);

    /**
     * Copies specified rectangular area (bitmap) from this @c Graphics system into @c targetSystem.
     * @param sourceArea bounds of source bitmap in this @c Graphics system.
     */         
    void copyArea(const ArsRectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft);
    
    void copyArea(const ArsRectangle& sourceArea, const Point& targetTopLeft)
    {copyArea(sourceArea, *this, targetTopLeft);}

    void drawLine(Coord_t x0, Coord_t y0, Coord_t x1, Coord_t y1)
    {WinDrawLine(x0, y0, x1, y1);}

    void drawLine(const Point& start, const Point& end)
    {drawLine(start.x, start.y, end.x, end.y);}
            
    Color_t setForegroundColor(Color_t color)
    {return WinSetForeColor(color);}
    
    Color_t setBackgroundColor(Color_t color)
    {return WinSetBackColor(color);}
    
    Color_t setTextColor(Color_t color)
    {return WinSetTextColor(color);}
    
    enum ColorChoice
    {
        colorText,
        colorForeground,
        colorBackground
    };
    
    Color_t setColor(ColorChoice choice, Color_t color);
    
    class ColorSetter: private NonCopyable
    {
        Graphics& graphics_;
        ColorChoice choice_;
        Color_t originalColor_;
    public:
    
        ColorSetter(Graphics& graphics, ColorChoice choice, Color_t newColor):
            graphics_(graphics),
            choice_(choice),
            originalColor_(graphics_.setColor(choice_, newColor))
        {}
        
        ~ColorSetter()
        {graphics_.setColor(choice_, originalColor_);}
        
        void changeTo(Color_t color)
        {graphics_.setColor(choice_, color);}
        
    };  
    
    Font_t setFont(const Font_t& font);
    
    Font_t font() const
    {return font_;}
    
    class FontSetter: private NonCopyable
    {
        Graphics& graphics_;
        Font_t originalFont_;

    public:
    
        FontSetter(Graphics& graphics, const Font_t& font):
            graphics_(graphics),
            originalFont_(graphics_.setFont(font))
        {}
        
        ~FontSetter()
        {graphics_.setFont(originalFont_);}
        
        void changeTo(const Font_t& font)
        {graphics_.setFont(font);}
        
    };
    
    class ClipRectangleSetter: private NonCopyable
    {
        Graphics& graphics_;
        RectangleType original_;
    public:
        
        ClipRectangleSetter(Graphics& graphics, const ArsRectangle& rectangle);
        
        ~ClipRectangleSetter();
    };
    
    State_t pushState()
    {
        WinPushDrawState();
        return font_;
    }

    void popState(const State_t& state)
    {
        setFont(state);
        WinPopDrawState();
    }
    
    class StateSaver: private NonCopyable
    {
        Graphics& graphics_;
        State_t state_;
    public:
        explicit StateSaver(Graphics& graphics):
            graphics_(graphics),
            state_(graphics_.pushState())
        {}
        
        ~StateSaver()
        {graphics_.popState(state_);}
    };      
    
    uint_t fontHeight() const
    {return effectiveLineHeight_;}
    
    uint_t fontBaseline() const
    {return effectiveBaseline_;}
    
/*        
    enum TextVideoMode {
        textVideoNormal,
        textVideoInverseSelection,
        textVideoInverseHyperlink
    }
*/
     
    void drawText(const char_t* text, ulong_t length, const Point& topLeft, bool inverted=false);
    
    void drawCenteredText(const char_t* str, const Point& topLeft, uint_t width);
    
    ulong_t wordWrap(const char_t* text, uint_t width);

    ulong_t wordWrap(const char_t* text, uint_t availableDx, uint_t& textDx);

    uint_t textWidth(const char_t* text, ulong_t length);
    
    void charsInWidth(const char_t* text, ulong_t& length, uint_t& width);
    
    Handle_t handle() 
    {return handle_;}
    
    void resetClipping()
    {WinResetClip();}
    
    void setClipping(const ArsRectangle& rect);
    
    void drawBitmap(uint_t bitmapId, const Point& topLeft);

    void stripToWidthWithEllipsis(char_t* textInOut, ulong_t& lengthOut, uint_t& widthInOut, bool fFullWords = true);

    void stripToWidthWithEllipsis(String& textInOut, ulong_t& lengthInOut, uint_t& widthInOut, bool fFullWords = true);

    void drawTextInBounds(const String& text, const ArsRectangle& itemBounds, int totalLines, bool allowCenter = true);

private:
    void drawTextInBoundsInternal(const ArsLexis::String& text, const ArsRectangle& itemBounds, int totalLines, bool allowCenter, int lines);

    friend class ScalingSetter;
    
public:
    void invertRectangle(const ArsRectangle& rect); 

    void applyStyle(const DefinitionStyle* style, bool isHyperlink);
};

class ActivateGraphics: private NonCopyable
{
    Graphics::Handle_t handle_;
public:
    explicit ActivateGraphics(Graphics& g):
        handle_(WinSetDrawWindow(g.handle()))
    {}
    
    ~ActivateGraphics()
    {WinSetDrawWindow(handle_);}
    
};

inline void setRgbColor(RGBColorType& rgb, UInt8 r, UInt8 g, UInt8 b) 
{
    rgb.index=0;
    rgb.r=r;
    rgb.g=g;
    rgb.b=b;
}

#endif
