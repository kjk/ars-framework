#ifndef __RENDERING_PREFERENCES_HPP__
#define __RENDERING_PREFERENCES_HPP__

#include "Debug.hpp"
#include "BaseTypes.hpp"
#include "Graphics.hpp"

enum HyperlinkType
{
    hyperlinkBookmark,
    hyperlinkTerm,
    hyperlinkExternal
};

enum ElementStyle
{
    styleDefault,
    styleHeader
};

class RenderingPreferences
{

    enum {
        stylesCount_=2,
        hyperlinkTypesCount_=3,
    };        
    
    uint_t standardIndentation_;
    
public:

    enum BulletType 
    {
        bulletCircle,
        bulletDiamond
    };
    
    RenderingPreferences();
    
    enum SynchronizationResult 
    {
        noChange,
        repaint,
        recalculateLayout
    };

    /**
     * @todo Implement RenderingPreferences::synchronize()
     */
    SynchronizationResult synchronize(const RenderingPreferences&)
    {return noChange;}
    
    BulletType bulletType() const
    {return bulletCircle;}

    struct StyleFormatting
    {
        ArsLexis::Graphics::Font_t font;
        ArsLexis::Graphics::Color_t textColor;
        
        StyleFormatting():
            font(stdFont),
            textColor(1)
        {}
        
    };
    
    const StyleFormatting& hyperlinkDecoration(HyperlinkType hyperlinkType) const
    {
        assert(hyperlinkType<hyperlinkTypesCount_);
        return hyperlinkDecorations_[hyperlinkType];
    }
    
    const StyleFormatting& styleFormatting(ElementStyle style) const
    {
        assert(style<stylesCount_);
        return styles_[style];
    }
    
    uint_t standardIndentation() const
    {return standardIndentation_;}
    
    ArsLexis::Graphics::Color_t backgroundColor() const
    {return 0;}

private:
    
    StyleFormatting hyperlinkDecorations_[hyperlinkTypesCount_];
    StyleFormatting styles_[stylesCount_];
};



#endif