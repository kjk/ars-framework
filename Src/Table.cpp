#include <Table.hpp>
#include <SysUtils.hpp>

Table::Table(Form& form, ScrollBar* scrollBar):
    FormObjectWrapper(form),
    scrollBar_(scrollBar),
    topItem_(0),
    itemsCount_(0),
    notifyChangeSelection_(false)
{}

Table::~Table() {}

void Table::adjustVisibleItems()
{
/*
    ArsRectangle rect;
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

uint_t Table::visibleItems() const
{
    ArsRectangle rect;
    bounds(rect);
    uint_t visibleItems = rect.height()/itemHeight();
    return visibleItems;
}

void Table::setTopItem(uint_t topItem, bool updateScrollbar)
{
    uint_t visibleItems = this->visibleItems();
    uint_t maxRows = rowsCount();
    uint_t visible = itemsCount_;
    if (itemsCount_ > visibleItems)
    {
        topItem_ = topItem;
        if (topItem_ > itemsCount_ - visibleItems)
            topItem_ = itemsCount_ - visibleItems;
        for (uint_t i = 0; i < topItem_; ++i)
            setRowUsable(i, false);
        for (uint_t i = topItem_; i < topItem_ + visibleItems; ++i)
            setRowUsable(i, true);
        for (uint_t i = topItem_ + visibleItems; i < maxRows; ++i)
            setRowUsable(i, false);
        visible = visibleItems;
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

void Table::fireItemSelected(Int16 row, Int16 col)
{
    EventType event;
    MemSet(&event, sizeof(event), 0);
    event.eType = tblSelectEvent;
    event.data.tblSelect.tableID = id();
    event.data.tblSelect.pTable = reinterpret_cast<TableType*>(object());
    event.data.tblSelect.row = row;
    event.data.tblSelect.column = col;
    EvtAddEventToQueue(&event);
}

void Table::setSelection(Int16 row, Int16 column)
{ 
    // makes sure that selected item is visible
    if (-1 != row)
    {
        assert((row >= 0) && (row < rowsCount()));
        if (row < topItem())
        {
            setTopItem(row, true);
            redraw();
        }
        else if (row - topItem_ > visibleItems()-1)
        {
            setTopItem(row - (visibleItems()-1), true);
            redraw();
        }
    }
    TblSelectItem(object(), row, column);
    
    if (notifyChangeSelection_)
    {
        sendEvent(selChangedEvent);
    }
}

bool Table::handleKeyDownEvent(const EventType& event)
{
    Form& form = *this->form();
    assert(valid());
    //if (form.application().runningOnTreo600() && !hasFocus())
    //    return false;
    assert(keyDownEvent==event.eType);

    Int16 origRow, origCol;
    getSelection(&origRow, &origCol);
    Int16 curRow = origRow;
    Int16 curCol = origCol;
    Int16 rowCount = rowsCount();
    Int16 colCount = columnsCount();
    // exit if there is no valid current selection
    if ( (curRow<0) || (curRow>=rowCount) )
        return false;
    if ( (curCol<0) || (curCol>=colCount) )
        return false;

    if (form.fiveWayUpPressed(&event))
        --curRow;
    else if (form.fiveWayDownPressed(&event))
        ++curRow;
    else if (form.fiveWayLeftPressed(&event))
        --curCol;
    else if (form.fiveWayRightPressed(&event))
        ++curCol;
    else if (form.fiveWayCenterPressed(&event) ||
        chrLineFeed == event.data.keyDown.chr || chrCarriageReturn == event.data.keyDown.chr)
    {
        if (NULL == itemPtr(curRow, curCol))
            return false; // there's no item at this position
        fireItemSelected(curRow, curCol);
        return true;
    }
    if ((curRow == origRow) && (curCol == origCol) )
        return false;
    if ( (curRow < 0) || (curRow >= rowCount) )
        return false;
    if ( (curCol < 0) || (curCol >= colCount) )
        return false;
    if (NULL == itemPtr(curRow, curCol))
        return false; // there's no item at this position
    setSelection(curRow, curCol);
    return true;
}

void Table::setChangeSelectionNotification(bool notify)
{
    notifyChangeSelection_ = notify;
}
