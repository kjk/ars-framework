/**
 * @file Definition.cpp
 * Implementation of class @c Definition.
 *
 * @author Andrzej Ciarkowski (a.ciarkowski@interia.pl)
 */
#include "Definition.hpp"
#include "DefinitionElement.hpp"
#include <Utility.hpp>
#include "GenericTextElement.hpp"
#include <algorithm>

using namespace ArsLexis;

Definition::HotSpot::HotSpot(const Rectangle& rect, DefinitionElement& element):
    element_(element)
{
    rectangles_.push_back(rect);
}

bool Definition::HotSpot::hitTest(const Point& point) const
{
    return rectangles_.end()!=std::find_if(rectangles_.begin(), rectangles_.end(), Rectangle::HitTest(point));
}

Definition::HotSpot::~HotSpot()
{
    element_.invalidateHotSpot();
}

void Definition::HotSpot::move(const Point& delta, const Rectangle& validArea)
{
    Rectangles_t::iterator end=rectangles_.end();
    Rectangles_t::iterator it=rectangles_.begin();
    while (it!=end)
    {
        Rectangle& rect=*it;
        rect+=delta;
        Rectangles_t::iterator next=it;
        ++next;
        if (!(validArea && rect))
            rectangles_.erase(it);
        it=next;
    }
}

namespace {

    inline 
    static bool operator<(const Rectangle& rect1, const Rectangle& rect2) {
         if (rect1.y()<rect2.y() || (rect1.y()==rect2.y() && rect1.x()<rect2.x()))
            return true;
        else
            return false;
    }
    
}

void Definition::HotSpot::addRectangle(const Rectangle& rect)
{
    rectangles_.insert(std::lower_bound(rectangles_.begin(), rectangles_.end(), rect), rect);
}

bool Definition::HotSpot::operator<(const Definition::HotSpot& other) const
{
    assert(!rectangles_.empty());
    assert(!other.rectangles_.empty());
    return rectangles_.front()<other.rectangles_.front();
}


Definition::Definition():
    firstLine_(0),
    lastLine_(0),
    hyperlinkHandler_(0),
    selectionStartElement_(elements_.end()),
    selectionEndElement_(elements_.end()),
    inactiveSelectionStartElement_(elements_.end()),
    inactiveSelectionEndElement_(elements_.end()),
    selectionStartProgress_(LayoutContext::progressCompleted),
    selectionEndProgress_(LayoutContext::progressCompleted),
    trackingSelection_(false),
    selectedHotSpot_(0),
    renderingProgressReporter_(0),
    interactionBehavior_(0)
{}

namespace {
	
    struct HotSpotNotLess {

        const Definition::HotSpot* hotSpot;

        HotSpotNotLess(const Definition::HotSpot* hs): hotSpot(hs) {}

        bool operator()(const Definition::HotSpot* hs) const 
        {return !((*hs)<(*hotSpot));}
    };

}

void Definition::addHotSpot(HotSpot* hotSpot)
{
    hotSpots_.insert(std::find_if(hotSpots_.begin(), hotSpots_.end(), HotSpotNotLess(hotSpot)), hotSpot);
}

void Definition::clearHotSpots()
{
    std::for_each(hotSpots_.begin(), hotSpots_.end(), ObjectDeleter<HotSpot>());
    hotSpots_.clear();
    selectedHotSpot_=0;
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
    std::for_each(elements_.begin(), elements_.end(), ObjectDeleter<DefinitionElement>());
    elements_.clear();
    selectionStartElement_=elements_.end();
    selectionEndElement_=elements_.end();
    bounds_.clear();
}

Definition::~Definition()
{
    clear();
}

