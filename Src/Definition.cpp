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
#include <Text.hpp>
#include "LineBreakElement.hpp"

using namespace ArsLexis;

Definition::HotSpot::HotSpot(const ArsLexis::Rectangle& rect, DefinitionElement& element):
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

void Definition::HotSpot::move(const Point& delta, const ArsLexis::Rectangle& validArea)
{
    Rectangles_t::iterator end=rectangles_.end();
    Rectangles_t::iterator it=rectangles_.begin();
    while (it!=end)
    {
        ArsLexis::Rectangle& rect=*it;
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
        static bool operator<(const ArsLexis::Rectangle& rect1, const ArsLexis::Rectangle& rect2) {
         if (rect1.y()<rect2.y() || (rect1.y()==rect2.y() && rect1.x()<rect2.x()))
            return true;
        else
            return false;
    }
    
}

void Definition::HotSpot::addRectangle(const ArsLexis::Rectangle& rect)
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
    mouseDownElement_(elements_.end()),
    mouseDownProgress_(LayoutContext::progressCompleted),
    selectionStartProgress_(LayoutContext::progressCompleted),
    selectionEndProgress_(LayoutContext::progressCompleted),
    trackingSelection_(false),
    renderingProgressReporter_(0),
    interactionBehavior_(0),
    selectionIsHyperlink_(false),
    navOrderOptions_(0),
    navigatingUp_(false),
    navigatingDown_(false)
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

        ArsLexis::Rectangle unionRect(bounds_.x(), bounds_.y()+unionTop, bounds_.width(), unionHeight);
        Point pointDelta;
        
        Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, prefs.backgroundColor());
        
        if (delta>0) 
        {
            pointDelta.y =- int(unionTop);
            graphics.copyArea(unionRect, bounds_.topLeft);
            graphics.erase(ArsLexis::Rectangle(bounds_.x(), bounds_.y() + unionHeight, bounds_.width(), bounds_.height() - unionHeight));
            moveHotSpots(pointDelta);
            renderLineRange(graphics, prefs, lines_.begin()+unionLast, lines_.begin()+newLastLine, unionHeight, elements_.end(), elements_.end());
        }
        else
        {
            pointDelta.y = 0;
            for (uint_t index=newFirstLine; index<firstLine_; ++index)
            {
                pointDelta.y += lines_[index].height;
            }
                
            graphics.copyArea(unionRect, bounds_.topLeft + pointDelta);
            graphics.erase(ArsLexis::Rectangle(bounds_.x(), bounds_.y()+unionHeight+pointDelta.y, bounds_.width(), bounds_.height()-unionHeight-pointDelta.y));
            
            moveHotSpots(pointDelta);
            renderLineRange(graphics, prefs, lines_.begin()+newFirstLine, lines_.begin()+unionFirst, 0, elements_.end(), elements_.end());
        }
        firstLine_ = newFirstLine;
        lastLine_ = newLastLine;
    }
    else
    {
        clearHotSpots();
        firstLine_ = newFirstLine;
        lastLine_ = newLastLine;
        renderLayout(graphics, prefs, elements_.end(), elements_.end());
    }
}

void Definition::elementAtWidth(Graphics& graphics, const RenderingPreferences& prefs, const LinePosition_t& line, Coord_t width, ElementPosition_t& elem, uint_t& progress, uint_t& wordEnd, bool word) 
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
    if (width < 0 || line == lines_.begin() + lastLine_)
        return;
    if (width >= bounds_.width())
    {
        if (lines_.end() == nextLine)
        {
            elem = end;
            --elem;
            progress = DefinitionElement::offsetOutsideElement;
            return;
        }
        elem = nextLine->firstElement;
        progress = nextLine->renderingProgress;
        return;
    }
    while (end != elem) 
    {
        if (elem == nextLine->firstElement && progress == nextLine->renderingProgress)
            break;
        ElementPosition_t nextElem = elem;
        ++nextElem;
        if (end != nextElem && (*nextElem)->isTextElement())
            layoutContext.nextTextElement = static_cast<GenericTextElement*>(*nextElem);

        if (DefinitionElement::justifyRightLastElementInLine == (*elem)->justification()) 
        {
            //move last element to right
            LayoutContext lc = layoutContext;
            (*elem)->calculateLayout(lc);
            layoutContext.usedWidth += lc.availableWidth();
        }
     
        LayoutContext after = layoutContext;
        progress = (*elem)->charIndexAtOffset(after, width);
        if (DefinitionElement::offsetOutsideElement != progress)
            break;
            
        layoutContext.renderingProgress = after.renderingProgress;
        layoutContext.usedWidth = after.usedWidth;
        
        ++elem;
        layoutContext.renderingProgress = 0;
        progress = 0;
    }
    if (end == elem) {
        --elem;
        progress = DefinitionElement::offsetOutsideElement;
    }
    if (word && DefinitionElement::offsetOutsideElement != progress)
        (*elem)->wordAtIndex(layoutContext, progress, progress, wordEnd);
}

