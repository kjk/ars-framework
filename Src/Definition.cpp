/**
 * @file Definition.cpp
 * Implementation of class @c Definition.
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include "Definition.hpp"
#include "DefinitionElement.hpp"
#include "Utility.hpp"

Definition::HotSpot::HotSpot(const ArsLexis::Rectangle& rect, const DefinitionElement& element):
    element_(element)
{
    rectangles_.push_back(rect);
}

const DefinitionElement* Definition::HotSpot::hitTest(const PointType& point) const
{
    const DefinitionElement* result=0;
    Rectangles_t::const_iterator it=std::find_if(rectangles_.begin(), rectangles_.end(), ArsLexis::Rectangle::HitTest(point));
    if (it!=rectangles_.end())
        result=&element_;
    return result;
}

Definition::HotSpot::~HotSpot()
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

void Definition::calculateLastLine()
{
    UInt16 height=0;
    lastLine_=0;
    Lines_t::iterator end=lines_.end();
    for (Lines_t::iterator line=lines_.begin()+firstLine_; line!=end; ++line)
    {
        height+=line->height;
        if (height>lastBounds_.height())
        {
            lastLine_=line-lines_.begin();
            break;
        }
    }
    if (!lastLine_)
        lastLine_=lines_.size();
}

void Definition::renderLayout()
{
    ElementPosition_t last=elements_.end();
    Lines_t::iterator lastLine=lines_.begin()+lastLine_;
    DefinitionElement::RenderContext renderContext(*this, lastBounds_.x(), lastBounds_.y(), topOffset_, lastBounds_.width());
    for (Lines_t::iterator line=lines_.begin()+firstLine_; line!=lastLine; ++line)
    {
        renderContext.usedWidth=0;
        renderContext.usedHeight=line->height;
        renderContext.baseLine=line->baseLine;
        renderContext.renderingProgress=line->renderingProgress;
        ElementPosition_t current=line->firstElement;
        Boolean lineFinished=false;    
        while (!lineFinished && current!=last)
        {
            (*current)->render(renderContext);
            if (renderContext.isElementCompleted())
            {
                ++current;
                renderContext.renderingProgress=0;
                if (renderContext.availableWidth()==0 || current==last || (*current)->requiresNewLine())
                    lineFinished=true;
            }
            else
                lineFinished=true;
        }
        renderContext.top+=renderContext.usedHeight;
    }    
}

void Definition::calculateLayout(const ElementPosition_t& firstElement, UInt16 renderingProgress)
{
    Coord topOffset=0;
    ElementPosition_t last=elements_.end();
    ElementPosition_t current=elements_.begin();
    DefinitionElement::LayoutContext layoutContext(lastBounds_.width());
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
            if (layoutContext.availableWidth()==0 || current==last || (*current)->requiresNewLine())
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
    calculateLastLine();
}

void Definition::render(const ArsLexis::Rectangle& bounds, const RenderingPreferences& preferences)
{
    if (bounds.width()!=lastBounds_.width() || lastPreferences_.synchronize(preferences))
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
        lastBounds_=bounds;
        calculateLayout(firstElement, renderingProgress);
    }
    else if (bounds.height()!=lastBounds_.height())
    {
        lastBounds_=bounds;
        calculateLastLine();
    }
    else
        lastBounds_=bounds;
    renderLayout();
}
   
 
void Definition::swap(Definition& other)
{
    elements_.swap(other.elements_);
    lines_.swap(other.lines_);
    std::swap(firstLine_, other.firstLine_);
    std::swap(lastLine_, other.lastLine_);
    std::swap(lastBounds_, other.lastBounds_);
    hotSpots_.swap(other.hotSpots_);
}

void Definition::appendElement(DefinitionElement* element)
{
    elements_.push_back(element);
}