void Definition::calculateVisibleRange(uint_t& firstLine, uint_t& lastLine, int delta)
{
    if (!lines_.empty())
    {
        uint_t linesCount=lines_.size();
        
        int newFirstLine=firstLine_+delta;
        if (newFirstLine<0)
            newFirstLine=0;
        else if (newFirstLine>=(int)linesCount)
            newFirstLine=linesCount-1;

        int height=0;
        uint_t newLastLine=0;
        for (uint_t lineIndex=newFirstLine; lineIndex<linesCount; ++lineIndex)
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
            for (int lineIndex=newFirstLine-1; lineIndex>=0; --lineIndex)
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

void Definition::moveHotSpots(const Point& delta)
{
    HotSpots_t::iterator end=hotSpots_.end();
    HotSpots_t::iterator it=hotSpots_.begin();
    while (it!=end)
    {
        (*it)->move(delta, bounds_);
        HotSpots_t::iterator next=it;
        ++next;
        if (!(*it)->valid())
        {
            delete *it;
            hotSpots_.erase(it);
        }
        it=next;
    }
}

void Definition::scroll(Graphics& graphics, const RenderingPreferences& prefs, int delta)
{
    uint_t newFirstLine = 0;
    uint_t newLastLine = 0;
    calculateVisibleRange(newFirstLine, newLastLine, delta);

    uint_t unionFirst = std::max(firstLine_, newFirstLine);
    uint_t unionLast = std::min(lastLine_, newLastLine);
    if (unionFirst<unionLast)
    {
        uint_t unionTop = 0;
        {
            for (uint_t index=firstLine_; index<unionFirst; ++index)
            {
                unionTop += lines_[index].height;
            }
        }
        uint_t unionHeight = 0;
        for (uint_t index=unionFirst; index<unionLast; ++index)
        {
            unionHeight += lines_[index].height;
        }

        Rectangle unionRect(bounds_.x(), bounds_.y()+unionTop, bounds_.width(), unionHeight);
        Point pointDelta;
        
        Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, prefs.backgroundColor());
        
        if (delta>0) 
        {
            pointDelta.y =- (int)unionTop;
            graphics.copyArea(unionRect, bounds_.topLeft);
            graphics.erase(Rectangle(bounds_.x(), bounds_.y()+unionHeight, bounds_.width(), bounds_.height()-unionHeight));
            moveHotSpots(pointDelta);
            renderLineRange(graphics, prefs, lines_.begin()+unionLast, lines_.begin()+newLastLine, unionHeight);
        }
        else
        {
            pointDelta.y = 0;
            for (uint_t index=newFirstLine; index<firstLine_; ++index)
            {
                pointDelta.y += lines_[index].height;
            }
                
            graphics.copyArea(unionRect, bounds_.topLeft + pointDelta);
            graphics.erase(Rectangle(bounds_.x(), bounds_.y()+unionHeight+pointDelta.y, bounds_.width(), bounds_.height()-unionHeight-pointDelta.y));
            
            moveHotSpots(pointDelta);
            renderLineRange(graphics, prefs, lines_.begin()+newFirstLine, lines_.begin()+unionFirst, 0);
        }
        firstLine_ = newFirstLine;
        lastLine_ = newLastLine;
    }
    else
    {
        clearHotSpots();
        firstLine_ = newFirstLine;
        lastLine_ = newLastLine;
        renderLayout(graphics, prefs);
    }
}

void Definition::elementAtWidth(Graphics& graphics, const RenderingPreferences& prefs, const LinePosition_t& line, Coord_t width, ElementPosition_t& elem, uint_t& progress) 
{
    ElementPosition_t end = elements_.end();
    if (lines_.end() == line)
    {
        elem = end;
        --elem;
        progress = DefinitionElement::offsetOutsideElement;
        return;
    }
    LinePosition_t nextLine = line;
    ++nextLine;
    LayoutContext layoutContext(graphics, prefs, bounds_.width());
    layoutContext.usedWidth=line->leftMargin;
    layoutContext.usedHeight=line->height;
    layoutContext.renderingProgress = line->renderingProgress;
    elem = line->firstElement;
    progress = line->renderingProgress;
    while (end != elem) 
    {
        if (elem == nextLine->firstElement && progress == nextLine->renderingProgress)
            break;
        ElementPosition_t nextElem = elem;
        ++nextElem;
        if (end != nextElem && (*nextElem)->isTextElement())
            layoutContext.nextTextElement = static_cast<GenericTextElement*>(*nextElem);
        progress = (*elem)->charIndexAtOffset(layoutContext, width);
        if (DefinitionElement::offsetOutsideElement != progress)
            break;
        ++elem;
        layoutContext.renderingProgress = 0;
    }
    if (end == elem) {
        --elem;
        progress = DefinitionElement::offsetOutsideElement;
    }
}

bool Definition::renderLine(RenderingContext& renderContext, const LinePosition_t& line, DefinitionElement* elementToRepaint)
{
    bool finished=false;
    renderContext.usedWidth=0;
    renderContext.usedHeight=line->height;
    
    bool doRender=false;
    if (0==elementToRepaint)
    {
        renderContext.graphics.erase(Rectangle(bounds_.x(), renderContext.top, bounds_.width(), renderContext.usedHeight));
        doRender=true;
    }
    else
    {
        LinePosition_t nextLine=line;
        ++nextLine;
        ElementPosition_t lastElement=elements_.end();
        if (nextLine!=lines_.end())
        {
            lastElement=nextLine->firstElement;
            if (nextLine->renderingProgress!=0)
                ++lastElement;
        }                
        ElementPosition_t found=std::find(line->firstElement, lastElement, elementToRepaint);
        if (found!=lastElement)
            doRender=true;
    }
    
    if (doRender)
    {
        renderContext.baseLine=line->baseLine;
        renderContext.renderingProgress=line->renderingProgress;
        renderContext.left=bounds_.x()+line->leftMargin;
        ElementPosition_t last=elements_.end();
        ElementPosition_t current=line->firstElement;
        bool lineFinished=false;  
        DefinitionElement::Justification justify=(last!=current?(*current)->justification():DefinitionElement::justifyLeft);  
        while (!lineFinished && current!=last)
        {
            if (current>=selectionStartElement_ && current<=selectionEndElement_)
            {
                if (current!=selectionStartElement_)
                    renderContext.selectionStart=0;
                else
                    renderContext.selectionStart=selectionStartProgress_;
                if (current!=selectionEndElement_)
                    renderContext.selectionEnd=LayoutContext::progressCompleted;
                else
                    renderContext.selectionEnd=selectionEndProgress_;
            }
            else
                renderContext.selectionStart=renderContext.selectionEnd=LayoutContext::progressCompleted;
            if (current!=last)
            {
                ElementPosition_t next=current;
                ++next;
                if (last!=next && (*next)->isTextElement())
                    renderContext.nextTextElement=static_cast<GenericTextElement*>(*next);
                else
                    renderContext.nextTextElement=0;
            }
            
            if (DefinitionElement::justifyRightLastElementInLine == (*current)->justification() && line->firstElement != current)
            {
                //move last element to right
                LayoutContext lc(renderContext);
                (*current)->calculateLayout(lc);
                renderContext.left += lc.availableWidth();
            }
            
            (*current)->render(renderContext);
            if (renderContext.isElementCompleted())
            {
                if (*current==elementToRepaint)
                {
                    finished=true;
                    break;
                }
                ++current;
                renderContext.renderingProgress=0;
                if (renderContext.availableWidth()==0 || current==last || (*current)->breakBefore(renderContext.preferences) || (justify!=(*current)->justification() && DefinitionElement::justifyRightLastElementInLine != (*current)->justification()))
                    lineFinished=true;
            }
            else
                lineFinished=true;
        }
    }        
    renderContext.top+=renderContext.usedHeight;
    return finished;
}

void Definition::renderLineRange(Graphics& graphics, const RenderingPreferences& prefs, const Definition::LinePosition_t& begin, const Definition::LinePosition_t& end, uint_t topOffset, DefinitionElement* elementToRepaint)
{
    RenderingContext renderContext(graphics, prefs, *this, bounds_.x(), bounds_.y()+topOffset, bounds_.width());
    for (Lines_t::iterator line=begin; line!=end; ++line)
    {
        if (renderLine(renderContext, line, elementToRepaint))
            break;
    }
}

void Definition::calculateLayout(Graphics& graphics, const RenderingPreferences& prefs, const ElementPosition_t& firstElement, uint_t renderingProgress)
{
    ElementPosition_t end(elements_.end());
    ElementPosition_t element(elements_.begin());
    LayoutContext layoutContext(graphics, prefs, bounds_.width());
    LineHeader lastLine;
    lastLine.firstElement=element;
    DefinitionElement::Justification justify = DefinitionElement::justifyLeft;
    if (element!=end)
        justify = (*element)->justification();

    while (element!=end)
    {
        if (renderingProgressReporter_)
        {
            uint_t percent=(100L*(element-elements_.begin()))/elements_.size();
            renderingProgressReporter_->reportProgress(percent);
        }
        uint_t progressBefore=layoutContext.renderingProgress;
        (*element)->calculateLayout(layoutContext);
        
        if (element==firstElement && progressBefore<=renderingProgress && layoutContext.renderingProgress>renderingProgress)
            firstLine_=lines_.size();
        
        bool startNewLine=false;    
        if (layoutContext.isElementCompleted())
        {
            ++element;
            layoutContext.renderingProgress=0;
            layoutContext.nextTextElement=0;
            if (element!=end)
            {   
                ElementPosition_t next(element);
                ++next;
                if (next!=end && (*next)->isTextElement())
                    layoutContext.nextTextElement=static_cast<GenericTextElement*>(*next);
                else
                    layoutContext.nextTextElement=0;
            }
            if (element==end || (*element)->breakBefore(prefs) || ((*element)->justification()!=justify && DefinitionElement::justifyRightLastElementInLine != (*element)->justification()))
                startNewLine=true;
        }
        else
            startNewLine=true;
            
        if (startNewLine)
        {
            lastLine.height=layoutContext.usedHeight;
            lastLine.baseLine=layoutContext.baseLine;
            switch (justify) 
            {
                case DefinitionElement::justifyRightLastElementInLine: 
                    //no break (when longer than one line)
                case DefinitionElement::justifyRight:
                    lastLine.leftMargin=layoutContext.screenWidth-layoutContext.usedWidth;
                    break;
                
                case DefinitionElement::justifyCenter:
                    if (layoutContext.usedWidth>layoutContext.screenWidth) // This means line ends with whitespace we should ignore...
                        lastLine.leftMargin=0;
                    else
                        lastLine.leftMargin=(layoutContext.screenWidth-layoutContext.usedWidth)/2;
                    break;
                    
                default:
                    lastLine.leftMargin=0;
            }            
            lines_.push_back(lastLine);
            if (end!=element)
                justify=(*element)->justification();
            layoutContext.startNewLine();
            lastLine.firstElement=element;
            lastLine.renderingProgress=layoutContext.renderingProgress;
        }
    }
    calculateVisibleRange(firstLine_, lastLine_);
}

void Definition::doRender(Graphics& graphics, const Rectangle& bounds, const RenderingPreferences& prefs, bool forceRecalculate)
{
    if (bounds.width()!=bounds_.width() || forceRecalculate)
    {
        ElementPosition_t firstElement=elements_.begin(); // This will be used in calculating first line we should show.
        uint_t renderingProgress=0;
        if (firstLine_!=0) // If there's actually some first line set, use it - it's position won't change if window width remains the same.
        {
            firstElement=lines_[firstLine_].firstElement;  // Store first element and its progress so that we'll restore it as our firstLine_ if we need to recalculate.
            renderingProgress=lines_[firstLine_].renderingProgress;
        }
        clearHotSpots();
        clearLines();
        bounds_=bounds;
        calculateLayout(graphics, prefs, firstElement, renderingProgress);
    }
    else {
        clearHotSpots();
        bool heightChanged = false;
        if (bounds_.height()!=bounds.height())
            heightChanged = true;
        bounds_ = bounds;
        if (heightChanged)
            calculateVisibleRange(firstLine_, lastLine_);
    }
    renderLayout(graphics, prefs);
}

status_t Definition::render(Graphics& graphics, const Rectangle& bounds, const RenderingPreferences& prefs, bool forceRecalculate)
{
    volatile status_t error=errNone;
    ErrTry {
        doRender(graphics, bounds, prefs, forceRecalculate);
    }
    ErrCatch(ex) {
        error=ex;
    } ErrEndCatch
    return error;
}
 
void Definition::renderLayout(Graphics& graphics, const RenderingPreferences& prefs, DefinitionElement* elementToRepaint)
{
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, prefs.backgroundColor());
    renderLineRange(graphics, prefs, lines_.begin()+firstLine_, lines_.begin()+lastLine_, 0, elementToRepaint);
    uint_t rangeHeight=0;
    for (uint_t i=firstLine_; i<lastLine_; ++i)
    {
        rangeHeight+=lines_[i].height;
    }
    graphics.erase(Rectangle(bounds_.x(), bounds_.y()+rangeHeight, bounds_.width(), bounds_.height()-rangeHeight));        
}