void Definition::renderLine(RenderingContext& renderContext, LinePosition_t line, ElementPosition_t begin, ElementPosition_t end)
{
    bool finished=false;
    renderContext.usedWidth = 0;
    renderContext.usedHeight = line->height;
    renderContext.baseLine = line->baseLine;
    renderContext.renderingProgress = line->renderingProgress;
    renderContext.left = bounds_.x() + line->leftMargin;
    ElementPosition_t last = elements_.end();
    ElementPosition_t current = line->firstElement;
    bool lineFinished = false;  

    DefinitionElement::Justification justify;
    if (last!=current)
        justify = (*current)->justification();
    else
        DefinitionElement::justifyLeft;

    if (elements_.end() == begin)
        renderContext.graphics.erase(ArsLexis::Rectangle(bounds_.x(), renderContext.top, bounds_.width(), renderContext.usedHeight));

    while (!lineFinished && current != last)
    {
        if (current>=selectionStartElement_ && current<=selectionEndElement_)
        {
            if (current != selectionStartElement_)
                renderContext.selectionStart = 0;
            else
                renderContext.selectionStart = selectionStartProgress_;
            if (current != selectionEndElement_)
                renderContext.selectionEnd = LayoutContext::progressCompleted;
            else
                renderContext.selectionEnd = selectionEndProgress_;
        }
        else
            renderContext.selectionStart = renderContext.selectionEnd = LayoutContext::progressCompleted;
        if (current != last)
        {
            ElementPosition_t next = current;
            ++next;
            if (elements_.end() != next && (*next)->isTextElement())
                renderContext.nextTextElement = static_cast<GenericTextElement*>(*next);
            else
                renderContext.nextTextElement = 0;
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
            ++current;
            renderContext.renderingProgress = 0;
            if (renderContext.availableWidth() == 0 || current == last || 
                (*current)->breakBefore(renderContext.preferences) || 
                (justify != (*current)->justification() && DefinitionElement::justifyRightLastElementInLine != (*current)->justification()))
                lineFinished = true;
        }
        else
            lineFinished = true;
    }
    renderContext.top += renderContext.usedHeight;
}

void Definition::renderLineRange(Graphics& graphics, const RenderingPreferences& prefs, LinePosition_t begin, LinePosition_t end, uint_t topOffset, ElementPosition_t startElem, ElementPosition_t endElem)
{
    RenderingContext renderContext(graphics, prefs, *this, bounds_.x(), bounds_.y() + topOffset, bounds_.width());
    renderContext.selectionIsHyperlink = selectionIsHyperlink_;
    Lines_t::iterator line = begin;
    while (line != end)
    {
        Lines_t::iterator nextLine = line;
        ++nextLine;
        ElementPosition_t lastElem = elements_.end();
        if (lines_.end() != nextLine)
            lastElem = nextLine->firstElement;
        bool renderThisLine = false;
        if (elements_.end() == startElem)
            renderThisLine = true;
        else if (startElem <= line->firstElement && endElem >= line->firstElement)
            renderThisLine = true;
        else if (startElem <= lastElem && endElem >= lastElem)
            renderThisLine = true;
        else if (startElem >= line->firstElement && endElem <= lastElem)
            renderThisLine = true;
        if (renderThisLine)
            renderLine(renderContext, line, startElem, endElem);
        else
            renderContext.top += line->height;
        line = nextLine;
        if (line->firstElement > endElem)
            break;
    }
}

