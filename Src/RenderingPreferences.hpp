#ifndef __RENDERING_PREFERENCES_HPP__
#define __RENDERING_PREFERENCES_HPP__

#include "Debug.hpp"
#include "BaseTypes.hpp"

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
    SynchronizationResult synchronize(const RenderingPreferences& preferences)
    {return noChange;}
    
    BulletType bulletType() const
    {return bulletCircle;}

    struct HyperlinkDecoration
    {
        UnderlineModeType underlineMode;
        IndexedColorType textColor;
        
        HyperlinkDecoration():
            underlineMode(grayUnderline),
            textColor(UIColorGetTableEntryIndex(UIObjectForeground))
        {}
        
        HyperlinkDecoration(UnderlineModeType um, IndexedColorType tc):
            underlineMode(um),
            textColor(tc)
        {}
    };
    
    const HyperlinkDecoration& hyperlinkDecoration(HyperlinkType hyperlinkType) const
    {
        assert(hyperlinkType<3);
        return hyperlinkDecorations_[hyperlinkType];
    }
    
    struct StyleFormatting
    {
        FontID fontId;
        IndexedColorType textColor;
        
        StyleFormatting():
            fontId(stdFont),
            textColor(UIColorGetTableEntryIndex(UIObjectForeground))
        {}
        
    };
    
    const StyleFormatting& styleFormatting(ElementStyle style) const
    {
        assert(style<2);
        return styles_[style];
    }
    
    uint_t standardIndentation() const
    {return 20;}
    
    UnderlineModeType standardUnderline() const
    {return solidUnderline;}
    
    IndexedColorType backgroundColor() const
    {return 0;}

private:
    
    HyperlinkDecoration hyperlinkDecorations_[3];
    StyleFormatting styles_[2];
};



#endif