void Definition::selectionToText(String& out) const
{
    Elements_t::const_iterator end=elements_.end();
    for (Elements_t::const_iterator it=elements_.begin(); it!=end; ++it)
    {
        (*it)->toText(out);
    }
}

void Definition::renderSingleElement(Graphics& graphics, const RenderingPreferences& prefs, DefinitionElement& element)
{
    renderLayout(graphics, prefs, &element);
}

static bool hyperlinksEqual(const DefinitionElement* e1, const DefinitionElement* e2) 
{
    if (!(e1->isTextElement() && e2->isTextElement()))
        return false;
    const GenericTextElement* t1 = static_cast<const GenericTextElement*>(e1);
    const GenericTextElement* t2 = static_cast<const GenericTextElement*>(e2);
    if (!(t1->isHyperlink() && t2->isHyperlink()))
        return false;
    const GenericTextElement::HyperlinkProperties* p1 = t1->hyperlinkProperties();
    const GenericTextElement::HyperlinkProperties* p2 = t2->hyperlinkProperties();
    if (p1->type != p2->type || p1->resource != p2->resource)
        return false;
    return true;
}

bool Definition::trackHyperlinkHighlight(Graphics& graphics, const RenderingPreferences& prefs, const Point& point, bool endTracking) 
{
    if (trackingSelection_ && NULL == selectedHotSpot_)
        return false;
    if (NULL != selectedHotSpot_) 
    {
        assert(trackingSelection_);
        assert(selectionEndProgress_ == LayoutContext::progressCompleted);
        bool insideHyperlink = selectedHotSpot_->hitTest(point);
        if (!endTracking) 
        {
            if (!insideHyperlink && selectionStartProgress_ != selectionEndProgress_)
            {
                selectionStartProgress_ = LayoutContext::progressCompleted;
                renderSingleElement(graphics, prefs, selectedHotSpot_->element());
            }
            else if (insideHyperlink && selectionStartProgress_ == selectionEndProgress_)
            {
                selectionStartProgress_ =  0;
                renderSingleElement(graphics, prefs, selectedHotSpot_->element());
            }
        }
        else 
        {
            selectionStartProgress_ = LayoutContext::progressCompleted;
            selectionStartElement_  = selectionEndElement_ = elements_.end();
            trackingSelection_ = false;
            renderSingleElement(graphics, prefs, selectedHotSpot_->element());
            if (insideHyperlink)
                selectedHotSpot_->element().performAction(*this);
            selectedHotSpot_ = NULL;
        }
        return true;
    }
    else 
    {
        HotSpots_t::const_iterator end=hotSpots_.end();
        for (HotSpots_t::const_iterator it=hotSpots_.begin(); it!=end; ++it)
        {
            if ((*it)->hitTest(point))
            {
                selectedHotSpot_ = (*it);
                break;
            }
        }
        if (NULL != selectedHotSpot_)
        {
            trackingSelection_ = true;
            selectionStartProgress_ = 0;
            selectionEndProgress_ = LayoutContext::progressCompleted;
            selectionStartElement_ = selectionEndElement_ = std::find(elements_.begin(), elements_.end(), &selectedHotSpot_->element());
            renderSingleElement(graphics, prefs, selectedHotSpot_->element());
/*            
            ElementPosition_t pos = selectionStartElement_;
            while (true)
            {
                if (pos == elements_.begin())
                    break;
                --pos;
               if (!hyperlinksEqual(*pos, *selectionStartElement_))
                    break;
               selectionStartElement_ = pos;
            }
            pos = selectionEndElement_;
            while (true) 
            {
                ++pos;
                if (pos == elements_.end())
                    break;
                if (!hyperlinksEqual(*pos, *selectionEndElement_))
                    break;
                selectionEndElement_ = pos;
                renderSingleElement(graphics, prefs, *(*pos));
            }
            inactiveSelectionStartElement_ = selectionStartElement_;
            inactiveSelectionEndElement_ = selectionEndElement_;
*/
            return true;
        }
        return false;
    }
}