void Definition::calculateLayout(Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t firstElement, uint_t renderingProgress)
{
    ElementPosition_t end(elements_.end());
    ElementPosition_t element(elements_.begin());
    LayoutContext layoutContext(graphics, prefs, bounds_.width());
    LineHeader lastLine;
    lastLine.firstElement = element;
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

void Definition::doRender(Graphics& graphics, const ArsLexis::Rectangle& bounds, const RenderingPreferences& prefs, bool forceRecalculate)
{
    if (bounds.width() != bounds_.width() || forceRecalculate || lines_.empty())
    {
        ElementPosition_t firstElement = elements_.begin(); // This will be used in calculating first line we should show.
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
    renderLayout(graphics, prefs, elements_.end(), elements_.end());
}

status_t Definition::render(Graphics& graphics, const ArsLexis::Rectangle& bounds, const RenderingPreferences& prefs, bool forceRecalculate)
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
 
void Definition::renderLayout(Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t begin, ElementPosition_t end)
{
    Graphics::ColorSetter setBackground(graphics, Graphics::colorBackground, prefs.backgroundColor());
    renderLineRange(graphics, prefs, lines_.begin() + firstLine_, lines_.begin() + lastLine_, 0, begin, end);
    uint_t rangeHeight = 0;
    for (uint_t i=firstLine_; i<lastLine_; ++i)
        rangeHeight+=lines_[i].height;
    if (elements_.end() == begin)
        graphics.erase(ArsLexis::Rectangle(bounds_.x(), bounds_.y() + rangeHeight, bounds_.width(), bounds_.height() - rangeHeight));        
}

// if there is selection, copy selection to text. Otherwise, copy
// the whole definition
void Definition::selectionOrAllToText(String& out) const
{
    if (hasSelection())
        selectionToText(out);
    else
        allToText(out);
}

void Definition::selectionToText(String& out) const
{
    if (!hasSelection())
        return;
    Elements_t::const_iterator end=elements_.end();
    uint_t start, stop;
    for (Elements_t::const_iterator it=elements_.begin(); it!=end; ++it)
    {
        if (it >= selectionStartElement_ && it <=selectionEndElement_)
        {
            if (it == selectionStartElement_)
                start = selectionStartProgress_;
            else 
                start = 0;
            if (it == selectionEndElement_)
                stop = selectionEndProgress_;
            else
                stop = LayoutContext::progressCompleted;
            (*it)->toText(out, start, stop);
        }
    }
}

void Definition::allToText(String& out) const
{
    Elements_t::const_iterator end=elements_.end();
    uint_t start, stop;
    for (Elements_t::const_iterator it=elements_.begin(); it!=end; ++it)
    {
        if (it == selectionStartElement_)
            start = selectionStartProgress_;
        else 
            start = 0;
        if (it == selectionEndElement_)
            stop = selectionEndProgress_;
        else
            stop = LayoutContext::progressCompleted;
        (*it)->toText(out, start, stop);
    }
}

void Definition::renderSingleElement(Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t element)
{
    assert(elements_.end() != element);
    ElementPosition_t next = element;
    renderLayout(graphics, prefs, element, ++next);
}

void Definition::renderElementRange(Graphics& graphics, const RenderingPreferences& prefs, ElementPosition_t begin, ElementPosition_t end)
{
    renderLayout(graphics, prefs, begin, end);
}


static bool hyperlinksEqual(const DefinitionElement* e1, const DefinitionElement* e2) 
{
    if (!(e1->isHyperlink() && e2->isHyperlink()))
        return false;
    const DefinitionElement::HyperlinkProperties* p1 = e1->hyperlinkProperties();
    const DefinitionElement::HyperlinkProperties* p2 = e2->hyperlinkProperties();
    if (p1->type != p2->type || p1->resource != p2->resource)
        return false;
    return true;
}

static bool insideHotSpot(const Point& p, Definition::ElementPosition_t begin, Definition::ElementPosition_t end)
{
    while (begin != end)
    {
        const DefinitionElement::HyperlinkProperties* props = (*begin)->hyperlinkProperties();
        assert(NULL != props);
        const Definition::HotSpot* hotSpot = props->hotSpot;
        if (NULL != hotSpot && hotSpot->hitTest(p))
            return true;
        ++begin;
    }
    return false;
}

void Definition::extendSelectionToFullHyperlink()
{
    assert(elements_.end() != selectionStartElement_);
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
    }
    inactiveSelectionStartElement_ = selectionStartElement_;
    inactiveSelectionEndElement_ = selectionEndElement_;
    ++inactiveSelectionEndElement_;
}


bool Definition::trackHyperlinkHighlight(Graphics& graphics, const RenderingPreferences& prefs, const Point& point, uint_t clickCount) 
{
    if (trackingSelection_ && !selectionIsHyperlink_)
        return false;
    else if (trackingSelection_ && selectionIsHyperlink_) 
    {
        assert(trackingSelection_);
        assert(selectionEndProgress_ == LayoutContext::progressCompleted);
        bool insideHyperlink = insideHotSpot(point, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
        if (0 == clickCount) 
        {
            if (!insideHyperlink && selectionStartProgress_ != selectionEndProgress_)
            {
                selectionStartProgress_ = LayoutContext::progressCompleted;
                selectionStartElement_ = selectionEndElement_;
                renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
            }
            else if (insideHyperlink && selectionStartProgress_ == selectionEndProgress_)
            {
                selectionStartProgress_ =  0;
                selectionStartElement_ = inactiveSelectionStartElement_;
                renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
            }
        }
        else 
        {
            selectionStartProgress_ = LayoutContext::progressCompleted;
            ElementPosition_t elem = inactiveSelectionStartElement_;
            selectionStartElement_  = selectionEndElement_ = elements_.end();
            renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
            inactiveSelectionStartElement_ = inactiveSelectionEndElement_ = elements_.end();
            trackingSelection_ = false;
            selectionIsHyperlink_ = false;
            if (insideHyperlink)
                (*elem)->performAction(*this);
        }
        return true;
    }
    else 
    {
        if (selectionIsHyperlink_)
        {
            assert(elements_.end() != inactiveSelectionStartElement_);
            selectionStartElement_  = selectionEndElement_ = elements_.end();
            renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
            inactiveSelectionStartElement_ = inactiveSelectionEndElement_ = elements_.end();
            selectionIsHyperlink_ = false;
        }
        HotSpot* hotSpot = NULL;
        HotSpots_t::const_iterator end=hotSpots_.end();
        for (HotSpots_t::const_iterator it=hotSpots_.begin(); it!=end; ++it)
        {
            if ((*it)->hitTest(point))
            {
                hotSpot = (*it);
                break;
            }
        }
        if (NULL != hotSpot)
        {
            trackingSelection_ = true;
            selectionIsHyperlink_ = true;
            selectionStartProgress_ = 0;
            selectionEndProgress_ = LayoutContext::progressCompleted;
            selectionStartElement_ = selectionEndElement_ = std::find(elements_.begin(), elements_.end(), &hotSpot->element());
            extendSelectionToFullHyperlink();
            renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
            return true;
        }
        return false;
    }
}

Definition::LinePosition_t Definition::lineAtHeight(Coord_t height)
{
    if (height < 0)
        return lines_.begin() + firstLine_;
    LinePosition_t end = lines_.begin() + lastLine_;
    Coord_t actHeight = 0;
    for (LinePosition_t line = lines_.begin() + firstLine_; line != end; ++line)
        if (height >= actHeight && height < actHeight + int(line->height))
            return line;
        else
            actHeight += line->height;
    return end;
}

void Definition::removeSelection(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs)
{
    ElementPosition_t start = selectionStartElement_;
    ElementPosition_t end = selectionEndElement_;
    if (elements_.end() != end)
        ++end;
    selectionStartElement_ = selectionEndElement_ = elements_.end();
    selectionStartProgress_ = selectionEndProgress_ = LayoutContext::progressCompleted;
    renderElementRange(graphics, prefs, start, end);
}


bool Definition::trackTextSelection(Graphics& graphics, const RenderingPreferences& prefs, const Point& point, uint_t clickCount) 
{
    Point p(point.x - bounds_.x(), point.y - bounds_.y());
    if (p.y < 0)
        p.x = 0;
    ElementPosition_t elem;
    uint_t progress;
    LinePosition_t line = lineAtHeight(p.y);
    uint_t wordEnd;
    if (2 == clickCount && usesDoubleClickSelection())
    {
        elementAtWidth(graphics, prefs, line, p.x, elem, progress, wordEnd, true);
        trackingSelection_ = false;
        if (elements_.end() == elem || DefinitionElement::offsetOutsideElement == progress)
            return false;
        selectionStartElement_ = selectionEndElement_ = elem;
        selectionStartProgress_ = progress;
        selectionEndProgress_ = wordEnd;
        ++elem;
        renderElementRange(graphics, prefs, selectionStartElement_, selectionEndElement_);
        return true;
    }
    elementAtWidth(graphics, prefs, line, p.x, elem, progress, wordEnd, false);
    if (!trackingSelection_) 
    {
        mouseDownElement_ = selectionStartElement_ = selectionEndElement_ = elem;
        mouseDownProgress_ = selectionStartProgress_ = selectionEndProgress_ = progress;
        trackingSelection_ = true;
        selectionIsHyperlink_ = false;
    }
    else 
    {
        ElementPosition_t prevStart = selectionStartElement_;
        ElementPosition_t prevEnd = selectionEndElement_;
        uint_t prevStartProg = selectionStartProgress_;
        uint_t prevEndProg = selectionEndProgress_;
        if (elem > mouseDownElement_ || (elem == mouseDownElement_ && progress > mouseDownProgress_)) 
        {
            selectionStartElement_ = mouseDownElement_;
            selectionStartProgress_ = mouseDownProgress_;
            selectionEndElement_ = elem;
            selectionEndProgress_ = progress;
        }
        else if (elem < mouseDownElement_ || (elem == mouseDownElement_ && progress < mouseDownProgress_))
        {
            selectionStartElement_ = elem;
            selectionStartProgress_ = progress;
            selectionEndElement_ = mouseDownElement_;
            selectionEndProgress_ = mouseDownProgress_;
        }
        else 
        {
            selectionStartElement_ = selectionEndElement_ = mouseDownElement_;
            selectionStartProgress_ = selectionEndProgress_ = mouseDownProgress_;
        }
        ElementPosition_t start, end;
        if (prevStart != selectionStartElement_ || prevStartProg != selectionStartProgress_)
        {
            start = std::min(selectionStartElement_, prevStart);
            end = std::max(selectionStartElement_, prevStart);
            ++end;
            renderElementRange(graphics, prefs, start, end);
        }
        if (prevEnd != selectionEndElement_ || prevEndProg != selectionEndProgress_)
        {
            start = std::min(selectionEndElement_, prevEnd);
            end = std::max(selectionEndElement_, prevEnd);
            ++end;
            renderElementRange(graphics, prefs, start, end);
        }
    }
    if (0 != clickCount) 
        trackingSelection_ = false;
    lastLineUnderMouse_ = line;
    return true;
}

bool Definition::extendSelection(Graphics& graphics, const RenderingPreferences& prefs, const Point& point, uint_t clickCount)
{
    navigatingDown_ = navigatingUp_ = false;
    if (elements_.empty())
        return false;
    if (!trackingSelection_ && !(bounds_ && point))
        return false;
    if (!trackingSelection_ && elements_.end() != selectionStartElement_)
        removeSelection(graphics, prefs);
    if (trackHyperlinkHighlight(graphics, prefs, point, clickCount))
        return true;
    if (usesMouseSelection())
        return trackTextSelection(graphics, prefs, point, clickCount);
    return false;
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
    selectionStartElement_ = selectionEndElement_ = mouseDownElement_ = 
        inactiveSelectionStartElement_ = inactiveSelectionEndElement_ = elements_.end();
    selectionStartProgress_ = selectionEndProgress_ = mouseDownProgress_ = LayoutContext::progressCompleted;
    trackingSelection_ = false;
    selectionIsHyperlink_ = false;
}

bool Definition::mouseDown(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point)
{
    return extendSelection(graphics, prefs, point, 0);
}

bool Definition::mouseUp(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point, uint_t clickCount)
{
    return extendSelection(graphics, prefs, point, clickCount);
}

bool Definition::mouseDrag(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, const ArsLexis::Point& point)
{
    return extendSelection(graphics, prefs, point, 0);
}


#if defined(_PALM_OS)
#pragma segment Segment1
#endif

Definition::HyperlinkHandler::~HyperlinkHandler()
{}

bool Definition::navigatorKey(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, NavigatorKey navKey)
{
    if (usesHyperlinkNavigation())
    {
        switch (navKey)
        {
            case navKeyRight:
                navigatingUp_ = false;
                if (!navigatingDown_ && navigateHyperlink(graphics, prefs, true))
                    return true;
                navigatingDown_ = true;
                navKey = navKeyDown;
                break;
            
            case navKeyLeft:
                navigatingDown_ = false;
                if (!navigatingUp_ && navigateHyperlink(graphics, prefs, false))
                    return true;
                navigatingUp_ = true;
                navKey = navKeyUp;
                break;
            
            case navKeyCenter:
                if (!selectionIsHyperlink_)
                    return false;
                assert(inactiveSelectionStartElement_ != elements_.end());
                selectionIsHyperlink_ = false;
                trackingSelection_ = false;
                selectionStartElement_ = selectionEndElement_ = elements_.end();
                selectionStartProgress_ = selectionEndProgress_ = LayoutContext::progressCompleted;
                renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
                (*inactiveSelectionStartElement_)->performAction(*this);
                inactiveSelectionStartElement_ = inactiveSelectionEndElement_ = elements_.end();
                return true;
            
        }
    }
    if (navigatingUp_ || navigatingDown_ || usesUpDownScroll())
    {
        int items = 0;
        switch (navKey)
        {
            case navKeyUp:
                navigatingDown_ = false;
                items = -int(shownLinesCount() + 1);
                break;
                
            case navKeyDown:
                navigatingUp_ = false;
                items = shownLinesCount() - 1;
                break;
        }
        if (0 != items)
        {
            uint_t top = firstShownLine();
            scroll(graphics, prefs, items);
            if (firstShownLine() != top)
                return true;
            bool unselect = true;
            if (navigatingUp_ && isFirstInNavOrder())
                unselect = false;
            else if (navigatingDown_ && isLastInNavOrder())
                unselect = false;
            if (unselect) 
            {
                clearSelection(graphics, prefs);
                navigatingUp_ = navigatingDown_ = false;
            }
        }
    }
    return false;
}

static inline bool isHyperlink(Definition::const_iterator pos)
{
    return (*pos)->isHyperlink();
}

bool Definition::navigateHyperlink(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs, bool next)
{
    if (empty())
        return false;
    Lines_t::iterator firstLine = lines_.begin() + firstLine_;
    Lines_t::iterator lastLine = lines_.begin() + lastLine_;
    ElementPosition_t firstElem = firstLine->firstElement;
    ElementPosition_t lastElem = elements_.end();
    if (lines_.end() != lastLine)
    {
        lastElem = lastLine->firstElement;
        if (lastLine->renderingProgress != 0)
            ++lastElem;
    }
    if (selectionIsHyperlink_)
    {
        assert(elements_.end() != inactiveSelectionStartElement_);
        if ((inactiveSelectionStartElement_ >= firstElem && inactiveSelectionStartElement_ <lastElem) 
         || (inactiveSelectionEndElement_ > firstElem && inactiveSelectionEndElement_ <= lastElem)
         || (inactiveSelectionStartElement_ < firstElem && inactiveSelectionEndElement_ >= lastElem))
        {
            if (next)
                firstElem = inactiveSelectionEndElement_;
            else
                lastElem = inactiveSelectionStartElement_;
        }
    }
    bool needToScroll = false;
    ElementPosition_t pos;
    if (next)
    {
        pos = firstElem;
        while (true)
        {
            if (elements_.end() == pos)
                return false;
            if (isHyperlink(pos))
                break;
            ++pos;
        }
        if (pos >= lastElem)
            needToScroll = true;
    }
    else
    {
        pos = lastElem;
        while (true)
        {
            if (elements_.begin() == pos)
                return false;
            --pos;
            if (isHyperlink(pos))
                break;
        }
        if (pos < firstElem)
            needToScroll = true;
    }
    selectionStartElement_ = selectionEndElement_ = elements_.end();
    selectionStartProgress_ = selectionEndProgress_ = LayoutContext::progressCompleted;
    renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);

    selectionIsHyperlink_ = true;
    selectionStartElement_ = selectionEndElement_ = pos;
    selectionStartProgress_ = 0;
    if (needToScroll)
    {
        int linesToScroll = 0;
        Lines_t::iterator line;
        if (next)
        {
            line = lastLine;
            while (true)
            {
                ++linesToScroll;
                if (lines_.end() == line)
                    break;
                if (line->firstElement > pos)
                    break;
                ++line;
            }
        }
        else
        {
            line = firstLine;
            while (true)
            {
                --linesToScroll;
                if (line->firstElement <= pos)
                    break;
                --line;
            }
        }
        scroll(graphics, prefs, linesToScroll);
    }
    extendSelectionToFullHyperlink();
    renderElementRange(graphics, prefs, inactiveSelectionStartElement_, inactiveSelectionEndElement_);
    return true;
}

bool Definition::hasSelection() const
{
    return selectionStartElement_ != elements_.end();
}

void Definition::clearSelection(ArsLexis::Graphics& graphics, const RenderingPreferences& prefs) {
    if (!hasSelection())
        return;
    ElementPosition_t start = selectionStartElement_;
    ElementPosition_t end = selectionEndElement_;
    selectionStartElement_ = selectionEndElement_ = elements_.end();
    selectionStartProgress_ = selectionEndProgress_ = LayoutContext::progressCompleted;
    inactiveSelectionStartElement_ = inactiveSelectionEndElement_ = elements_.end();
    selectionIsHyperlink_ = false;
    renderElementRange(graphics, prefs, start, end);
}

bool Definition::isFirstLinkSelected() const 
{
    if (!hasSelection())
        return false;
    if (!selectionIsHyperlink_)
        return false;
    Elements_t::const_iterator elem = selectionStartElement_;
    Elements_t::const_iterator begin = elements_.begin();
    while (elem != begin) 
    {
        --elem;
        if (!isHyperlink(elem))
            continue;
        return false;
    }
    return true;
}

bool Definition::isLastLinkSelected() const
{
    if (!hasSelection())
        return false;
    if (!selectionIsHyperlink_)
        return false;
    Elements_t::const_iterator elem = selectionEndElement_;
    Elements_t::const_iterator end = elements_.end();
    while (elem != end) 
    {
        ++elem;
        if (end == elem)
            return true;
        if (!isHyperlink(elem))
            continue;
        return false;
    }
    return true;
}

#define brTag       _T("<br>")
#define bTagStart   _T("<b>")
#define bTagEnd     _T("</b>")
#define aTagStart   _T("<a>")
#define aTagEnd     _T("</a>")

void parseSimpleFormatting(Definition::Elements_t& out, const ArsLexis::String& text, bool useHyperlink, HyperlinkType hyperlinkType)
{
    using namespace std;
    uint_t bold=0;
    String::size_type start=0;
    String::size_type pos=start;
    while (true) 
    {
        String::size_type next=text.find('<', pos);
        if (text.npos==next) 
        {
            out.push_back(new GenericTextElement(String(text, start, next)));
            break;
        }
        if (startsWithIgnoreCase(text, brTag, next))
        {
            out.push_back(new GenericTextElement(String(text, start, next-pos)));
            out.push_back(new LineBreakElement());
            start=pos=next+tstrlen(brTag);
        }
        else if (startsWithIgnoreCase(text, bTagStart, next))
        {
            out.push_back(new GenericTextElement(String(text, start, next-pos)));
            bold++;
            start=pos=next+tstrlen(bTagStart);
        }
        else if (startsWithIgnoreCase(text, bTagEnd, next))
        {
            out.push_back(new GenericTextElement(String(text, start, next-pos)));
            bold--;
            start=pos=next+tstrlen(bTagEnd);
        }
        else if (startsWithIgnoreCase(text, aTagStart, next) && useHyperlink)
        {
            out.push_back(new GenericTextElement(String(text, start, next-pos)));
            start=pos=next+tstrlen(aTagStart);
        }
        else if (startsWithIgnoreCase(text, aTagEnd, next) && useHyperlink)
        {
            GenericTextElement* gText;
            out.push_back(gText = new GenericTextElement(String(text, start, next-pos)));
            gText->setHyperlink(String(text, start, next-pos), hyperlinkType);
            start=pos=next+tstrlen(aTagEnd);
        }
        else
            pos=next+1;
        if (text.length()==pos)
        {
            out.push_back(new GenericTextElement(String(text, start, pos)));
            break;
        }
    }
}
