/**
 * @file Definition.cpp
 * Implementation of class @c Definition.
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include "Definition.hpp"
#include "DefinitionElement.hpp"
#include "Utility.hpp"

Definition::HotSpot::HotSpot(const ArsLexis::Rectangle& rect, DefinitionElement& element):
    element_(element)
{
    rectangles_.push_back(rect);
}

Boolean Definition::HotSpot::hitTest(const PointType& point) const
{
    Boolean result=false;
    Rectangles_t::const_iterator it=std::find_if(rectangles_.begin(), rectangles_.end(), ArsLexis::Rectangle::HitTest(point));
    if (it!=rectangles_.end())
        result=true;
    return result;
}

Definition::HotSpot::~HotSpot()
{
    element_.invalidateHotSpot();
}

Boolean Definition::HotSpot::move(const PointType& delta, const ArsLexis::Rectangle& validArea)
{
    Rectangles_t::iterator end=rectangles_.end();
    Rectangles_t::iterator it=rectangles_.begin();
    while (it!=end)
    {
        ArsLexis::Rectangle& rect=*it;
        rect.topLeft().x+=delta.x;
        rect.topLeft().y+=delta.y;
        if (!rect.intersectsWith(validArea))
        {
            Rectangles_t::iterator next=it;
            ++next;
            rectangles_.erase(it);
            it=next;
        }
    }
    return !rectangles_.empty();
}

Definition::Definition():
    firstLine_(0),
    lastLine_(0)
{}

void Definition::addHotSpot(HotSpot* hotSpot)
{
    hotSpots_.push_front(hotSpot);
}

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
    bounds_=ArsLexis::Rectangle();
}

Definition::~Definition()
{
    clear();
}

void Definition::calculateVisibleRange(UInt16& firstLine, UInt16& lastLine, Int16 delta)
{
    if (!lines_.empty())
    {
        UInt16 linesCount=lines_.size();
        
        Int16 newFirstLine=firstLine_+delta;
        if (newFirstLine<0)
            newFirstLine=0;
        else if (newFirstLine>=linesCount)
            newFirstLine=linesCount-1;

        Coord height=0;
        Int16 newLastLine=0;
        for (Int16 lineIndex=newFirstLine; lineIndex<linesCount; ++lineIndex)
        {
            height+=lines_[lineIndex].height;
            if (height>bounds_.height())
            {
                newLastLine=lineIndex;
                break;
            }
        }
        if (!newLastLine)
        {
            newLastLine=linesCount;
            for (Int16 lineIndex=newFirstLine-1; lineIndex>=0; --lineIndex)
            {
                height+=lines_[lineIndex].height;
                if (height<bounds_.height())
                    --newFirstLine;
                else 
                    break;
            }
        }
        firstLine=newFirstLine;
        lastLine=newLastLine;   
    }
    else
        firstLine=lastLine=0;
}

void Definition::moveHotSpots(const PointType& delta)
{
    HotSpots_t::iterator end=hotSpots_.end();
    HotSpots_t::iterator it=hotSpots_.begin();
    while (it!=end)
    {
        Boolean stillValid=(*it)->move(delta, bounds_);
        if (!stillValid)
        {
            HotSpots_t::iterator next=it;
            ++next;
            delete *it;
            hotSpots_.erase(it);
            it=next;
        }
    }
}


/**
 * @todo Erase garbage that occurs during scrolling of lines of non-uniform height in Definition::scroll().
 */
void Definition::scroll(Int16 delta)
{
    UInt16 newFirstLine=0;
    UInt16 newLastLine=0;
    calculateVisibleRange(newFirstLine, newLastLine, delta);

    UInt16 unionFirst=std::max(firstLine_, newFirstLine);
    UInt16 unionLast=std::min(lastLine_, newLastLine);
    if (unionFirst<unionLast)
    {
        Coord unionTop=0;
        for (UInt16 index=firstLine_; index<unionFirst; ++index)
            unionTop+=lines_[index].height;
        Coord unionHeight=0;
        for (UInt16 index=unionFirst; index<unionLast; ++index)
            unionHeight+=lines_[index].height;
        ArsLexis::Rectangle unionRect(bounds_.x(), bounds_.y()+unionTop, bounds_.width(), unionHeight);
        
        PointType pointDelta;
        pointDelta.x=0;
        if (delta>0) 
        {
            pointDelta.y=-unionTop;
            WinCopyRectangle(NULL, NULL, unionRect, bounds_.x(), bounds_.y(), winPaint);
            renderLineRange(lines_.begin()+unionLast, lines_.begin()+newLastLine, unionHeight);
        }
        else
        {
            pointDelta.y=0;
            for (UInt16 index=newFirstLine; index<firstLine_; ++index)
                pointDelta.y+=lines_[index].height;
            WinCopyRectangle(NULL, NULL, unionRect, bounds_.x(), bounds_.y()+pointDelta.y, winPaint);
            renderLineRange(lines_.begin()+newFirstLine, lines_.begin()+unionFirst, 0);
        }
        moveHotSpots(pointDelta);
        
        firstLine_=newFirstLine;
        lastLine_=newLastLine;
    }
    else
    {
        clearHotSpots();
        firstLine_=newFirstLine;
        lastLine_=newLastLine;
        renderLayout();
    }
}