Definition::LinePosition_t Definition::lineAtHeight(Coord_t height)
{
    assert(height >= 0);
    assert(height < bounds_.height());
    LinePosition_t end = lines_.begin() + lastLine_;
    Coord_t actHeight = 0;
    for (LinePosition_t line = lines_.begin() + firstLine_; line != end; ++line)
        if (height >= actHeight && height < actHeight + line->height)
            return line;
        else
            actHeight += line->height;
    return lines_.end();
}

bool Definition::trackTextSelection(Graphics& graphics, const RenderingPreferences& prefs, const Point& point, bool endTracking) 
{
    Point p(point.x - bounds_.x(), point.y - bounds_.y());
    ElementPosition_t elem;
    uint_t progress;
    LinePosition_t line = lineAtHeight(p.y);
    elementAtWidth(graphics, prefs, line, p.x, elem, progress);
    if (!trackingSelection_) 
    {
        selectionStartElement_ = selectionEndElement_ = elem;
        selectionStartProgress_ = selectionEndProgress_ = progress;
        trackingSelection_ = true;
    }
    else 
    {
        /*
        ElementPosition_t pos;
        if (elem > selectionEndElement_ || (elem == selectionEndElement_ && progress > selectionEndProgress_))
        {
            pos = selectionEndElement_;
            selectionEndElement_ = elem;
            selectionEndProgress_ = progress;
            ++elem;
            while (pos != elem)
                renderSingleElement(graphics, prefs, *pos++);
        } 
        */   
    }
    if (endTracking) 
        trackingSelection_ = false;
    return true;
}

bool Definition::extendSelection(Graphics& graphics, const RenderingPreferences& prefs, const Point& point, bool endTracking)
{
    if (elements_.empty())
        return false;
    if (!trackingSelection_ && !(bounds_ && point))
        return false;
    if (trackHyperlinkHighlight(graphics, prefs, point, endTracking))
        return true;
    return trackTextSelection(graphics, prefs, point, endTracking);
}

Definition::LineHeader::LineHeader():
    renderingProgress(0),
    height(0),
    baseLine(0),
    leftMargin(0)
{}

void Definition::replaceElements(Elements_t& elements)
{
    clear();
    elements_.swap(elements);
}


#if defined(_PALM_OS)
#pragma segment Segment1
#endif

Definition::HyperlinkHandler::~HyperlinkHandler()
{}

