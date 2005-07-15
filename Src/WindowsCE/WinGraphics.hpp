#ifndef __ARSLEXIS_WINGRAPHICS_HPP__
#define __ARSLEXIS_WINGRAPHICS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Geometry.hpp>
#include <Utility.hpp>
#include <WindowsCE/WinFont.h>

class DefinitionStyle;

typedef COLORREF NativeColor_t;
typedef HDC NativeGraphicsHandle_t;    

enum WinFontMetricsFlags {
	winFontMetricSmall = 1,
	winFontMetricSuperscript = 2,
	winFontMetricSubscript = 4
};

struct GraphicsState
{
    int state;
    uint_t fontHeight;
    uint_t fontBaseline;
	uint_t fontMetricsFlags;
	WinFont font;
};

typedef GraphicsState NativeGraphicsState_t;

class Graphics: private NonCopyable
{
    NativeGraphicsHandle_t handle_;
    uint_t          fontHeight_;
    uint_t          fontBaseline_;
	uint_t			fontMetricsFlags_;
    HWND            hwnd_;

    LOGPEN          pen_;
    COLORREF   penColor_;

	HFONT	originalFont_;
	WinFont  currentFont_;

	HPEN originalPen_;

#ifdef DEBUG
    int             statePushCounter_;
#endif

	void init();

	void queryFontMetrics();

public:
    typedef NativeGraphicsHandle_t Handle_t;
    typedef NativeColor_t Color_t;
    typedef NativeGraphicsState_t State_t;

	typedef WinFont Font_t;

    explicit Graphics(const Handle_t& handle);
	explicit Graphics(HWND hwnd);
    
    ~Graphics();

    // Fills specified rectangle with current background color.
    void erase(const ArsRectangle& rect);
    
    /**
     * Copies specified rectangular area (bitmap) from this @c Graphics system into @c targetSystem.
     * @param sourceArea bounds of source bitmap in this @c Graphics system.
     */         
    void copyArea(const ArsRectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft);
    
    void copyArea(const ArsRectangle& sourceArea, const Point& targetTopLeft)
    {copyArea(sourceArea, *this, targetTopLeft);}

    void drawLine(Coord_t x0, Coord_t y0, Coord_t x1, Coord_t y1);

    void drawLine(const Point& start, const Point& end)
    {drawLine(start.x, start.y, end.x, end.y);}
            
    Color_t setForegroundColor(Color_t color);
    
    Color_t setBackgroundColor(Color_t color);
    
    Color_t setTextColor(Color_t color);
    
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
    
    State_t pushState();

    void popState(const State_t& state);
    
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
    
    uint_t fontHeight() const {return fontHeight_;}
    
    uint_t fontBaseline() const {return fontBaseline_;}
    
    void drawText(const char_t* text, uint_t length, const Point& topLeft, bool inverted=false);
    
    void drawCenteredText(const char_t* str, const Point& topLeft, uint_t width);
    
    uint_t wordWrap(const char_t* text, uint_t width);

    uint_t wordWrap(const char_t* text, uint_t availableDx, uint_t& textDx);
    
    uint_t textWidth(const char_t* text, uint_t length);
    
    void charsInWidth(const char_t* text, uint_t& length, uint_t& width);

    void stripToWidthWithEllipsis(ArsLexis::String& textInOut, uint_t &lengthInOut, uint_t& widthInOut, bool fFullWords = true);

    void stripToWidthWithEllipsis(char_t* textInOut, uint_t& lengthInOut, uint_t& widthInOut, bool fFullWords = true);

    void drawTextInBounds(const ArsLexis::String& text, const ArsRectangle& itemBounds, int totalLines, bool allowCenter = true);

	void setFont(const WinFont& font);

private:
    void drawTextInBoundsInternal(const ArsLexis::String& text, const ArsRectangle& itemBounds, int totalLines, bool allowCenter, int lines);

public:
    void invertRectangle(const ArsRectangle& rect);

    Handle_t handle() 
    {return handle_;}

    void applyStyle(const DefinitionStyle* style, bool isHyperlink);

	const WinFont& font() const {return currentFont_;}

};

typedef Graphics::Color_t Color;

#endif
