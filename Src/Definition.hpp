/**
 * @file Definition.hpp
 * Interface to class @c Definition that handles rendering of definition text.
 *
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __DEFINITION_HPP__
#define __DEFINITION_HPP__

#include "Debug.hpp"
#include <list>
#include <vector>
#include "Graphics.hpp"
#include "Rendering.hpp"
#include "RenderingPreferences.hpp"

class DefinitionElement;

/**
 * Handles rendering and user interactions (clicking parts of, selecting etc.) with definition text .
 */
class Definition
{
    /**
     * @internal
     * Type used to store @c DefinitonElement objects that represent various parts of definition.
     */
    typedef std::list<DefinitionElement*> Elements_t;
    
    /**
     * @internal
     * Stores definition parts in their order of appearance from top-left to bottom-right.
     */
    Elements_t elements_;
    
    typedef Elements_t::iterator ElementPosition_t;

    /**
     * @internal
     * Stores useful information about each line of text, so that we can render each line without the need to
     * recalculate the whole content.
     */    
    struct LineHeader
    {
        /**
         * @internal 
         * Initializes members to default values.
         */
        LineHeader():
            renderingProgress(0),
            height(0),
            baseLine(0)
        {}
        
        /**
         * @internal
         * Index of first element in line.
         */
        ElementPosition_t firstElement;
        
        /**
         * @internal
         * Rendering progress of @c firstElement that line starts with.
         */
        uint_t renderingProgress;
        
        /**
         * @internal
         * Line height.
         */
        uint_t height;
        
        /**
         * @internal
         * Position of baseline ralative to height.
         */
        uint_t baseLine;
    };
    
    typedef std::vector<LineHeader> Lines_t;
    typedef Lines_t::iterator LinePosition_t;
    
    /**
     * @internal
     * Caches information about lines of text.
     */
    Lines_t lines_;
    
    /**
     * @internal
     * First currently displayed line index.
     */
    uint_t firstLine_;
    
    /**
     * @internal
     * Index of one-past-last currently displayed line.
     */
    uint_t lastLine_;
    
    /**
     * @internal
     * Bounds that definition is displayed within.
     */
    ArsLexis::Rectangle bounds_;
    
public:

    class HyperlinkHandler
    {
    public:
    
        virtual void handleHyperlink(const ArsLexis::String& resource, HyperlinkType type)=0;
        
        virtual ~HyperlinkHandler()
        {}
        
    };
    
    /**
     * Hot spot is a place in definition that allows to execute some action on clicking it.
     * It's made of one or more rectangular areas, that represent the space in which 
     * some @c DefinitionElement is rendered.
     */
    class HotSpot 
    {
        typedef std::list<ArsLexis::Rectangle> Rectangles_t;
        
        /**
         * @internal 
         * Stores rectangles belonging to @c element_.
         */
        Rectangles_t rectangles_;
        
        /**
         * @internal
         * @c DefinitionElement associated with this @c HotSpot.
         */
        DefinitionElement& element_;
     
     public:
     
        HotSpot(const ArsLexis::Rectangle& rect, DefinitionElement& element);
        
        void addRectangle(const ArsLexis::Rectangle& rect)
        {rectangles_.push_back(rect);}
        
        bool hitTest(const ArsLexis::Point& point) const;
        
        DefinitionElement& element()
        {return element_;}
        
        /**
         * Moves all rectangles belonging to this @c HotSpot. If rectangle falls outside
         * @c validArea rectangle, it's removed from the @c HotSpot.
         * @param delta coordinates to move rectangles by.
         */
        void move(const ArsLexis::Point& delta, const ArsLexis::Rectangle& validArea);
        
        bool valid() const
        {return !rectangles_.empty();}
        
        ~HotSpot();
        
    };
    
    /**
     * Renders (paints) this @c Definition into bounds.
     */
    void render(ArsLexis::Graphics& graphics, const ArsLexis::Rectangle& bounds, const RenderingPreferences& prefs, bool forceRecalculate);
    
    uint_t totalLinesCount() const
    {return lines_.size();}
    
    uint_t firstShownLine() const
    {return firstLine_;}
    
    uint_t shownLinesCount() const
    {return lastLine_-firstLine_;}
    
    /**
     * Scrolls this @c Definition by @c delta lines, bounding it as neccessary.
     */
    void scroll(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, int delta);

    Definition();
    
    ~Definition();
    
    void swap(Definition& other);
    
    /**
     * Appends element to the end of this definition's element list. Definition takes ownership 
     * of the appended element.
     */
    void appendElement(DefinitionElement* element);
    
    /**
     * Adds hot spot to the collection of this definition's hot spots. Definition takes ownership 
     * of the hot spot.
     */
    void addHotSpot(HotSpot* hotSpot);
    
    const ArsLexis::Rectangle& bounds() const
    {return bounds_;}
    
    /**
     * Checks if @c point falls within bounds of any currently displayed @c HotSpot. If so,
     * calls @c DefinitionElement::hotSpotClicked() of element associated with @c HotSpot.
     */
    void hitTest(const ArsLexis::Point& point);
    
    /**
     * Resets this definition to its default state (without any elements, hot spots etc.).
     */
    void clear();
    
    HyperlinkHandler* hyperlinkHandler()
    {return hyperlinkHandler_;}
    
    void setHyperlinkHandler(HyperlinkHandler* handler)
    {hyperlinkHandler_=handler;}
    
    void goToBookmark(const ArsLexis::String& bookmark)
    {}
    
private:

    HyperlinkHandler* hyperlinkHandler_;

    typedef std::list<HotSpot*> HotSpots_t;
    HotSpots_t hotSpots_;
    
    /**
     * Deletes all currently registered hot spots.
     */
    void clearHotSpots();

    /**
     * Clears lines cache.
     */
    void clearLines();

    void calculateLayout(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ElementPosition_t& firstElement, uint_t renderingProgress);
    
    void calculateVisibleRange(uint_t& firstLine, uint_t& lastLine, int delta=0);
    
    void renderLine(RenderingContext& renderContext, const LinePosition_t& line);
    
    void renderLineRange(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const LinePosition_t& begin, const LinePosition_t& end, uint_t topOffset);

    void renderLayout(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs);
    
    void moveHotSpots(const ArsLexis::Point& delta);
    
};

template<> 
inline void std::swap(Definition& def1, Definition& def2)
{def1.swap(def2);}

#endif