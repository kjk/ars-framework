#ifndef __ARSLEXIS_UTILITY_HPP__
#define __ARSLEXIS_UTILITY_HPP__

#include "Debug.hpp"

namespace ArsLexis
{

    template<typename T>
    struct ObjectDeleter
    {
        void operator() (T* object) const 
        {
            delete object;
        }
    };


    class FontSetter 
    {
        FontID originalFontId_;
        FontSetter(const FontSetter&);
        FontSetter& operator=(const FontSetter&);
    public:
    
        explicit FontSetter(FontID fontId):
            originalFontId_(FntSetFont(fontId))
        {}
        
        ~FontSetter()
        {
            FntSetFont(originalFontId_);
        }
    };

    class UnderlineModeSetter
    {
        UnderlineModeType originalUnderlineMode_;
        UnderlineModeSetter(const UnderlineModeSetter&);
        UnderlineModeSetter& operator=(const UnderlineModeSetter&);
    public:
    
        explicit UnderlineModeSetter(UnderlineModeType underlineMode):
            originalUnderlineMode_(WinSetUnderlineMode(underlineMode))
        {}
        
        ~UnderlineModeSetter()
        {
            WinSetUnderlineMode(originalUnderlineMode_);
        }
    };
    
    class ForegroundColorSetter
    {
        IndexedColorType originalColor_;
        ForegroundColorSetter(const ForegroundColorSetter&);
        ForegroundColorSetter& operator=(const ForegroundColorSetter&);
    public:
    
        ForegroundColorSetter(IndexedColorType color):
            originalColor_(WinSetForeColor(color))
        {}
        
        ~ForegroundColorSetter()
        {
            WinSetForeColor(originalColor_);
        }
    };

    class BackgroundColorSetter
    {
        IndexedColorType originalColor_;
        BackgroundColorSetter(const BackgroundColorSetter&);
        BackgroundColorSetter& operator=(const BackgroundColorSetter&);
    public:
    
        BackgroundColorSetter(IndexedColorType color):
            originalColor_(WinSetBackColor(color))
        {}
        
        BackgroundColorSetter()
        {
            WinSetBackColor(originalColor_);
        }
    };
    
    FontID getBoldFont(FontID normal);
    
    struct C_StringLess
    {
        bool operator ()(const char* str1, const char* str2) const
        {return StrCompare(str1, str2)<0;}
    };
    
    void trim(String& str);
    
    Boolean isWhitespace(char chr);
    
}

#endif