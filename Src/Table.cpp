#include <Table.hpp>

using namespace ArsLexis;

Table::~Table() {}

void Table::adjustVisibleItems()
{
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
}
