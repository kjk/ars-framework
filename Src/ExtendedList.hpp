#ifndef ARSLEXIS_EXTENDED_LIST_HPP__
#define ARSLEXIS_EXTENDED_LIST_HPP__

#include <FormGadget.hpp>
#include <Logging.hpp>
#include <Application.hpp>

namespace ArsLexis {

    class ExtendedList: public FormGadget {
        
    public:
    
        enum {
            selChangedEvent = Application::extListSelectionChangedEvent
        };
    
        explicit ExtendedList(Form& form, UInt16 id=frmInvalidObjectId);
    
        ~ExtendedList();
    
        class ItemRenderer: NonCopyable {
            
        public:
            
            ItemRenderer();
            
            virtual ~ItemRenderer();
            
            virtual void drawItem(Graphics& graphics, ExtendedList& list, uint_t item, const Rectangle& itemBounds)=0;
            
            virtual uint_t itemsCount() const=0;
            
        };
        
        enum RedrawOption {
            redrawNot,
            redraw
        };
        
        uint_t visibleItemsCount() const;
        
        void setSelection(int item, RedrawOption ro=redrawNot);

        int selection() const
        {return selection_;}

        void makeItemVisible(uint_t item);

        bool scroll(WinDirectionType direction, uint_t items);
        
        bool scrollUp(uint_t items)
        {return scroll(winUp, items);}
        
        bool scrollDown(uint_t items)
        {return scroll(winDown, items);}

        uint_t itemsCount() const
        {return 0==itemRenderer_?0:itemRenderer_->itemsCount();}
        
        int topItem() const
        {return topItem_;}

        void setSelectionDelta(int delta, RedrawOption ro=redraw);
        
        //! @note this @c ExtendedList doesn't take ownership of @c ItemRenderer. 
        void setItemRenderer(ItemRenderer* itemRenderer, RedrawOption ro=redrawNot);
        
        //! For compatibility with List.
        typedef ItemRenderer CustomDrawHandler;

        //! For compatibility with List.
        void setCustomDrawHandler(CustomDrawHandler* customDrawHandler)
        {setItemRenderer(customDrawHandler, redrawNot);}
        
        enum KeyHandlerOptions {
            optionScrollPagesWithLeftRight=1,
            optionFireListSelectOnCenter=2
        };

        bool handleKeyDownEvent(const EventType& event, uint_t options=0);
        
        void adjustVisibleItems(RedrawOption ro=redrawNot);
        
        void setTopItem(uint_t item, RedrawOption ro=redrawNot);

        void notifyItemsChanged();
        
        void setItemHeight(uint_t height, RedrawOption ro=redrawNot);
        
        uint_t itemHeight() const
        {return itemHeight_;}
        
        bool scrollBarVisible() const;
        
        uint_t scrollBarWidth() const
        {return scrollBarWidth_;}
        
        uint_t height() const;
        
        void setItemBackground(const RGBColorType& color)
        {itemBackground_ = color;}
        
        void setSelectedItemBackground(const RGBColorType& color)
        {selectedItemBackground_ = color;}
        
        void setListBackground(const RGBColorType& color)
        {listBackground_ = color;}
        
        void setForeground(const RGBColorType& color)
        {foreground_ = color;}
        
        const RGBColorType& itemBackground() const {return itemBackground_;}
        
        const RGBColorType& selectedItemBackground() const {return selectedItemBackground_;}
        
        const RGBColorType& listBackground() const {return listBackground_;}
        
        const RGBColorType& foreground() const {return foreground_;}
        
        void setUpBitmapId(uint_t id) {upBitmapId_ = id;}

        void setDownBitmapId(uint_t id) {downBitmapId_ = id;}
        
        uint_t upBimapId() const {return upBitmapId_;}
        
        uint_t downBitmapId() const {return downBitmapId_;}
        
        void setChangeSelectionNotification(bool notify);
        
    protected:
    
        void drawFocusRing();
    
        void removeFocusRing();

        const ItemRenderer* itemRenderer() const
        {return itemRenderer_;}
    
        void handleDraw(Graphics& graphics);
        
        virtual void drawItemBackground(Graphics& graphics, Rectangle& bounds, uint_t item, bool selected);
        
        virtual void drawItem(Graphics& graphics, const Rectangle& bounds, uint_t item, bool selected);
        
        virtual void drawBackground(Graphics& graphics, const Rectangle& bounds);
        
        void setScrollBarWidth(uint_t width) {scrollBarWidth_ = width;}
        
        void setScrollButtonHeight(uint_t height) {scrollButtonHeight_ = height;}
        
        virtual void drawScrollBar(Graphics& graphics, const Rectangle& bounds);
        
        bool handleEvent(EventType& event);
        
        bool handleEnter(const EventType& event);
        
        bool screenIsDoubleDensity() const {return screenIsDoubleDensity_;}
        
        void handleNilEvent();
        
        uint_t scrollButtonHeight() {return scrollButtonHeight_;}
        
    private:
    
        void drawItemProxy(Graphics& graphics, const Rectangle& listBounds, uint_t item, bool showScrollbar);
        
        void handlePenInScrollBar(const Rectangle& bounds, const Point& penPos, bool penUp, bool enter);
        
        void handlePenInItemsList(const Rectangle& bounds, const Point& penPos, bool penUp);
        
        void handlePenUp(const EventType& event);
        
        void handlePenMove(const EventType& event);
        
        void fireItemSelectEvent();
        
        uint_t visibleScrollBarWidth() const
        {return scrollBarVisible()?scrollBarWidth_:0;}
        
        ItemRenderer* itemRenderer_;
        int selection_;
        int topItem_;
        uint_t itemHeight_;
        uint_t scrollBarWidth_; 
        uint_t scrollButtonHeight_;
 
        RGBColorType itemBackground_;
        RGBColorType selectedItemBackground_;
        RGBColorType listBackground_;
        RGBColorType foreground_;
        
        bool hasHighDensityFeatures_;
        bool screenIsDoubleDensity_;
        bool windowSettingsChecked_;
        bool trackingScrollbar_;
        bool notifyChangeSelection_;
        int topItemBeforeTracking_;
        uint_t upBitmapId_;
        uint_t downBitmapId_;
        
        UInt32 scheduledNilEventTicks_;
        enum ScheduledScrollDirection {
            scheduledScrollAbandoned,
            scheduledScrollUp,
            scheduledScrollDown
        };
        
        ScheduledScrollDirection scheduledScrollDirection_;
    };
    
    class BasicStringItemRenderer: public ExtendedList::ItemRenderer {
    public:
    
        BasicStringItemRenderer();
        
        ~BasicStringItemRenderer();
        
        void drawItem(Graphics& graphics, ExtendedList& list, uint_t item, const Rectangle& itemBounds);
         
    protected:
    
        virtual void getItem(String& out, uint_t item)=0;
        
    };
    
}

#endif