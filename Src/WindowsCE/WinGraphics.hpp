#ifndef __ARSLEXIS_WINGRAPHICS_HPP__
#define __ARSLEXIS_WINGRAPHICS_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Geometry.hpp>
#include <Utility.hpp>
#include <list>
#include "WinFont.h"

namespace ArsLexis 
{

    typedef COLORREF NativeColor_t;
    typedef HDC NativeGraphicsHandle_t;    
    typedef int NativeGraphicsState_t;
    typedef WinFont NativeFont_t;
    
    class Graphics: private NonCopyable
    {
        struct StackElement
        {
            NativeGraphicsState_t state;
            NativeFont_t font;
        };

        NativeGraphicsHandle_t handle_;
        NativeFont_t font_;
        HWND hwnd_;
        std::list<StackElement> fontStack_;

        LOGFONT fontDescr_;
        HFONT newFont_;
        HGDIOBJ oldFont_;

        void setEffects(FontEffects::Weight weight, bool italic, bool index, bool isSmall);
        void resetEffects();

    public:
    
        typedef NativeGraphicsHandle_t Handle_t;
        typedef NativeColor_t Color_t;
        typedef NativeFont_t Font_t;
        typedef NativeGraphicsState_t State_t;

        explicit Graphics(const Handle_t& handle, HWND hwnd);
        
        ~Graphics();

        /**
         * Fills specified rectangle with current background color.
         * @param rect @c Rectangle to erase.
         */        
        void erase(const Rectangle& rect);
        
        /**
         * Copies specified rectangular area (bitmap) from this @c Graphics system into @c targetSystem.
         * @param sourceArea bounds of source bitmap in this @c Graphics system.
         */         
        void copyArea(const Rectangle& sourceArea, Graphics& targetSystem, const Point& targetTopLeft);
        
        void copyArea(const Rectangle& sourceArea, const Point& targetTopLeft)
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
        
        Font_t setFont(const Font_t& font);
        
        Font_t font() const;

        
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
        
        uint_t fontHeight() const;
        
        uint_t fontBaseline() const;      
        
        void drawText(const char_t* text, uint_t length, const Point& topLeft, bool inverted=false);
        
        void drawCenteredText(const char_t* str, const Point& topLeft, uint_t width);
        
        uint_t wordWrap(const char_t* text, uint_t width);
        
        uint_t textWidth(const char_t* text, uint_t length);
        
        void charsInWidth(const char_t* text, uint_t& length, uint_t& width);
        
        Handle_t handle() 
        {return handle_;}

    };
    
    typedef Graphics::Font_t Font;
    typedef Graphics::Color_t Color;

}
#endif