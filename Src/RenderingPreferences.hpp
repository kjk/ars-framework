#ifndef __RENDERING_PREFERENCES_HPP__
#define __RENDERING_PREFERENCES_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <Graphics.hpp>

enum HyperlinkType
{
    hyperlinkBookmark,
    hyperlinkTerm,
    hyperlinkExternal,
    hyperlinkClicked
};

enum ElementStyle
{
    styleDefault,
    styleHeader
};

namespace ArsLexis {
    class PrefsStoreReader;
    class PrefsStoreWriter;
}    

class RenderingPreferences
{

    enum {
        stylesCount_=2,
        hyperlinkTypesCount_=4,
    };        
    
    uint_t standardIndentation_;
    uint_t bulletIndentation_;
    
    void calculateIndentation();
    
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
    {return bulletType_;}

    struct StyleFormatting
    {
        ArsLexis::Font font;
        ArsLexis::Color textColor;
        
        StyleFormatting():
            textColor(1)
        {}
        
        enum {reservedPrefIdCount=3};
        Err serializeOut(ArsLexis::PrefsStoreWriter& writer, int uniqueId) const;
        Err serializeIn(ArsLexis::PrefsStoreReader& reader, int uniqueId);
        
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
    
    uint_t bulletIndentation() const
    {return bulletIndentation_;}
    
    ArsLexis::Color backgroundColor() const
    {return backgroundColor_;}
    
    void setBulletType(BulletType type);
    
    void setBackgroundColor(ArsLexis::Color color)
    {backgroundColor_=color;}
    
    enum {reservedPrefIdCount=3+(hyperlinkTypesCount_+stylesCount_)*StyleFormatting::reservedPrefIdCount};
    Err serializeOut(ArsLexis::PrefsStoreWriter& writer, int uniqueId) const;
    Err serializeIn(ArsLexis::PrefsStoreReader& reader, int uniqueId);

private:
    
    BulletType bulletType_;
    ArsLexis::Color backgroundColor_;
    StyleFormatting hyperlinkDecorations_[hyperlinkTypesCount_];
    StyleFormatting styles_[stylesCount_];
};



#endif