#ifndef __ARSLEXIS_GRAPHICS_HPP__
#define __ARSLEXIS_GRAPHICS_HPP__

#include "Geometry.hpp"
#include "NativeGraphics.hpp"

namespace ArsLexis
{

    class Graphics
    {
        NativeGraphicsHandle_t handle_;
#ifdef USE_NATIVE_GRAPHICS_STATE
        NativeGraphicsState graphicsState_;
#endif          

    public:
    
        typedef NativeGraphicsHandle_t Handle_t;
        typedef NativeColor_t Color_t;
        typedef NativeFont_t Font_t;

        explicit Graphics(const NativeGraphicsHandle_t& handle
#ifdef USE_DEFAULT_NATIVE_GRAPHICS_HANDLE
            =NativeGraphicsHandle_t(0)
#endif
        );
        
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
                
        // Windows CE: Use SetDCPenColor()                
        Color_t setForegroundColor(Color_t color);
        
        // Windows CE: Use SetDCBrushColor() or SetBkColor()
        Color_t setBackgroundColor(Color_t color);
        
        // Windows CE: Use SetTextColor()
        Color_t setTextColor(Color_t color);
        
        enum ColorChoice
        {
            colorText,
            colorForeground,
            colorBackground
        };
        
        Color_t setColor(ColorChoice choice, Color_t color);
        
        class ColorSetter
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
        
        Font_t setFont(Font_t font);
        
        class FontSetter
        {
            Graphics& graphics_;
            Font_t originalFont_;
        public:
        
            FontSetter(Graphics& graphics, Font_t font):
                graphics_(graphics),
                originalFont_(graphics_.setFont(font))
            {}
            
            ~FontSetter()
            {graphics_.setFont(originalFont_);}
            
            void changeTo(Font_t font)
            {graphics_.setFont(font);}
            
        };

    };                    

}

#if defined(__PALMOS_H__)
#include "PalmGraphicsInline.hpp"
#endif

#endif