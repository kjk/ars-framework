#include <Table.hpp>

using namespace ArsLexis;

Table::Table(Form& form, ScrollBar* scrollBar):
    FormObjectWrapper(form),
    scrollBar_(scrollBar),
    topItem_(0),
    itemsCount_(0)
{}

Table::~Table() {}

void Table::adjustVisibleItems()
{
/*
    Rectangle rect;
    bounds(rect);
    UInt16 lastVisibleRow = rect.height()/rowHeight(0) - 1;
    UInt16 maxRowsInTable = rowsCount() - 1;
    if (lastVisibleRow > maxRowsInTable)
        lastVisibleRow = maxRowsInTable;

    for (UInt16 row = 0; row <= lastVisibleRow; row++)
    	setRowUsable(row, true);
    for (UInt16 row = maxRowsInTable; row > lastVisibleRow; row--)
    	setRowUsable(row, false);
    invalidate();
*/
    setTopItem(topItem_, true);
}

uint_t Table::viewCapacity() const 
{
    Rectangle rect;
    bounds(rect);
    return rect.height()/itemHeight();
}

void Table::setTopItem(uint_t topItem, bool updateScrollbar) 
{
    uint_t viewCapacity = this->viewCapacity();
    uint_t maxRows = rowsCount();
    uint_t visible = itemsCount_;
    if (itemsCount_ > viewCapacity) 
    {
        topItem_ = topItem;
        if (topItem_ > itemsCount_ - viewCapacity)
            topItem_ = itemsCount_ - viewCapacity;
        for (uint_t i = 0; i < topItem_; ++i)
            setRowUsable(i, false);
        for (uint_t i = topItem_; i < topItem_ + viewCapacity; ++i)
            setRowUsable(i, true);
        for (uint_t i = topItem_ + viewCapacity; i < maxRows; ++i)
            setRowUsable(i, false);
        visible = viewCapacity;
    }
    else 
    {
        topItem_ = 0;
        for (uint_t i = 0; i < itemsCount_; ++i)
            setRowUsable(i, true);
        for (uint_t i = itemsCount_; i < maxRows; ++i)
            setRowUsable(i, false);
    }
    if (updateScrollbar && NULL != scrollBar_)
        scrollBar_->setPosition(topItem_, 0, itemsCount_ - visible, visible);
    invalidate();
    redraw();
}

bool Table::handleEventInForm(EventType& event)
{
    if (sclRepeatEvent != event.eType)
        return false;
    if (NULL == scrollBar_)
        return false;
    if (scrollBar_->id() != event.data.sclRepeat.scrollBarID)
        return false;
    int items = event.data.sclRepeat.newValue - event.data.sclRepeat.value;
    int newTop = std::max<int>(0, topItem_ + items);
    newTop = std::min<int>(newTop, itemsCount_);
    setTopItem(newTop, false);
    return false;
}    
