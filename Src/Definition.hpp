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
#include <Rendering.hpp>

#include <DefinitionStyle.hpp>

class HyperlinkHandlerBase;
class DefinitionElement;
class Definition;
class PopupMenu;

enum HyperlinkType
{
#ifdef INFOMAN
    hyperlinkUrl,
    hyperlinkCallback
#else // iPedia
    hyperlinkUrl,
    hyperlinkTerm,
    hyperlinkExternal,
    hyperlinkBookmark
#endif
};   

class DefinitionModel: private NonCopyable
{
public:

    typedef std::vector<DefinitionElement*> Elements_t;
    Elements_t elements;

    void setTitle(const char_t* txt, long len = -1);

private:
    
    DefinitionStyle** styles_;
    ulong_t styleCount_;
    char_t* title_;
 
public:

    void swap(DefinitionModel& other);
    
    DefinitionModel();
    
    ~DefinitionModel();
    
    const char_t * title() const {return title_;}
   
    status_t append(DefinitionElement* e);
    status_t appendText(const char_t* text, long len = -1); 
    status_t appendLineBreak();
    status_t appendBullet();
   
    DefinitionElement* last() {return elements.back();} 

    friend class ByteFormatParser;
};     


/**
 * Handles rendering and user interactions (clicking parts of, selecting etc.) with definition text .
 */
class Definition: private NonCopyable
{
    DefinitionModel* model_;
    
public:

    enum ModelOwnerFlag
    {
        ownModelNot,
        ownModel
    };

    typedef DefinitionModel::Elements_t Elements_t;
    typedef Elements_t::iterator ElementPosition_t;
    
    typedef Elements_t::const_iterator const_iterator;
    typedef Elements_t::iterator iterator;
    
    const_iterator begin() const {return elements_.begin();}
    iterator begin() {return elements_.begin();}
    const_iterator end() const {return elements_.end();}
    iterator end() {return elements_.end();}
    
    typedef HyperlinkHandlerBase HyperlinkHandler;
    
private:
    
    ModelOwnerFlag modelOwner_;
    
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
    Rect bounds_;
    
    LinePosition_t lineAtHeight(Coord_t height);
    
    void elementAtWidth(Graphics& graphics, const LinePosition_t& line, Coord_t width, ElementPosition_t& elem, uint_t& progress, uint_t& wordEnd, bool word = false);
    
    bool removeSelectionOrShowPopup(const Point& point, Graphics& graphics);
    
public:

    void setModel(DefinitionModel* model, ModelOwnerFlag owner = ownModelNot);

    /**
     * Hot spot is a place in definition that allows to execute some action on clicking it.
     * It's made of one or more rectangular areas, that represent the space in which 
     * some @c DefinitionElement is rendered.
     */
    class HotSpot: private NonCopyable
    {
        typedef std::list<Rect> Rectangles_t;
        
        /**
         * @internal 
         * Stores rectangles belonging to @c element_.
         */
        Rectangles_t rectangles_;
        
        /**
         * @internal
         * @c DefinitionElement associated with this @c HotSpot.
         */
        DefinitionElement* element_;
        
        friend class DefinitionElement;
     
     public:
     
        HotSpot(const Rect& rect, DefinitionElement& element);
        
        void addRectangle(const Rect& rect);
        
        bool hitTest(const Point& point) const;
        
        DefinitionElement* element()
        {return element_;}
        
        /**
         * Moves all rectangles belonging to this @c HotSpot. If rectangle falls outside
         * @c validArea rectangle, it's removed from the @c HotSpot.
         * @param delta coordinates to move rectangles by.
         */
        void move(const Point& delta, const Rect& validArea);
        
        bool valid() const
        {return !rectangles_.empty();}
        
        bool operator<(const HotSpot& other) const;
        
        ~HotSpot();
        
        bool center(Point& point) const;
        
    };
    
    /**
     * Renders (paints) this @c Definition into bounds.
     */
    status_t render(Graphics& graphics, const Rect& bounds, bool forceRecalculate = false);
    
    uint_t totalLinesCount() const
    {return lines_.size();}
    
    uint_t firstShownLine() const
    {return firstLine_;}
    
    uint_t shownLinesCount() const
    {return lastLine_-firstLine_;}
    
    /**
     * Scrolls this @c Definition by @c delta lines, bounding it as neccessary.
     */
    void scroll(Graphics& graphics, int delta);

    Definition();
    
    ~Definition();

    /**
     * Adds hot spot to the collection of this definition's hot spots. Definition takes ownership 
     * of the hot spot.
     */
    void addHotSpot(HotSpot* hotSpot);
    
    const Rect& bounds() const {return bounds_;}
    
    void bounds(Rect& out) const {out = bounds_;}
    
    bool mouseDown(Graphics& graphics, const Point& point);
    
    bool mouseUp(Graphics& graphics, const Point& point, uint_t clickCount);
    
    bool mouseDrag(Graphics& graphics, const Point& point);
    
    enum NavigatorKey {
        navKeyLeft,
        navKeyRight,
        navKeyUp,
        navKeyDown,
        navKeyCenter
    };
    
    bool navigatorKey(Graphics& graphics, NavigatorKey navKey);
    
    enum InteractionBehaviorOption {
        behavDoubleClickSelection = 1,
        behavMouseSelection = 2,
        behavUpDownScroll = 4,
        behavHyperlinkNavigation = 8,
        behavSelectionClickAction = 16,
        behavLast = 32
    };
    
    void setInteractionBehavior(uint_t ib) { interactionBehavior_ = ib;}
    
