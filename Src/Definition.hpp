#ifndef __DEFINITION_HPP__
#define __DEFINITION_HPP__

#include "Debug.hpp"
#include <list>
#include <vector>
#include "Geometry.hpp"

class HyperlinkElement;

class HotSpot 
{
    typedef std::list<ArsLexis::Rectangle, ArsLexis::Allocator<ArsLexis::Rectangle> > Rectangles_t;
    Rectangles_t rectangles_;
    const HyperlinkElement& hyperlink_;
 
 public:
 
    HotSpot(const ArsLexis::Rectangle& rect, const HyperlinkElement& hyperlink);
    
    void addRectangle(const ArsLexis::Rectangle& rect)
    {rectangles_.push_back(rect);}
    
    const HyperlinkElement* hitTest(const PointType& point) const;
    
    ~HotSpot();
    
};

class DefinitionElement;

class Definition
{
    typedef std::list<DefinitionElement*, ArsLexis::Allocator<DefinitionElement*> > Elements_t;
    Elements_t elements_;
    
    typedef std::list<HotSpot*, ArsLexis::Allocator<HotSpot*> > HotSpots_t;
    HotSpots_t hotSpots_;
    
    struct LineHeader
    {
        LineHeader();
        Elements_t::iterator firstElement;
        UInt16 renderingProgress;
        UInt16 lineHeight;
        UInt16 baseLine;
    };
    
    typedef std::vector<LineHeader, ArsLexis::Allocator<LineHeader> > Lines_t;
    Lines_t lines_;
    UInt16 firstLine_;
    UInt16 lastLine_;
    
    ArsLexis::Rectangle lastBounds_;
    
    void clearHotSpots();
    void clearLines();
    void clear();
    
    void calculate(const Elements_t::iterator& firstElement, UInt16 renderingProgress);
    
    void renderCalculated();
    
public:

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
    
    void render(const ArsLexis::Rectangle& bounds);
    
    UInt16 totalLinesCount() const
    {return lines_.size();}
    
    UInt16 firstShownLine() const
    {return firstLine_;}
    
    UInt16 shownLinesCount() const
    {return lastLine_-firstLine_;}
    
    void scroll(Int16 delta);

    Definition();
    
    ~Definition();

private:

    void calculateLine(RenderingContext& context, Elements_t::iterator& currentElement, const Elements_t::iterator& firstElement, UInt16 renderingProgress);

    Boolean renderSingleElement(RenderingContext& context, const Elements_t::iterator& currentElement, Boolean firstInLine);
    
    
};

#endif