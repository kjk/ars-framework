#ifndef ARSLEXIS_TABLE_HPP__
#define ARSLEXIS_TABLE_HPP__

#include <FormObject.hpp>

namespace ArsLexis {

    class Table: public FormObjectWrapper<TableType> {
    
        uint_t topItem_;
        uint_t itemsCount_;
        ScrollBar* scrollBar_;
        
    public:
    
        explicit Table(Form& form, ScrollBar* scrollBar = NULL);

        ~Table();
        
        void redraw() {TblRedrawTable(object());}

        void invalidate() {TblMarkTableInvalid(object());}
        
        void invalidateRow(UInt16 row) {TblMarkRowInvalid(object(), row);}

        bool isRowInvalid(UInt16 row) const {return TblRowInvalid(object(), row);}
        
        bool isBeingEdited() const {return TblEditing(object());}
        
        void erase() {TblEraseTable(object());}
        
        Coord columnSpacing(UInt16 column) const {return TblGetColumnSpacing(object(), column);}
        
        void setColumnSpacing(UInt16 column, Coord spacing) {TblSetColumnSpacing(object(), column, spacing);}
        
        Coord columnWidth(UInt16 column) const {return TblGetColumnWidth(object(), column);}
        
        void setColumnWidth(UInt16 column, Coord width) {TblSetColumnWidth(object(), column, width);}
        
        UInt16 columnsCount() const {return TblGetNumberOfColumns(object());}
        
        UInt16 rowsCount() const {return TblGetNumberOfRows(object());}
        
        Coord rowHeight(UInt16 row) const {return TblGetRowHeight(object(), row);}
        
        void insertRow(UInt16 row) {TblInsertRow(object(), row);}
        
        void removeRow(UInt16 row) {TblRemoveRow(object(), row);}
        
        void setRowUsable(UInt16 row, bool usable) {TblSetRowUsable(object(), row, usable);}
        
        bool isRowUsable(UInt16 row) const {return TblRowUsable(object(), row);}
        
        void setItemStyle(UInt16 row, UInt16 column, TableItemStyleType style) {TblSetItemStyle(object(), row, column, style);}
        
        void setItemInt(UInt16 row, UInt16 column, Int16 value) {TblSetItemInt(object(), row, column, value);}
        
        void setItemPtr(UInt16 row, UInt16 column, void* ptr) {TblSetItemPtr(object(), row, column, ptr);}
        
        Int16 itemInt(UInt16 row, UInt16 column) const {return TblGetItemInt(object(), row, column);}
        
        void* itemPtr(UInt16 row, UInt16 column) const {return TblGetItemPtr(object(), row, column);}
        
        void setColumnUsable(UInt16 column, bool usable) {TblSetColumnUsable(object(), column, usable);}
        
        void setCustomDrawFunction(UInt16 column, TableDrawItemFuncType* function) {TblSetCustomDrawProcedure(object(), column, function);}
        
        void setRowHeight(UInt16 row, Coord height) {TblSetRowHeight(object(), row, height);}
        
        void adjustVisibleItems();
        
        void setHasScrollbar(bool value) {TblHasScrollBar(object(), value);}
        
        void setItemsCount(uint_t itemsCount) {itemsCount_ = itemsCount;}
        uint_t itemsCount() const {return itemsCount_;}
        uint_t itemHeight() const {return rowHeight(0);}
        uint_t viewCapacity() const;
        uint_t topItem() const {return topItem_;}
        void setTopItem(uint_t topItem, bool updateScrollbar = false);
        
        bool handleEventInForm(EventType& event);        
    };
    
}

#endif