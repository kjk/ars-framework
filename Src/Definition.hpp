/**
 * @file Definition.hpp
 * Interface to class @c Definition that handles rendering of definition text.
 *
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __DEFINITION_HPP__
#define __DEFINITION_HPP__

#include <Debug.hpp>
#include <list>
#include <vector>
#include <Graphics.hpp>
#include "Rendering.hpp"
#include <RenderingPreferences.hpp>

class DefinitionElement;

/**
 * Handles rendering and user interactions (clicking parts of, selecting etc.) with definition text .
 */
class Definition: private ArsLexis::NonCopyable
{
public:

    typedef std::vector<DefinitionElement*> Elements_t;
    typedef Elements_t::iterator ElementPosition_t;
    
    typedef Elements_t::const_iterator const_iterator;
    typedef Elements_t::iterator iterator;
    
    const_iterator begin() const {return elements_.begin();}
    iterator begin() {return elements_.begin();}
    const_iterator end() const {return elements_.end();}
    iterator end() {return elements_.end();}
    
private:
    
    /**
     * @internal
     * Stores definition parts in their order of appearance from top-left to bottom-right.
     */
    Elements_t elements_;
    

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
        LineHeader();
        
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
        
        uint_t leftMargin;
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
    
    LinePosition_t lineAtHeight(ArsLexis::Coord_t height);
    
    void elementAtWidth(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const LinePosition_t& line, ArsLexis::Coord_t width, ElementPosition_t& elem, uint_t& progress, uint_t& wordEnd, bool word = false);
    
    void removeSelection(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs);
    
public:

    class HyperlinkHandler
    {
    public:
    
        virtual void handleHyperlink(Definition& definition, DefinitionElement& hyperlinkElement)=0;
        
        virtual ~HyperlinkHandler();
        
    };
    
    /**
     * Hot spot is a place in definition that allows to execute some action on clicking it.
     * It's made of one or more rectangular areas, that represent the space in which 
     * some @c DefinitionElement is rendered.
     */
    class HotSpot: private ArsLexis::NonCopyable
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
        
        void addRectangle(const ArsLexis::Rectangle& rect);
        
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
        
        bool operator<(const HotSpot& other) const;
        
        ~HotSpot();
        
    };
    
    /**
     * Renders (paints) this @c Definition into bounds.
     */
    ArsLexis::status_t render(ArsLexis::Graphics& graphics, const ArsLexis::Rectangle& bounds, const RenderingPreferences& prefs, bool forceRecalculate = false);
    
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


    void replaceElements(Elements_t& elements);
    
    /**
     * Adds hot spot to the collection of this definition's hot spots. Definition takes ownership 
     * of the hot spot.
     */
    void addHotSpot(HotSpot* hotSpot);
    
    const ArsLexis::Rectangle& bounds() const {return bounds_;}
    
    void bounds(ArsLexis::Rectangle& out) const {out = bounds_;}
    
    bool mouseDown(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point);
    
    bool mouseUp(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point, uint_t clickCount);
    
    bool mouseDrag(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point);
    
    enum NavigatorKey {
        navKeyLeft,
        navKeyRight,
        navKeyUp,
        navKeyDown,
        navKeyCenter
    };
    
    bool navigatorKey(NavigatorKey navKey);
    
    enum InteractionBehaviorOption {
        behavDoubleClickSelection = 1,
        behavMouseSelection = 2,
        behavUpDownScroll = 4,
        behavHyperlinkNavigation = 8
    };
    
    void setInteractionBehavior(uint_t ib) { interactionBehavior_ = ib;}
    
    uint_t interactionBehavior() const { return interactionBehavior_;}
    
    bool usesInteractionBehavior(InteractionBehaviorOption ibo) const {return 0 != (interactionBehavior_ & ibo);}
    
    bool usesDoubleClickSelection() const { return usesInteractionBehavior(behavDoubleClickSelection);}
    
    bool usesMouseSelection() const { return usesInteractionBehavior(behavMouseSelection);}
    
    bool usesUpDownScroll() const { return usesInteractionBehavior(behavUpDownScroll);}
    
    bool usesHyperlinkNavigation() const { return usesInteractionBehavior(behavHyperlinkNavigation);}
    
    bool extendSelection(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point, uint_t clickCount = 0);
    
    /**
     * Resets this definition to its default state (without any elements, hot spots etc.).
     */
    void clear();
    
    HyperlinkHandler* hyperlinkHandler()
    {return hyperlinkHandler_;}
    
    void setHyperlinkHandler(HyperlinkHandler* handler)
    {hyperlinkHandler_=handler;}
    
    void goToBookmark(const ArsLexis::String&)
    {}
    
    void selectionToText(ArsLexis::String& out) const;
    
    bool empty() const
    {return elements_.empty();}
    
    class RenderingProgressReporter
    {
    public:
    
        virtual ~RenderingProgressReporter() {}
        
        virtual void reportProgress(uint_t percent)=0;
    };
    
    void setRenderingProgressReporter(RenderingProgressReporter* reporter)
    {renderingProgressReporter_=reporter;}
    
    ElementPosition_t firstElementPosition()
    { return elements_.begin();}

    ElementPosition_t lastElementPosition()
    { return elements_.end();}

private:

    void renderSingleElement(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t element);

    void renderElementRange(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t begin, ElementPosition_t end);
    
    bool trackHyperlinkHighlight(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point, uint_t clickCount);
    
    bool trackTextSelection(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point, uint_t clickCount);

    void doRender(ArsLexis::Graphics& graphics, const ArsLexis::Rectangle& bounds, const RenderingPreferences& prefs, bool forceRecalculate);


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

    void calculateLayout(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t firstElement, uint_t renderingProgress);
    
    void calculateVisibleRange(uint_t& firstLine, uint_t& lastLine, int delta=0);
    
    void renderLine(RenderingContext& renderContext, LinePosition_t line, ElementPosition_t begin, ElementPosition_t end);
    
    void renderLineRange(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, LinePosition_t begin, LinePosition_t end, uint_t topOffset, ElementPosition_t startElem, ElementPosition_t endElem);

    void renderLayout(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t begin, ElementPosition_t end);
    
    void moveHotSpots(const ArsLexis::Point& delta);
    
    ElementPosition_t selectionStartElement_;
    ElementPosition_t selectionEndElement_;
    ElementPosition_t mouseDownElement_;
    uint_t mouseDownProgress_;
    Lines_t::iterator lastLineUnderMouse_;
    bool selectionIsHyperlink_;
    
    ElementPosition_t inactiveSelectionStartElement_;
    ElementPosition_t inactiveSelectionEndElement_;
    uint_t selectionStartProgress_;
    uint_t selectionEndProgress_;
    bool trackingSelection_;
    RenderingProgressReporter* renderingProgressReporter_; 
    uint_t interactionBehavior_;
    
};

#endif