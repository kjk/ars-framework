
#include "Definition.hpp"
#include "DefinitionElement.hpp"
#include "Utility.hpp"

HotSpot::HotSpot(const ArsLexis::Rectangle& rect, const HyperlinkElement& hyperlink):
    hyperlink_(hyperlink)
{
    rectangles_.push_back(rect);
}

const HyperlinkElement* HotSpot::hitTest(const PointType& point) const
{
    const HyperlinkElement* result=0;
    Rectangles_t::const_iterator it=std::find_if(rectangles_.begin(), rectangles_.end(), ArsLexis::Rectangle::HitTest(point));
    if (it!=rectangles_.end())
        result=&hyperlink_;
    return result;
}

HotSpot::~HotSpot()
{}


Definition::Definition():
    firstLine_(0),
    lastLine_(0)
{}

void Definition::clearHotSpots()
{
    std::for_each(hotSpots_.begin(), hotSpots_.end(), ArsLexis::ObjectDeleter<HotSpot>());
    hotSpots_.clear();
}

void Definition::clearLines()
{
    lines_.clear();
    Lines_t tmp;
    lines_.swap(tmp); // Reclaim some memory...
    firstLine_=lastLine_=0;
}

void Definition::clear()
{
    clearHotSpots();
    clearLines();
    std::for_each(elements_.begin(), elements_.end(), ArsLexis::ObjectDeleter<DefinitionElement>());
    lastBounds_=ArsLexis::Rectangle();
}

Definition::~Definition()
{
    clear();
}

void Definition::scroll(Int16 delta)
{
    
}

Boolean Definition::renderSingleElement(RenderingContext& context, const Elements_t::iterator& currentElement, Boolean firstInLine)
{
    (*currentElement)->render(context);
    if (context.requestNewLine)
    {
        context.requestNewLine=false;
        if (!firstInLine) // If it's not the sole element in the line, check if it fits if it gets a new line.
        {
            UInt16 backupRenderingProgress=context.renderingProgress;
            Coord backupAvailableWidth=context.availableWidth;
            context.availableWidth=lastBounds_.width();
            Boolean backupCalculateOnly=context.calculateOnly; 
            context.calculateOnly=true;
            (*currentElement)->render(context);
            context.calculateOnly=backupCalculateOnly;
            context.renderingProgress=backupRenderingProgress;
            if (!context.requestNewLine) // It will fit if it gets a new line - finish with this one, so that element won't be splitted in ugly way.
            {
                context.renderingProgress=backupRenderingProgress; // Restore element's rendering progress, so it will start in place we finished.
                return true;
            }
            else // Restore available width to previous setting and prepare for splitting.
            {
                assert(context.renderingProgress==backupRenderingProgress);
                context.availableWidth=backupAvailableWidth;
                context.requestNewLine=false;
            }
        }            
        context.force=true; // It won't fit other way, so force splitting.
        (*currentElement)->render(context);
        context.force=false;
        return true; 
    }
    context.availableWidth-=context.width;
    return false;
}


void Definition::calculateLine(RenderingContext& context, Elements_t::iterator& currentElement, const Elements_t::iterator& firstElement, UInt16 renderingProgress)
{
    LineHeader lineHeader;
    lineHeader.firstElement=currentElement;
    lineHeader.renderingProgress=context.renderingProgress;
    context.availableWidth=lastBounds_.width();    context.height=0;
    context.baseLine=0;
    context.width=0;
    Boolean firstInLine=true;
    while (currentElement!=elements_.end())
    {
        if (!firstInLine && (*currentElement)->requiresNewLine() && 0==context.renderingProgress)  // If we have new-line element, and it's not a first element in line, we must finish this line.
            break;
            
        UInt16 progressBefore=context.renderingProgress; // Save element's current progress so that we can detect where should be our new firstLine_
        Boolean forceNewLine=renderSingleElement(context, currentElement, firstInLine);
        firstInLine=false;
        if ((*currentElement==*firstElement) && (renderingProgress>=progressBefore) && (renderingProgress<context.renderingProgress))
            firstLine_=lines_.size();

        if (context.elementCompleted)
        {
            ++currentElement;
            context.renderingProgress=0;
            context.elementCompleted=false;
        }
        
        if (0==context.availableWidth || forceNewLine)
            break;
    }
    lineHeader.lineHeight=context.height;
    lineHeader.baseLine=context.baseLine;
    lines_.push_back(lineHeader);
}


void Definition::calculate(const Elements_t::iterator& firstElement, UInt16 renderingProgress)
{
    RenderingContext context(*this, lastBounds_);
    Elements_t::iterator end=elements_.end();
    Elements_t::iterator currentElement=elements_.begin();
    while (currentElement!=end)
        calculateLine(context, currentElement, firstElement, renderingProgress);
    
}

void Definition::renderCalculated()
{
    
}


void Definition::render(const ArsLexis::Rectangle& bounds)
{
    if (bounds.width()!=lastBounds_.width())    {
        Elements_t::iterator firstElement=elements_.begin(); // This will be used in calculating first line we should show.
        UInt16 renderingProgress=0;
        if (firstLine_!=0) // If there's actually some first line set, use it - it's position won't change if window width remains the same.
        {
            firstElement=lines_[firstLine_].firstElement;  // Store first element and its progress so that we'll restore it as our firstLine_ if we need to recalculate.
            renderingProgress=lines_[firstLine_].renderingProgress;
        }
        clearHotSpots();
        clearLines();
        lastBounds_=bounds;
        calculate(firstElement, renderingProgress);
    }
    renderCalculated();
}
    
