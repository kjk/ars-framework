#ifndef ARSLEXIS_POPUP_MENU_HPP__
#define ARSLEXIS_POPUP_MENU_HPP__

#include <Debug.hpp>
#include <BaseTypes.hpp>
#include <FormObject.hpp>
#include <Definition.hpp>

class PopupMenuModel: public List::CustomDrawHandler
{
public:

    struct Item
    {
        ArsLexis::char_t* text;
        ArsLexis::char_t* hyperlink;
        bool active;
        bool bold;
        bool separator;
        bool underlined;
                
        Item();
        
        ~Item();
    };        
       
    Item* items;
    UInt16 count;
    RGBColorType inactiveTextColor;
    
    PopupMenuModel();
    
    ~PopupMenuModel();
    
    uint_t maxTextWidth() const;
    
    void drawItem(Graphics& graphics, List& list, uint_t item, const Rectangle& itemBounds);
    
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
    List list;
    Int16 initialSelection;

    PopupMenu(Form& form);
    
    ~PopupMenu();
    
    bool handleEventInForm(EventType& event);

    Int16 popup(UInt16 id, const Point& point);
    
    // Passes model ownership to PopupMenu and deletes previous model
    void setModel(PopupMenuModel* model);
    
    PopupMenuModel* model() {return model_;}
   
};



#endif