/**
 * @file Definition.hpp
 * Interface to class @c Definition, that handles rendering of definition text.
 *
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#ifndef __DEFINITION_HPP__
#define __DEFINITION_HPP__

#include "Debug.hpp"
#include <list>
#include <vector>
#include "Geometry.hpp"

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
    typedef std::list<DefinitionElement*, ArsLexis::Allocator<DefinitionElement*> > Elements_t;
    
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
        UInt16 renderingProgress;
        
        /**
         * @internal
         * Line height.
         */
        Coord height;
        
        /**
         * @internal
         * Position of baseline ralative to height.
         */
        Coord baseLine;
    };
    
    typedef std::vector<LineHeader, ArsLexis::Allocator<LineHeader> > Lines_t;
    
    /**
     * @internal
     * Caches information about lines of text.
     */
    Lines_t lines_;
    
    /**
     * @internal
     * First currently displayed line index.
     */
    UInt16 firstLine_;
    
    /**
     * @internal
     * Index of one-past-last currently displayed line.
     */
    UInt16 lastLine_;
    
    /**
     * @internal
     * Bounds that definition is displayed within.
     */
    ArsLexis::Rectangle lastBounds_;
    
public:

    /**
     * Hot spot is a place in definition that allows to execute some action on clicking it.
     * It's made of one or more rectangular areas, that represent the space in which 
     * some @c DefinitionElement is rendered.
     */
    class HotSpot 
    {
        typedef std::list<ArsLexis::Rectangle, ArsLexis::Allocator<ArsLexis::Rectangle> > Rectangles_t;
        
        /**
         * @internal 
         * Stores rectangles belonging to @c element_.
         */
        Rectangles_t rectangles_;
        
        /**
         * @internal
         * @c DefinitionElement associated with this @c HotSpot.
         */
        const DefinitionElement& element_;
     
     public:
     
        HotSpot(const ArsLexis::Rectangle& rect, const DefinitionElement& element);
        
        void addRectangle(const ArsLexis::Rectangle& rect)
        {rectangles_.push_back(rect);}
        
        const DefinitionElement* hitTest(const PointType& point) const;
        
        ~HotSpot();
        
    };


    /**
     * Used to pass data to @c DefienitionElement::render() function.
     * On entry, @c Definition sets all input members. @c DefinitionElement
     * answers with setting appropriate output members.
     */
    struct RenderingContext 
    {
        /**
         * Input. Reference to @c Definition being rendered, so that @c HyperlinkElement objects
         * can add @c HotSpot objects to definition.
         */
        Definition& definition;

        Coord x;
        Coord y;
    
        /**
         * Input. Screen width that @c DefinitionElement must not exceed when rendering. 
         * If it can't fit in the provided width it should set @c requestNewLine member
         * to @c true.
         */
        Coord availableWidth;
        
        /**
         * Output. Actual screen width used by rendered element.
         */
        Coord width;
        
        /**
         * Output. Line height that @c DefinitionElement requires.
         */
        Coord height;
        
        /**
         * Output. Placement of text base line relative to @c height.
         */
        UInt16 baseLine;
        
        /**
         * Input/Output. On input contains element's rendering progress made so far
         * (0 when first run with given element). On output should be increased by
         * progress made with current rendering stage.
         */
        UInt16 renderingProgress;
        
        /**
         * Output. By setting this member to @c true element indicates that it won't fit 
         * in @c availableWidth. If @c force is set to @c true, then element should try to
         * fit some text in this space, possibly truncating it, so that we won't get infinite loop
         * when we have word that exceeds whole screen width.
         */
        UInt16 requestNewLine:1;
        
        /**
         * Output. Element sets this member to indicate that it's whole contents has been rendered.
         */
        UInt16 elementCompleted:2;
        
        /**
         * Input. If set to @c true, element shouldn't paint itself, but rather calculate all the properties.
         * Otherwise, paint using provided @c height and @c baseLine.
         */
        UInt16 calculateOnly:3;
        
        /**
         * Input. If set to @c true, element should render some content, even if it has to be truncated.
         */
        UInt16 force:4;
        
        UInt16 notUsed:12;        
        
        RenderingContext(Definition& def, const ArsLexis::Rectangle& bounds):
            definition(def),
            x(bounds.x()),
            y(bounds.y()),
            availableWidth(bounds.width()),
            width(0),
            height(0),
            baseLine(0),
            renderingProgress(0),
            requestNewLine(false),
            elementCompleted(false),
            calculateOnly(true),
            force(false)
        {}
        
        void extendHeight(Coord minNewHeight, Coord baseLine);

    };
    
    /**
     * Renders (paints) this @c Definition into bounds.
     */
    void render(const ArsLexis::Rectangle& bounds);
    
    UInt16 totalLinesCount() const
    {return lines_.size();}
    
    UInt16 firstShownLine() const
    {return firstLine_;}
    
    UInt16 shownLinesCount() const
    {return lastLine_-firstLine_;}
    
    /**
     * Scrolls this @c Definition by @c delta lines, bounding it as neccessary.
     */
    void scroll(Int16 delta);

    Definition();
    
    ~Definition();
    
    void swap(Definition& other);

private:

    typedef std::list<HotSpot*, ArsLexis::Allocator<HotSpot*> > HotSpots_t;
    HotSpots_t hotSpots_;
    
    /**
     * Deletes all currently registered hot spots.
     */
    void clearHotSpots();

    /**
     * Clears lines cache.
     */
    void clearLines();

    void clear();
    
    void calculate(const ElementPosition_t& firstElement, UInt16 renderingProgress);
    
    void renderCalculated();
    
    void calculateLastLine();

    void calculateLine(RenderingContext& context, ElementPosition_t& currentElement, const ElementPosition_t& firstElement, UInt16 renderingProgress);

    void renderLine(RenderingContext& context, const Lines_t::iterator& line);

    Boolean renderSingleElement(RenderingContext& context, const ElementPosition_t& currentElement, Boolean firstInLine);
    
};

template<> 
inline void std::swap(Definition& def1, Definition& def2)
{def1.swap(def2);}

#endif