void Definition::renderLine(RenderingContext& renderContext, const LinePosition_t& line)
{
    renderContext.usedWidth=0;
    renderContext.usedHeight=line->height;
    ArsLexis::Rectangle rect(bounds_.x(), renderContext.top, bounds_.width(), renderContext.usedHeight);
    WinEraseRectangle(rect, 0);
    renderContext.baseLine=line->baseLine;
    renderContext.renderingProgress=line->renderingProgress;
    ElementPosition_t last=elements_.end();
    ElementPosition_t current=line->firstElement;
    Boolean lineFinished=false;    
    while (!lineFinished && current!=last)
    {
        (*current)->render(renderContext);
        if (renderContext.isElementCompleted())
        {
            ++current;
            renderContext.renderingProgress=0;
            if (renderContext.availableWidth()==0 || current==last || (*current)->requiresNewLine(preferences_))
                lineFinished=true;
        }
        else
            lineFinished=true;
    }
    renderContext.top+=renderContext.usedHeight;
}

void Definition::renderLineRange(const Definition::LinePosition_t& begin, const Definition::LinePosition_t& end, Coord topOffset)
{
    RenderingContext renderContext(preferences_, *this, bounds_.x(), bounds_.y()+topOffset, bounds_.width());
    for (Lines_t::iterator line=begin; line!=end; ++line)
        renderLine(renderContext, line);
}

void Definition::calculateLayout(const ElementPosition_t& firstElement, UInt16 renderingProgress)
{
    Coord topOffset=0;
    ElementPosition_t last=elements_.end();
    ElementPosition_t current=elements_.begin();
    LayoutContext layoutContext(preferences_, bounds_.width());
    LineHeader lastLine;
    lastLine.firstElement=current;
    while (current!=last)
    {
        UInt16 progressBefore=layoutContext.renderingProgress;
        (*current)->calculateLayout(layoutContext);
        
        if (current==firstElement && progressBefore<=renderingProgress && layoutContext.renderingProgress>renderingProgress)
        {
            topOffset_=topOffset;
            firstLine_=lines_.size();
        }
        
        Boolean startNewLine=false;    
        if (layoutContext.isElementCompleted())
        {
            ++current;
            layoutContext.renderingProgress=0;
            if (layoutContext.availableWidth()==0 || current==last || (*current)->requiresNewLine(preferences_))
                startNewLine=true;
        }
        else
            startNewLine=true;
            
        if (startNewLine)
        {
            lastLine.height=layoutContext.usedHeight;
            lastLine.baseLine=layoutContext.baseLine;
            topOffset+=lastLine.height;
            lines_.push_back(lastLine);
            layoutContext.startNewLine();
            lastLine.firstElement=current;
            lastLine.renderingProgress=layoutContext.renderingProgress;
        }
    }
    calculateVisibleRange(firstLine_, lastLine_);
}

void Definition::render(const ArsLexis::Rectangle& bounds, const RenderingPreferences& preferences)
{
    if (bounds.width()!=bounds_.width() || preferences_.synchronize(preferences))
    {
        ElementPosition_t firstElement=elements_.begin(); // This will be used in calculating first line we should show.
        UInt16 renderingProgress=0;
        if (firstLine_!=0) // If there's actually some first line set, use it - it's position won't change if window width remains the same.
        {
            firstElement=lines_[firstLine_].firstElement;  // Store first element and its progress so that we'll restore it as our firstLine_ if we need to recalculate.
            renderingProgress=lines_[firstLine_].renderingProgress;
        }
        clearHotSpots();
        clearLines();
        bounds_=bounds;
        calculateLayout(firstElement, renderingProgress);
    }
    else {
        clearHotSpots();
        Boolean heightChanged=(bounds_.height()!=bounds.height());
        bounds_=bounds;
        if (heightChanged)
            calculateVisibleRange(firstLine_, lastLine_);
    }
    renderLayout();
}
   
 
void Definition::swap(Definition& other)
{
    elements_.swap(other.elements_);
    lines_.swap(other.lines_);
    std::swap(firstLine_, other.firstLine_);
    std::swap(lastLine_, other.lastLine_);
    std::swap(bounds_, other.bounds_);
    hotSpots_.swap(other.hotSpots_);
}

void Definition::appendElement(DefinitionElement* element)
{
    elements_.push_back(element);
}

void Definition::hitTest(const PointType& point)
{
    HotSpots_t::iterator end=hotSpots_.end();
    for (HotSpots_t::iterator it=hotSpots_.begin(); it!=end; ++it)
    {
        if ((*it)->hitTest(point))
        {
            (*it)->element().elementClicked(*this);
            break;
        }
    }
}