    uint_t interactionBehavior() const { return interactionBehavior_;}
    
    bool usesInteractionBehavior(InteractionBehaviorOption ibo) const {return 0 != (interactionBehavior_ & ibo);}
    
    bool usesDoubleClickSelection() const { return usesInteractionBehavior(behavDoubleClickSelection);}
    
    bool usesMouseSelection() const { return usesInteractionBehavior(behavMouseSelection);}
    
    bool usesUpDownScroll() const { return usesInteractionBehavior(behavUpDownScroll);}
    
    bool usesHyperlinkNavigation() const { return usesInteractionBehavior(behavHyperlinkNavigation);}
    
    bool extendSelection(Graphics& graphics, const Point& point, uint_t clickCount = 0);
    
    /**
     * Resets this definition to its default state (without any elements, hot spots etc.).
     */
    void clear();
    
    HyperlinkHandlerBase* hyperlinkHandler()
    {return hyperlinkHandler_;}
    
    void setHyperlinkHandler(HyperlinkHandlerBase* handler)
    {hyperlinkHandler_=handler;}
    
    void allToText(ArsLexis::String& out) const;

    void selectionToText(ArsLexis::String& out) const;

    void selectionOrAllToText(ArsLexis::String& out) const;

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
    
    bool hasSelection() const;
    
    void clearSelection(Graphics& graphics);
    
    bool isFirstLinkSelected() const;

    bool isLastLinkSelected() const;
    

    enum NavOrderOption {
        navOrderFirst = 1,
        navOrderLast = 2
    };
    
    void setNavOrderOptions(uint_t options) { navOrderOptions_ = options; }
    
    uint_t navOrderOptions() const { return navOrderOptions_; }
    
    bool isFirstInNavOrder() const 
    {
        if ( navOrderFirst == (navOrderOptions_ & navOrderFirst))
            return true;
        else
            return false;
    }

    bool isLastInNavOrder() const 
    {
        if ( navOrderLast == (navOrderOptions_ & navOrderLast) )
            return true;
        else
            return false;
    }

    typedef void (*SelectionClickHandler)(const Point& point, const String& text, void* context);
    SelectionClickHandler selectionClickHandler;
    void* selectionClickHandlerContext;
    
    void setupSelectionClickHandler(SelectionClickHandler handler, void* context)
    {
        selectionClickHandler = handler;
        selectionClickHandlerContext = context;
    }
    
    void setSelection(Graphics& graphics, const ElementPosition_t& startElement, uint_t startProgress, const ElementPosition_t& endElement, uint_t endProgress, bool isHyperlink = false);
    
    void highlightHyperlink(Graphics& graphics, const ElementPosition_t& hyperlinkElement);
    
    void highlightHyperlink(Graphics& graphics, uint_t index);

    void calculateLayout(Graphics& graphics, const Rect& bounds, bool force = false);

	bool layoutChanged(const Rect& bounds) const; 

private:

    void renderSingleElement(Graphics& graphics, ElementPosition_t element);

    void renderElementRange(Graphics& graphics, ElementPosition_t begin, ElementPosition_t end);
    
    bool trackHyperlinkHighlight(Graphics& graphics, const Point& point, uint_t clickCount);
    
    bool trackTextSelection(Graphics& graphics, const Point& point, uint_t clickCount);

    void doRender(Graphics& graphics, const Rect& bounds, bool forceRecalculate);


    HyperlinkHandlerBase* hyperlinkHandler_;

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

    void calculateLayout(Graphics& graphics, ElementPosition_t firstElement, uint_t renderingProgress);
    
    void calculateVisibleRange(uint_t& firstLine, uint_t& lastLine, int delta=0);
    
    void renderLine(RenderingContext& renderContext, LinePosition_t line, ElementPosition_t begin, ElementPosition_t end);
    
    void renderLineRange(Graphics& graphics, LinePosition_t begin, LinePosition_t end, uint_t topOffset, ElementPosition_t startElem, ElementPosition_t endElem);

    void renderLayout(Graphics& graphics, ElementPosition_t begin, ElementPosition_t end);
    
    void moveHotSpots(const Point& delta);
    
    void extendSelectionToFullHyperlink();
    
    RenderingProgressReporter* renderingProgressReporter_; 

    ElementPosition_t selectionStartElement_;
    ElementPosition_t selectionEndElement_;
    ElementPosition_t mouseDownElement_;
    Lines_t::iterator lastLineUnderMouse_;
    
    ElementPosition_t inactiveSelectionStartElement_;
    ElementPosition_t inactiveSelectionEndElement_;

    uint_t mouseDownProgress_;
    uint_t selectionStartProgress_;
    uint_t selectionEndProgress_;
    uint_t interactionBehavior_;
    uint_t navOrderOptions_;

    bool navigatingUp_;
    bool navigatingDown_;
    bool trackingSelection_;
    bool selectionIsHyperlink_;
    bool consumeNextPenUp_;

    bool navigateHyperlink(Graphics& graphics, bool next);
};

void DestroyElements(Definition::Elements_t& elems);

/**
 * parse text with "<b>", "</b>" and "<br>" tags inside.
 * if useHyperlink is true:
 *   handle "<a>" and "</a>" tags calling:
 *   gText->setHyperlink(text, hyperlinkType);
 *   where text is text between <a> and </a>
 */
status_t DefinitionParseSimple(DefinitionModel& out, const char* text, long textLen = -1, bool useHyperlink = false, const char* hyperlinkSchema = NULL);

#endif
