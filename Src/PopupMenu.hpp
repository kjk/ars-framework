#ifndef ARSLEXIS_POPUP_MENU_HPP__
#define ARSLEXIS_POPUP_MENU_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <FormObject.hpp>
#include <Definition.hpp>

class PopupMenuModel: public ArsLexis::List::CustomDrawHandler
{
public:

    struct Item
    {
        ArsLexis::char_t* text;
        ArsLexis::char_t* hyperlink;
        bool active;
        bool bold;
        bool separator;
        
        Item();
        
        ~Item();
    };        
       
    Item* items;
    UInt16 count;
    RGBColorType inactiveTextColor;
    
    PopupMenuModel();
    
    ~PopupMenuModel();
    
    uint_t maxTextWidth() const;
    
    void drawItem(ArsLexis::Graphics& graphics, ArsLexis::List& list, uint_t item, const ArsLexis::Rectangle& itemBounds);
    
    uint_t itemsCount() const;
    
    // Passes ownership of items to PopupMenuModel.
    void setItems(Item* items, uint_t itemsCount);

    bool itemsFromString(const char_t* data, long length);
    
};

class PopupMenu
{
    UInt16 prevFocusIndex_;
    PopupMenuModel* model_;

public:

    HyperlinkHandlerBase* hyperlinkHandler;
    ArsLexis::List list;
    Int16 initialSelection;

    PopupMenu(ArsLexis::Form& form);
    
    ~PopupMenu();
    
    bool handleEventInForm(EventType& event);

    Int16 popup(UInt16 id, const ArsLexis::Point& point);
    
    // Passes model ownership to PopupMenu and deletes previous model
    void setModel(PopupMenuModel* model);
    
    PopupMenuModel* model() {return model_;}
   
};



#endif